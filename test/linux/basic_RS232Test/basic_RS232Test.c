/** \file
 * \brief Example code for Simple Open EtherCAT master
 *
 * Usage : simple_test [ifname1]
 * ifname is NIC interface, f.e. eth0
 *
 * This is a minimal test.
 *
 * (c)Arthur Ketels 2010 - 2011
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "ethercat.h"

//-------NAME---SLAVE#------
#define EK1100    1
#define MEM_SIZE  48

char IOmap[MEM_SIZE];
int expectedWKC;
boolean needlf;
volatile int wkc;
boolean inOP;
uint8 currentgroup = 0;



void basicInit(char *ifname) {
    if (ec_init(ifname)) {
        printf("%s", "Network Initializtion complete.\n");
        if (ec_config_init(FALSE) > 0) {
            printf("%s", "Slave config Initialization complete.\n");
            int usedmem = ec_config_map(&IOmap);
            printf("UsedMem: %d\n", usedmem);
            
            ec_configdc();
            ec_send_processdata();
            wkc = ec_receive_processdata(EC_TIMEOUTRET);
            
            ec_slave[0].state = EC_STATE_OPERATIONAL;
            ec_writestate(0);
            ec_statecheck(0, EC_STATE_OPERATIONAL,  EC_TIMEOUTSTATE);
        }
    } else {
        printf("No socket connection on %s\nExcecute as root\n",ifname);
    }
}

void printSlaveInformation() {
    int cnt,j;

    printf("%d slaves found and configured.\n",ec_slavecount);
    for( cnt = 1 ; cnt <= ec_slavecount ; cnt++) {
        printf("\nSlave:%d\n Name:%s\n Output size: %dbits\n Input size: %dbits\n State: %d\n Delay: %d[ns]\n Itype: %d\n Dtype: %d\n",
                  cnt, ec_slave[cnt].name,
                  ec_slave[cnt].Obits,
                  ec_slave[cnt].Ibits,
                  ec_slave[cnt].state,
                  ec_slave[cnt].pdelay,
                  ec_slave[cnt].Itype,
                  ec_slave[cnt].Dtype);
         printf(" Configured address: %4.4x\n", ec_slave[cnt].configadr);
         for(j = 0 ; j < ec_slave[cnt].FMMUunused ; j++) {
               printf(" FMMU%1d Ls:%8.8x Ll:%4d Lsb:%d Leb:%d Ps:%4.4x Psb:%d Ty:%2.2x Act:%2.2x\n",
                        j,
                        (int)ec_slave[cnt].FMMU[j].LogStart,
                        ec_slave[cnt].FMMU[j].LogLength, 
                        ec_slave[cnt].FMMU[j].LogStartbit,
                        ec_slave[cnt].FMMU[j].LogEndbit, 
                        ec_slave[cnt].FMMU[j].PhysStart, 
                        ec_slave[cnt].FMMU[j].PhysStartBit,
                        ec_slave[cnt].FMMU[j].FMMUtype, 
                        ec_slave[cnt].FMMU[j].FMMUactive);
         }
         printf("\n");
    }
}

void printIOMap() {
    int cnt = 0;
    for (cnt = 0; cnt < MEM_SIZE; ++cnt) {
        printf("%u ", IOmap[cnt]);
    }
    printf("\n");
}

int findSpecificSlaveAddress(char * name) {
    int cnt;
    for( cnt = 1 ; cnt <= ec_slavecount ; cnt++) {
        if (!strcmp(ec_slave[cnt].name, name)) {
            return cnt;
        }
    }
    return -1;
}

long unsigned int read_input_uint64 (uint16 slave_number, uint8 module_index) {
    uint8* data_ptr;
    
    data_ptr = ec_slave[slave_number].inputs;
    data_ptr += module_index * 2;
    
    return *data_ptr;
}


void set_output_int16 (uint16 slave_no, uint8 module_index, uint16 value)
{
   uint8 *data_ptr;

   data_ptr = ec_slave[slave_no].outputs;
   /* Move pointer to correct module index*/
   data_ptr += module_index * 2;
   
   /* Read value byte by byte since all targets can't handle misaligned
    * addresses
    */
    ++value;
   *data_ptr++ = 0b1101;
//   *data_ptr++ = 0b1110;
//   *data_ptr++ = 0;
//   *data_ptr++ = 1;
    data_ptr = data_ptr + 20;
   *data_ptr = 1;

}


/* Main Program Execution */
int main(int argc, char *argv[]) {

    printf("\n--------------------------------------------\n");
    printf("-    SOEM (Simple Open EtherCAT Master)    -\n");
    printf("- Please See README for setup instructions -\n");
    printf("--------------------------------------------\n\n");
    
    if (argc > 1) {
        basicInit(argv[1]);
        printSlaveInformation();
        int address = findSpecificSlaveAddress("EL6001");
        
        printIOMap();
        int ii = 0;
        for (ii= 0; ii < 2; ++ii) {
            set_output_int16(address,0, (0b1001 + ii));
            ec_send_processdata();
            printIOMap();
            osal_usleep(50000);
        }
 
        
        /*
        int value = 0;
        
        while(value < 10) {
            ++value;
            set_output_int16(address,0, value);
            ec_send_processdata();
            printIOMap();
//            ec_receive_processdata(EC_TIMEOUTRET);
//            printf("sending: %d\r",value);
            osal_usleep(50000);
        }
        */
        
        
        ec_close();
        
    } else {
        printf("Usage: basic_RS232Test ifname1\nifname = eth0 for example\n");
    }

    printf("End program\n");
    return (0);
}

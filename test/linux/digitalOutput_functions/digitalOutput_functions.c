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
#define MEM_SIZE  2

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
    /* Print Information From Each Slave */
    for(cnt = 1 ; cnt <= ec_slavecount ; cnt++) {
        printf("\nSlave:%d\n"
               " Name:%s\n"
               " Output size: %dbits\n"
               " Input size: %dbits\n"
               " State: %d\n"
               " Delay: %d[ns]\n"
               " Itype: %d\n"
               " Dtype: %d\n"
               " Configured address: %4.4x\n",
                  cnt, ec_slave[cnt].name,
                  ec_slave[cnt].Obits,
                  ec_slave[cnt].Ibits,
                  ec_slave[cnt].state,
                  ec_slave[cnt].pdelay,
                  ec_slave[cnt].Itype,
                  ec_slave[cnt].Dtype,
                  ec_slave[cnt].configadr);
         /* If Available Print the Fieldbus Memory Management Unit for Associated Slave */         
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

/* Outputs Current State of I/O Map */
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

/* 
 * Digital Output
 * Takes in SLAVE_NUMBER, and channel that you want to set. 
 * -- Pin will be set high for 100ms. This is a built in function
 */
void set_DigitalOutputPin(uint16 slave_number, uint8 channel) {
    uint8 *data_ptr = ec_slave[slave_number].outputs;
    channel -= 1;
    *data_ptr = (1 << channel) & 0xFF;
}

void clear_DigitalOutputPin(uint16 slave_number, uint8 channel) {
    uint8 *data_ptr = ec_slave[slave_number].outputs;
    channel -= 1;
    *data_ptr = (0 << 4) & 0xFF;
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
        int slave_number = findSpecificSlaveAddress("EL2024");
        
        printIOMap();
        
        while (1) {
            set_DigitalOutputPin(slave_number, 1);
            ec_send_processdata();
//            printIOMap();
            ec_receive_processdata(EC_TIMEOUTRET);
            osal_usleep(200000);
//            clear_DigitalOutputPin(slave_number,1);
//            osal_usleep(200);
//            printIOMap();
//            ec_send_processdata();
        }        
        
        ec_close();
        
    } else {
        printf("Usage: basic_RS232Test ifname1\nifname = eth0 for example\n");
    }

    printf("End program\n");
    return (0);
}

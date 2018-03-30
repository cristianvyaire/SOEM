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
#define EL2024    2
#define EL1014    3
#define SLAVE_COUNT 3
#define MEM_SIZE  12


char IOmap[12];
int expectedWKC;
boolean needlf;
volatile int wkc;
boolean inOP;
uint8 currentgroup = 0;


void basicInit(char *ifname) {
    if (ec_init(ifname)) {
        printf("%s", "network Initializtion complete\n");
        if (ec_config_init(FALSE) > 0) {
            printf("%s", "Slave config Initializtion complete\n");
           
            int usedmem = ec_config_map(&IOmap);
            printf("UsedMem: %d\n", usedmem);
            ec_configdc();

            ec_send_processdata();
            wkc = ec_receive_processdata(EC_TIMEOUTRET);
            
            ec_slave[0].state = EC_STATE_OPERATIONAL;
            ec_writestate(0);
        }
    } else {
        printf("No socket connection on %s\nExcecute as root\n",ifname);
    }
}

void printSlaveInformation() {
    printf("%d slaves found and configured.\n",ec_slavecount);

    int cnt;
    for( cnt = 1 ; cnt <= ec_slavecount ; cnt++)
         {
         printf("\nSlave:%d\n Name:%s\n Output size: %dbits\n Input size: %dbits\n State: %d\n Delay: %d[ns]\n",
                  cnt, ec_slave[cnt].name, ec_slave[cnt].Obits, ec_slave[cnt].Ibits,
                  ec_slave[cnt].state, ec_slave[cnt].pdelay);
          printf(" Configured address: %4.4x\n", ec_slave[cnt].configadr);
    }
}

void printIOMap() {
    int cnt = 0;
    for (cnt = 0; cnt < MEM_SIZE; ++cnt) {
        printf("%d ", IOmap[cnt]);
    }
    printf("\n");
}

int findSpecificSlaveAddress() {
    int cnt;
    for( cnt = 1 ; cnt <= ec_slavecount ; cnt++) {
        if (!strcmp(ec_slave[cnt].name, "EL3062")) {
            return cnt;
        }
    }
    return -1;
}

unsigned int read_input_uint64 (uint16 slave_number, uint8 channel) {
    uint8* data_ptr = ec_slave[slave_number].inputs;
    data_ptr += channel * 2;
    
    return *data_ptr;
}

void set_AnalogOutput(uint16 slave_number, uint8 channel) {
    uint8 *data_ptr = ec_slave[slave_number].outputs;
    data_ptr += channel * 2;
    *data_ptr = (80);
}

/* Main Program Execution */
int main(int argc, char *argv[]) {

    printf("\n--------------------------------------------\n");
    printf("-    SOEM (Simple Open EtherCAT Master)    -\n");
    printf("- Please See README for setup instructions -\n");
    printf("--------------------------------------------\n\n");
    
    if (argc > 1) {
        uint16 state = ec_slave[0].state;
        printf("\nState of EL3062: %d\n", state); 
        
        basicInit(argv[1]);

        int address = 0;
        printSlaveInformation();
        address = findSpecificSlaveAddress();
        printf("\nAddress for EL3062: %d\n",address);

        state = ec_slave[0].state;
        printf("\nState of EL3062: %d\n", state);      
        printIOMap();
                
        while(1) {
            set_AnalogOutput(2,0);      //output
                                        // channel 0 output -> 1 input    
                                        // channel 1 output -> 3 input
                                            
            osal_usleep(500);
            ec_send_processdata();
            ec_receive_processdata(EC_TIMEOUTRET);
            unsigned int value = read_input_uint64(address,1);  // input
            printIOMap();
            printf("reading: %u\n",value);
            osal_usleep(50000);
        }
        
        ec_close();
        
    } else {
        printf("Usage: basic_analogTest ifname1\nifname = eth0 for example\n");
    }

    printf("End program\n");
    return (0);
}

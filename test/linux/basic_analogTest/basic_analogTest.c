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

char IOmap[4096];
int expectedWKC;
boolean needlf;
volatile int wkc;
boolean inOP;
uint8 currentgroup = 0;


void set_output_int16 (uint16 slave_number, uint8 module_index, int16 value, int16 address) {
    uint8 *data_ptr;

    data_ptr = ec_slave[slave_number].outputs;
    /* Move pointer to correct module index*/
    data_ptr += module_index * 2;
    /* Read value byte by byte since all targets can't handle misaligned
     * addresses
     */
    *data_ptr++ = (value >> 0) & 0xFF;
    *data_ptr++ = (value >> address) & 0xFF;
}

/* Verify that the appropriate hardware is connected in correct order */
uint32 verifyNetworkConfiguration() {
    uint32 isOK = 1;

    if (ec_slavecount < SLAVE_COUNT) {
        isOK = 0;
    } else if (strcmp(ec_slave[EK1100].name, "EK1100")) {
        isOK = 0;
    } else if (strcmp(ec_slave[EL1014].name, "EL1014")) {
        isOK = 0;
        printf("%s",ec_slave[EL1014].name);
    } else if (strcmp(ec_slave[EL2024].name, "EL2024")) {
        isOK = 0;
    }

    return isOK;
}


void basicInit(char *ifname) {
    if (ec_init(ifname)) {
        printf("%s", "network Initializtion complete\n");
        if (ec_config_init(FALSE) > 0) {
            printf("%s", "Slave config Initializtion complete\n");
            //if (verifyNetworkConfiguration()) {
                printf("%s", "Network verified\n");
                ec_config_map(&IOmap);
                ec_configdc();

                /* wait for all slaves to reach OP state */
//                printf("waiting for slaves to reach OP state...\n");
//                ec_statecheck(0, EC_STATE_OPERATIONAL,  EC_TIMEOUTSTATE);
//                printf("...slaves reached OP state\n");

                ec_send_processdata();
                wkc = ec_receive_processdata(EC_TIMEOUTRET);
                
                ec_slave[0].state = EC_STATE_OPERATIONAL;
                ec_writestate(0);
            //} else {
            //    printf("Incorrect EtherCAT Terminals!\n");
            //    printf("Please see README for Setup instructions\n");
            //}
        }
    } else {
        printf("No socket connection on %s\nExcecute as root\n",ifname);
    }
}

void printSlaveInformation() {
    int cnt;

    printf("%d slaves found and configured.\n",ec_slavecount);
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
    for (cnt = 0; cnt < 64; ++cnt) {
        printf("%d ", IOmap[cnt]);
        
        if(cnt % 64 == 0) {
            printf("\r");
        }
    }
    printf("\r");
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

long unsigned int read_input_uint64 (uint16 slave_number, uint8 module_index) {
    uint8* data_ptr;
    
    data_ptr = ec_slave[slave_number].inputs;
    data_ptr += module_index * 2;
    
    return *data_ptr;
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
                
        while(1) {
            ec_send_processdata();
            ec_receive_processdata(EC_TIMEOUTRET);
            long unsigned int value = read_input_uint64(address,1);
            printf("reading: %lu\r",value);
            osal_usleep(5000);
        }
        
        ec_close();
        
    } else {
        printf("Usage: basic_analogTest ifname1\nifname = eth0 for example\n");
    }

    printf("End program\n");
    return (0);
}

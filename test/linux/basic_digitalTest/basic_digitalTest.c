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

#define EC_TIMEOUTMON 500

/* Order matters when hooking up the hardware */
//-------NAME---SLAVE#------
#define EK1100    1
#define EL2024    2
#define EL1014    3
#define SLAVE_COUNT 3

char IOmap[4096];
OSAL_THREAD_HANDLE thread1;
int expectedWKC;
boolean needlf;
volatile int wkc;
boolean inOP;
uint8 currentgroup = 0;


uint8 read_input_uint8 (uint16 slave_number, uint8 module_index) {
    uint8 *data_ptr;
    
    data_ptr = ec_slave[slave_number].inputs;
    data_ptr += module_index * 2;
    

    return *data_ptr;
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
            if (verifyNetworkConfiguration()) {
                printf("%s", "Network verified\n");
                ec_config_map(&IOmap);
                ec_configdc();

                ec_send_processdata();
                wkc = ec_receive_processdata(EC_TIMEOUTRET);
                
                ec_slave[0].state = EC_STATE_OPERATIONAL;
                ec_writestate(0);
            } else {
                printf("Incorrect Network Units!\n");
                printf("Please see README for Setup instructions\n");
            }
        }
    } else {
        printf("No socket connection on %s\nExcecute as root\n",ifname);
    }
}

void set_output_int16 (uint16 slave_number, uint8 module_index, int16 value) {
    uint8 *data_ptr;

    data_ptr = ec_slave[slave_number].outputs;
    /* Move pointer to correct module index*/
    data_ptr += module_index * 2;
    /* Read value byte by byte since all targets can't handle misaligned
     * addresses
     */
    *data_ptr++ = (value >> 0) & 0xFF;
    *data_ptr++ = (value >> 2) & 0xFF;
}


/* Main Program Execution */
int main(int argc, char *argv[]) {
    int ii = 0;
    int jj = 0;

    printf("\n--------------------------------------------\n");
    printf("-    SOEM (Simple Open EtherCAT Master)    -\n");
    printf("- Please See README for setup instructions -\n");
    printf("--------------------------------------------\n\n");
    
    if (argc > 1) {
        /* create thread to handle slave error handling in OP */
        //osal_thread_create(&thread1, 128000, &ecatcheck, (void*) &ctime);
        
        /* start cyclic part */
        basicInit(argv[1]);
        
        printf("\nBegin Toggle of Digital Output");
//        while (ii > 0) {
        for (jj = 0; jj < 10; ++jj) {
            uint8 channel = 0b0001111;
            for (ii = 0; ii < ec_slave[EL2024].Obits; ++ii) {
                set_output_int16(EL2024,0,channel);
                ec_send_processdata();
                channel <<= 1;
                channel &= 0b00001111;  // Turns off 1 channel at a time
                osal_usleep(50000);
            }
            osal_usleep(50000);
            for (ii = 0; ii < ec_slave[EL2024].Obits; ++ii) {
                set_output_int16(EL2024,0,channel);
                channel |= 0b00010000;  // Turns on 1 channel at a time
                channel >>= 1;
                ec_send_processdata();
                osal_usleep(50000);
            }
        }
        
        ec_close();
    } else {
        printf("Usage: simple_test ifname1\nifname = eth0 for example\n");
    }

    printf("\nEnd program\n");
    return (0);
}

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


void set_output_int16 (uint16 slave_number, uint8 module_index, int16 value) {
    uint8 *data_ptr;

    data_ptr = ec_slave[slave_number].outputs;
    /* Move pointer to correct module index*/
    data_ptr += module_index * 2;
    /* Read value byte by byte since all targets can't handle misaligned
     * addresses
     */
    *data_ptr++ = (value >> 0) & 0xFF;
    *data_ptr++ = (value >> 8) & 0xFF;
}

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
    } else if (strcmp(ec_slave[EL2024].name, "EL2024")) {
        isOK = 0;
    } else if (strcmp(ec_slave[EL1014].name, "EL1014")) {
        isOK = 0;
    }
    return isOK;
}


void basicInit(char *ifname) {
    if (ec_init(ifname)) {
        if (ec_config_init(FALSE) > 0) {
            if (verifyNetworkConfiguration()) {
                ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE *4);
                ec_config_map(&IOmap);
                ec_configdc();
                ec_send_processdata();
                wkc = ec_receive_processdata(EC_TIMEOUTRET);

                ec_writestate(0);
                /* wait for all slaves to reach OP state */
                ec_statecheck(0, EC_STATE_OPERATIONAL,  EC_TIMEOUTSTATE);
            } else {
                printf("Incorrect Network Units!\n");
                printf("Please see README for Setup instructions\n");
            }
        }
    } else {
        printf("No socket connection on %s\nExcecute as root\n",ifname);
    }
}


/* 
 * Slave Error Handling
 */
OSAL_THREAD_FUNC ecatcheck( void *ptr ) {
    int slave;
    (void)ptr;                  /* Not used */
    
    while(1) {
        wkc = ec_receive_processdata(EC_TIMEOUTRET);
        expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
        
        if (inOP && ((wkc < expectedWKC) || ec_group[currentgroup].docheckstate)) {
            if (needlf) {
               needlf = FALSE;
               printf("\n");
            }
            /* one ore more slaves are not responding */
            ec_group[currentgroup].docheckstate = FALSE;
            ec_readstate();
            for (slave = 1; slave <= ec_slavecount; slave++) {
               if ((ec_slave[slave].group == currentgroup) 
                    && (ec_slave[slave].state != EC_STATE_OPERATIONAL)) {

                  ec_group[currentgroup].docheckstate = TRUE;
                  if (ec_slave[slave].state == (EC_STATE_SAFE_OP + EC_STATE_ERROR)) {
                     printf("ERROR : slave %d is in SAFE_OP + ERROR, attempting ack.\n", slave);
                     ec_slave[slave].state = (EC_STATE_SAFE_OP + EC_STATE_ACK);
                     ec_writestate(slave);
                  } else if(ec_slave[slave].state == EC_STATE_SAFE_OP) {
                     printf("WARNING : slave %d is in SAFE_OP, change to OPERATIONAL.\n", slave);
                     ec_slave[slave].state = EC_STATE_OPERATIONAL;
                     ec_writestate(slave);
                  } else if(ec_slave[slave].state > EC_STATE_NONE) {
                     if (ec_reconfig_slave(slave, EC_TIMEOUTMON)) {
                        ec_slave[slave].islost = FALSE;
                        printf("MESSAGE : slave %d reconfigured\n",slave);
                     }
                  } else if(!ec_slave[slave].islost) {
                     /* re-check state */
                     ec_statecheck(slave, EC_STATE_OPERATIONAL, EC_TIMEOUTRET);
                     if (ec_slave[slave].state == EC_STATE_NONE) {
                        ec_slave[slave].islost = TRUE;
                        printf("ERROR : slave %d lost\n",slave);
                     }
                  }
               }
               if (ec_slave[slave].islost) {
                  if(ec_slave[slave].state == EC_STATE_NONE) {
                     if (ec_recover_slave(slave, EC_TIMEOUTMON)) {
                        ec_slave[slave].islost = FALSE;
                        printf("MESSAGE : slave %d recovered\n",slave);
                     }
                  } else {
                     ec_slave[slave].islost = FALSE;
                     printf("MESSAGE : slave %d found\n",slave);
                  }
               }
            }
            if(!ec_group[currentgroup].docheckstate) {
               printf("OK : all slaves resumed OPERATIONAL.\n");
            }
        }
        osal_usleep(10000);
    }
}

/* Main Program Execution */
int main(int argc, char *argv[]) {
    int ii = 0;

    printf("\n--------------------------------------------\n");
    printf("-    SOEM (Simple Open EtherCAT Master)    -\n");
    printf("- Please See README for setup instructions -\n");
    printf("--------------------------------------------\n\n");
    
    if (argc > 1) {
        /* create thread to handle slave error handling in OP */
        osal_thread_create(&thread1, 128000, &ecatcheck, (void*) &ctime);
        /* start cyclic part */
        basicInit(argv[1]);
        
        /* Set outputs to zero for EL2024. ii cycles through the channel number */
        for (ii = 0; ii < ec_slave[EL2024].Obits; ++ii) {
            set_output_int16(EL2024,ii,0);
        }
        
        //Sleep here for some amount of time
        
        /* Read inputs from EL1014 */
        for (ii = 0; ii < ec_slave[EL1014].Ibits; ++ii) {
            printf("Channel %d, value: %d",ii,read_input_uint8(EL1014,ii));
        }
        
        /* Set outputs to zero for EL2024. ii cycles through the channel number */
        for (ii = 0; ii < ec_slave[EL2024].Obits; ++ii) {
            set_output_int16(EL2024,ii,1);
        }
        
        /* Read inputs from EL1014 */
        for (ii = 0; ii < ec_slave[EL1014].Ibits; ++ii) {
            printf("Channel %d, value: %d",ii,read_input_uint8(EL1014,ii));
        }
        
    } else {
        printf("Usage: simple_test ifname1\nifname = eth0 for example\n");
    }

    printf("End program\n");
    return (0);
}

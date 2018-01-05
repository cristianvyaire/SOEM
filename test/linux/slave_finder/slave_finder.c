/*
 * Simple Utility to find all slaves currently attached
 */
 
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "ethercat.h"

int expectedWKC;
char IOmap[4096];   // This doesnt need to be this big. See tutorial for info

void initSlaves (char *interface) {
    /* Initialize SOEM, Bind Socket to Interface */
    if (ec_init(interface)) {
        /* Find and Auto-Config Slaves */
        if (ec_config_init(FALSE)) {
            ec_config_map(&IOmap);
            ec_configdc();

            ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE * 4);
            expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
            ec_slave[0].state = EC_STATE_OPERATIONAL;
            /* Send one valid process data to make outputs in slaves "happy" */
            ec_send_processdata();
            ec_receive_processdata(EC_TIMEOUTRET);
        }
    }
}

void printSlaves (int slaveCount) {
    int ii, jj;
    
    printf("Simple Open EtherCAT Slave Finder\n");
    for (ii = 1; ii <= slaveCount; ii++) {                            // ii=1 
        printf("\n Slave: %d\n", ii);
        printf(" Name: %s\n", ec_slave[ii].name);
        printf(" Output size: %d-bits\n", ec_slave[ii].Obits);
        printf(" Input size:  %d-bits\n", ec_slave[ii].Ibits);
        printf(" State: %d\n", ec_slave[ii].state);
        printf(" Delay: %d[ns]\n", ec_slave[ii].pdelay);
        printf(" Has DC: %d\n", ec_slave[ii].hasdc);

        if (ec_slave[ii].hasdc) {
            printf(" DCParentport: %d\n", ec_slave[ii].parentport);
        }
        printf(" Configured address: %4.4x\n", ec_slave[ii].configadr);
        printf(" Man: %8.8x ID: %8.8x Rev: %8.8x\n",
               (int)ec_slave[ii].eep_man,
               (int)ec_slave[ii].eep_id,
               (int)ec_slave[ii].eep_rev);

        for (jj = 0; jj < EC_MAXSM; ++jj) {
            if (ec_slave[ii].SM[jj].StartAddr > 0) {
                printf(" SM%1d A:%4.4x L:%4d F:%8.8x Type:%d\n",
                       jj,
                       ec_slave[ii].SM[jj].StartAddr,
                       ec_slave[ii].SM[jj].SMlength,
                       (int)ec_slave[ii].SM[jj].SMflags,
                       ec_slave[ii].SMtype[jj]);
            }
        }

        for (jj = 0; jj < ec_slave[ii].FMMUunused; ++jj) {
            printf(" FMMU%1d Ls:%8.8x Ll:%d Lsb:%d Leb:%d Ps:%4.4x Psb:%d Ty:%2.2x Act:%2.2x\n",
                   jj,
                   (int)ec_slave[ii].FMMU[jj].LogStart,
                   ec_slave[ii].FMMU[jj].LogLength,
                   ec_slave[ii].FMMU[jj].LogStartbit,
                   ec_slave[ii].FMMU[jj].LogEndbit,
                   ec_slave[ii].FMMU[jj].PhysStart,
                   ec_slave[ii].FMMU[jj].PhysStartBit,
                   ec_slave[ii].FMMU[jj].FMMUtype,
                   ec_slave[ii].FMMU[jj].FMMUactive);
        }
    }
}





/* --- Main Program Execution --- */
int main(int argc, char *argv[]) {
    if (argc > 1) {
        initSlaves(argv[1]);
        if (ec_slavecount) {
            printSlaves(ec_slavecount);
        }
    } else {
        printf("Usage: simple_finder ifname1\nifname = eth0 for example\n");
    }

    return 0;
}


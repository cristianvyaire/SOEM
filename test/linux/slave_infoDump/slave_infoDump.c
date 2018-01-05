/**
 * Gets slave info and prints to standard out.
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "ethercat.h"

int workCounter;
char IOmap[4096];
char hstr[1024];

char* dtype2string(uint16 dtype)
{
    switch (dtype)
    {
    case ECT_BOOLEAN:
        sprintf(hstr, "BOOLEAN");
        break;
    case ECT_INTEGER8:
        sprintf(hstr, "INTEGER8");
        break;
    case ECT_INTEGER16:
        sprintf(hstr, "INTEGER16");
        break;
    case ECT_INTEGER32:
        sprintf(hstr, "INTEGER32");
        break;
    case ECT_INTEGER24:
        sprintf(hstr, "INTEGER24");
        break;
    case ECT_INTEGER64:
        sprintf(hstr, "INTEGER64");
        break;
    case ECT_UNSIGNED8:
        sprintf(hstr, "UNSIGNED8");
        break;
    case ECT_UNSIGNED16:
        sprintf(hstr, "UNSIGNED16");
        break;
    case ECT_UNSIGNED32:
        sprintf(hstr, "UNSIGNED32");
        break;
    case ECT_UNSIGNED24:
        sprintf(hstr, "UNSIGNED24");
        break;
    case ECT_UNSIGNED64:
        sprintf(hstr, "UNSIGNED64");
        break;
    case ECT_REAL32:
        sprintf(hstr, "REAL32");
        break;
    case ECT_REAL64:
        sprintf(hstr, "REAL64");
        break;
    case ECT_BIT1:
        sprintf(hstr, "BIT1");
        break;
    case ECT_BIT2:
        sprintf(hstr, "BIT2");
        break;
    case ECT_BIT3:
        sprintf(hstr, "BIT3");
        break;
    case ECT_BIT4:
        sprintf(hstr, "BIT4");
        break;
    case ECT_BIT5:
        sprintf(hstr, "BIT5");
        break;
    case ECT_BIT6:
        sprintf(hstr, "BIT6");
        break;
    case ECT_BIT7:
        sprintf(hstr, "BIT7");
        break;
    case ECT_BIT8:
        sprintf(hstr, "BIT8");
        break;
    case ECT_VISIBLE_STRING:
        sprintf(hstr, "VISIBLE_STRING");
        break;
    case ECT_OCTET_STRING:
        sprintf(hstr, "OCTET_STRING");
        break;
    default:
        sprintf(hstr, "Type 0x%4.4X", dtype);
    }
    return hstr;
}

/* Pretty much a copy of ec_siiPDO function in ethercatmain.c with print 
 * statements added 
 */
int sii_PDO(uint16 slave, uint8 t, int mapOffset, int bitOffset) {
    uint16 a, w, c, e, er, Size;
    uint8 eectl;
    uint16 obj_idx;
    uint8 obj_subidx;
    uint8 obj_name;
    uint8 obj_datatype;
    uint8 bitlen;
    int totalsize;
    ec_eepromPDOt eepPDO;
    ec_eepromPDOt *PDO;
    int abs_offset, abs_bit;
    char str_name[EC_MAXNAME + 1];

    eectl = ec_slave[slave].eep_pdi;
    Size = 0;
    totalsize = 0;
    PDO = &eepPDO;
    PDO->nPDO = 0;
    PDO->Length = 0;
    PDO->Index[1] = 0;
    for (c = 0; c < EC_MAXSM; c++) {
        PDO->SMbitsize[c] = 0;
    }
    
    if (t > 1) {
        t = 1;
    }
    PDO->Startpos = ec_siifind(slave, ECT_SII_PDO + t);                         // System hangs when no slaves detected
    if (PDO->Startpos > 0) {
        a = PDO->Startpos;
        w = ec_siigetbyte(slave, a++);
        w += (ec_siigetbyte(slave, a++) << 8);
        PDO->Length = w;
        c = 1;
        /* traverse through all PDOs */
        do {
            PDO->nPDO++;
            PDO->Index[PDO->nPDO] = ec_siigetbyte(slave, a++);
            PDO->Index[PDO->nPDO] += (ec_siigetbyte(slave, a++) << 8);
            PDO->BitSize[PDO->nPDO] = 0;
            c++;
            /* number of entries in PDO */
            e = ec_siigetbyte(slave, a++);
            PDO->SyncM[PDO->nPDO] = ec_siigetbyte(slave, a++);
            a++;
            obj_name = ec_siigetbyte(slave, a++);
            a += 2;
            c += 2;
            if (PDO->SyncM[PDO->nPDO] < EC_MAXSM) /* active and in range SM? */
            {
                str_name[0] = 0;
                if (obj_name)
                    ec_siistring(str_name, slave, obj_name);
                if (t)
                    printf("  SM%1d RXPDO 0x%4.4X %s\n", 
                        PDO->SyncM[PDO->nPDO], 
                        PDO->Index[PDO->nPDO], 
                        str_name);
                else
                    printf("  SM%1d TXPDO 0x%4.4X %s\n", 
                        PDO->SyncM[PDO->nPDO],
                        PDO->Index[PDO->nPDO], 
                        str_name);
                printf("     addr b   index: sub bitl data_type    name\n");
                /* read all entries defined in PDO */
                for (er = 1; er <= e; er++) {
                    c += 4;
                    obj_idx = ec_siigetbyte(slave, a++);
                    obj_idx += (ec_siigetbyte(slave, a++) << 8);
                    obj_subidx = ec_siigetbyte(slave, a++);
                    obj_name = ec_siigetbyte(slave, a++);
                    obj_datatype = ec_siigetbyte(slave, a++);
                    bitlen = ec_siigetbyte(slave, a++);
                    abs_offset = mapOffset + (bitOffset / 8);
                    abs_bit = bitOffset % 8;

                    PDO->BitSize[PDO->nPDO] += bitlen;
                    a += 2;

                    /* skip entry if filler (0x0000:0x00) */
                    if (obj_idx || obj_subidx) {
                        str_name[0] = 0;
                        if (obj_name) {
                            ec_siistring(str_name, slave, obj_name);
                        }

                        printf("  [0x%4.4X.%1d] 0x%4.4X:0x%2.2X 0x%2.2X",
                               abs_offset,
                               abs_bit,
                               obj_idx,
                               obj_subidx,
                               bitlen);
                        printf(" %-12s %s\n",
                               dtype2string(obj_datatype),
                               str_name);
                    }
                    bitOffset += bitlen;
                    totalsize += bitlen;
                }
                PDO->SMbitsize[PDO->SyncM[PDO->nPDO]] += PDO->BitSize[PDO->nPDO];
                Size += PDO->BitSize[PDO->nPDO];
                c++;
            /* PDO deactivated because SM is 0xff or > EC_MAXSM */
            } else {
            
                c += 4 * e;
                a += 8 * e;
                c++;
            }
            if (PDO->nPDO >= (EC_MAXEEPDO - 1)) {
                /* limit number of PDO entries in buffer */
                c = PDO->Length;
            }
            
        } while (c < PDO->Length);
    }
    if (eectl) {
        /* if eeprom control was previously pdi then restore */
        ec_eeprom2pdi(slave); 
    }
    
    return totalsize;
}

/* Maps out Inputs/Outputs of Current Slave Information Interface */
int sii_map(int slave) {
    int foundIO = 0;

    int totalSize, outputs_bitOffset, inputs_bitOffset;
    outputs_bitOffset = inputs_bitOffset = 0;
    /* Read the Assign RXPDOs */
    totalSize = sii_PDO(slave, 1, (int)(ec_slave[slave].outputs - (uint8*)&IOmap), outputs_bitOffset);
    outputs_bitOffset += totalSize;
    /* Read the Assign TXPDOs */
    totalSize = sii_PDO(slave, 0, (int)(ec_slave[slave].inputs - (uint8*)&IOmap), inputs_bitOffset);
    inputs_bitOffset += totalSize;

    if (outputs_bitOffset > 0 || inputs_bitOffset > 0) {
        foundIO = 1;
    }

    return foundIO;
}

/* Main functionality of program */
void slave_infoDump(char *ifname)
{
//    uint16 ssigen;
    int ii, jj;
    int expectedWKC;

    printf("Starting slave information dump\n");
    /* initialize SOEM, bind socket to ifname */
    if (!ec_init(ifname)) {                                                     // (ec_init(ifname))
        printf(" ...ec_init on %s succeeded\n", ifname);
        /* Map and Initialize all slave devices */
        workCounter = ec_config(FALSE, &IOmap);
        //printf(" ...ec_config found %d work counters\n", workCounter);
        if (workCounter < 0) {                                                  // (workCounter > 0)
            /* Configure Distributed Clock and Measure Propagation Delays */
            ec_configdc();

            /* Check for Errors on Slave devices */
            while (EcatError) {
                printf("%s", ec_elist2string());
            }
            printf(" ...found and configured %d slaves\n", ec_slavecount);
            expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
            printf(" ...calculated workcounter %d\n", expectedWKC);

            /* Wait for all slaves to reach SAFE_OP state */
            ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE * 3);
            if (ec_slave[0].state != EC_STATE_SAFE_OP) {
                printf("Not all slaves reached safe operational state.\n");
                ec_readstate();
                for (ii = 1; ii <= ec_slavecount; ++ii) {
                    if (ec_slave[ii].state != EC_STATE_SAFE_OP) {
                        printf("Slave %d State=%2x StatusCode=%4x : %s\n",
                               ii,
                               ec_slave[ii].state, ec_slave[ii].ALstatuscode,
                               ec_ALstatuscode2string(ec_slave[ii].ALstatuscode));
                    }
                }
            }

            ec_readstate();
            /* Print out Slave Status Members */
            for (ii = 0; ii <= ec_slavecount; ii++) {                            // ii=1 
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

                for (jj = 0; jj <= ec_slave[ii].FMMUunused; ++jj) {             // jj < ec_slave[ii].FMMUunused
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
                printf(" FMMUfunc 0:%d 1:%d 2:%d 3:%d\n",
                       ec_slave[ii].FMMU0func,
                       ec_slave[ii].FMMU1func,
                       ec_slave[ii].FMMU2func,
                       ec_slave[ii].FMMU3func);
                printf(" MBX length wr: %d rd: %d MBX protocols : %2.2x\n",
                       ec_slave[ii].mbx_l,
                       ec_slave[ii].mbx_rl,
                       ec_slave[ii].mbx_proto);

                //               ssigen = ec_siifind(ii, ECT_SII_GENERAL);                     // System hang if no slave attached
                /* Slave Information Interface (SII) general section */
                /*               if (ssigen)
                 {
                 ec_slave[ii].CoEdetails = ec_siigetbyte(ii, ssigen + 0x07);
                 ec_slave[ii].FoEdetails = ec_siigetbyte(ii, ssigen + 0x08);
                 ec_slave[ii].EoEdetails = ec_siigetbyte(ii, ssigen + 0x09);
                 ec_slave[ii].SoEdetails = ec_siigetbyte(ii, ssigen + 0x0a);
                 if((ec_siigetbyte(ii, ssigen + 0x0d) & 0x02) > 0)
                 {
                 ec_slave[ii].blockLRW = 1;
                 ec_slave[0].blockLRW++;
                 }
                 ec_slave[ii].Ebuscurrent = ec_siigetbyte(ii, ssigen + 0x0e);
                 ec_slave[ii].Ebuscurrent += ec_siigetbyte(ii, ssigen + 0x0f) << 8;
                 ec_slave[0].Ebuscurrent += ec_slave[ii].Ebuscurrent;
                 }
                 */
                printf(" CoE details: %2.2x FoE details: %2.2x EoE details: %2.2x SoE details: %2.2x\n",
                       ec_slave[ii].CoEdetails,
                       ec_slave[ii].FoEdetails,
                       ec_slave[ii].EoEdetails,
                       ec_slave[ii].SoEdetails);
                printf(" Ebus current: %d[mA] only LRD/LWR:%d\n",
                       ec_slave[ii].Ebuscurrent,
                       ec_slave[ii].blockLRW);

                /* Print out Slave Information Interface Process Data Objects */
                sii_map(ii);
            }
        } else {
            printf("No slaves found!\n");
        }

        printf("\n ...End slaveinfo, close socket\n");
        /* stop SOEM, close socket */
        ec_close();

    } else {
        printf("No socket connection on %s\nExcecute as root\n", ifname);
    }
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        slave_infoDump(argv[1]);
    } else {
        printf("Usage: slave_infoDump ifname1\nifname = eth0 for example\n");
    }

    return (0);
}


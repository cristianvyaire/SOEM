
This is a program designed to run a basic task using a limited set of hardware.

Hardware Components:
1. 24v PS  - DIN Power Supply to power the EtherCAT Cluster
2. EK1100  - EtherCAT terminal coupler (24V)
3. EL2024  - 4-channel digital output (24v)
4. EL1014  - 4-channel digital input (24v)

Setup instructions: 
1. Assemble EtherCAT cluster in this order such that the program can run properly.
    a. Power Supply hooked up to EK1100
    b. EL2024 attached to the EK1100
    c. EL1014 attached to the EL2024
    d. Master ethernet hooked up to the EK1100 coupler input.

Purpose:
  a) First a setup will be completed of the hardware.
  b) The program will verify that the specified hardware is installed.
  c) A sillyscope can be used to verify that the intended commands are being issued.
     1. A set of outputs will be sent to the EL2024 to zero out the ouptuts
     2. Then a set of command will be sent to set outputs to logic high.
  d) A power supply/or other 24 volt source can be hooked up to the inputs of the 
        EL1014 to simulate input.
     1.The program will then read the 4 channels from the EL1014 to verify 
        successfull values are being read.
    

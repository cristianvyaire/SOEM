
This is a program designed to run a basic task using a limited set of hardware.

Hardware Components:
1. 24v PS  - DIN Power Supply to power the EtherCAT Cluster
2. EK1100  - EtherCAT terminal coupler (24V)
3. EL3062  - 2-channel analog input terminal (0-10v, 12-bit) 
4. TSI Flowmeter - 4000 Series

Setup instructions: 
1. Assemble EtherCAT cluster in this order such that the program can run properly.
    a. Power Supply hooked up to EK1100
    b. EL3062 attached to the EK1100
    c. Analog outputs of TSI meter attached to the EL3062.
    d. Master ethernet hooked up to the EK1100 coupler input.

Purpose:
  a) First a setup will be completed of the hardware.
  b) The program will verify that the specified hardware is installed.
  c) Once the specified hardware is found, the address of that harware will be recorded.
  d) The readings coming from the TSI Flowmeter will be output to the EL3062 Unit.
  e) Using the discovered address, we will record the readings coming from the analog input
  

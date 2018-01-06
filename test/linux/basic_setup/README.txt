
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
        successful values are being read.
    
    
   * Alternative method of Hardware assembly *
   Apply steps a) through b) the same as above.
   c) ** NOTE: The jumper wires being used for this step should have a 2A capable resistor 
         attached to them in order to prevent the equiptment from being fried. **
        1) Using a wire hook channel 1 of EL2024 into channel 1 of EL1014.
        2) repeat step c) for all 4 channels, using the next channel of the EL2024 unit 
        to hook into the cooresponding channel of the EL1014 unit.
        
   d) Apply this step the same as above. 
    **Plug the EL2024 OUTPUT channels into the EL1014 INPUT Channels to read out states.
        (could be risky if slaves are not properly configured)

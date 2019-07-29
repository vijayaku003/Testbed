# Testbed
This repository contains the code for an Autonomous Vehicle Testbed. Objective of the setup is to test the performance of different degradation strategies in an Mixed Criticality System.  For details regarding the architecture of the setup, please refer to the research paper  https://ieeexplore.ieee.org/document/8759239

Major components required to run this Setup 
1. Electronic Control Unit implemented on the Texas Instruments Hercules Development Kit  (http://www.ti.com/tool/TMDS570LS31HDK)
2. Gateway implemented on the FreeScale Kinetis KEA  (https://www.nxp.com/products/processors-and-microcontrollers/arm-based-processors-and-mcus/general-purpose-mcus/ea-series-automotive-cortex-m0-plus/ultra-reliable-kea-automotive-microcontrollers-mcus-based-on-arm-cortex-m0-plus-core:KEA)
3. Simulink
4. Arduino 

Software tools required to use this setup:

1. Code Composer Studeon for compiling ECU code
2. Codewarrior IDE for compiling the gateway code
3. Arduino IDE for compiling the arduino code
4. Matlab/Simulink environment 

For details regarding installation of TORCS and compiling the code for linking TORCS and the Simulink model refer to https://github.com/VerifiableAutonomy/TORCSLink.

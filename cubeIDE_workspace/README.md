# cubeIDE_workspace

This workspace contains all the production and testing firmware used on the current iteration of the Smart VAD system.

The [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) development environment is used for all the firmware in this directory.
Please install the latest version of STM32CubeIDE before attempting to modify this code or download it to the receiver or transmitter.

## Prototype3_RadioTest
The main transmitter firmware running on the custom PCB mounted to the VAD.
This code samples the accelerometer at 100 Hz and runs a step counter algorithm, then transmits x,y,z acceleration and step count every 0.5 seconds
using the nRF24L01+ radio chip. This firmware is already uploaded to the transmitter PCBs included in the final deliverable package.

## Nucleo_Radio_Receiver
The main receiver firmware running on the [STM32L432KC Nucleo Board](https://www.st.com/en/evaluation-tools/nucleo-l432kc.html) connected to the 
nRF24L01+ radio receiver module and plugged in to a computer over USB. This firmware assumes that an nRF24L01+ radio module is connected to the microcontroller
board on SPI1, with PA0 as CE and PA4 as CS. This firmware is already uploaded to the Nucleo receiver system included in the final deliverable package.

## SPI_TEST
Radio testing code used to debug the nRF24L01+ radio system. Not a component in the final deliverable firmware.

## radio_mutex
Additional radio testing code used to debug the nRF24L01+ radio system. Not a component in the final deliverable firmware.

## test_cube
More radio testing code used to debug the nRF24L01+ radio system. Not a component in the final deliverable firmware.

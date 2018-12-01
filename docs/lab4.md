---
title: Lab 4
---

Lab 4: FPGA and Shape Detection
=================================================

## Overview

The purpose of Lab 4 was to implement a our treasure detection system on the robot. We did this by connecting the OV7670 Camera to the arduino and connecting that to a DEO-Nano FPGA which must generate a 24 Hz signal to run the camera. The FPGA will detect the image and its color, and communicate that to the Arduino to then react to that information.

### Required Parts

* Team Box (two arduinos + USB cables)
* 1 OV7670 Arduino Camera Module
* 1 VGA cable
* Female-female wires
* 2 FPGAs

## Arduino Team

The arduino team was in charge of looking at the datasheet for the OV7670 and set the appropriate registers of the device in order to give it the settings we desired. The functions we set were resetting all registers, setting up color bars, defining the resolution type to be rgb 565, setting the gain ceiling, enabling scaling, setting up the external clock, and allowing mirroring. Most of these were specifically outlined in the prelab.

We chose to use rgb 565 based on the fact that the VGA cable can transmit one byte of data, so 565 would require 2 bytes to be sent per pixel, which would be simple enough to program. We didn’t want to use any of the 1 byte formats because that could result in misclassification with the colors of the treasures where blue and green are more likely to be mixed up.

The only things we needed to add into the arduino code were definitions for the registers. The following is an example: 
    #define COM7  0x12

And writing the correct values to the registers:
    OV7670_write_register(COM7, 0x06);

We also needed to take into account that the FPGA and Arduino deal with read and write bits differently where the Arduino actually adds a bit onto the end while the FPGA simply has the full number of bits and specifies based on whether the LSB is 1 or 0. Because of this, we defined the camera address as being the right-shifted value of its original address which is 0x42 or 0x43 based on read or write. 0x42 right shifted is 0x21.

We connected the FPGA and assigned the GPIO_00 to be the 24-MHz output clock for the OV7670 camera module. We used oscilloscope to check the output signal to be 24-MHz. The ground line was shared between FPGA and arduino and camera. The code provided was modified to in order to write and read each registers to our desired values. We were able to print out the values of each register onto the serial monitor to check the correctness. We first reset all the registers (0x12) by setting it to 1000000 which in hex is 0x80. We then enable our desired VGA format setting by setting the register to 00000110 (0x06).  

## FPGA Team

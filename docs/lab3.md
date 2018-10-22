---
Title: Lab 3
---

Lab 3: System Integration and Radio Communication
=================================================

## Overview

The purpose of Lab 3 was to implement radio communication between the robot and
base station, and to integrate previous labs and milestones. By implementing the
radio communication, we could see the the maze’s walls appear on our base
station’s GUI. By integrating our previous labs and milestones, our robot could
start on a 660 Hz tone, avoid other robots, and ignore decoys.

### Required Parts
* Team Box (two arduinos + USB cables)
* 1 Phototransistor (and amplifier circuit)
* 1 Microphone (and active filter circuit)
* 1 IR hat
* 1 IR Decoy
* 2 Nordic nRF24L01+ transceivers
* 2 radio breakout boards with headers

## Radio Team

The radio team’s goal was to implement the radio communication and base station GUI. To test the radios, we downloaded the RF24 Arduino library and uploaded the “Getting Started” sketch to both Arduinos. We had difficulty transmitting and receiving stable responses, as shown in the video:

<div class="video"><iframe width="560" height="315" src="https://www.youtube.com/embed/LDkMGAEesIU" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe></div>

From this point, we continued to write code to transmit the robot’s facing direction to the base station. The starting directing was denoted as (0,0). As the robot turned right, this became (1,0), (1,1), and (0,1).

<div class="video"><iframe width="560" height="315" src="https://www.youtube.com/embed/fn5JD9jBUOY" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe></div>

From this point forward, the radio communication became so sporadic that we were unable to communicate effectively with the base station. We predict that our radios were problematic. Check back on Thursday for mapping to the GUI!

## Robot Team

The robot team’s goal was the integrate the previous labs and milestones. To do so, the robot was first rebuilt. Previously, the robot was structured in two layers: the top contained the IR phototransistor circuit, and the bottom contained the Arduino. These two layers were switched so the Arduino’s pins could be more easily accessible. The microphone’s active filter was also added to the robot, and wires were shortened to obtain a cleaner breadboard.

Breadboard with both circuits:

![Breadboard](media/lab3-breadboard.jpg "Breadboard")

Exploring the maze at the start of a tone:

<div class="video"><iframe width="560" height="315" src="https://www.youtube.com/embed/2kM76B30kLQ" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe></div>

Avoiding other robots:

<div class="video"><iframe width="560" height="315" src="https://www.youtube.com/embed/TtICiw_sR3o" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe></div>

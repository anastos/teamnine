---
title: Milestone 2
---

Milestone 2
===========

## Goal

The goal of this milestone was to show a robot that could right-hand wall follow, line track, and avoid other robots.

## Procedure

To implement right-hand wall following, we added two wall sensors. One was placed on robot’s front, and one was placed on the robot’s right side. At every intersection, our robot would check for walls at its front and right sides. We found that if the analog input of the wall sensor was greater than 150, then there is a wall next to the robot.

The robot would always turn right if there was no right wall. If there were a wall to the robot’s right and front, the robot would turn left. If there was a wall to the right but not front, the robot would continue moving forward.

<div class="video"><iframe width="560" height="315" src="https://www.youtube.com/embed/5in6LcYA7uQ" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe></div>

To implement robot avoidance, we added our phototransistor circuit from Lab 2. We also added our FFT setup and loop code from Lab 2. If our robot detected another robot’s IR hat, it would stop.

However, there is a timer issue with the servos and the FFT library, as the FFT function requires timer0 to be disabled, and the servos need it on. In order to fix this issue, we only initialize the FFT right before it is used and deinitialize it afterwards. The values for ADCSRA and TIMSK0 need to be reverted to their initial values immediately after the FFT runs so that the servos can continue to function correctly.

The IR circuit was able to detect an IR hat from approximately two blocks away. Depending on where we position the phototransistor and what threshold value we put in for the detecting condition, the detecting range may vary. We positioned it at the center of our robot between the first and second platform pointing straight ahead. Testing shows that this gives the best range and time for the robot to make appropriate responses. 

For purpose of demonstration, we made the robot stop after the detection of another robot, future adjustments can be made if we want to turn around and keep moving.

<div class="video"><iframe width="560" height="315" src="https://www.youtube.com/embed/_RDMJgwFjMg" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe></div>


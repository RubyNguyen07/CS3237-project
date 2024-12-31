## Introduction
In an era where technological integration into everyday life is paramount, the development of automated systems for service industries has become increasingly essential. The objective of this project is to innovate how services, particularly in fast-paced environments, can be optimised using robotic solutions. This report outlines the design and implementation of a robotic system designed to improve customer service efficiency through automation and real-time data processing.

### Project Motivation
The motivation behind this project stems from the necessity to enhance customer service interactions and reduce the human error factor in order processing environments. Traditionally, taking orders in busy settings, such as restaurants or retail stores, is prone to inaccuracies and inefficiencies. By integrating voice recognition and real-time data handling into a robotic platform, we aim to streamline these interactions, making them more efficient, accurate, and scalable.
### Problem Statement
The project seeks to develop a robot capable of navigating to a customer to take orders autonomously. Utilising advanced sensory technologies, including camera vision and microphone input, coupled with robust communication protocols via MQTT, the robot aims to interpret customer gestures and vocal commands accurately to process orders effectively.
### Proposed Solution
The proposed solution is structured around a three-part ESP32 module system, designed to optimise the robot's interactive and operational capabilities. ESP32-1 module serves as the primary communication hub, interfacing with an edge computer that processes visual data from a connected camera. This camera system is programmed to recognize customer gestures indicating service requests, which are then communicated to ESP32-1 via MQTT for appropriate action, such as triggering movement or initiating interaction protocols.The ESP32-2 module is dedicated to mobility and navigation, controlling the motors for the robot's wheels and managing inputs from line detectors and ultrasonic sensors to navigate the service environment effectively.
The ESP32-3 module houses a TensorFlow Lite-based voice recognition model, enabling the robot to process and understand spoken orders. The results from this module are sent back to ESP32-1, which logs the information into an inventory management system through HTTP communication. This setup ensures real-time inventory tracking and maintains order accuracy, integrating complex data processing seamlessly across the modules to enhance service efficiency and customer interaction.

## IoT System Vision
We will be designing an IoT system consisting of a robot that is able to on command receive a trigger signal from a restaurant user using an external monitoring system. The trigger signal could be a waving of a hand by a restaurant customer, requiring manual intaking of food orders. This trigger signal once detected by the external monitoring system will trigger a mobile robot to move its starting position A to the customer’s location B using a wifi-based communication protocol, and upon reaching its designated location B, it will activate its voice recognition program on board the robot to take in food orders. Food orders declared by the customer will be recognised and detected using voice recognition on the robot, and communicated back over a wifi based communication protocol to an inventory dashboard of the restaurant, where the order will be directly sent to the kitchen for preparation and also assist in stock inventory prediction for the restaurant. After sending the order, the robot will move from the customer’s location B back to its original starting point A, awaiting new command signals from the customer.

## Navigate through codebase 
A separate folder is created for each module of the system with corresponding name. 

## Design and Considerations
More information about system design and considerations can be found in the pdf here: [Chuckie Bytes Report](ChuckieBytesReport.pdf)

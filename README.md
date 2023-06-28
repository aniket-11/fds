# Flood Detection System (FDS) README

## Overview
The Flood Detection System (FDS) is an embedded project that utilizes the Arduino NodeMCU ESP8266 board to detect and monitor flood conditions. It incorporates various sensors including the DHT11 Sensor for measuring humidity, temperature, and heat index, the Ultrasonic sensor HC-SR04 for detecting the distance between the sensor and the water surface, the Water Flow Sensor YT-201 for identifying the flow rate in liters per hour, and a water level sensor to detect if the water has reached a specific point.

The collected sensor data is transmitted to a remote server in JSON format using a TLS connection. The remote server, written in Rust, accepts the payload and processes it accordingly. The processed data is then transformed into Prometheus metrics and exposed. These metrics are scraped by the Grafana agent every 10 seconds and written to the Amazon Managed Prometheus remote write URL. Additionally, Prometheus sends alerts using the SNS endpoint with the help of Alertmanager. Grafana is used to query the metrics and visualize them on a dashboard.

## Features
- Measures humidity, temperature, heat index, distance to water surface, flow rate, and water level
- Sends sensor data to a remote server in JSON format over a TLS connection
- Remote server, written in Rust, accepts and processes the payload
- Transforms processed data into Prometheus metrics on the remote server
- Exposes Prometheus metrics and scrapes them using the Grafana agent
- Writes metrics to an Amazon Managed Prometheus remote write URL
- Visualizes metrics on a Grafana dashboard
- Sends alerts using the SNS endpoint with Alertmanager

## Hardware Requirements
- Arduino NodeMCU ESP8266 board
- DHT11 Sensor
- Ultrasonic sensor HC-SR04
- Water Flow Sensor YT-201
- Water level sensor (Switch on/off)

## Software Dependencies
- Arduino IDE (for programming the Arduino NodeMCU ESP8266 board)
- Rust (for the remote server)
- Grafana
- Prometheus
- Prometheus Alertmanager
- Amazon Managed Prometheus
- TLS certificate for secure communication

## Setup Instructions
1. Connect the Arduino NodeMCU ESP8266 board to the required sensors as per the circuit diagram.
2. Install the Arduino IDE on your computer and configure it to work with the NodeMCU ESP8266 board.
3. Open the Arduino IDE and create a new sketch for the FDS project.
4. Import the necessary libraries for the DHT11, Ultrasonic, Water Flow, and water level sensors.
5. Write the code to read data from the sensors and transmit it to the remote server in JSON format over a TLS connection.
6. Upload the sketch to the Arduino NodeMCU ESP8266 board.
7. Set up the remote server in Rust to accept the payload, process the data, and transform it into Prometheus metrics.
8. Configure Prometheus to scrape the metrics exposed by the remote server and write them to the Amazon Managed Prometheus remote write URL.
9. Install and configure the Grafana agent to scrape the metrics from Prometheus and write them to the Grafana endpoint.
10. Install and configure promxy as a middleware for Grafana to handle requests.
11. Create a Grafana dashboard to visualize the collected metrics.
12. Configure Alertmanager to send alerts using the SNS endpoint.
13. Start the Flood Detection System and monitor the sensor data and metrics on the Grafana dashboard.



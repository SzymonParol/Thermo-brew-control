# Temperature Controller for Brewing

This repository contains a project designed to control temperature during the beer brewing process. The project is based on an STM32 F042K6 microcontroller and aims to simplify and improve the brewing experience by ensuring precise temperature control and providing additional features for monitoring and control.

## Features

### Current Features:
- **Temperature Sensor (DS18B20):** Measures the current temperature.
- **LCD Display (4x16, HD44780):** Displays temperature readings and other information.
- **Stepper Motor Control (A4988):** Controls a stepper motor for precise mechanical adjustments.

### Planned Features:
- **Thermocouple:** Detects if the flame has extinguished.
- **Rotary Encoder and Button:** Provides manual control and input options.
- **Wi-Fi Module (ESP8266):** Enables remote monitoring and control via a mobile application.

## Hardware Components
- **Microcontroller:** STM32 F042K6
- **Temperature Sensor:** DS18B20
- **LCD Display:** 4x16 with HD44780 controller
- **Stepper Motor Driver:** A4988
- **Planned Additions:** Thermocouple, rotary encoder, button, and ESP8266 module

## Software Features
The software is being developed using:
- STM32 HAL libraries
- 1-Wire protocol for the DS18B20 sensor
- Custom libraries for LCD control and stepper motor driver
- Future integration with ESP8266 for Wi-Fi connectivity

## Getting Started

### Prerequisites
To replicate or contribute to this project, you will need:
- An STM32 F042K6 development board
- The listed hardware components
- STM32CubeIDE or other compatible development environment

### Setup
1. Clone this repository:
   ```bash
   git clone [repository_url]
   ```
2. Connect the hardware components as follows:
   - DS18B20: GPIO pin configured for 1-Wire communication
   - LCD: GPIO pins for HD44780 interface
   - Stepper motor driver: GPIO pins for step/direction control
3. Flash the firmware to the STM32 microcontroller.

### Usage
Once the hardware is set up and firmware is flashed:
- The current temperature will be displayed on the LCD.
- Stepper motor controls will function as per the configuration.

## Future Enhancements
- **Thermocouple Integration:** For flame detection.
- **Encoder and Button:** For intuitive manual controls.
- **Mobile App Integration:** Using ESP8266 for remote monitoring and adjustments.

## Contribution
Contributions are welcome! Feel free to fork this repository, make changes, and submit a pull request.

## License
This project is licensed under the MIT License. See the `LICENSE` file for details.

## Contact
For any questions or suggestions, please feel free to reach out via [email/contact info].

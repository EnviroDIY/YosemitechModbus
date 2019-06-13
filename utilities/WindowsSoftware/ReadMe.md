# YosemiTech Windows Desktop Software

YosemiTech provides with each order a mini CD-ROM disk with their Windows desktop software designed to directly communicate with their sensors. This software is especially useful to test your sensor and:
- GET sensor info (e.g. modbus address, serial number, software version, calibration coefficients, and brush interval),
- GET measured values,
- SET sensor info, which is required for calibrating your sensor.

YosemiTech produces two different programs:
- **ModbusRunner**, to connect to any single sensor
- **MultiSensor**, to connect to a Y4000 MultiParemter Sonde, with up to six sensors.
We share the latest versions of each in this directory. Download the folder to you computer and click on the .exe file to run.

### Required USB-to-RS485 Converter Device
To connect your sensor to this software, you will need a USB-to-RS485 Converter Devices, which Yosemitech sells ($5, shown below) or you can optain separately. Note that the Y4000 Sonde requires 12V of power, so you will need a separate 12V power supply.

| <img src="https://github.com/EnviroDIY/YosemitechModbus/blob/master/doc/Images/USB-RS485-converter-front.JPG"  height="200"> | <img src="https://github.com/EnviroDIY/YosemitechModbus/blob/master/doc/Images/USB-RS485-converter-back.JPG"  height="200"> |


### Instructions to use ModbusRunner Software
- Once you have physically connected your sensor to your computer with the USB-to-RS485 Converter, confirm that Windows recognizes it as a COM port, using Windows Device Manager under "Ports (COM & LPT)". It should be listed as something like "USB-Serial CH341A (COM3)".
  - Note the COM port number. It is usually COM3 by default, but only if you don't already have a device connected as COM3.
- Manually type in the number of the COM port into ModbusRunner, without a space, exactly as how it is show in the screen-shot below (i.e. `COM3`).

<img src="https://github.com/EnviroDIY/YosemitechModbus/blob/master/doc/Images/ModbusRunner-v2.4_1-Probe.png">

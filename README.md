# Yosemitech Modbus

A library to use an Arduino as a master to control and communicate with the modbus sensors produced by Yosemitech. These sensors only support these modbus commands:
* 3 (0x03, Read holding registers)
* 16 (0x10, Write multiple registers).

## Hardware

#### RS485 communication/connection:
The sensors need to communicate with the Arduino via a RS485 adapter (we used this [SCM TTL to RS485 Adapter / 485 to Serial Port UART Level Converter Module (XY-017) ](https://www.amazon.com/gp/product/B01J9C7JNA), sold by many vendors for $3-10 ).

The RS485 adapter receives power separate from the sensors, and can use any of the Mayfly board's 3.3v switched power ports. For convenience, we soldered a grove connector to the RS485 adapter so we connect it to the Mayfly's grove ports, but to do this we had to flip the wire configuration of the grove cables (yes, the flipped grove cable can connect in either direction, but I find it a best practice to keep the colors aligned for easy cross-check).
Note the cable colors:
* Green/Yellow = Transmit (A)
* White = Receive (B)
* Red = Power (VCC)
* Black = Ground

#### Sensor connection:
The Yosemitech sensors require 5-12v of power. For convenience, We have been giving them 5v switched power off an analog grove port on the Mayfly, as you can see in the photo, to keep digital ports open for other sensors. (Be sure that the jumper for the analog grove port is in the 5v position.) Also for convenience, we merge the RS485 signals from the adapter board with the 5v power via a 4-port grove hub, which allows us to use a grove screw terminal to receive the bare wires from the sensor.

![Photo of hardware configuration](https://github.com/EnviroDIY/YosemitechModbus/blob/yosemitech/doc/IMG_3529.JPG)

#### How to add more sensors:
Modbus/RS485 allows 247 devices to be attached to the same pair of receive/transmit wires. You can use the spare port on the grove hub to connect additional sensors (not yet tested). Some software considerations for adding additional sensors: the Yosemitech sensors all arrive with a "slaveID" (sensor address) of 0x01. These need to be re-assigned to run multiple sensors on the same Arduino board, which will become part of the procedure, probably as a re-assign utility prior to connection to the modular library.

#### Software serial:
This hardware configuration, which uses the digital pins, requires the use of the software serial library. This is because RX1 and TX1 on the Mayfly are dedicated to the XBee radio, so we cannot conveniently use them while also using the radio. (Also, RX0 and TX0 are dedicated to the USB connection to your computer.)

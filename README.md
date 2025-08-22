# Yosemitech Modbus<!--! {#mainpage} -->

A library to use an Arduino as a master to control and communicate with the sensors produced by [Yosemitech](http://en.yosemitech.com) via [Modbus RTU](https://en.wikipedia.org/wiki/Modbus) over [RS-485](https://en.wikipedia.org/wiki/RS-485).
This library requires the use of the [EnviroDIY/SensorModbusMaster](https://github.com/EnviroDIY/SensorModbusMaster) library.

YosemiTech sensors only support these modbus commands:

- 3 (0x03, Read holding registers)
- 16 (0x10, Write multiple registers)

The communication rate is set at 9600 baud/[8-N-1](https://en.wikipedia.org/wiki/8-N-1).
The default slave ID from the factory is 0x01.

The implementation of modbus by these sensors is _not_ fully compliant with the [official modbus standards](http://modbus.org/specs.php).
These are some of the irregularities we have found so far:

- The sensors do not act on server broadcast messages (that is, write commands sent to address 0x00).
- The sensors give no response to properly formed requests for data from any registers except those few listed in the modbus manuals.
(See the "doc" folder for copies of the manuals for the sensors we have.)

Another note:  The sensors seem to be somewhat.. finicky about resolving similar addresses when several are connected to the same bus.
I recommend giving the sensors addresses a few numbers away from each other to help separate them on the bus.
(That is, instead of numbering them as 0x01, 0x02, 0x03, 0x04, and 0x05 try 0x01, 0x03, 0x05, 0x07, and 0x09.)

<!--! @tableofcontents -->

<!--! @m_footernavigation -->

<!--! @if GITHUB -->

- [Yosemitech Modbus](#yosemitech-modbus)
  - [Hardware](#hardware)
    - [Power Supply](#power-supply)
      - [Current Consumption](#current-consumption)
    - [RS485 communication/connection](#rs485-communicationconnection)
    - [Receiving TTL Data](#receiving-ttl-data)
  - [Suggested setup with an EnviroDIY Mayfly](#suggested-setup-with-an-envirodiy-mayfly)
  - [Library installation](#library-installation)

<!--! @endif -->

## Hardware<!--! {#mainpage_hardware} -->

### Power Supply<!--! {#mainpage_power} -->

All of these sensors require a 5-12V DC power supply and the power supply can be stopped between measurements.
Older sensors may require higher voltage power supplies.
Do note that any time the sensor looses power, some internal configurations (such as brushing interval for sensors with wipers) will be reset to their factory values, whereas other internal configurations (such as calibration coefficients) are saved..
The red wire from the sensor should connect to the positive pole of the 5-12V power supply and the black to ground.

#### Current Consumption<!--! {#mainpage_current} -->

The current consumption of the Yosemitech sensors is specified to be <50mA, this is not accurate for their sensors which include a wiper brush, for these sensors an inductive spike exists when the brush begins to spin, which draws significantly more current than specified.
Below is a table describing the current draws for several different operating conditions of the Yosemitech 511 Turbidity senor (with brush).
All tests were driven by a power supply with a 9v output

| Condition      |      Current       |
| -------------- | :----------------: |
| Brush Starting | <290mA<sup>1</sup> |
| Brush Running  |        52mA        |
| Sensing        |        27mA        |
| Idle           |        7mA         |

Note <sup>1</sup>: The current on brush start can be seen in the image below, where the yellow line shows current into the sensor with a gain of 10 V/A, and the blue line is the output voltage to the sensor.
The spike peak is approximately 290mA, with an exponential decay over the next 10-15ms, while this is short, it is more than enough time to brown out many low current power supplies found in data logger systems, so appropriate precautions must be taken to avoid this problem.

![CurrentSpike](doc/TEK00013.PNG)

### RS485 communication/connection<!--! {#mainpage_rs485} -->

The sensors need to communicate with the Arduino via a RS485-to-TTL adapter.
The white wire of the Yosemitech sensor will connect to the "B" pin of the adapter and the green wire will connect to "A".
Note that most RS485 adapters require 3.3-5V of power and do NOT provide any power to the sensors.
For adapters that require it, this library does include code to toggle the RE and DE (receive/data enable), though the timing for this is very persnickety.
I recommend an adapter board with built in flow control.
Also, be mindful of the logic level of the TTL output by the adapter.
The MAX485, one of the most popular adapters, has a 5V logic level in the TTL signal.
Up to 247 different devices can be connected in the same RS485 circuit, provided that each sensor has been separately assigned a uniques slave id.

### Receiving TTL Data<!--! {#mainpage_ttl} -->

If possible, always connect the RS485 adapter so the TTL signal is going to a Hardware Serial port on the Arduino.
Hardware serial ports are more stable than software/pin change serial emulation.
If you do not have an available hardware serial port, [AltSoftSerial](https://github.com/PaulStoffregen/AltSoftSerial) is probably the best Arduino alternative, if your board is supported.
[NeoSWSerial](https://github.com/SlashDevin/NeoSWSerial) is also a decent alternative at the Yosemitech's 9600 baud.
Use the standard Arduino SoftwareSerial if nothing else is available, but be aware that some sensors may not respond correctly.

## Suggested setup with an EnviroDIY Mayfly<!--! {#mainpage_mayfly} -->

We suggest using a [Modbus-Mayfly_WingShield](https://github.com/EnviroDIY/SensorModbusMaster/tree/master/hardware/Modbus-Mayfly_WingShield), which combines this  [TTL to RS485 Adapter / 485 to Serial Port UART Level Converter Module (XY-017)](https://www.amazon.com/gp/product/B06XHH6B6R) with a step-up boost regulator and capacitors.
Plans to construct one can be found at <https://github.com/EnviroDIY/SensorModbusMaster/tree/master/hardware/Modbus-Mayfly_WingShield>.

<img src="https://github.com/EnviroDIY/SensorModbusMaster/blob/master/hardware/Modbus-Mayfly_WingShield/Photos/IMG_6733.JPG"  width="600">

## Library installation

This library is available through both the Arduino and PlatformIO library registries.
[Here is the PlatformIO registry page.](https://registry.platformio.org/libraries/envirodiy/YosemitechModbus)
Use the Arduino IDE to find the library in that registry.
The build and ingest logs for this library into the Arduino library registry are available [here](https://downloads.arduino.cc/libraries/logs/github.com/EnviroDIY/YosemitechModbus/).

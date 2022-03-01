# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)
and its stricter, better defined, brother [Common Changelog](https://common-changelog.org/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

***

## [Unreleased]

### Changed

### Added

### Removed

### Fixed

***

## [0.3.2] 2022-03-01
Version bump only for reindexing by PlatformIO and Arduino library managers.

***

## [0.3.1] 2022-03-01

### Changed
- Modified example to demonstrate use with Espressif processor

### Added
- Restored backward compatibility with Y550
- Added github actions to test code
- Created ChangeLog and Version files


## [0.3.0] 2021-12-15
Add Y551 COD and Y560 Ammonium sensor support Latest

### Added
- Added support for the Y551 COD Sensor, which makes a UV254 light absorption and translates it to estimates of Chemical Oxygen Demand (COD) (or Total Organic Carbon (TOC)) and Turbidity.
  - NOTE that this upgrade removes the earlier Y550 from the library, as it was never tested and is no longer available form YosemiTech. If anyone has a Y550 sensor, the Y551 commands should work. Let us know if they don't.
- Added support for the Y560 Ammonium Probe, which is a mini sonde for three Ion Selective Electrode (ISE) sensors (pH, NH4+, K+) that together are used to provide a corrected estimate of total ammonium nitrogen (NH4_N) in mg/L.
  - NOTE that this release only includes outputs for NH4_N, pH, and temperature. A future release will also include estimates of potassium (K) and raw potential values from each of the electrodes.
- Added User Manuals and Modbus Manuals for both new sensors

***

## [0.2.4] 2021-07-01
Update version tags for PlatformIO Library Registry

### Changed
- Update version numbers for proper release so it registers in the PlatformIO Library Registry at https://platformio.org/lib.

***

## [0.2.3] 2021-07-01
Fix to Y533 ORP; updated manuals & software

### Changed
- Separation of the Y533 ORP getValues from Y532 pH, fixing the issue raised with #19.
  - The new getValues command for the Y533 now returns Electrical Potential (mV) as the primary parmValue.
  - Replicated fix to startMeasurement return, which is required to get Y532/Y533 to work as a stand-alone sensors in ModularSensors.
- Updates to Windows software utilities and documentation
  - new SmartPC v2.4 software
- Many updates to YosemiTech manuals
  - Note that YosemiTech is now providing product manuals on their website under the Resources menu. See http://en.yosemitech.com/aspcms/downlist/list-23-1.html
- Several updates to calibration spreadsheets & protocols.

***

## [0.2.2] 2020-01-02
Better Y532 pH support, updated manuals & software

### Changed
- Fix to Y532 pH startMeasurement return, which is required to get Y532 pH to work as a stand-alone sensor in ModularSensors. Commit 574a31c.
- Updates to Windows software utilities and documentation
  - ModbusRunner v2.5.1.5
  - MulitSensor v1.9
- Updates to YosemiTech manuals
- Updates to calibration spreadsheets & protocols. Issues #14, #16.

***

## [0.2.1] 2018-09-21
Fixed setCalibration

### Changed
Fixed problem with setting calibration constants. Both values must be set in a single command.

***

## [0.2.0] 2018-04-26
Added Y4000 sonde support

### Added
- Support for the Y4000 multi-parameter sonde added by Anthony Aufdenkampe.

***

## [0.1.9] 2018-02-05
Newest Modbus Manuals

### Changed
- Updates based on the newest Modbus manuals, acquired from Yosemitech Dec 2017.

***

## [0.1.6] 2018-02-02
Initial release
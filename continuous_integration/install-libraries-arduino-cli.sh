#!/bin/bash

# Makes the bash script print out every command before it is executed, except echo
trap '[[ $BASH_COMMAND != echo* ]] && echo $BASH_COMMAND' DEBUG

# Exit with nonzero exit code if anything fails
set -e

echo "\n\e[32mCurrent Arduino CLI version:\e[0m"
arduino-cli version

echo "::group::Installing Libraries"
echo "\n\e[32mUpdating the library index\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml lib update-index

echo "\n\e[32mInstalling EnviroDIY SensorModbusMaster library from Arduino library index\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml lib install SensorModbusMaster

echo "\n\e[32mInstalling AltSoftSerial library from GitHub\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml lib install --git-url https://github.com/PaulStoffregen/AltSoftSerial.git

echo "::group::Current globally installed packages"
echo "\n\e[32mCurrently installed libraries:\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml lib update-index
arduino-cli --config-file continuous_integration/arduino_cli.yaml lib list
echo "::endgroup::"

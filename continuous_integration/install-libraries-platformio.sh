#!/bin/bash

# Makes the bash script print out every command before it is executed, except echo
trap '[[ $BASH_COMMAND != echo* ]] && echo $BASH_COMMAND' DEBUG

# Exit with nonzero exit code if anything fails
set -e

echo "::group::Installing Libraries"
echo "\e[32m\nCurrently installed libraries:\e[0m"
pio pkg list -g -v --only-libraries
echo "::endgroup::"

echo "\e[32mInstalling envirodiy/SensorModbusMaster\e[0m"
pio pkg install -g --library envirodiy/SensorModbusMaster

echo "\e[32mInstalling https://github.com/PaulStoffregen/AltSoftSerial.git\e[0m"
pio pkg install -g --library https://github.com/PaulStoffregen/AltSoftSerial.git

echo "::group::Current globally installed libraries"
echo "\e[32m\nCurrently installed packages:\e[0m"
pio pkg list -g -v --only-libraries
echo "::endgroup::"

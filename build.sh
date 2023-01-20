#!/bin/bash

ARDUINO_RUNTIME=~/.arduino15
ARDUINO_HOME=/usr/share/arduino
INO_FILE=ESPEInk_ESP32.ino
LIBRARY_PATH=~/Arduino/libraries
BOARD_DEFINITION="esp32:esp32:esp32:PSRAM=disabled,PartitionScheme=default,CPUFreq=240,FlashMode=qio,FlashFreq=80,FlashSize=4M,UploadSpeed=921600,DebugLevel=none"
TOOL_VERSION=1.22.0-97-gc752ad5-5.2.0
FS_VERSION=${FS_VERSION}
IDE_VERSION=10819
ESP_TOO_VERSION=3.0.0

# prepare
mkdir -p /tmp/ESP32/cache

arduino-builder \
	-dump-prefs \
	-logger=machine \
	-hardware ${ARDUINO_HOME}/hardware \
	-hardware ${ARDUINO_RUNTIME}/packages \
	-tools ${ARDUINO_HOME}/hardware/tools/avr \
	-tools ${ARDUINO_RUNTIME}/packages \
	-libraries ${LIBRARY_PATH} \
	-fqbn=${BOARD_DEFINITION} \
	-ide-version=${IDE_VERSION} \
	-build-path /tmp/ESP32 \
	-warnings=all \
	-build-cache /tmp/ESP32/cache \
	-prefs=build.warn_data_percentage=75 \
	-prefs=runtime.tools.mkspiffs.path=${ARDUINO_RUNTIME}/packages/esp32/tools/mkspiffs/0.2.3 \
	-prefs=runtime.tools.mkspiffs-0.2.3.path=${ARDUINO_RUNTIME}/packages/esp32/tools/mkspiffs/0.2.3 \
	-prefs=runtime.tools.esptool_py.path=${ARDUINO_RUNTIME}/packages/esp32/tools/esptool_py/${ESP_TOO_VERSION} \
	-prefs=runtime.tools.esptool_py-2.6.1.path=${ARDUINO_RUNTIME}/packages/esp32/tools/esptool_py/${ESP_TOO_VERSION} \
	-prefs=runtime.tools.xtensa-esp32-elf-gcc.path=${ARDUINO_RUNTIME}/packages/esp32/tools/xtensa-esp32-elf-gcc/${TOOL_VERSION} \
	-prefs=runtime.tools.xtensa-esp32-elf-gcc-${TOOL_VERSION}.path=${ARDUINO_RUNTIME}/packages/esp32/tools/xtensa-esp32-elf-gcc/${TOOL_VERSION} \
	-verbose \
	${INO_FILE}

arduino-builder \
	-compile \
	-logger=machine \
	-hardware ${ARDUINO_HOME}/hardware \
	-hardware ${ARDUINO_RUNTIME}/packages \
	-tools ${ARDUINO_HOME}/hardware/tools/avr \
	-tools ${ARDUINO_RUNTIME}/packages \
	-libraries ${LIBRARY_PATH} \
	-fqbn=${BOARD_DEFINITION} \
	-ide-version=${IDE_VERSION} \
	-build-path /tmp/ESP32 \
	-warnings=none \
	-build-cache /tmp/ESP32/cache \
	-prefs=build.warn_data_percentage=75 \
	-prefs=runtime.tools.mkspiffs.path=${ARDUINO_RUNTIME}/packages/esp32/tools/mkspiffs/0.2.3 \
	-prefs=runtime.tools.mkspiffs-0.2.3.path=${ARDUINO_RUNTIME}/packages/esp32/tools/mkspiffs/0.2.3 \
	-prefs=runtime.tools.esptool_py.path=${ARDUINO_RUNTIME}/packages/esp32/tools/esptool_py/${ESP_TOO_VERSION} \
	-prefs=runtime.tools.esptool_py-2.6.1.path=${ARDUINO_RUNTIME}/packages/esp32/tools/esptool_py/${ESP_TOO_VERSION} \
	-prefs=runtime.tools.xtensa-esp32-elf-gcc.path=${ARDUINO_RUNTIME}/packages/esp32/tools/xtensa-esp32-elf-gcc/${TOOL_VERSION} \
	-prefs=runtime.tools.xtensa-esp32-elf-gcc-${TOOL_VERSION}.path=${ARDUINO_RUNTIME}/packages/esp32/tools/xtensa-esp32-elf-gcc/${TOOL_VERSION} \
	-verbose \
	${INO_FILE}

# upload to local firmware server
echo
echo "uploading to firmware server..."
scp /tmp/ESP32/ESPEInk_ESP32.ino.bin volker:

echo
echo "compressing image..."
gzip -f /tmp/ESP32/ESPEInk_ESP32.ino.bin

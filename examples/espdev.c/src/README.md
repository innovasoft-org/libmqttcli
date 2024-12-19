# Espcli

I) Tools preparation
Source: https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/get-started/linux-setup.html

1. Download the compiler file:
- for 64-bit linux: https://dl.espressif.com/dl/xtensa-lx106-elf-gcc8_4_0-esp-2020r3-linux-amd64.tar.gz
- for 32-bit linux: https://dl.espressif.com/dl/xtensa-lx106-elf-gcc8_4_0-esp-2020r3-linux-i686.tar.gz
- for older gcc: https://dl.espressif.com/dl/xtensa-lx106-elf-linux32-1.22.0-100-ge567ec7-5.2.0.tar.gz

2. Change permissions to /opt folder
$ chown -R kuba:kuba ./

3. Copy xtensa-lx106-elf into /opt/Espressif

4. Update the path in the project Makefile
XPATH=/opt/Espressif/xtensa-lx106-elf/bin

See https://github.com/esp8266/esp8266-wiki/wiki/gpio-registers if you get compile errors

5. esptool installation
$ sudo apt install python
$ sudo apt install pip
$ sudo pip install esptool

II) Compilation

1. Run the script
$ cd src
$ ./gen_misc.sh

2. Select the following options:
- for Non FOTA: 1 (boot_v1.2+), 0 (eagle.flash.bin+eagle.irom0text.bin), 3 (80MHz), 3 (DOUT), 2 (1024KB( 512KB+ 512KB))
- for FOTA: tbs

FOTA = Firmware Over The Air

III) Loading the firmware
1. Only first time, erase and prepare device
sudo esptool.py -p /dev/ttyUSB0 -b 115200 erase_flash
sudo esptool.py -p /dev/ttyUSB0 -b 115200 write_flash 0xfb000 blank.bin 0xfc000 esp_init_data_default_v08.bin 0xfe000 blank.bin 

2. Load the new firmware
sudo ./esptool.py -p /dev/ttyUSB0 -b 115200 write_flash 0x00000 firmware/eagle.flash.bin 0x10000 firmware/eagle.irom0text.bin 

IV) TESTING)

Join nt multicast group
$ iperf -s -u -B 225.0.0.1 -i 1 -p 1025

Check available Access Poits
sudo iwlist wlp2s0 scan | grep 'ESSID'

Listening debuf information
sudo screen /dev/ttyUSB0 115200
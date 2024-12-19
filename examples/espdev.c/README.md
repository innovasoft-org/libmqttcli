# espdev

## Compilation
1. Configure ESP SDK
2. Run the script `src/gen_misc.sh`
3. Binaries will be available in bin folder
> [!NOTE]
> If you do not want to compile the source code, its compiled version is also available in `bin` folder.

## Loading into device
1. Download `esptool.py` software
2. Connect the programmer with the device
3. Erase existing software using: `esptool.py -p /dev/ttyUSB0 -b 115200 erase_flash`
4. Set defaults using: `sudo esptool.py -p /dev/ttyUSB0 -b 115200 write_flash 0xfb000 blank.bin 0xfc000 esp_init_data_default_v08.bin 0xfe000 blank.bin`
5. Load the software using: `esptool.py -p /dev/ttyUSB0 -b 115200 write_flash 0x00000 eagle.flash.bin 0x10000 eagle.irom0text.bin`

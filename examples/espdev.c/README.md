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

## Configuration
1. Select the Wi-Fi on your phone/laptop which is formatted as follows `ESP-XXXXXXXX`
2. In the web browser enter the following web site `http://192.168.4.1` (<a href="#fig01">Fig. 1</a>)
3. Set the following mandatory fields:
  - Your Wi-Fi name: `SSID`
  - Your Wi-Fi password: `Password`
  - Your HA's server name: `Host Name`; default value is `homeassistant.local:8123`
  - Your HA's user name: `User Name`
  - Your HA's user password: `Password`
  - Write again Wi-Fi name: `Device ID`, to confirm your settings
4. Click the `Update` button
5. If configuration was successfull on the web page will be displayed `Data saved successfully.` (<a href="#fig02">Fig. 2</a>)

<p align="center">
  <a name="fig01"> 
  <img src="doc/conf_web.png" /> </br>
  <b>Fig. 1. Configuration web page. </b>
  </a>
</p>

<p align="center">
  <a name="fig02"> 
  <img src="doc/conf_success.png" /> </br>
  <b>Fig. 2. Configuration successfull. </b>
  </a>
</p>

## Usage
1. Log into your HA service
2. Select `Overview` panel
3. New `switch` should be dicplayed with the name formatted as follows `ESP-XXXXXXXX`
4. Toggle on the `switch`, it should turn on the connected device


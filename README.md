# CLI interface for DS18X20 temperature probe 

### Istallation

Make sure you have the following packages: __gcc__, __libusb-dev__ and __make__ installed. Otherwise run `sudo apt-get install gcc libusb-dev make` or similar command.

Then, build binary:

```
git clone https://github.com/jaka/usbtemp-cli.git
cd usbtemp-cli/
make
```

To enable users access USBasp v2.0 DS18X20 temperature probe run commands:

```
echo 'SUBSYSTEMS=="usb", ATTRS{idVendor}=="03eb", GROUP="users", MODE="0666"' | sudo tee /etc/udev/rules.d/60-atmelusb.rules
sudo udevadm trigger
```

### Usage

To print sensors' 64-bit unique registration numbers, issue
`./usbtemp rom`

To obtain a temperature first start measurement `./usbtemp measure` and then read the temperature `./usbtemp temp`.

```
USBtemp CLI 2016

S:0 T: 19.00 °C
```

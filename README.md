### Brother DCP-7030 Scanner

While setting up my usb scanner/printer combination on an OpenWRT/LEDE router (TP-Link TL-WDR4300), I could not get the scanner to work.
The proprietary SANE driver "brscan3" from Brother is not available for MIPS CPUs, and the source code they supply seems incomplete.

After some searching, I found the project from davidar, who reverse engineered the usb protocol of his Brother MFC-7400C a few years ago.
Go see his repo for a great and detailed writeup on how he did it: https://github.com/davidar/mfc7400c

After snooping on my scanner with wireshark, I realized that the protocol is basically the same.
So I forked his project, and made the necessary changes to run with my printer.

## How it works
This is a small utility that will scan a single A4 image from the scanner.

Just compile with `make` and run the binary `./scan`

Then use `./raw2pnm image.raw` to create a viewable image.

## How to crosscompile for OpenWRT

### 1.
Get the correct OpenWRT SDK for your router. You can also download the full source and compile the SDK yourself, but you should bring some time. I only used the SDK, follow these instructions:
https://openwrt.org/docs/guide-developer/using_the_sdk

I had to disable package signing in the menuconfig.
In the step "Select Packages", you only need to install libusb-1.0.

### 2.
Get your environment ready for crosscompiling. This guide gives a good overview:
https://openwrt.org/docs/guide-developer/crosscompile

The "staging_dir" is inside your sdk folder.

### 3.
The Makefile of this project has a "crosscompile" target.
Edit the SDK_DIR variable in the script. You may also have to change other commands
if your router architecture is not "mips_24kc".

`make crosscompile` should output both binaries. Copy them to your router, install libusb (`opkg install libusb-1.0`), connect your scanner and run `./scan`.
Keep in mind that the flash memory of the router does not like writing data often, so it's best to do this on external storage.

I have also included precompiled files in the "bin-mips" folder, but they probably only work on "ar71xx" routers with OpenWrt/LEDE 17.

## Work in progress:
Since I want to use this on my router, I want to create a complete workflow
that outputs a jpeg file which I can then show in a webserver, based on this project:
https://github.com/zed-0xff/openwrt-scan-server

If I have too much time in the future, I might attempt to create a SANE backend out of this. That is probably too much work.

### Brother DCP-7030 Scanner

While setting up my usb scanner/printer combination on an OpenWRT/LEDE router (TP-Link TL-WDR4300), I could not get the scanner to work.
The proprietary SANE driver "brscan3" from Brother is not available for MIPS CPUs, and the source they supply seems incomplete.

After some searching, I found the project from davidar, who reverse engineered the usb protocol of his Brother MFC-7400C a few years ago.
Go see his repo for a great and detailed writeup on how he did it: https://github.com/davidar/mfc7400c

After snooping on my scanner with wireshark, I realized that the protocol is basically the same.
So I forked his project, and made the necessary changes to run with my printer.

## How it works
This is a small utility that will scan a single A4(?) image from the scanner.

Just compile with "make" and run the binary "./scan"

Then use "./raw2pnm image.raw" to create a viewable image.

## Work in progress:
Since I want to use this on my router, I want to create a complete workflow
that outputs a jpeg file which I can then show in a webserver, based on this project:
https://github.com/zed-0xff/openwrt-scan-server

If I have too much time in the future, I might attempt to create a SANE backend out of this. That is probably too much work.

SDK_DIR=/home/user/openwrt-sdk-ar71xx-generic_gcc-7.3.0_musl.Linux-x86_64

all:
	gcc -std=gnu99 ${CFLAGS} -I/usr/include/libusb-1.0 -o scan scan.c -lusb-1.0
	gcc -std=gnu99 ${CFLAGS} -o raw2pnm raw2pnm.c

debug:
	$(MAKE) CFLAGS=-DDEBUG

convert:
	./raw2pnm image.raw

clean:
	rm scan raw2pnm

crosscompile:
	mips-openwrt-linux-gcc -std=gnu99 ${CFLAGS} -I${SDK_DIR}/staging_dir/target-mips_24kc_musl/usr/include/libusb-1.0 -o scan scan.c -L${SDK_DIR}/staging_dir/target-mips_24kc_musl/usr/lib/ -lusb-1.0
	mips-openwrt-linux-gcc -std=gnu99 ${CFLAGS} -o raw2pnm raw2pnm.c

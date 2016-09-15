#Reference: http://www.devdrv.co.jp/linux/kernel26-makefile.htm
TARGET:= led.ko

all: ${TARGET}

led.ko: led.c
	make -C /usr/src/linux-headers-`uname -r` M=`pwd` V=1 modules

clean:
	make -C /usr/src/linux-headers-`uname -r` M=`pwd` V=1 clean

obj-m:= led.o

clean-files := *.o *.ko *.mod.[co] *~

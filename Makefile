#Reference: http://www.devdrv.co.jp/linux/kernel26-makefile.htm
TARGET:= myled.ko

all: ${TARGET}

myled.ko: myled.c
	make -C /usr/src/linux-headers-`uname -r` M=`pwd` V=1 modules

clean:
	make -C /usr/src/linux-headers-`uname -r` M=`pwd` V=1 clean

obj-m:= myled.o

clean-files := *.o *.ko *.mod.[co] *~

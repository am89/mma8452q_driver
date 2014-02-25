
TARGET ?= ~/Development/KernelPsyke83/kernel_huawei_u8160/

all:
	$(MAKE) ARCH=arm CROSS_COMPILE=~/Development/Toolchain/arm-eabi-4.6/bin/arm-eabi- -C $(TARGET) M=$(shell pwd) modules

obj-m = bm.o

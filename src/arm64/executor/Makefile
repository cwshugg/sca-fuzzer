NAME = arm64_executor

ccflags-y += -ggdb3 -std=gnu11 -Wno-declaration-after-statement -DL1D_ASSOCIATIVITY=$(shell cat /sys/devices/system/cpu/cpu0/cache/index0/ways_of_associativity) -DL1D_SIZE=$(shell L1D_SIZE=$(cat /sys/devices/system/cpu/cpu0/cache/index0/size); echo $((${L1D_SIZE/K/*1024})))

obj-m += $(NAME).o
$(NAME)-objs += main.o templates.o measurement.o


KDIR=/lib/modules/$(shell uname -r)/build

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

install:
	sudo insmod $(NAME).ko

uninstall:
	sudo rmmod $(NAME)
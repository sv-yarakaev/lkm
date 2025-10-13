obj-m += dbmain.o
dbmain-y :=  pr_blk.o bd_sysfs.o
KDIR ?= /home/stv/kernel/linux-6.1.130
PWD:= $(CURDIR)

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean


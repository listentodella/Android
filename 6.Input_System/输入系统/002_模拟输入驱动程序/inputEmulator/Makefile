KERN_DIR = ~/workdir/linux-3.0.86

all:
	make -C $(KERN_DIR) M=`pwd` modules

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order

obj-m += input_emulator.o

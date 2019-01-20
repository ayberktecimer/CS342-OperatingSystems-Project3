obj-m += module.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc app.c

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
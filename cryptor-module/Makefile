obj-m += cryptor.o #obj-m indica que o modulo deve ser construido no diretorio atual

all:
			make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
			make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

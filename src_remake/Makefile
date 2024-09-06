
all:
	make -C /lib/modules/$(uname -r)/build M=$(pwd) modules
	gcc user_program.c -o user_program

#sudo insmod simple_module.ko
#sudo mknod /dev/lime c <major_number> 0
#Substitua <major_number> pelo número mostrado ao carregar o módulo (verifique no log com dmesg).
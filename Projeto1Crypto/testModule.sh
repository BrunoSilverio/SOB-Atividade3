#!/bin/bash
clear
make

# check if user is root
if [ $(id -un) != "root" ]; then
   echo -e "Esse script deve executar com usuário root" 1>&2
   echo -e "This script must be run as root user\n" 1>&2
   echo -e "e.g: sudo bash test.sh\n\n"
   exit 1
fi;

#Remove Modulo
rmmod cryptomodule
read -p "Continuar ... "
echo -e "----------------------------------------------"

#install the module
echo -e "instala o modulo\n"
echo -e "# insmod cryptomodule.ko\n"
insmod cryptomodule.ko key="1"
read -p "Continuar ... "
echo -e "----------------------------------------------"

# message on dmesg
echo -e "mensagem no dmesg (display messages)\n"
echo -e "$ dmesg | tail\n"
dmesg | tail
echo -e "\n"
read -p "Continuar ... "
echo -e "----------------------------------------------"

# check if there is the node on /dev 
echo -e "Valida se o nó de dispositivo em /dev/teste existe"
if [[ $(find /dev -name teste) != "" ]]; then
	echo -e "/dev/teste foi criado pelo udev\n"
fi;
echo -e "$ ls -l /dev/teste --color=auto\n"
ls -l /dev/cryptomodule --color=auto
echo -e "\n"
read -p "Continuar ... "
echo -e "----------------------------------------------"

# write once and read again from /dev/fisl15rd 
echo -e "escreve uma vez e lê de novo 5x a partir de /dev/teste\n"
echo -e "# echo \"0123456789ABCDEF\" > /dev/cryptomodule\n"
echo -e "c 0123456789ABCDEF" > /dev/cryptomodule

echo "# for i in {1..5}; do cat /dev/teste; echo -e \"\\n\"; done;"
echo -e "\n"
for i in {1..5}; do
	cat /dev/cryptomodule
	echo -e "\n"
done;
echo -e "\n"
read -p "Continuar ... "
echo -e "----------------------------------------------"

# message on dmesg
echo -e "mensagem no dmesg (display messages)\n"
echo -e "$ dmesg | tail -n 50\n"
dmesg | tail -n 50
echo -e "\n"
read -p "Continuar ... "
echo -e "----------------------------------------------"

#remove the module 
echo -e "remove o module\n"
echo -e "# rmmod cryptomodule.ko\n"
rmmod cryptomodule.ko
read -p "Continuar ... "
echo -e "----------------------------------------------"


# message on dmesg
echo -e "mensagem no dmesg (display messages)\n"
echo -e "dmesg | tail\n"
dmesg | tail
echo -e "----------------------------------------------"
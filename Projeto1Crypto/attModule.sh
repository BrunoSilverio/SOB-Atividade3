#!/bin/bash
clear
echo -e "Makefile"
make

echo -e "Removendo modulo"
rmmod cryptomodule

echo -e "Inserindo modulo"
insmod cryptomodule.ko key='0123456789ABCDEF'
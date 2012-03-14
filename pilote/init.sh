chmod a+x conf.sh
source conf.sh
make
msp430-jtag -e test.elf

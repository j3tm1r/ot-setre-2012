chmod a+x conf.sh
source conf.sh
msp430-jtag -e ./exe/main.elf
msp430-gdbproxy --port=2159 msp430 

#ifndef _DRV_EEPROM_H_
#define _DRV_EEPROM_H_


//extern
void eeprom_byte_write(unsigned int address, unsigned char data);
//void eeprom_page_write(unsigned int	start_addr, unsigned char *data, unsigned char size); // TO DO
unsigned char eeprom_random_read(unsigned int address);
unsigned char eeprom_current_read(void);
//void eeprom_sequential_read(unsigned int start_addr, unsigned char *data, unsigned char size); // TO DO
int eeprom_ack_polling(void);

//static
static void delayx100us_iic(unsigned int b);
static void delay_iic(unsigned int a);
static void init(void);
static void start(void);
static void stop(void);
static void acknowledge(void);
static int receive_ack(void);
static void write_byte(unsigned char write_data);
static unsigned char read_byte(void);

#endif

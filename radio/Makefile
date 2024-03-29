# this makefile only work if examples are placed in directory like Ex3 in this example : ...SOFTWARE\EvalBoards\EasyWeb3\GNU\Ex3
# makefile configuration
NAME            = main

OBJDIR          = ./obj
OUTDIR          = ./exe

RTOSDIR         = ./os/ucosII
PORTDIR         = ./os/port/MSP430/F1611/GNU

SRCDIR		= ./radio/src
SERVDIR		= ./radio/src/ServicesES
#LIBDIRS 	=
RADIODIR	= ./radio/include


INCDIR		= -I./ -I$(RTOSDIR) -I$(PORTDIR) -I$(RADIODIR) -I$(SERVDIR)

OBJECTS         = $(OBJDIR)/app.o				\
                  $(OBJDIR)/os_cpu_a.o			\
                  $(OBJDIR)/os_cpu_c.o			\
                  $(OBJDIR)/os_core.o			\
                  $(OBJDIR)/os_flag.o			\
                  $(OBJDIR)/os_mbox.o			\
                  $(OBJDIR)/os_mem.o			\
                  $(OBJDIR)/os_mutex.o			\
                  $(OBJDIR)/os_q.o				\
                  $(OBJDIR)/os_sem.o			\
                  $(OBJDIR)/os_task.o			\
                  $(OBJDIR)/os_time.o			\
		    	  $(OBJDIR)/Display.o			\
		  		  $(OBJDIR)/ServiceOutput.o 	\
		  		  $(OBJDIR)/GestionMode.o		\
		  		  $(OBJDIR)/StatLogger.o 		\
		  		  $(OBJDIR)/TraitementInput.o	\
		  		  $(OBJDIR)/cmdBuffer.o			\
		  		  $(OBJDIR)/drv_eeprom.o
		  		  
                  
CPU             = msp430x149
CFLAGS          = -mmcu=${CPU} $(INCDIR) -Wall -c -g -v

#switch the compiler (for the internal make rules)
CC              = msp430-gcc

.PHONY: all FORCE clean download download-jtag download-bsl dist

#all should be the first target. it's built when make is run without args
all: $(OUTDIR)/${NAME}.elf $(OUTDIR)/${NAME}.a43 $(OUTDIR)/${NAME}.lst

#additional rules for files

#**************************************************
# ONLY SELECT ONE OF THE TWO FOLLOWING TARGETS
#  - The first one locates everything in RAM
#  - The second puts the code in Flash
#$(OUTDIR)/${NAME}.elf: ${OBJECTS}
#	${CC} -mmcu=${CPU} -Wl,-Tram.ld -o $@ ${OBJECTS}

$(OUTDIR)/${NAME}.elf: ${OBJECTS}
	${CC} -mmcu=${CPU} -v -o $@ ${OBJECTS}

# End Memory selection
#**************************************************
$(OUTDIR)/${NAME}.a43: $(OUTDIR)/${NAME}.elf
	msp430-objcopy -O ihex $^ $@

$(OUTDIR)/${NAME}.lst: $(OUTDIR)/${NAME}.elf
	msp430-objdump -dSt $^ >$@

download-jtag: all
	msp430-jtag -e $(OUTDIR)/${NAME}.elf

download-bsl: all
	msp430-bsl -e $(OUTDIR)/${NAME}.elf

clean:
	rm -rf $(OUTDIR)/${NAME}.elf $(OUTDIR)/${NAME}.a43 $(OUTDIR)/${NAME}.lst ${OBJECTS}

miniclean:
	rm -rf $(OUTDIR)/${NAME}.elf $(OUTDIR)/${NAME}.a43 $(OUTDIR)/${NAME}.lst $(OBJDIR)/app.o 

#backup archive
megadist:
	tar czf dist.tgz *
minidist:
	tar czf dist.tgz *.c *.h *.txt Makefile makefile

#dummy target as dependecy if something has to be build everytime
FORCE:

#project dependencies

$(OBJDIR)/app.o : $(SRCDIR)/main.c
	$(CC) $(CFLAGS) -o $@ $<
	
$(OBJDIR)/Display.o : $(SERVDIR)/Display.c
	$(CC) $(CFLAGS) -o $@ $<
	
$(OBJDIR)/ServiceOutput.o : $(SRCDIR)/ServiceOutput.c
	$(CC) $(CFLAGS) -o $@ $<
	
$(OBJDIR)/GestionMode.o : $(SRCDIR)/GestionMode.c
	$(CC) $(CFLAGS) -o $@ $<
	
$(OBJDIR)/StatLogger.o : $(SRCDIR)/StatLogger.c
	$(CC) $(CFLAGS) -o $@ $<
	
$(OBJDIR)/TraitementInput.o : $(SRCDIR)/TraitementInput.c
	$(CC) $(CFLAGS) -o $@ $<
	
$(OBJDIR)/cmdBuffer.o : $(SRCDIR)/util/cmdBuffer.c
	$(CC) $(CFLAGS) -o $@ $<
	
$(OBJDIR)/drv_eeprom.o : $(SERVDIR)/drv_eeprom.c
	$(CC) $(CFLAGS) -o $@ $<
		
#################################	
	
$(OBJDIR)/os_cpu_c.o : $(PORTDIR)/os_cpu_c.c
	$(CC) $(CFLAGS) -o $@ $<
	
$(OBJDIR)/os_core.o : $(RTOSDIR)/os_core.c
	$(CC) $(CFLAGS) -I$(RTOSDIR) -o $@ $<
	
$(OBJDIR)/os_flag.o : $(RTOSDIR)/os_flag.c
	$(CC) $(CFLAGS) -o $@ $<
	
$(OBJDIR)/os_mbox.o : $(RTOSDIR)/os_mbox.c
	$(CC) $(CFLAGS) -o $@ $<
	
$(OBJDIR)/os_mem.o : $(RTOSDIR)/os_mem.c
	$(CC) $(CFLAGS) -o $@ $<
	
$(OBJDIR)/os_mutex.o : $(RTOSDIR)/os_mutex.c
	$(CC) $(CFLAGS) -o $@ $<
	
$(OBJDIR)/os_q.o : $(RTOSDIR)/os_q.c
	$(CC) $(CFLAGS) -o $@ $<
	
$(OBJDIR)/os_sem.o : $(RTOSDIR)/os_sem.c
	$(CC) $(CFLAGS) -o $@ $<
	
$(OBJDIR)/os_task.o : $(RTOSDIR)/os_task.c
	$(CC) $(CFLAGS) -o $@ $<
	
$(OBJDIR)/os_time.o : $(RTOSDIR)/os_time.c
	$(CC) $(CFLAGS) -o $@ $<
	
$(OBJDIR)/os_cpu_a.o : $(PORTDIR)/os_cpu_a.S
	$(CC) -g -mmcu=$(CPU) -D_GNU_ASSEMBLER_ -gstabs -c -o $@ $<
	

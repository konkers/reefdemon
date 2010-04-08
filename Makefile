FOSC=16000000


CROSS_CPU=atmega328
FOSCCFLAG=-DFOSC=${FOSC}

OBJS=main.o twi_master.o ow.o lcd.o font_pc8x8.o ds18b20.o
TARGETS=reefdevil.bin
all: ${TARGETS}

reefdevil.elf: ${OBJS}
	${LINK}

CC=avr-gcc
OBJCOPY=avr-objcopy
OBJDUMP=avr-objdump
SIZE=avr-size
AR=avr-ar
RANLIB=avr-ranlib

OPT=-O2 -g

CFLAGS=-mmcu=${CROSS_CPU} -DFOSC=${FOSC} \
	${OPT} -Wall -Werror \
	-I${FLG_DIR}/lib/c/include \
	-I${FLG_DIR}/avr/lib/include

LDFLAGS=-mmcu=${CROSS_CPU}


LINK =   @echo "  LINK   " $@; ${CC} ${LDFLAGS} -o $@ ${filter-out %.a, $^} ${LIBS}
%.o: %.c
	@echo "  CC     " `basename $<`
	@${CC} -c ${CFLAGS} -o $@ $<


%.bin: %.elf
	@echo "  OBJCPY " $@; ${OBJCOPY} -O binary $^ $@

%.lst: %.elf
	${OBJDUMP} --disassemble-all --source $^ > $@

ifeq ("${CROSS_CPU}","atmega48")
AVRDUDE_CPU=m48
else ifeq ("${CROSS_CPU}","atmega88")
AVRDUDE_CPU=m88
FUSES=-U lfuse:w:0xe7:m -U hfuse:w:0xdf:m -U efuse:w:0x01:m
else ifeq ("${CROSS_CPU}","atmega168")
AVRDUDE_CPU=m168
else ifeq ("${CROSS_CPU}","atmega328")
AVRDUDE_CPU=m328p
FUSES=-U lfuse:w:0xe7:m -U hfuse:w:0xdf:m -U efuse:w:0x01:m
else ifeq ("${CROSS_CPU}","atmega8")
AVRDUDE_CPU=m8
FUSES=
endif

flash: flash-reefdevil

flash-%: %.bin
	avrdude -p ${AVRDUDE_CPU} -c usbtiny -B 1 -e -U flash:w:$< ${FUSES}

clean:
	@rm -f ${OBJS} ${TARGETS} ${TARGETS:.bin=.elf}


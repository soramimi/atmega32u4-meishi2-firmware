
TARGET=atmega32u4-meishi2-firmware
MCU=atmega32u4
CFLAGS=-Os -mmcu=$(MCU) -DF_CPU=16000000UL
CXXFLAGS=$(CFLAGS)
LDFLAGS=-mmcu=$(MCU)

all: $(TARGET).hex
	avr-size -C --mcu $(MCU) $(TARGET).elf

%.o: %.c
	avr-gcc -c $(CFLAGS) $^ -o $@

%.o: %.cpp
	avr-g++ -c $(CXXFLAGS) $^ -o $@

$(TARGET).elf: main.o usb.o
	avr-g++ $(LDFLAGS) $^ -o $(TARGET).elf

$(TARGET).hex: $(TARGET).elf
	avr-objcopy -O ihex $^ $@

clean:
	rm -f *.o
	rm -f *.elf
	rm -f *.hex

write: $(TARGET).hex
	avrdude -c avrisp -P /dev/ttyACM0 -b 19200 -p m32u4 -U hfuse:w:0xd9:m  -U lfuse:w:0x5e:m -U flash:w:$(TARGET).hex

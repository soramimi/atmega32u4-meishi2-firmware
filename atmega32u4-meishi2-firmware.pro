
INCLUDEPATH += /usr/avr/include
INCLUDEPATH += /usr/lib/avr/include

DEFINES += __AVR_ATmega32U4__

HEADERS += \
    usb.h
SOURCES += \
    usb.c \
	main.cpp


#include <string.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
extern "C" {
#include "usb.h"
}

void clear_key(uint8_t key)
{
	if (key > 0) {
		if (key >= 0xe0 && key < 0xe8) {
			keyboard_data[0] &= ~(1 << (key - 0xe0));
		}
		uint8_t i = 8;
		while (i > 2) {
			i--;
			if (keyboard_data[i] == key) {
				if (i < 7) {
					memmove(keyboard_data + i, keyboard_data + i + 1, 7 - i);
				}
				keyboard_data[7] = 0;
			}
		}
	}
}

void release_all_keys()
{
	memset(keyboard_data + 2, 0, 6);
}

void release_key(uint8_t key)
{
	clear_key(key);
	keyboard_data[1] = 0;
}

void press_key(uint8_t key)
{
	if (key >= 0xe0 && key < 0xe8) {
		keyboard_data[0] |= 1 << (key - 0xe0);
	} else if (key > 0) {
		clear_key(key);
		memmove(keyboard_data + 3, keyboard_data + 2, 5);
		keyboard_data[2] = key;
	}
	keyboard_data[1] = 0;
}


void select_row(uint8_t row)
{
	uint8_t d = PORTD;
	uint8_t e = PORTE;
	d |= 0x80;
	e |= 0x40;
	switch (row) {
	case 0:
		d &= ~0x80; // R0 (PD7)
		break;
	case 1:
		e &= ~0x40; // R1 (PE6)
		break;
	}
	PORTD = d;
	PORTE = e;
}

uint8_t read_bits()
{
	uint8_t v = 0;
	uint8_t f = PINF;
	if (f & 0x20) { // C0 (PF5)
		v |= 0x01;
	}
	if (f & 0x40) { // C1 (PF6)
		v |= 0x02;
	}
	return v;
}

uint8_t scan_keys()
{
	uint8_t bits = 0;
	for (uint8_t row = 0; row < 2; row++) {
		select_row(row);
		bits = (bits << 2) | read_bits();
	}
	return bits;
}

static bool key_changed = false;

void led(bool f)
{
	if (f) {
		PORTB &= ~0x01;
	} else {
		PORTB |= 0x01;
	}
}

// USB HID keyboard scan codes
#define KEY_A 0x04 // A
#define KEY_B 0x05 // B
#define KEY_C 0x06 // C
#define KEY_D 0x07 // D
#define KEY_E 0x08 // E
#define KEY_F 0x09 // F
#define KEY_G 0x0a // G
#define KEY_H 0x0b // H
#define KEY_I 0x0c // I
#define KEY_J 0x0d // J
#define KEY_K 0x0e // K
#define KEY_L 0x0f // L
#define KEY_M 0x10 // M
#define KEY_N 0x11 // N
#define KEY_O 0x12 // O
#define KEY_P 0x13 // P
#define KEY_Q 0x14 // Q
#define KEY_R 0x15 // R
#define KEY_S 0x16 // S
#define KEY_T 0x17 // T
#define KEY_U 0x18 // U
#define KEY_V 0x19 // V
#define KEY_W 0x1a // W
#define KEY_X 0x1b // X
#define KEY_Y 0x1c // Y
#define KEY_Z 0x1d // Z
#define KEY_1 0x1e // 1
#define KEY_2 0x1f // 2
#define KEY_3 0x20 // 3
#define KEY_4 0x21 // 4
#define KEY_5 0x22 // 5
#define KEY_6 0x23 // 6
#define KEY_7 0x24 // 7
#define KEY_8 0x25 // 8
#define KEY_9 0x26 // 9
#define KEY_0 0x27 // 0

int main()
{
	// 16 MHz clock
	CLKPR = 0x80;
	CLKPR = 0;
	// Disable JTAG
	MCUCR |= 0x80;
	MCUCR |= 0x80;

	DDRB = 0x01; // LED output

	// COL pins: input with pull-up
	// ROW pins: output
	DDRD = 0x80;
	DDRE = 0x40;
	DDRF = 0x00;
	PORTD = 0x80;
	PORTE = 0x40;
	PORTF = 0x60;

	TCCR0B = 0x02; // 1/8 prescaling
	TIMSK0 |= 1 << TOIE0;

	usb_init();
	while (!usb_configured()) {
		_delay_ms(10);
	}

	sei();  // enable interrupts

	while (1) {
		static uint8_t bits_last = 0;
		uint8_t bits_curr = scan_keys();
		if (bits_curr != bits_last) {
			auto PressKey = [&](uint8_t code, bool bit){
				if ((bits_curr ^ bits_last) & bit) {
					if (bits_curr & bit) {
						press_key(code);
					} else {
						release_key(code);
					}
					key_changed = true;
				}
			};
			PressKey(KEY_Q, 0x08);
			PressKey(KEY_W, 0x04);
			PressKey(KEY_E, 0x02);
			PressKey(KEY_R, 0x01);
			if (key_changed) {
				key_changed = false;
				usb_keyboard_send();
			}
			bits_last = bits_curr;
		}
		_delay_ms(10);
	}
}


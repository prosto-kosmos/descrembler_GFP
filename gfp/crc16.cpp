//class CRC16

#include <vcl.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <stdint.h>

using namespace std;

class CRC16 {
public:
	CRC16() {
		crc = 0;
	};

	uint16_t processByte(uint8_t data) {
		uint8_t i;
		crc = crc ^ ((uint16_t)data << 8);
		for (i = 0; i < 8; i++) {
			if (crc & 0x8000)
				crc = (crc << 1) ^ 0x1021;
			else
				crc <<= 1;
		}
		return crc;
	};

	uint16_t getCrc() {
		return crc;
	};

private:
	uint16_t crc;
};
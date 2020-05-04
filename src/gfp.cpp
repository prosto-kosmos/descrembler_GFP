// class GFP
#include <vcl.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <stdint.h>

using namespace std;

class GFP {
	char out_byte;
	char in_byte;
	long long registr;
	long long buf;
	unsigned int buf_find_header;
	char mask[8];
	char mask_L[8];
	int flag;
	unsigned char ost;
	int delta;
	int count_h;
	int lenght;
	unsigned short crc;
	unsigned int header;
	unsigned short buf_lenght;
	unsigned short buf_crc;
	int count_packege;

public:
	GFP() {
		flag = 0;
		registr = 0;
		ost = 0;
		delta = 0;
		buf_find_header = 0;
		count_h = 0;
		lenght = 0;
		crc = 0;
		header = 0;
		buf_lenght = 0;
		buf_crc = 0;
		count_packege = 0;

		mask[0] = 0x01;
		mask[1] = 0x02;
		mask[2] = 0x04;
		mask[3] = 0x08;
		mask[4] = 0x10;
		mask[5] = 0x20;
		mask[6] = 0x40;
		mask[7] = 0x80;

		mask_L[0] = 0x00;
		mask_L[7] = 0x80;
		mask_L[6] = 0xC0;
		mask_L[5] = 0xE0;
		mask_L[4] = 0xF0;
		mask_L[3] = 0xF8;
		mask_L[2] = 0xFC;
		mask_L[1] = 0xFE;
	}

	inline char reverseByte(char byte) {
		char out_byte(0);
		for (int i = 0; i < 8; i++) {
			out_byte = out_byte << 1;
			out_byte = out_byte | (byte & 0x01);
			byte = byte >> 1;
		}
		return out_byte;
	}

	inline unsigned short Crc16(unsigned char *pcBlock, unsigned short len) {
		unsigned short crc_16 = 0x0000;
		unsigned char i;

		while (len--) {
			crc_16 ^= *pcBlock++ << 8;

			for (i = 0; i < 8; i++)
				crc_16 = crc_16 & 0x8000 ? (crc_16 << 1) ^ 0x1021 : crc_16 << 1;
		}
		return crc_16;
	}

	inline void descrembler_and_out_byte(char byte, FILE* of) {
		if (count_h < 4) {
			header = header << 8;
			byte = reverseByte(byte);
			header = header | (int(byte) & 0x000000ff);
			if (count_h == 3) {
				lenght = ((header ^ 0xB6AB31E0) & 0xFFFF0000) >> 16;
				crc = (header ^ 0xB6AB31E0) & 0xFFFF;
				unsigned char pc[2] = {(lenght >> 8) & 0xff, lenght & 0xff};

				// проверка CRC заголовка пакета
				if (crc != Crc16(pc, 2)) {
					if (crc == 0) {
						lenght = 0;
					}
				}

				if (lenght == 0) {
					header = 0;
					count_h = 0;
					return;
				}
			}
			count_h += 1;
		}
		else if (lenght != 0) {
			if (count_h == 4) {
				count_packege+=1;
				if (count_packege>1) {
					lenght = lenght - 4;
					fwrite((char*) & lenght, 4, 1, of);
					lenght = lenght + 4;
				}
			}
			out_byte = 0;
			// дескремблируем
			for (int j = 0; j < 8; j++) {
				registr = registr >> 1;
				out_byte = out_byte << 1;
				buf = (byte >> j) & 0x01; // снимаем верхний бит
				registr = registr | (buf << 43);
				buf = (registr & 0x01) ^ buf;
				out_byte = out_byte | (buf & 0x01);
			}
			if (count_h > 7) {
				if (count_packege>1) {
                	fwrite(&out_byte, sizeof(char), 1, of);
				}
			}
			lenght -= 1;
			count_h += 1;
			if (lenght == 0) {
				count_h = 0;
			}
		}
		return;
	}

	inline unsigned int FindNextHeder(unsigned int hdr, FILE* f, int d) {
		unsigned short len = ((0xffff0000 & hdr) >> 16) ^ 0xB6AB;
		fseek(f, len - 1, SEEK_CUR);

		unsigned char nextHeader;
		unsigned long long outHeaderLong = 0;
		// 1
		outHeaderLong <<= 8;
		fread(&nextHeader, sizeof(unsigned char), 1, f);
		outHeaderLong |= reverseByte(nextHeader) & 0x00000000000000ff;
		// 2
		outHeaderLong <<= 8;
		fread(&nextHeader, sizeof(unsigned char), 1, f);
		outHeaderLong |= reverseByte(nextHeader) & 0x00000000000000ff;
		// 3
		outHeaderLong <<= 8;
		fread(&nextHeader, sizeof(unsigned char), 1, f);
		outHeaderLong |= reverseByte(nextHeader) & 0x00000000000000ff;
		// 4
		outHeaderLong <<= 8;
		fread(&nextHeader, sizeof(unsigned char), 1, f);
		outHeaderLong |= reverseByte(nextHeader) & 0x00000000000000ff;
		// 5
		outHeaderLong <<= 8;
		fread(&nextHeader, sizeof(unsigned char), 1, f);
		outHeaderLong |= reverseByte(nextHeader) & 0x00000000000000ff;

		unsigned int outHeader = (outHeaderLong >> d) & 0xffffffff;

		return outHeader;
	}

	inline int TrueHeader(unsigned int hdr) {
		unsigned short l = (0xffff0000 & hdr) >> 16;
		unsigned short c = 0xffff & hdr;
		unsigned char pcc[2] = {
			((l >> 8) & 0xff) ^ 0xB6, (l & 0xff) ^ 0xAB};
		l = Crc16(pcc, 2);
		c = c ^ 0x31E0;
		if (l == c){
            return 1;
		}
		else{
            return 0;
		}
	}

	inline void AddData(char in_byte, FILE* f, FILE* of) {
		if (!flag) {
			for (int i = 0; i < 8; i++) {
				buf_find_header = buf_find_header << 1;
				if (in_byte & mask[i]) {
					buf_find_header = buf_find_header | 0x01;
				}

				if (TrueHeader(buf_find_header)) { // проверка 1
					int flag_cur = ftell(f);
					buf_find_header = FindNextHeder(buf_find_header, f, 7 - i);
					if (TrueHeader(buf_find_header)) { // проверка 2
						buf_find_header = FindNextHeder(buf_find_header, f, 7 - i);
						if (TrueHeader(buf_find_header)) { // проверка 3
							buf_find_header = FindNextHeder(buf_find_header, f, 7 - i);
							if (TrueHeader(buf_find_header)) { // проверка 4
								fseek(f, flag_cur - ftell(f), SEEK_CUR);
								delta = (i + 1) % 8;
								fseek(f, -5, SEEK_CUR);
								fread(&in_byte, sizeof(char), 1, f);
								ost = 0xff & (in_byte & mask_L[delta]);
								flag = 1;
								return;
							}
						}
					}
				}
			}
		}
		else {
			unsigned char byte(0);
			byte = in_byte;
			if (delta != 0) {
				byte = byte << (8 - delta);
			}
			byte = byte & 0xff;
			ost = ost >> delta;
			byte = byte | ost;
			ost = in_byte & mask_L[delta]; // >> (8 - delta);
			descrembler_and_out_byte(byte, of);
		}
	}

	void runGFP(FILE* f, FILE* of) {
		fseek(f, 0, SEEK_END);

		int sz = ftell(f);
		cout << "Size file: " << sz << endl;
		fseek(f, 0, SEEK_SET);

		fwrite("IP_STREAM", sizeof("IP_STREAM") - 1, 1, of);

		char in_byte(0);
		int countAllByte = ftell(f);

		while (countAllByte < sz) {
			fread(&in_byte, sizeof(char), 1, f);
			AddData(in_byte, f, of);
			countAllByte += 1;
			if (countAllByte % 100000 == 0) {
				cout << "Complite: " << double(countAllByte) / double(sz)
					* 100 << "%  ";
				cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b";
			}
		}
		cout << "Complite: 100%            " << endl;
	}
};

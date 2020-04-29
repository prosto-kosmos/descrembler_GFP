// ---------------------------------------------------------------------------

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

class GFP {
	char out_byte;
	char in_byte;
	long long registr;
	long long buf;
	int buf_find_header;
	char mask[8];
	char mask_L[8];
	int flag;
	unsigned char ost;
	int delta;

public:
	GFP() {
		flag = 0;
		registr = 0;
		ost = 0;
		delta = 0;
		buf_find_header = 0;
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

	inline void descrembler_and_out_byte(char byte, FILE* f, FILE* of) {
		out_byte = 0;
		// ��������������
		for (int j = 0; j < 8; j++) {
			registr = registr >> 1;
			out_byte = out_byte << 1;
			buf = (in_byte >> j) & 0x01; // ������� ������� ���
			registr = registr | (buf << 43);
			buf = (registr & 0x01) ^ buf;
			out_byte = out_byte | (buf & 0x01);
		}
	}

	void AddData(char in_byte, FILE* f, FILE* of) {
		if (!flag) {
			for (int i = 0; i < 8; i++) {
				buf_find_header = buf_find_header << 1;
				char v1 = in_byte & mask[i];
				if (v1) {
                	buf_find_header = buf_find_header | 0x01;
				}
				if ((0xffffffff & buf_find_header) == 0xB6AB31E0) {
					delta = (i+1) % 8;
					ost = 0xff & (in_byte & mask_L[delta]);
					flag = 1;
					return;
				}
			}
		}
		else {
			char byte(0);
			byte = in_byte;
			if (delta!=0) {
            	byte = byte << (8-delta);
			}
			byte = byte & 0xff;
			ost = ost >> delta;
			byte = byte | ost;
			ost = in_byte & mask_L[delta];//>> (8 - delta);
			descrembler_and_out_byte(byte, f, of);
		}
	}

	/* void find_first_null_header(FILE* f, int sz) {
	int b1(0);
	int b2(0);
	int b3(0);
	int b4(0);
	int b5(0);
	int byte(0);
	int flag(0);
	for (int i = 0; i < sz; i++) {
	fread(&byte, sizeof(char), 1, f);
	for (int j = 0; j < 8; j++) {
	b5 = b5 << 1;
	b5 = b5 | (byte & 0x01);
	byte = byte >> 1;
	}
	for (int k = 0; k < 8; k++) {
	flag = b1 << 25 + k | b2 << 17 + k | b3 << 9 + k | b4 << 1 +
	k | b5 >> 7 - k;
	if (flag == 0xB6AB31E0) {
	delta = 7 - k;
	if (delta == 0) {
	fread(&ost, sizeof(char), 1, f);
	ost = reverseByte(ost);
	ost = ost & 0xff;
	}
	else {
	ost = b5 >> delta;
	}
	return;
	}
	}
	b1 = b2;
	b2 = b3;
	b3 = b4;
	b4 = b5;
	b5 = 0;
	}
	cout << "Not find null header" << endl;
	cout << endl << "\nPress any key to continue...";
	getch();
	exit(1);
	}

	int find_data_header(FILE* f, int sz) {
	if (ftell(f) == sz) {
	// ����� �����
	return -1;
	}
	int data(0);
	int rev_data(0);
	fread(&data, sizeof(int), 1, f);
	for (int i = 0; i < 32; i++) {
	rev_data = rev_data << 1;
	rev_data = rev_data | (data & 0x01);
	data = data >> 1;
	}
	int out_data;
	if (delta == 0) {
	out_data = ((0xffffff & (rev_data >> 8)) | (ost << 24))
	& 0xffffffff;
	ost = rev_data & 0xff;
	}
	else {
	out_data = ((rev_data >> delta) | (ost << (24 + delta)))
	& 0xffffffff;
	ost = ((rev_data << (8 - delta)) & 0xff) >> (8 - delta);
	}

	return out_data;
	}

	char GetData(FILE* f) {
	fread(&in_byte, sizeof(char), 1, f);
	out_byte = 0;
	// ��������������
	for (int j = 0; j < 8; j++) {
	registr = registr >> 1;
	out_byte = out_byte << 1;
	buf = (in_byte >> (j)) & 0x01;
	registr = registr | (buf << 43);
	buf = (registr & 0x01) ^ buf;
	out_byte = out_byte | (buf & 0x01);
	}
	return out_byte;
	} */

};

int main(int argc, char* argv[]) {
	const char* out_filename;
	const char* in_filename;

	in_filename = "gfp5.bin";
	out_filename = "out.ips";

	/*
	if (argc != 3) {
	printf("Please specify the parameters: descrembler_GFP <path_input_file> <path_output_file>\n"
	);
	printf("Press any key to continue...\n");
	getch();
	return 0;
	}

	in_filename = argv[1];
	out_filename = argv[2]; */

	FILE *f, *of;
	f = fopen(in_filename, "rb");
	of = fopen(out_filename, "wb");

	if (!f) {
		cout << "No find file: " << in_filename << endl;
		cout << endl << "\nPress any key to continue...";
		getch();
		return 0;
	}
	cout << "Open file: " << in_filename << endl;

	fseek(f, 0, SEEK_END);

	int sz = ftell(f);
	cout << "Size file: " << sz << endl;
	fseek(f, 0, SEEK_SET);

	fwrite("IP_STREAM", sizeof("IP_STREAM") - 1, 1, of);

	GFP gfp;
	char in_byte(0);
	int countAllByte = ftell(f);

	while (countAllByte < sz) {
		fread(&in_byte, sizeof(char), 1, f);
		gfp.AddData(in_byte, f, of);
		countAllByte += 1;
		if (countAllByte % 100000 == 0) {
			cout << "Complite: " << double(countAllByte) / double(sz)
				* 100 << "%";
			cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b";
		}
	}
	cout << "Complite: 100%            " << endl;
	fclose(f);
	fclose(of);
	cout << endl << "\nPress any key to continue...";
	getch();
	return 0;

	/* while (1) {
	c += 1;
	if (c % 100000 == 0) {

	}

	header = frame.find_data_header(f, sz);

	if (header == -1) {
	cout << "Complite: 100%            " << endl;
	fclose(f);
	fclose(of);
	cout << endl << "\nPress any key to continue...";
	getch();
	return 0;
	}
	lenght = ((header ^ 0xB6AB31E0) & 0xFFFF0000) >> 16;
	crc = (header ^ 0xB6AB31E0) & 0xFFFF;

	// ��� �������� crc
	CRC16 crc_obj;
	crc_obj.processByte(lenght);
	if (crc_obj.getCrc()!=crc) {
	continue;
	}

	for (int i = 0; i < lenght; i++) {
	if (i == 0) {
	ips_lenght = lenght - 4;
	fwrite((char*) & ips_lenght, 4, 1, of);
	}

	out_byte = frame.GetData(f);

	if (i > 3) {
	fwrite(&out_byte, sizeof(char), 1, of);
	}

	}
	} */
}
// ---------------------------------------------------------------------------

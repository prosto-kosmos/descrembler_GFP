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

void find_first_null_header(FILE* f, int sz) {
	int b1(0);
	int b2(0);
	int b3(0);
	int b4(0);
	int byte(0);
	int flag(0);
	for (int i = 0; i < sz; i++) {
		fread(&byte, sizeof(char), 1, f);
		for (int j = 0; j < 8; j++) {
			b4 = b4 << 1;
			b4 = b4 | (byte & 0x01);
			byte = byte >> 1;
		}
		flag = b1 << 24 | b2 << 16 | b3 << 8 | b4;
		if (flag == 0xB6AB31E0) {
			return;
		}
		b1 = b2;
		b2 = b3;
		b3 = b4;
		b4 = 0;
	}
	cout << "Not find null header" << endl;
	cout << endl << "\nPress any key to continue...";
	getch();
	exit(1);
}

int find_data_header(FILE* f, int sz) {
	if (ftell(f) == sz) {
		// конец файла
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
	return rev_data;
}

char reverseByte(char byte) {
	char out_byte(0);
	for (int i = 0; i < 8; i++) {
		out_byte = out_byte << 1;
		out_byte = out_byte | (byte & 0x01);
		byte = byte >> 1;
	}
	return out_byte;
}

int main(int argc, char* argv[]) {
	const char* out_filename;
	const char* in_filename;

//	 in_filename = "gfp.bin";
//	 out_filename = "out.ips";

	if (argc != 3) {
		printf("Please specify the parameters: descrembler_GFP <path_input_file> <path_output_file>\n"
			);
		printf("Press any key to continue...\n");
		getch();
		return 0;
	}

	in_filename = argv[1];
	out_filename = argv[2];

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

	int header(0);
	int lenght(0);
	int ips_lenght(0);
	int crc(0);
	char out_byte(0);
	char in_byte(0);
	int c(0);
	long long registr(0);
	long long buf;

	find_first_null_header(f, sz);
	fwrite("IP_STREAM", sizeof("IP_STREAM") - 1, 1, of);

	while (1) {
		c += 1;
		if (c % 100000 == 0) {
			cout << "Complite: " << double(ftell(f)) / double(sz)*100 << "%";
			cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b";
		}

		header = find_data_header(f, sz);
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

		// тут проверка crc
		/* CRC16 crc_obj;
		 crc_obj.processByte(lenght);
		 if (crc_obj.getCrc()!=crc) {
		 continue;
		 } */

		for (int i = 0; i < lenght; i++) {
			if (i == 0) {
				ips_lenght = lenght - 4;
				fwrite((char*)&ips_lenght, 4, 1, of);
			}
			fread(&in_byte, sizeof(char), 1, f);
			out_byte = 0;

			// дескремблируем
			for (int j = 0; j < 8; j++) {
				registr = registr >> 1;
				out_byte = out_byte << 1;
				buf = (in_byte >> (j)) & 0x01;
				registr = registr | (buf << 43);
				buf = (registr & 0x01) ^ buf;
				out_byte = out_byte | (buf & 0x01);
			}

			if (i > 3) {
				fwrite(&out_byte, sizeof(char), 1, of);
			}

		}
	}
}
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <stdint.h>
#include "gfp.cpp"

using namespace std;

int main(int argc, char* argv[]) {
	const char* out_filename;
	const char* in_filename;

//	 in_filename = "gfp.bin";
//	 out_filename = "out.ips";

	if (argc != 3) {
		printf(
			"Please specify the parameters: descrembler_GFP <path_input_file> <path_output_file>\n");
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

	GFP gfp;
	gfp.runGFP(f, of);

	fclose(f);
	fclose(of);
	cout << endl << "\nPress any key to continue...";
	getch();
	return 0;
}
// ---------------------------------------------------------------------------


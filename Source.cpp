// Hayes, Jude CS230 Section 12159  3/31/2021
// Third Laboratory Assignment - Map NYC Taxi Destinations
// Environment: Windows 10 Visual Studio 2019

#include <iostream>
#include <fstream>
#include "windows.h"
using namespace std;

BITMAPFILEHEADER bmfh;
BITMAPINFOHEADER bmih;

#define BMP_SIZE 1024

int main()
{
	int RGB_White = 255,//White in RGB
		tableSize = BMP_SIZE,
		len = 0,//Used to display length of buffer
		x = 0,
		y = 0;

	//Size Limits of pixels on bmp
	float northLimit = 40.830509f,
		southLimit = 40.700455f,
		eastLimit = -73.914979f,
		westLimit = -74.045033f,

		* buffer;//Buffer where input data is stored

	char colorTable[BMP_SIZE],//Color table used to create bmp
		bits[BMP_SIZE][BMP_SIZE] = { 0 };//stores where to add color to bmp

	ifstream data("L2Data10K.dat");//Opens .dat
	if (!data) {//If .dat does not load, exit the program
		cout << "File Read Error!";
		return 0;
	}

	//Finds length of the .dat file and assigns it to len
	data.seekg(0, data.end);
	len = int(data.tellg() / 4);
	data.close();

	buffer = new float[len];//Creates buffer array with the size of len

	data.open("L2Data10K.dat", ios::binary);
	data.read((char*)buffer, len * 4.0);//reads in all values to buffer, with even being latitude and odd being longitude
	data.close();

	//Moves all invalid values to end of buffer and adjusts length so invalid values are no longer used
	int j = len - 1;//assign j to len -1
	for (int i = 0; i <= j; i += 2)//increments 2 values at a time, until j and i intersect
		//Searches through all values of buffer and moves all coords that are out of the bounds to the end of the buffer
		//Loops until a valid value replaces the current invalid value in position i
		for (j; buffer[i] <= southLimit || buffer[i] >= northLimit || buffer[i + 1] <= westLimit || buffer[i + 1] >= eastLimit; j -= 2) {
			buffer[i] = buffer[j - 1];
			buffer[i + 1] = buffer[j];
		}
	len -= len - j - 1;//Adjusts len to only go until the last valid value in buffer

	_asm {

		MOV ECX, len;//Assigns len to ECX
		MOV ESP, buffer;//Assigns buffer to ESP

		;//Main Loop, calculates position of the coords on the bmp, 
		;//assigns the coords to an index value in bits array and and sets color to white
	ML:
		;//Calculate Latitude

		FLD southLimit;//Loads southLimit into stack as a float
		FLD northLimit;//Loads northLimit into stack as a float
		FSUB ST(0), ST(1);//Subtracts southLimit from northLimit, saves result to ST(0)

		FILD tableSize;//Loads tableSize into stack as a int
		FLD[ESP];//Loads current value of buffer into stack as a float

		FSUB ST(0), ST(3);//Subtracks southLimit from curent value of buffer, saves result to ST(0)
		FDIV ST(0), ST(2);//divides result by (northLimit - southLimit), saves result to ST(0)
		FMUL ST(0), ST(1);//Multiplies result by tableSize 
		FRNDINT;//Rounds and converts ST(0) to an int

		LEA EBX, x;//Loads memory address of x to EBX
		FISTP[EBX];//Saves value of ST(0) to x as an int

		ADD ESP, 4;//Increments to next value in buffer


		;//Calculate Longitude

		FLD westLimit;//Loads westLimit into stack as a float
		FLD eastLimit;//Loads eastLimit into stack as a float
		FSUB ST(0), ST(1);//Subtracts westLimit from eastLimit, saves result to ST(0)

		FILD tableSize;//Loads tableSize into stack as a int
		FLD[ESP];//Loads current value of buffer into stack as a float

		FSUB ST(0), ST(3);//Subtracks westLimit from curent value of buffer, saves result to ST(0)
		FDIV ST(0), ST(2);//divides result by (eastLimit - westLimit), saves result to ST(0)
		FMUL ST(0), ST(1);//Multiplies result by tableSize 
		FRNDINT;//Rounds and converts ST(0) to an int

		LEA EBX, y;//Loads memory address of y to EBX
		FISTP[EBX];//Saves value of ST(0) to y as an int

		;//Store white pixel to bits array based on calculated coord

		FILD RGB_White;//Loads white pixel value into stack as an int
		MOV EAX, x;//Moves x into EAX
		MOV EBX, y;//Moves y into EBX
		MOV EDX, tableSize;//Moves tableSize into EDX
		MUL EDX;//Multiplies x by EDX to move to correct row in index
		ADD EAX, EBX;//Adds y to EAX to move to correct column in index

		LEA EBX, bits[EAX];//Loads memory address of bits array at the index value stored in EAX to EBX
		FISTP[EBX];//Saves value of ST(0) "RGB_White" to bits[x][y] as an int

		FFREE ST(0);//Frees floating point registers in stack
		FFREE ST(1);
		FFREE ST(2);
		FFREE ST(3);
		FFREE ST(4);
		FFREE ST(5);

		ADD ESP, 4;//Increments buffer to next index
		SUB ECX, 2;//Subtract 2 from counter

		CMP ECX, 0;//Checks if all value have been saved, otherwise repeat loop
		JNE ML;
	}

	// Define and open the output file. 
	ofstream bmpOut("foo.bmp", ios::out + ios::binary);
	if (!bmpOut) {
		cout << "...could not open file, ending.";
		return -1;
	}
	// Initialize the bit map file header with static values.
	bmfh.bfType = 0x4d42;
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = sizeof(bmfh) + sizeof(bmih) + sizeof(colorTable);
	bmfh.bfSize = bmfh.bfOffBits + sizeof(bits);

	// Initialize the bit map information header with static values.
	bmih.biSize = 40;
	bmih.biWidth = BMP_SIZE;
	bmih.biHeight = BMP_SIZE;
	bmih.biPlanes = 1;
	bmih.biBitCount = 8;
	bmih.biCompression = 0;
	bmih.biSizeImage = BMP_SIZE * BMP_SIZE;
	bmih.biXPelsPerMeter = 2835;
	bmih.biYPelsPerMeter = 2835;
	bmih.biClrUsed = 256;
	bmih.biClrImportant = 0;

	// Build color table.
	for (int i = 0, k = 0; i < 256; i++, k = i * 4)
		colorTable[k] = colorTable[k + 1] = colorTable[k + 2] = colorTable[k + 3] = i;

	// Write out the bit map.  
	char* workPtr;
	workPtr = (char*)&bmfh;
	bmpOut.write(workPtr, 14);
	workPtr = (char*)&bmih;
	bmpOut.write(workPtr, 40);
	workPtr = &colorTable[0];
	bmpOut.write(workPtr, sizeof(colorTable));
	workPtr = &bits[0][0];
	bmpOut.write(workPtr, BMP_SIZE * BMP_SIZE);
	bmpOut.close();

	system("mspaint foo.bmp");
	exit(0);

	return 0;
}
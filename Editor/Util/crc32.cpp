#include "stdafx.h"
#include "crc32.h"

Crc32Gen::Crc32Gen()
{
	init_CRC32_Table();
}

// Call this function only once to initialize the CRC table.
void Crc32Gen::init_CRC32_Table()
{
	// This is the official polynomial used by CRC-32
	// in PKZip, WinZip and Ethernet.
	unsigned int ulPolynomial = 0x04c11db7;
	
	// 256 values representing ASCII character codes.
	for(int i = 0; i <= 0xFF; i++)
	{
		crc32_table[i] = reflect(i, 8) << 24;
		for (int j = 0; j < 8; j++)
			crc32_table[i] = (crc32_table[i] << 1) ^ (crc32_table[i] & (1 << 31) ? ulPolynomial : 0);
		crc32_table[i] = reflect(crc32_table[i], 32);
	}
}

unsigned int Crc32Gen::reflect(unsigned int ref, char ch)
{// Used only by Init_CRC32_Table().
	unsigned int value = 0;
	
	// Swap bit 0 for bit 7
	// bit 1 for bit 6, etc.
	for(int i = 1; i < (ch + 1); i++)
	{
		if(ref & 1)
			value |= 1 << (ch - i);
		ref >>= 1;
	}
	return value;
}

unsigned int Crc32Gen::get_CRC32( const char *data, int size, unsigned int ulCRC )
{// Pass a text string to this function and it will return the CRC.
	
	// Once the lookup table has been filled in by the two functions above,
	// this function creates all CRCs using only the lookup table.
	
	// Be sure to use unsigned variables, because negative values introduce high bits where zero bits are required.
	
	// Start out with all bits set high.
	int len; 
	unsigned char* buffer;
	
	// Get the length. 
	len = size; 
	
  // Save the text in the buffer. 
	buffer = (unsigned char*)data; 
	// Perform the algorithm on each character in the string, using the lookup table values. 
	
	while(len--) 
		ulCRC = (ulCRC >> 8) ^ crc32_table[(ulCRC & 0xFF) ^ *buffer++]; 
	// Exclusive OR the result with the beginning value. 
	return ulCRC ^ 0xffffffff; 
} 

unsigned int Crc32Gen::GetCRC32( const char *text )
{
	int len = strlen(text);
	return GetCRC32( text,len,0xffffffff );
}

unsigned int Crc32Gen::GetCRC32( const char *data, int size, unsigned int ulCRC )
{
	static Crc32Gen stdCrc32Generator;
	return stdCrc32Generator.get_CRC32( data,size,ulCRC );
}
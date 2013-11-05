/*
 * hidtestosx.cpp
 *
 *  Created on: Nov 5, 2013
 *      Author: TestOSX107
 */


#ifdef WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include "hidapi.h"

#define MAX_STR 255

int main(int argc, char* argv[])
{
	int res;
	unsigned char buf[65];
	wchar_t wstr[MAX_STR];
	hid_device *handle;
	int i;

	// Initialize the hidapi library
	res = hid_init();

	// Open the device using the VID, PID,
	// and optionally the Serial number.
	handle = hid_open(0x044f, 0xd003, NULL);

	// Read the Manufacturer String
	res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
	wprintf(L"Manufacturer String: %ls\n", wstr);

	// Read the Product String
	res = hid_get_product_string(handle, wstr, MAX_STR);
	wprintf(L"Product String: %ls\n", wstr);

	// Read the Serial Number String
	res = hid_get_serial_number_string(handle, wstr, MAX_STR);
	wprintf(L"Serial Number String: (%d) %ls\n", wstr[0], wstr);

	// Read Indexed String 1
	res = hid_get_indexed_string(handle, 1, wstr, MAX_STR);
	wprintf(L"Indexed String 1: %ls\n", wstr);

	// Toggle LED (cmd 0x80). The first byte is the report number (0x0).
//	buf[0] = 0x0;
//	buf[1] = 0x0;
//	res = hid_write(handle, buf, 65);

	// Request state (cmd 0x81). The first byte is the report number (0x0).
//	buf[0] = 0x0;
//	buf[1] = 0x81;
//	res = hid_write(handle, buf, 65);

	#define MAX_DESCRIPTOR  4096
	unsigned char descr_buf[MAX_DESCRIPTOR];

	hid_dump_element_info( handle );
	hid_get_report_descriptor( handle, descr_buf, MAX_DESCRIPTOR );

	while( 1 ){
	// Read requested state
		res = hid_read(handle, buf, 65);

		// Print out the returned buffer.
		for (i = 0; i < 65; i++)
			printf(": %d", buf[i]);
		printf( "\n");
	}

	// Finalize the hidapi library
	res = hid_exit();

	return 0;
}



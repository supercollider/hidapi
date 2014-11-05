/* hidapi_parser $ 
 *
 * Copyright (C) 2013, Marije Baalman <nescivi _at_ gmail.com>
 * This work was funded by a crowd-funding initiative for SuperCollider's [1] HID implementation
 * including a substantial donation from BEK, Bergen Center for Electronic Arts, Norway
 * 
 * [1] http://supercollider.sourceforge.net
 * [2] http://www.bek.no
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*******************************************************
 Windows HID simplification

 Alan Ott
 Signal 11 Software

 8/22/2009

 Copyright 2009
 
 This contents of this file may be used by anyone
 for any reason without any conditions and may be
 used as a starting point for your own applications
 which use HIDAPI.
********************************************************/

//TODO: add copyright notice

#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include "hidapi.h"

// Headers needed for sleeping.
#ifdef _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
#endif


//// ---------- HID descriptor parser

#define MAX_DESCRIPTOR  4096


// main items
#define HID_INPUT 0x80
#define HID_OUTPUT 0x90
#define HID_COLLECTION 0xA0
#define HID_FEATURE 0xB0
// #define HID_COLLECTION 0xa1
#define HID_END_COLLECTION 0xC0

// HID Report Items from HID 1.11 Section 6.2.2
#define HID_USAGE_PAGE 0x04
// #define HID_USAGE_PAGE 0x05
// #define HID_USAGE_PAGE_EXT 0x06

#define HID_USAGE 0x08
// #define HID_USAGE 0x09
// #define HID_USAGE_EXT 0x0a

#define HID_USAGE_MIN 0x18
// #define HID_USAGE_MIN 0x19
#define HID_USAGE_MAX 0x28
// #define HID_USAGE_MAX 0x29

#define HID_DESIGNATOR_INDEX 0x38
#define HID_DESIGNATOR_MIN 0x48
#define HID_DESIGNATOR_MAX 0x58

#define HID_STRING_INDEX 0x78
#define HID_STRING_MIN 0x88
#define HID_STRING_MAX 0x98

#define HID_DELIMITER 0xA8

#define HID_LOGICAL_MIN 0x14
// #define HID_LOGICAL_MIN 0x15
// #define HID_LOGICAL_MIN_2 0x16
#define HID_LOGICAL_MAX 0x24
// #define HID_LOGICAL_MAX 0x25
// #define HID_LOGICAL_MAX_2 0x26

#define HID_PHYSICAL_MIN 0x34
// #define HID_PHYSICAL_MIN 0x35
// #define HID_PHYSICAL_MIN_2 0x36
// #define HID_PHYSICAL_MIN_3 0x37
#define HID_PHYSICAL_MAX 0x44
// #define HID_PHYSICAL_MAX 0x45
// #define HID_PHYSICAL_MAX_2 0x46

#define HID_UNIT_EXPONENT 0x54
// #define HID_UNIT 0x55
#define HID_UNIT 0x64
// #define HID_UNIT 0x65

#define HID_REPORT_SIZE 0x74
// #define HID_REPORT_SIZE 0x75
#define HID_REPORT_ID 0x84

#define HID_REPORT_COUNT 0x94
// #define HID_REPORT_COUNT 0x95

#define HID_PUSH 0xA4
#define HID_POP 0xB4

#define HID_RESERVED 0xC4 // above this it is all reserved


// HID Report Usage Pages from HID Usage Tables 1.12 Section 3, Table 1
#define HID_USAGE_PAGE_GENERICDESKTOP 0x01
#define HID_USAGE_PAGE_KEY_CODES       0x07
#define HID_USAGE_PAGE_LEDS            0x08
#define HID_USAGE_PAGE_BUTTONS         0x09

// HID Report Usages from HID Usage Tables 1.12 Section 4, Table 6
#define HID_USAGE_POINTER  0x01
#define HID_USAGE_MOUSE    0x02
#define HID_USAGE_JOYSTICK 0x04
#define HID_USAGE_KEYBOARD 0x06
#define HID_USAGE_X        0x30
#define HID_USAGE_Y        0x31
#define HID_USAGE_Z        0x32
#define HID_USAGE_RX       0x33
#define HID_USAGE_RY       0x34
#define HID_USAGE_RZ       0x35
#define HID_USAGE_SLIDER   0x36
#define HID_USAGE_DIAL     0x37
#define HID_USAGE_WHEEL    0x38


// HID Report Collection Types from HID 1.12 6.2.2.6
#define HID_COLLECTION_PHYSICAL    0
#define HID_COLLECTION_APPLICATION 1

// HID Input/Output/Feature Item Data (attributes) from HID 1.11 6.2.2.5
/// more like flags
#define HID_ITEM_CONSTANT 0x1
#define HID_ITEM_VARIABLE 0x2
#define HID_ITEM_RELATIVE 0x4
#define HID_ITEM_WRAP 0x8
#define HID_ITEM_LINEAR 0x10
#define HID_ITEM_PREFERRED 0x20
#define HID_ITEM_NULL 0x40
#define HID_ITEM_BITFIELD 0x80

// Report Types from HID 1.11 Section 7.2.1
#define HID_REPORT_TYPE_INPUT   1
#define HID_REPORT_TYPE_OUTPUT  2
#define HID_REPORT_TYPE_FEATURE 3

struct hid_device_descriptor {
	int num_elements;
// 	int usage_page;
// 	int usage;
	/** Pointer to the first element */
	struct hid_device_element *first;
};

struct hid_device_element {
	int index;

	int type;    // button/axis
	int vartype; // abs/relative
	int usage_page; // usage page
	int usage;   // some kind of index (as from descriptor)

	int logical_min;
	int logical_max;
	
	int phys_min;
	int phys_max;
	
	int resolution; // in bits

	int value;

	/** Pointer to the next element */
	struct hid_device_element *next;
};

struct hid_device_descriptor *descriptor;

#define BITMASK1(n) ((1ULL << (n)) - 1ULL)

int main(int argc, char* argv[])
{
	int res;
	unsigned char buf[256];
	unsigned char descr_buf[MAX_DESCRIPTOR];
	#define MAX_STR 255
	wchar_t wstr[MAX_STR];
	hid_device *handle;
	int i;
	
	descriptor = (struct hid_device_descriptor *) malloc( sizeof( struct hid_device_descriptor) );
	descriptor->num_elements = 0;

#ifdef WIN32
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
#endif

	struct hid_device_info *devs, *cur_dev;
	
	if (hid_init())
		return -1;

	devs = hid_enumerate(0x0, 0x0);
    cur_dev = devs;
    printf( "curdev %ls\n", cur_dev );
	while (cur_dev) {
        printf("Device Found\n  (vendor,product): %04hx %04hx\n  path: %s\n  serial_number: %ls", cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
		printf("\n");
		printf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
		printf("  Product:      %ls\n", cur_dev->product_string);
		printf("  Release:      %hx\n", cur_dev->release_number);
		printf("  Interface:    %d\n",  cur_dev->interface_number);
		printf("\n");
		cur_dev = cur_dev->next;
	}
	hid_free_enumeration(devs);

	// Set up the command buffer.
	memset(buf,0x00,sizeof(buf));
	buf[0] = 0x01;
	buf[1] = 0x81;
	

	// Open the device using the VID, PID,
	// and optionally the Serial number.
	////handle = hid_open(0x4d8, 0x3f, L"12345");
// 	handle = hid_open(0x4d8, 0x3f, NULL);
	// mouse:
// 	handle = hid_open(0x1241, 0x1166, NULL);
	// run'n'drive
//    handle = hid_open(0x044f, 0xd003, NULL);
    // impakt
    handle = hid_open(0x07b5, 0x0312, NULL);
	if (!handle) {
		printf("unable to open device\n");
 		return 1;
	}

	// Read the Manufacturer String
	wstr[0] = 0x0000;
	res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read manufacturer string\n");
	printf("Manufacturer String: %ls\n", wstr);

	// Read the Product String
	wstr[0] = 0x0000;
	res = hid_get_product_string(handle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read product string\n");
	printf("Product String: %ls\n", wstr);

	// Read the Serial Number String
	wstr[0] = 0x0000;
	res = hid_get_serial_number_string(handle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read serial number string\n");
	printf("Serial Number String: (%d) %ls", wstr[0], wstr);
	printf("\n");

	// Read Indexed String 1
	wstr[0] = 0x0000;
	res = hid_get_indexed_string(handle, 1, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read indexed string 1\n");
	printf("Indexed String 1: %ls\n", wstr);


	// Set the hid_read() function to be non-blocking.
    hid_set_nonblocking(handle, 1);

// 	// Try to read from the device. There shoud be no
// 	// data here, but execution should not block.
    res = hid_read(handle, buf, 17);

    printf( "res of read %i\n", res );

    hid_close(handle);

	/* Free static HIDAPI objects. */
	hid_exit();

#ifdef WIN32
	system("pause");
#endif

	return 0;
}

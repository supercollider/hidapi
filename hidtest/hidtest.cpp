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
		printf("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls", cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
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

/*
	//// PARSING THE DESCRIPTOR
	res = hid_get_report_descriptor( handle, descr_buf, MAX_DESCRIPTOR );
	if (res < 0)
		printf("Unable to read report descriptor\n");
	printf("Data read: %i\n   ", res);
	// Print out the returned buffer.
	
	struct hid_device_element * prev_element;
	int current_usage_page;
	int current_usage;
	int current_usages[256];
	int current_usage_index = 0;
	int current_usage_min = -1;
	int current_usage_max = -1;
	int current_logical_min;
	int current_logical_max;
	int current_physical_min;
	int current_physical_max;
	int current_report_count;
	int current_report_id;
	int current_report_size;
	char current_input;
	char current_output;
	int collection_nesting = 0;
	
	int next_byte_type = -1;
	int next_byte_size = 0;
	int next_val = 0;
	
	int toadd = 0;
	int byte_count = 0;
	
// 	for (i = 0; i < res; i++){
// 		printf("\n%02hhx ", descr_buf[i]);
// 	}
	for (i = 0; i < res; i++){
		printf("\n%02hhx ", descr_buf[i]);
		printf("\tbyte_type %i, %i, %i \t", next_byte_type, next_byte_size, next_val);
		if ( next_byte_type != -1 ){
		    toadd = descr_buf[i];
		    for ( int j=0; j<byte_count; j++ ){
			toadd = toadd << 8;
		    }
		    next_val += toadd;
		    byte_count++;
		    if ( byte_count == next_byte_size ){
		      switch( next_byte_type ){
			case HID_USAGE_PAGE:
			  current_usage_page = next_val;
			  printf("usage page: 0x%02hhx", current_usage_page);
			  break;
  // 		      case HID_USAGE_PAGE_EXT:
  // 			if ( count == 0 ){
  // 			  current_usage_page = descr_buf[i] * 256;
  // 			} else {
  // 			  current_usage_page += descr_buf[i];
  // 			  printf("usage_page: %i", current_usage_page);
  // 			  next_byte_type = -1;
  // 			}
  // 			count++;
  // 			break;
			case HID_USAGE:
			  current_usage = next_val;
			  current_usages[ current_usage_index ] = next_val;
  // 			current_usage = descr_buf[i];
  // 			current_usages[ current_usage_index ] = descr_buf[i];
			  printf("usage: 0x%02hhx, %i", current_usages[ current_usage_index ], current_usage_index );
			  current_usage_index++;
  // 			next_byte_type = -1;
			  break;
  // 		      case HID_USAGE_EXT:
  // 			if ( count == 0 ){
  // 			  current_usage = descr_buf[i] * 256;
  // 			} else {
  // 			  current_usage += descr_buf[i];
  // 			  printf("usage: %i", current_usage);
  // 			  next_byte_type = -1;
  // 			}
  // 			count++;
  // 			break;
			case HID_COLLECTION:
			  // COULD ALSO READ WHICH KIND OF COLLECTION
			  collection_nesting++;
			  printf("collection: %i, %i", collection_nesting, next_val );
  // 			next_byte_type = -1;
			  break;
			case HID_USAGE_MIN:
			  current_usage_min = next_val;
			  printf("usage min: %i", current_usage_min);
  // 			next_byte_type = -1;
			  break;
			case HID_USAGE_MAX:
			  current_usage_max = next_val;
			  printf("usage max: %i", current_usage_max);
  // 			next_byte_type = -1;
			  break;
			case HID_LOGICAL_MIN:
			  current_logical_min = next_val;
			  printf("logical min: %i", current_logical_min);
  // 			next_byte_type = -1;
			  break;
			case HID_LOGICAL_MAX:
			  current_logical_max = next_val;
			  printf("logical max: %i", current_logical_max);
  // 			next_byte_type = -1;
			  break;
  // 		      case HID_LOGICAL_MIN_2:
  // 			if ( count == 0 ){
  // 			  current_logical_min = descr_buf[i] * 256;
  // 			} else {
  // 			  current_logical_min += descr_buf[i];
  // 			  printf("logical min: %i", current_logical_min);
  // 			  next_byte_type = -1;
  // 			}
  // 			count++;
  // 			break;
  // 		      case HID_LOGICAL_MAX_2:
  // 			if ( count == 0 ){
  // 			  current_logical_max = descr_buf[i] * 256;
  // 			} else {
  // 			  current_logical_max += descr_buf[i];
  // 			  printf("logical max: %i", current_logical_max);
  // 			  next_byte_type = -1;
  // 			}
  // 			count++;
  // 			break;
			case HID_PHYSICAL_MIN:
			  current_physical_min = next_val;
			  printf("physical min: %i", current_physical_min);
  // 			next_byte_type = -1;
			  break;
			case HID_PHYSICAL_MAX:
			  current_physical_max = next_val;
			  printf("physical max: %i", current_physical_min);
  // 			next_byte_type = -1;
			  break;
  // 		      case HID_PHYSICAL_MIN_2:
  // 			if ( count == 0 ){
  // 			  current_physical_min = descr_buf[i] * 256;
  // 			} else {
  // 			  current_physical_min += descr_buf[i];
  // 			  printf("physical min: %i", current_physical_min);
  // 			  next_byte_type = -1;
  // 			}
  // 			count++;
  // 			break;
  // 		      case HID_PHYSICAL_MAX_2:
  // 			printf("setting physical max: %i", count );
  // 			if ( count == 0 ){
  // 			  current_physical_max = descr_buf[i] * 256;
  // 			} else {
  // 			  current_physical_max += descr_buf[i];
  // 			  printf("physical max: %i", current_physical_max);
  // 			  next_byte_type = -1;
  // 			}
  // 			count++;
  // 			break;
			case HID_REPORT_COUNT:
			  current_report_count = next_val;
			  printf("report count: %i", current_report_count);
  // 			next_byte_type = -1;
			  break;
			case HID_REPORT_SIZE:
			  current_report_size = next_val;
			  printf("report size: %i", current_report_size);
  // 			next_byte_type = -1;
			  break;
			case HID_REPORT_ID:
			  current_report_id = next_val;
			  printf("report id: %i", current_report_id);
  // 			next_byte_type = -1;
			  break;
			case HID_POP:
			  // TODO: something useful with unit information
			  printf("pop: %i", next_val );
  // 			next_byte_type = -1;
			  break;
			case HID_PUSH:
			  // TODO: something useful with unit information
			  printf("pop: %i", next_val );
  // 			next_byte_type = -1;
			  break;
			case HID_UNIT:
			  // TODO: something useful with unit information
			  printf("unit: %i", next_val );
  // 			next_byte_type = -1;
			  break;
			case HID_UNIT_EXPONENT:
			  // TODO: something useful with unit information
			  printf("unit exponent: %i", next_val );
  // 			next_byte_type = -1;
			  break;
			case HID_INPUT:
			  current_input = next_val;
			  printf("input: %i", current_input);
  // 			next_byte_type = -1;
			  /// add elements:
			  for ( int j=0; j<current_report_count; j++ ){
			      struct hid_device_element * new_element = (struct hid_device_element *) malloc( sizeof( struct hid_device_element ) );
			      new_element->index = descriptor->num_elements;
			      new_element->type = current_input; // & HID_ITEM_VARIABLE); 
  // 			    new_element->vartype = (current_input & HID_ITEM_RELATIVE);
			      new_element->usage_page = current_usage_page;
			      if ( current_usage_min != -1 ){
				new_element->usage = current_usage_min + j;
			      } else {
				new_element->usage = current_usages[j];
			      }
			      new_element->logical_min = current_logical_min;
			      new_element->logical_max = current_logical_max;
			      new_element->phys_min = current_physical_min;
			      new_element->phys_max = current_physical_max;
			      new_element->resolution = current_report_size;
			      new_element->value = 0;
			      if ( descriptor->num_elements == 0 ){
				  descriptor->first = new_element;
			      } else {
				  prev_element->next = new_element;
			      }
			      descriptor->num_elements++;
			      prev_element = new_element;
			  }
			  current_usage_min = -1;
			  current_usage_max = -1;
			  current_usage_index = 0;
			  break;
			case HID_OUTPUT:
			  current_output = next_val;
			  printf("output: %i", current_output);
			  printf("should create output element");
  // 			next_byte_type = -1;
			  current_usage_min = -1;
			  current_usage_max = -1;
			  current_usage_index = 0;
			  break;
			case HID_FEATURE:
  // 			current_output = next_val;
			  printf("feature: %i", next_val);
			  printf("should create feature element");
  // 			next_byte_type = -1;
			  current_usage_min = -1;
			  current_usage_max = -1;
			  current_usage_index = 0;
			  break;
			default:
			  if ( next_byte_type >= HID_RESERVED ){
			    printf("reserved bytes 0x%02hhx, %i", next_byte_type, next_val );
			  } else {
			    printf("undefined byte type 0x%02hhx, %i", next_byte_type, next_val );
			  }
		      }
		    next_byte_type = -1;
		    }
		} else {
// 		  printf("\n");
		  printf("\tsetting next byte type: %i, 0x%02hhx ", descr_buf[i], descr_buf[i] );
		  if ( descr_buf[i] == HID_END_COLLECTION ){ // JUST one byte
		    collection_nesting--;
		    printf("\tend collection: %i, %i\n", collection_nesting, descr_buf[i] );
		  } else {
		    byte_count = 0;
		    toadd = 0;
		    next_val = 0;
		    next_byte_type = descr_buf[i] & 0xFC;
		    next_byte_size = descr_buf[i] & 0x03;
		    if ( next_byte_size == 3 ){
			next_byte_size = 4;
		    }
		  printf("\t next byte type:  0x%02hhx, %i ", next_byte_type, next_byte_size );
		  }
		}
	}
	printf("\n");
	printf("number of elements, %i\n", descriptor->num_elements );
*/
	// Set the hid_read() function to be non-blocking.
	hid_set_nonblocking(handle, 1);
// 	
// 	// Try to read from the device. There shoud be no
// 	// data here, but execution should not block.
// 	res = hid_read(handle, buf, 17);
// 
// 	// Send a Feature Report to the device
// 	buf[0] = 0x2;
// 	buf[1] = 0xa0;
// 	buf[2] = 0x0a;
// 	buf[3] = 0x00;
// 	buf[4] = 0x00;
// 	res = hid_send_feature_report(handle, buf, 17);
// 	if (res < 0) {
// 		printf("Unable to send a feature report.\n");
// 	}
// 
// 	memset(buf,0,sizeof(buf));
// 
// 	// Read a Feature Report from the device
// 	buf[0] = 0x2;
// 	res = hid_get_feature_report(handle, buf, sizeof(buf));
// 	if (res < 0) {
// 		printf("Unable to get a feature report.\n");
// 		printf("%ls", hid_error(handle));
// 	}
// 	else {
// 		// Print out the returned buffer.
// 		printf("Feature Report\n   ");
// 		for (i = 0; i < res; i++)
// 			printf("%02hhx ", buf[i]);
// 		printf("\n");
// 	}
// 
// 	memset(buf,0,sizeof(buf));
// 
// 	// Toggle LED (cmd 0x80). The first byte is the report number (0x1).
// 	buf[0] = 0x1;
// 	buf[1] = 0x80;
// 	res = hid_write(handle, buf, 17);
// 	if (res < 0) {
// 		printf("Unable to write()\n");
// 		printf("Error: %ls\n", hid_error(handle));
// 	}
// 	
// 
// 	// Request state (cmd 0x81). The first byte is the report number (0x1).
// 	buf[0] = 0x1;
// 	buf[1] = 0x81;
// 	hid_write(handle, buf, 17);
// 	if (res < 0)
// 		printf("Unable to write() (2)\n");


// 	for ( int i=0; i<8; i++ ){
// 	  printf("bitmask test, %i, %i, %02hhx\n", i, BITMASK1(i), BITMASK1(i) );
// 	}

/*
	// Read requested state. hid_read() has been set to be
	// non-blocking by the call to hid_set_nonblocking() above.
	// This loop demonstrates the non-blocking nature of hid_read().
	res = 0;
	while (true) {
		res = hid_read(handle, buf, sizeof(buf));
// 		if (res == 0)
// 			printf("waiting...\n");
// 		if (res < 0)
// 			printf("Unable to read()\n");
		if ( res > 0 ) {
// 		  printf("Data read:\n");
		  // Print out the returned buffer.
		  hid_device_element * cur_element = descriptor->first;
		  for ( int i = 0; i < res; i++){
		    char curbyte = buf[i];
		    printf("byte %02hhx \t", buf[i]);
		    // read byte:
		    if ( cur_element->resolution < 8 ){
		      int bitindex = 0;
		      while ( bitindex < 8 ){
			// read bit
			cur_element->value = (curbyte >> bitindex) & BITMASK1( cur_element->resolution );
			printf("element page %i, usage %i, type %i, index %i, value %i\n", cur_element->usage_page, cur_element->usage, cur_element->type, cur_element->index, cur_element->value );
			bitindex += cur_element->resolution;
			cur_element = cur_element->next;
		      }
		    } else if ( cur_element->resolution == 8 ){
			cur_element->value = curbyte;
			printf("element page %i, usage %i, type %i,  index %i, value %i\n", cur_element->usage_page, cur_element->usage, cur_element->type, cur_element->index,cur_element->value );
			cur_element = cur_element->next;
		    } // else: larger than 8 bits
		  }
		  printf("\n");
		}
		#ifdef WIN32
		Sleep(500);
		#else
		usleep(500*100);
		#endif
	}
*/

	hid_close(handle);

	/* Free static HIDAPI objects. */
	hid_exit();

#ifdef WIN32
	system("pause");
#endif

	return 0;
}

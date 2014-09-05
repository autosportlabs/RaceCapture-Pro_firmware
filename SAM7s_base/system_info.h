//USB manufacturer string descriptor
//first element = length of this entire array, in bytes
//second element = 0x03 = descriptor type string (don't change)
#define USB_MFG_STRING_DESCRIPTOR 	\
	{ \
		30, \
		0x03, \
		'A', 0x00, 'u', 0x00, 't', 0x00, 'o', 0x00, 's', 0x00, 'p', 0x00, 'o', 0x00, 'r', 0x00, 't', 0x00, ' ', 0x00, 'L', 0x00, 'a', 0x00, 'b', 0x00, 's', 0x00 \
	}

//USB product string descriptor.
//first element = length of this entire array, in bytes
//second element = 0x03 = descriptor type string (don't change)
#define USB_PRODUCT_STRING_DESCRIPTOR \
	{ \
		32, \
		0x03, \
		'R', 0x00, 'a', 0x00, 'c', 0x00, 'e', 0x00, 'C', 0x00, 'a', 0x00, 'p', 0x00, 't', 0x00, 'u', 0x00, 'r', 0x00, 'e', 0x00, \
		'/', 0x00, 'P', 0x00, 'r', 0x00, 'o', 0x00 \
	}

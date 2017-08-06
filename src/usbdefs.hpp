#pragma once

#include <cstdint>

struct[[gnu::packed]] USBDeviceDescriptor{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdUSB;
	uint8_t bDeviceClass;
	uint8_t bDeviceSubClass;
	uint8_t bDeviceProtocol;
	uint8_t bMaxPacketSize;
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t bcdDevice;
	uint8_t iManufacturer;
	uint8_t iProduct;
	uint8_t iSerialNumber;
	uint8_t bNumConfigurations;
};

struct[[gnu::packed]] USBConfigurationDescriptor{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wTotalLength;
	uint8_t bNumInterfaces;
	uint8_t bConfigurationValue;
	uint8_t iConfiguration;
	uint8_t bmAttributes;
	uint8_t bMaxPower;
};

struct[[gnu::packed]] USBInterfaceDescriptor{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bInterfaceNumber;
	uint8_t bAlternateSetting;
	uint8_t bNumEndpoints;
	uint8_t bInterfaceClass;
	uint8_t bInterfaceSubClass;
	uint8_t bInterfaceProtocol;
	uint8_t iInterface;
};

struct[[gnu::packed]] USBEndpointDescriptor{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bEndpointAddress;
	uint8_t bmAttributes;
	uint16_t wMaxPacketSize;
	uint8_t bInterval;
};

template<int langCount>
struct[[gnu::packed]] USBStringDescriptor{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wLANGID[langCount];
};

template<int stringLen>
struct[[gnu::packed]] USBString{
	uint8_t bLength;
	uint8_t bDescriptorType;
	char string[stringLen];
};

struct[[gnu::packed]] HIDDescriptor{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdHID;
	uint8_t bCountryCode;
	uint8_t bNumDescriptors;
	uint8_t bDescriptorType;
	uint16_t wDescriptorLength;
	uint8_t bDescriptorType;
	uint16_t wDescriptorLength;
};

struct[[gnu::packed]] USBSetup{
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
};


static_assert(sizeof(USBSetup) == 8, "Wrong USB setup structure size!");
static_assert(sizeof(USBDeviceDescriptor) == 18, "Wrong USB device descriptor size!");
#ifndef _MSC_VER		//Suppress IntelliSense errors, since it is don't understand [[gnu::packed]]
static_assert(sizeof(USBConfigurationDescriptor) == 9, "Wrong USB configuration descriptor size!");
static_assert(sizeof(USBEndpointDescriptor) == 7, "Wrong USB endpoint descriptor size!");
#endif


#define  USB_DESC_TYPE_DEVICE                              1
#define  USB_DESC_TYPE_CONFIGURATION                       2
#define  USB_DESC_TYPE_STRING                              3
#define  USB_DESC_TYPE_INTERFACE                           4
#define  USB_DESC_TYPE_ENDPOINT                            5
#define  USB_DESC_TYPE_DEVICE_QUALIFIER                    6
#define  USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION           7
#define  USB_DESC_TYPE_BOS                                 0x0F


#define  USB_REQ_GET_STATUS                             0x00
#define  USB_REQ_CLEAR_FEATURE                          0x01
#define  USB_REQ_SET_FEATURE                            0x03
#define  USB_REQ_SET_ADDRESS                            0x05
#define  USB_REQ_GET_DESCRIPTOR                         0x06
#define  USB_REQ_SET_DESCRIPTOR                         0x07
#define  USB_REQ_GET_CONFIGURATION                      0x08
#define  USB_REQ_SET_CONFIGURATION                      0x09
#define  USB_REQ_GET_INTERFACE                          0x0A
#define  USB_REQ_SET_INTERFACE                          0x0B
#define  USB_REQ_SYNCH_FRAME                            0x0C


#define  USB_REQ_RECIPIENT_DEVICE                       0x00
#define  USB_REQ_RECIPIENT_INTERFACE                    0x01
#define  USB_REQ_RECIPIENT_ENDPOINT                     0x02
#define  USB_REQ_RECIPIENT_MASK                         0x03

#define	 HID_DESCRIPTOR_TYPE							0x21
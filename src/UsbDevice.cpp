/*
	Назначение - предоставление движку usb информации о реализованом устройстве.
*/

#include "usb.hpp"



#define  USBD_IDX_LANGID_STR                            0x00 
#define  USBD_IDX_MFC_STR                               0x01 
#define  USBD_IDX_PRODUCT_STR                           0x02
#define  USBD_IDX_SERIAL_STR                            0x03 
#define  USBD_IDX_CONFIG_STR                            0x04 
#define  USBD_IDX_INTERFACE_STR                         0x05 


const USBDeviceDescriptor g_deviceDescriptor = {
	sizeof(USBDeviceDescriptor),
	USB_DESC_TYPE_DEVICE,
	0x02'00,	//usb 2.0
	0,
	0,
	0,
	g_ep0MaxPacketSize,
	1155,		//VID
	22314,		//PID
	0x02'00,
	USBD_IDX_MFC_STR,
	USBD_IDX_PRODUCT_STR,
	USBD_IDX_SERIAL_STR,
	1
};


struct[[gnu::packed]] ConfigDescriptor{
	USBConfigurationDescriptor config;
	USBInterfaceDescriptor interface;
	HIDDescriptor hid;
	USBEndpointDescriptor hidEndp;
};

static const ConfigDescriptor g_confDesc = {
	{
		sizeof(USBConfigurationDescriptor),
		USB_DESC_TYPE_CONFIGURATION,
		sizeof(ConfigDescriptor),
		1,
		1,
		USBD_IDX_CONFIG_STR,
		0x80,
		0x32
	},
	{
		sizeof(USBInterfaceDescriptor),
		USB_DESC_TYPE_INTERFACE,
		0,
		0,
		1,
		0x03,
		0,
		1,
		USBD_IDX_INTERFACE_STR
	},
	{
		sizeof(HIDDescriptor),
		HID_DESCRIPTOR_TYPE,
		0x01'11,
		0,
		1,
		0x22,
		//report descriptor size
	},
	{
		sizeof(USBEndpointDescriptor),
		USB_DESC_TYPE_ENDPOINT,
		0x81,
		0x03,
		4,
		0x0A
	}
};


static const USBLangDescriptor<LangID::EnglishUS, LangID::Russian> g_langDesc;

static const auto g_mfcStringRus = "Дофига производитель"_usb;
static const auto g_productStringRus = "Дофига устройство"_usb;
static const auto g_serialStringRus = "Дофига серийный номер"_usb;
static const auto g_configStringRus = "дофига конфигурация"_usb;
static const auto g_interfStringRus = "Дофига интерфейс"_usb;

static const auto g_mfcStringEng = "Much Manufacturer"_usb;
static const auto g_productStringEng = "So Device"_usb;
static const auto g_serialStringEng = "Many Serial number"_usb;
static const auto g_configStringEng = "Wow Configuration"_usb;
static const auto g_interfStringEng = "Very Interface"_usb;


/*
==================
GetFullConfigDescriptor
==================
*/
const uint8_t *GetFullConfigDescriptor(uint16_t &len, const uint8_t cfgNum) noexcept {

}

/*
==================
GetStringDescriptor
==================
*/
const uint8_t *GetStringDescriptor(uint16_t &len, const uint8_t stringID, const LangID langID) noexcept {
	switch (stringID) {
		case USBD_IDX_LANGID_STR:
			RETURN_STRING(g_langDesc)
			break;

		case USBD_IDX_MFC_STR:
			if (langID == LangID::EnglishUS) {
				RETURN_STRING(g_mfcStringEng)
			} else if (langID == LangID::Russian) {
				RETURN_STRING(g_mfcStringRus)
			}
			break;

		case USBD_IDX_PRODUCT_STR:
			if (langID == LangID::EnglishUS) {
				RETURN_STRING(g_productStringEng)
			} else if (langID == LangID::Russian) {
				RETURN_STRING(g_productStringRus)
			}
			break;

		case USBD_IDX_SERIAL_STR:
			if (langID == LangID::EnglishUS) {
				RETURN_STRING(g_serialStringEng)
			} else if (langID == LangID::Russian) {
				RETURN_STRING(g_serialStringRus)
			}
			break;

		case USBD_IDX_CONFIG_STR:
			if (langID == LangID::EnglishUS) {
				RETURN_STRING(g_configStringEng)
			} else if (langID == LangID::Russian) {
				RETURN_STRING(g_configStringRus)
			}
			break;

		case USBD_IDX_INTERFACE_STR:
			if (langID == LangID::EnglishUS) {
				RETURN_STRING(g_interfStringEng)
			} else if (langID == LangID::Russian) {
				RETURN_STRING(g_interfStringRus)
			}
			break;

		default:
			len = 0;
			return nullptr;
	}
}

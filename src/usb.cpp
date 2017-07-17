#include "usb.hpp"
#include "usbHwRegs.hpp"
#include "usbdefs.hpp"
#include <scmRTOS_TARGET_CFG.h>



static const USBDeviceDescriptor g_devDesc = {
	sizeof(USBDeviceDescriptor),
	USB_DESC_TYPE_DEVICE,
	0x02'00,	//usb 2.0
	0,
	0,
	0,
	g_ep0MaxPacketSize,
	1155,		//VID
	22315,		//PID
	0x02'00,
	1,			//manufacturer string
	2,			//prodict string
	3,			//serial numder string
	1
};


/*
==================
WritePMA

	Write byteCnt bytes to Packet Memory Area
	 - PMABufAddr - address of PMA relative to usb peripherial
==================
*/
static void WritePMA(const uint8_t *in, const uint16_t PMABufAddr, const uint16_t byteCnt) noexcept {
	auto dataIn = reinterpret_cast<const uint16_t*>(in);
	auto pma = reinterpret_cast<uint32_t*>(USB_PMAADDR + (PMABufAddr << 1));

	for (auto i = 0; i < (byteCnt + 1) >> 1; i++) {
		*pma++ = *dataIn++;			//gaps in PMA skipped due 32-bit pointer increment
	}
}

/*
==================
ReadPMA

	Read byteCnt bytes from Packet Memory Area
	 - PMABufAddr - address of PMA relative to usb peripherial
==================
*/
static void ReadPMA(uint8_t *out, const uint16_t PMABufAddr, const uint16_t byteCnt) noexcept {
	auto dataOut = reinterpret_cast<uint16_t*>(out);
	auto pma = reinterpret_cast<uint32_t*>(USB_PMAADDR + (PMABufAddr << 1));
	for (auto i = 0; i < (byteCnt + 1) >> 1; i++) {
		*dataOut++ = *pma++;
	}
}

static class ControlPipe {
	const uint8_t *m_data;
	int m_len, m_remainLen;

public:
	ControlPipe() : m_data(nullptr), m_len(0), m_remainLen(0) {}

	void Send_i(const uint8_t * const data, const int len);

	void OnInToken();
} g_ctrlPipe;


/*
==================
ControlPipe::Send_i
==================
*/
void ControlPipe::Send_i(const uint8_t * const data, const int len) {
	m_data = const_cast<const uint8_t*>(data);
	m_len = m_remainLen = len;
	OnInToken();
}

/*
==================
ControlPipe::OnInToken
==================
*/
void ControlPipe::OnInToken() {
	if (m_remainLen > g_ep0MaxPacketSize) {
		m_remainLen -= g_ep0MaxPacketSize;

		WritePMA(m_data, g_ep0TxBufferStart, m_remainLen);
		PBT[0].COUNT_TX = m_remainLen;
		EPR[0].SetTxStatus(EpTxStatus::VALID);
	} else {
		WritePMA(m_data, g_ep0TxBufferStart, m_remainLen);
		PBT[0].COUNT_TX = m_remainLen;
		EPR[0].SetTxStatus(EpTxStatus::VALID);
	}
}

#define  USBD_IDX_LANGID_STR                            0x00 
#define  USBD_IDX_MFC_STR                               0x01 
#define  USBD_IDX_PRODUCT_STR                           0x02
#define  USBD_IDX_SERIAL_STR                            0x03 
#define  USBD_IDX_CONFIG_STR                            0x04 
#define  USBD_IDX_INTERFACE_STR                         0x05 

/*
==================
GetDescriptor
==================
*/
static void GetDescriptor(const USBSetup * const setup) noexcept {
	uint16_t len;
	uint8_t *pbuf;

	switch (setup->wValue >> 8) {
		case USB_DESC_TYPE_DEVICE:
			pbuf = reinterpret_cast<decltype(pbuf)>(const_cast<USBDeviceDescriptor*>(&g_devDesc));
			len = sizeof g_devDesc;
			break;

		case USB_DESC_TYPE_CONFIGURATION:
			
			break;

		case USB_DESC_TYPE_STRING:
			switch (uint8_t(setup->wValue)) {
				case USBD_IDX_LANGID_STR:
					
					break;

				case USBD_IDX_MFC_STR:
					
					break;

				case USBD_IDX_PRODUCT_STR:
					
					break;

				case USBD_IDX_SERIAL_STR:
					
					break;

				case USBD_IDX_CONFIG_STR:
					
					break;

				case USBD_IDX_INTERFACE_STR:
					
					break;

				default:
					//ctrl error
					return; 
			}
			break;

		case USB_DESC_TYPE_DEVICE_QUALIFIER:
			
			break;

		case USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION:
			
			break;

		default:
			//ctrl error
			return;
	}

	if (len && setup->wLength) {
		len = len < setup->wLength ? len : setup->wLength;
		g_ctrlPipe.Send_i(pbuf, len);
	}
}

static int8_t g_devAddress;

/*
==================
OnDeviceRequest
==================
*/
static void OnDeviceRequest(const USBSetup * const setup) noexcept {
	switch (setup->bRequest) {
		case USB_REQ_GET_DESCRIPTOR:
			GetDescriptor(setup);
			break;

		case USB_REQ_SET_ADDRESS:
			g_devAddress = setup->wValue & 0x7F;
			g_ctrlPipe.Send_i(nullptr, 0);			//ZLP ack
			break;

		case USB_REQ_SET_CONFIGURATION:
			
			break;

		case USB_REQ_GET_CONFIGURATION:
			
			break;

		case USB_REQ_GET_STATUS:
			
			break;

		case USB_REQ_SET_FEATURE:
			
			break;

		case USB_REQ_CLEAR_FEATURE:
			
			break;

		default:
			
			break;
	}
}

/*
==================
OnSetupStage
==================
*/
static void OnSetupStage(const USBSetup * const setup) noexcept {
	switch (setup->bmRequestType & 0x1F) {
		case USB_REQ_RECIPIENT_DEVICE:
			OnDeviceRequest(setup);
			break;

		case USB_REQ_RECIPIENT_INTERFACE:
			
			break;

		case USB_REQ_RECIPIENT_ENDPOINT:
			
			break;

		default:
			//stall
			break;
	}
}

/*
==================
OnReset

	Set EP0 to default state
==================
*/
static void OnReset() noexcept {
	EPR[0].SetType(EpType::CTRL);
	EPR[0].SetAddress(0);

	PBT[0].ADDR_TX = g_ep0TxBufferStart;
	EPR[0].ClearTxDTOG();
	EPR[0].SetTxStatus(EpTxStatus::NAK);

	PBT[0].ADDR_RX = g_ep0RxBufferStart;
	PBT[0].COUNT_RX.SetMaxSize(g_ep0MaxPacketSize);
	EPR[0].ClearRxDTOG();
	EPR[0].SetRxStatus(EpRxStatus::VALID);
}

/*
==================
OnCorrectTranfer
==================
*/
static void OnCorrectTranfer() noexcept {
	while (USB->ISTR & USB_ISTR_CTR) {
		EpNum endpNum = EpNum(USB->ISTR & USB_ISTR_EP_ID);

		if (endpNum == EpNum::ep0) {
			if (!(USB->ISTR & USB_ISTR_DIR)) {			//IN token
				EPR[0].ClearTxCTR();
				
				PBT[0].COUNT_RX.SetMaxSize(0);			//tmp code
				EPR[0].SetRxStatus(EpRxStatus::VALID);

				if (g_devAddress) {
					USB->DADDR = uint16_t(g_devAddress) | USB_DADDR_EF;
					g_devAddress = 0;
				}
			} else {
				if (USB->EP0R & USB_EP0R_SETUP) {		//SETUP token
					EPR[0].ClearRxCTR();

					uint16_t rxSize = PBT[0].COUNT_RX.GetReceivedSize();
					USBSetup setup;
					ReadPMA(reinterpret_cast<uint8_t*>(&setup), g_ep0RxBufferStart, rxSize);

					OnSetupStage(&setup);
				} else if (USB->EP0R & USB_EP0R_CTR_RX) {	//OUT token
					EPR[0].ClearRxCTR();

					__NOP();

					PBT[0].COUNT_RX.SetMaxSize(g_ep0MaxPacketSize);
					EPR[0].SetRxStatus(EpRxStatus::VALID);
				}
			}
		} else {

		}
	}
}

/*
==================
USB_LP_CAN1_RX0_IRQHandler
==================
*/
extern "C" void USB_LP_CAN1_RX0_IRQHandler() {
	if (USB->ISTR & USB_ISTR_CTR) {
		OnCorrectTranfer();
	}
	if (USB->ISTR & USB_ISTR_RESET) {
		USB->ISTR &= ~USB_ISTR_RESET;
		OnReset();
		USB->DADDR = USB_DADDR_EF;		//enable function
	}
	if (USB->ISTR & USB_ISTR_SOF) {
		USB->ISTR &= ~USB_ISTR_SOF;

	}
	if (USB->ISTR & USB_ISTR_WKUP) {
		USB->ISTR &= ~USB_ISTR_WKUP;

	}
	if (USB->ISTR & USB_ISTR_SUSP) {
		USB->ISTR &= ~USB_ISTR_SUSP;

	}
}

/*
==================
USBInit
==================
*/
void USBInit() {
	static_assert(SYSTICKFREQ == 72'000'000 || SYSTICKFREQ == 48'000'000, "Since usb module used, clocks must be 72 or 48 Mhz.");
	if constexpr(SYSTICKFREQ == 48'000'000) {
		RCC->CFGR |= RCC_CFGR_USBPRE;		//clock is not divided
	}

	RCC->APB1ENR |= RCC_APB1ENR_USBEN;

	USB->CNTR &= ~USB_CNTR_PDWN;			//Enable analog tranciver
	__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
	USB->ISTR = 0;							//Clear pending interrupts
	USB->BTABLE = 0x000;					//Set Btable Address

	//Enable interrupts, also clear FRES bit.
	USB->CNTR = USB_CNTR_CTRM | USB_CNTR_WKUPM | USB_CNTR_SUSPM | USB_CNTR_RESETM | USB_CNTR_SOFM;
	NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);

	//Clear packet memory for debug purposes. Not realy need to do this.
#ifdef DEBUG
	for (unsigned int *pmem = reinterpret_cast<unsigned int*>(USB_PMAADDR); pmem < reinterpret_cast<unsigned int*>(USB_PMAADDR + 512 * 2); *(pmem++) = 0);
#endif
}

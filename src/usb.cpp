#include "usb.hpp"
#include "usbHwRegs.hpp"
#include "usbdefs.hpp"
#include "commdef.hpp"
#include <scmRTOS_TARGET_CFG.h>



/*
==================
WritePMA

	Write byteCnt bytes to Packet Memory Area
	 - PMABufAddr - address of PMA relative to usb peripherial
==================
*/
void WritePMA(const uint8_t *in, const uint16_t PMABufAddr, const uint16_t byteCnt) noexcept {
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
void ReadPMA(uint8_t *out, const uint16_t PMABufAddr, const uint16_t byteCnt) noexcept {
	auto dataOut = reinterpret_cast<uint16_t*>(out);
	auto pma = reinterpret_cast<const uint32_t*>(USB_PMAADDR + (PMABufAddr << 1));

	for (auto i = 0; i < (byteCnt + 1) >> 1; i++) {
		*dataOut++ = *pma++;
	}
}

#pragma region Control Pipe

static class ControlPipe {
	const uint8_t *m_data;
	int m_len, m_remainLen;

private:
	enum class TransferStage {
		Setup,
		DataIN,
		DataINLast,
		DataOUT,
		StatusIN,
		StatusOUT
	} m_transferState;

public:
	ControlPipe() : m_data(nullptr), m_len(0), m_remainLen(0), m_transferState(TransferStage::Setup) {}

	void OnInTransferComplite() noexcept;
	void OnOutTransferComplite() noexcept;
	void OnSetupTransferComplite() noexcept;
	void Reset() noexcept;

private:
	void SendData(const uint8_t * const data, const int len) noexcept;
	void OnDeviceRequest(const USBSetup * const setup) noexcept;
	void GetDescriptor(const USBSetup * const setup) noexcept;
	void GoToStage(const TransferStage newStage) noexcept;
} g_defaultControlPipe;

/*
==================
ControlPipe::Reset
==================
*/
void ControlPipe::Reset() noexcept {
	EPR[0].SetType(EpType::CTRL);
	EPR[0].SetAddress(0);

	PBT[0].ADDR_TX = g_ep0TxBufferStart;
	EPR[0].ClearTxDTOG();
	EPR[0].SetTxStatus(EpTxStatus::NAK);

	PBT[0].ADDR_RX = g_ep0RxBufferStart;
	PBT[0].COUNT_RX.SetMaxSize(g_ep0MaxPacketSize);
	EPR[0].ClearRxDTOG();
	EPR[0].SetRxStatus(EpRxStatus::VALID);

	m_transferState = TransferStage::Setup;
	m_remainLen = m_len = 0;
	m_data = nullptr;
}

/*
==================
ControlPipe::GoToStage

	Switch endpoint status as described in RM0008 on page 633
==================
*/
void ControlPipe::GoToStage(const TransferStage newStage) noexcept {
	switch (newStage) {
		case TransferStage::StatusOUT:
			EPR[0].SetRxStatus(EpRxStatus::VALID);
			EPR[0].SetTxStatus(EpTxStatus::STALL);
			EPR[0].SetStatusOut();
			break;

		case TransferStage::Setup:
			EPR[0].SetRxStatus(EpRxStatus::VALID);
			EPR[0].SetTxStatus(EpTxStatus::NAK);
			break;

		case TransferStage::DataOUT:
			EPR[0].SetRxStatus(EpRxStatus::VALID);
			EPR[0].SetTxStatus(EpTxStatus::STALL);
			break;

		case TransferStage::DataINLast:
			EPR[0].SetRxStatus(EpRxStatus::NAK);
			EPR[0].SetTxStatus(EpTxStatus::VALID);
			break;

		default:		//DataIN or StatusIN
			EPR[0].SetRxStatus(EpRxStatus::STALL);
			EPR[0].SetTxStatus(EpTxStatus::VALID);
			break;
	}

	m_transferState = newStage;
}

/*
==================
ControlPipe::SendData
==================
*/
void ControlPipe::SendData(const uint8_t * const data, const int len) noexcept {
	m_data = data;
	m_len = m_remainLen = len;
	
	m_transferState = TransferStage::DataIN;
	OnInTransferComplite();
}

/*
==================
ControlPipe::OnInTransferComplite

	Prepare for send next pice of data or go to status stage
==================
*/
void ControlPipe::OnInTransferComplite() noexcept {
	if (m_transferState == TransferStage::DataIN || m_transferState == TransferStage::DataINLast) {
		if (m_remainLen > g_ep0MaxPacketSize) {
			m_remainLen -= g_ep0MaxPacketSize;
			WritePMA(m_data, g_ep0TxBufferStart, g_ep0MaxPacketSize);
			PBT[0].COUNT_TX = g_ep0MaxPacketSize;
			m_data += g_ep0MaxPacketSize;
			GoToStage(TransferStage::DataIN);
		} else if(m_remainLen) {
			WritePMA(m_data, g_ep0TxBufferStart, m_remainLen);
			PBT[0].COUNT_TX = m_remainLen;
			m_remainLen = 0;
			if (m_len % g_ep0MaxPacketSize) {
				GoToStage(TransferStage::DataINLast);
			} else {
				GoToStage(TransferStage::DataIN);		//data size is multiple of g_ep0MaxPacketSizes, so last packet will be ZLP
			}
		} else if (m_transferState == TransferStage::DataIN) {
			PBT[0].COUNT_TX = 0;						//Send ZLP as last packet
			GoToStage(TransferStage::DataINLast);
		} else {
			GoToStage(TransferStage::StatusOUT);				//Host transmit to us ZLP as status of all transfer
		}
	} else if (m_transferState == TransferStage::StatusIN) {	//Last stage in transfer, enable reception of next SETUP packets
		GoToStage(TransferStage::Setup);
	}
}

/*
==================
ControlPipe::OnOutTransferComplite
==================
*/
void ControlPipe::OnOutTransferComplite() noexcept {
	if (m_transferState == TransferStage::StatusOUT) {
		EPR[0].ClearStatusOut();
		GoToStage(TransferStage::Setup);
	}
}

/*
==================
ControlPipe::OnSetupTransferComplite
==================
*/
void ControlPipe::OnSetupTransferComplite() noexcept {
	if (m_transferState != TransferStage::Setup) {
		return;
	}

	uint16_t rxSize = PBT[0].COUNT_RX.GetReceivedSize();
	USBSetup setup;
	ReadPMA(reinterpret_cast<uint8_t*>(&setup), g_ep0RxBufferStart, rxSize);

	switch (setup.bmRequestType & 0x1F) {
		case USB_REQ_RECIPIENT_DEVICE:
			OnDeviceRequest(&setup);
			break;

		case USB_REQ_RECIPIENT_INTERFACE:
			__BKPT(14);
			break;

		case USB_REQ_RECIPIENT_ENDPOINT:
			__BKPT(14);
			break;

		default:
			//stall
			__BKPT(14);
			break;
	}
}

/*
==================
ControlPipe::GetDescriptor
==================
*/
void ControlPipe::GetDescriptor(const USBSetup * const setup) noexcept {
	uint16_t len;
	const uint8_t *pbuf;

	switch (setup->wValue >> 8) {
		case USB_DESC_TYPE_DEVICE:
			len = sizeof g_deviceDescriptor;
			pbuf = reinterpret_cast<const uint8_t*>(&g_deviceDescriptor);
			break;

		case USB_DESC_TYPE_CONFIGURATION:
			pbuf = GetFullConfigDescriptor(len, uint8_t(setup->wValue));
			break;

		case USB_DESC_TYPE_STRING:
			pbuf = GetStringDescriptor(len, uint8_t(setup->wValue), LangID(setup->wIndex));
			break;

		default:
			//ctrl error
			__BKPT(14);
			return;
	}

	if (len && setup->wLength) {
		len = len < setup->wLength ? len : setup->wLength;
		SendData(pbuf, len);
	}
}

static int8_t g_devAddress;

/*
==================
ControlPipe::OnDeviceRequest
==================
*/
void ControlPipe::OnDeviceRequest(const USBSetup * const setup) noexcept {
	uint16_t status;

	switch (setup->bRequest) {
		case USB_REQ_GET_DESCRIPTOR:
			GetDescriptor(setup);
			break;

		case USB_REQ_SET_ADDRESS:
			g_devAddress = setup->wValue & 0x7F;

			PBT[0].COUNT_TX = 0;					//send ZLP to host as status
			GoToStage(TransferStage::StatusIN);
			break;

		case USB_REQ_SET_CONFIGURATION:
			__BKPT(14);
			break;

		case USB_REQ_GET_CONFIGURATION:
			__BKPT(14);
			break;

		case USB_REQ_GET_STATUS:
			status = 1;
			SendData(reinterpret_cast<const uint8_t*>(&status), 2);
			break;

		case USB_REQ_SET_FEATURE:
			__BKPT(14);
			break;

		case USB_REQ_CLEAR_FEATURE:
			__BKPT(14);
			break;

		default:
			__BKPT(14);
			break;
	}
}
#pragma endregion

/*
==================
OnCorrectTransfer
==================
*/
static void OnCorrectTransfer() noexcept {
	while (USB->ISTR & USB_ISTR_CTR) {
		EpNum endpNum = EpNum(USB->ISTR & USB_ISTR_EP_ID);

		if (endpNum == EpNum::ep0) {
			if (!(USB->ISTR & USB_ISTR_DIR)) {				//IN Transfer Complite
				EPR[0].ClearTxCTR();

				g_defaultControlPipe.OnInTransferComplite();

				if (g_devAddress) {			//SET_ADDRESS perform processing after status stage
					USB->DADDR = uint16_t(g_devAddress) | USB_DADDR_EF;
					g_devAddress = 0;
				}
			} else {
				if (USB->EP0R & USB_EP0R_SETUP) {			//SETUP Transfer Complite
					EPR[0].ClearRxCTR();

					g_defaultControlPipe.OnSetupTransferComplite();
				} else if (USB->EP0R & USB_EP0R_CTR_RX) {	//OUT Transfer Complite
					EPR[0].ClearRxCTR();

					g_defaultControlPipe.OnOutTransferComplite();
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
		OnCorrectTransfer();
	}
	if (USB->ISTR & USB_ISTR_RESET) {
		USB->ISTR &= ~USB_ISTR_RESET;
		g_defaultControlPipe.Reset();
		USB->DADDR = USB_DADDR_EF;		//enable function
	}
/*	if (USB->ISTR & USB_ISTR_SOF) {
		USB->ISTR &= ~USB_ISTR_SOF;

	}
	if (USB->ISTR & USB_ISTR_WKUP) {
		USB->ISTR &= ~USB_ISTR_WKUP;

	}
	if (USB->ISTR & USB_ISTR_SUSP) {
		USB->ISTR &= ~USB_ISTR_SUSP;

	}*/
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
	USB->BTABLE = g_pbtStart;				//Set Btable Address

	//Enable interrupts, also clear FRES bit.
	USB->CNTR = USB_CNTR_CTRM | USB_CNTR_RESETM /*| USB_CNTR_WKUPM | USB_CNTR_SUSPM | USB_CNTR_SOFM*/;
	NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);

	//Clear packet memory for debug purposes. Not realy need to do this.
	if constexpr(g_debug) {
		for (unsigned int *pmem = reinterpret_cast<unsigned int*>(USB_PMAADDR); pmem < reinterpret_cast<unsigned int*>(USB_PMAADDR + 512 * 2); *(pmem++) = 0);
	}
}

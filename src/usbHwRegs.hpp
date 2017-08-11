#pragma once

#include <stm32f1xx.h>


/*
===============================================================================

	PacketBufTableCountRxReg

	Wrapper around PBT COUNT_RX register, since this register have non-trivial
	usage rules. For details see RM0008 

===============================================================================
*/
class PacketBufTableCountRxReg {
	volatile uint16_t m_hwReg;

public:
	PacketBufTableCountRxReg() = delete;

	inline uint16_t GetReceivedSize() const noexcept {
		return m_hwReg & 0x3FF;
	}

	inline void SetMaxSize(const uint16_t size) noexcept {
		uint16_t blocksCnt;
		if (size > 62) {			//block size 32
			blocksCnt = size >> 5;
			if (!(size & 0x1f)) {
				blocksCnt--;
			}
			m_hwReg = uint16_t(blocksCnt << 10) | 0x8000;
		} else {					//block size 2
			blocksCnt = size >> 1;
			if (size & 0x1) {
				blocksCnt++;
			}
			m_hwReg = uint16_t(blocksCnt << 10);
		}
	}
};

struct PacketBufTableEntry {
	union {
		volatile uint16_t ADDR_TX;
		volatile uint16_t ADDR_RX_0;
		volatile uint16_t ADDR_TX_0;
	};
private:
	uint16_t gap0;
public:
	union {
		volatile uint16_t COUNT_TX;
		PacketBufTableCountRxReg COUNT_RX_0;
		volatile uint16_t COUNT_TX_0;
	};
private:
	uint16_t gap1;
public:
	union {
		volatile uint16_t ADDR_RX;
		volatile uint16_t ADDR_RX_1;
		volatile uint16_t ADDR_TX_1;
	};
private:
	uint16_t gap2;
public:
	union {
		PacketBufTableCountRxReg COUNT_RX;
		PacketBufTableCountRxReg COUNT_RX_1;
		volatile uint16_t COUNT_TX_1;
	};
private:
	uint16_t gap3;
};

enum class EpNum : uint8_t {
	ep0,
	ep1,
	ep2,
	ep3,
	ep4,
	ep5,
	ep6,
	ep7
};

enum class EpType : uint32_t {
	BULK = 0x00'00'00'00,
	CTRL = 0x00'00'02'00,
	ISOC = 0x00'00'04'00,
	INTR = 0x00'00'06'00,
};

enum class EpTxStatus : uint32_t {
	DIS		= 0x00'00'00'00,
	STALL	= 0x00'00'00'10,
	NAK		= 0x00'00'00'20,
	VALID	= 0x00'00'00'30
};

enum class EpRxStatus : uint32_t {
	DIS		= 0x00'00'00'00,
	STALL	= 0x00'00'10'00,
	NAK		= 0x00'00'20'00,
	VALID	= 0x00'00'30'00
};


/*
===============================================================================

	EndPointReg

	Wrapper around EPxR registers, since this registers have non-trivial
	usage rules. For details see RM0008

===============================================================================
*/
class alignas(4) EndPointReg {
	volatile uint16_t m_hwReg;

public:
	EndPointReg() = delete;

	inline void ClearTxDTOG() noexcept {
		if (m_hwReg & USB_EP_DTOG_TX) {
			m_hwReg = USB_EP_CTR_RX | USB_EP_CTR_TX | USB_EP_DTOG_TX | (m_hwReg & USB_EPREG_MASK);
		}
	}

	inline void ClearRxDTOG() noexcept {
		if (m_hwReg & USB_EP_DTOG_RX) {
			m_hwReg = USB_EP_CTR_RX | USB_EP_CTR_TX | USB_EP_DTOG_RX | (m_hwReg & USB_EPREG_MASK);
		}
	}

	inline void SetTxStatus(const EpTxStatus status) noexcept {
		uint16_t tmp = m_hwReg & USB_EPTX_DTOGMASK;
		if (USB_EPTX_DTOG1 & uint32_t(status)) {
			tmp ^= USB_EPTX_DTOG1;
		}
		if (USB_EPTX_DTOG2 & uint32_t(status)) {
			tmp ^= USB_EPTX_DTOG2;
		}
		m_hwReg = tmp | USB_EP_CTR_RX | USB_EP_CTR_TX;
	}

	inline void SetRxStatus(const EpRxStatus status) noexcept {
		uint16_t tmp = m_hwReg & USB_EPRX_DTOGMASK;
		if (USB_EPRX_DTOG1 & uint32_t(status)) {
			tmp ^= USB_EPRX_DTOG1;
		}
		if (USB_EPRX_DTOG2 & uint32_t(status)) {
			tmp ^= USB_EPRX_DTOG2;
		}
		m_hwReg = tmp | USB_EP_CTR_RX | USB_EP_CTR_TX;
	}

	inline void SetAddress(const uint8_t addr) noexcept {
		m_hwReg = USB_EP_CTR_RX | USB_EP_CTR_TX | (m_hwReg & USB_EPREG_MASK) | addr;
	}

	inline void SetType(const EpType type) noexcept {
		m_hwReg = (m_hwReg & USB_EP_T_MASK) | uint32_t(type);
	}

	inline void ClearTxCTR() noexcept {
		m_hwReg &= (~USB_EP0R_CTR_TX) & USB_EPREG_MASK;
	}

	inline void ClearRxCTR() noexcept {
		m_hwReg &= (~USB_EP0R_CTR_RX) & USB_EPREG_MASK;
	}

	inline void SetStatusOut() {
		m_hwReg = USB_EP_CTR_RX | USB_EP_CTR_TX | ((m_hwReg | USB_EP_KIND) & USB_EPREG_MASK);
	}

	inline void ClearStatusOut() {
		m_hwReg = USB_EP_CTR_RX | USB_EP_CTR_TX | (m_hwReg & USB_EPKIND_MASK);
	}
};


static constexpr uint16_t g_pbtStart = 0;				//Start addres of Packet Buffer Table (relative to usb peripherial)
static constexpr uint8_t  g_ep0MaxPacketSize = 64;		//Max Packet size for endpoint 0
static constexpr uint16_t g_ep0RxBufferStart = (sizeof(PacketBufTableEntry) * 8) / 2;		//Start address of EP0 transmit buffer  (relative to usb peripherial)
static constexpr uint16_t g_ep0TxBufferStart = g_ep0RxBufferStart + g_ep0MaxPacketSize;		//Start address of EP0 receive buffer  (relative to usb peripherial)
static constexpr uint16_t g_freePMAStart = g_ep0TxBufferStart + g_ep0MaxPacketSize;			//Start address of free PMA


static_assert(!(g_pbtStart & 0b111), "Buffer table location must be aligned to 8 bytes!");
static_assert(!(g_ep0RxBufferStart & 1), "Buffers addresses must be aligned to 2 bytes!");
static_assert(	g_ep0MaxPacketSize == 64 || g_ep0MaxPacketSize == 32 || 
				g_ep0MaxPacketSize == 16 || g_ep0MaxPacketSize == 8, "Endpoint 0 max packet size can be only 8, 16, 32 or 64 bytes!");



//Use this pointer for access endpoint registers by number (as generic array; zero access overhead for constexpr index)
static EndPointReg * const EPR = reinterpret_cast<EndPointReg*>(USB);

//Packet Buffer Table
static PacketBufTableEntry * const PBT = reinterpret_cast<PacketBufTableEntry*>(USB_PMAADDR + g_pbtStart);


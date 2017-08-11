#pragma once

#include "usbdefs.hpp"
#include "usbHwRegs.hpp"


extern const USBDeviceDescriptor g_deviceDescriptor;		//Can be only one per physical device

const uint8_t *GetFullConfigDescriptor(uint16_t &len, const uint8_t cfgNum) noexcept;

#define RETURN_STRING(string) \
	len = sizeof string; \
	return reinterpret_cast<const uint8_t*>(&string);

const uint8_t *GetStringDescriptor(uint16_t &len, const uint8_t stringID, const LangID langID) noexcept;


class USBConfiguration {
public:
	USBConfiguration() = default;
	void OnSelect() noexcept;
	void OnTransferComplite(const uint8_t endpNum) noexcept;
};


void ReadPMA(uint8_t *out, const uint16_t PMABufAddr, const uint16_t byteCnt) noexcept;

void WritePMA(const uint8_t *in, const uint16_t PMABufAddr, const uint16_t byteCnt) noexcept;

void USBInit() noexcept;

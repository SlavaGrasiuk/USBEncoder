#include "Pipes.hpp"
#include "usbHwRegs.hpp"
#include "usb.hpp"


/*
==================
InterruptPipe::InitOUT
==================
*/
void InterruptPipe::InitOUT() {
	EPR[m_endpNum].SetType(EpType::INTR);
	EPR[m_endpNum].SetAddress(m_endpNum);
	EPR[m_endpNum].ClearRxDTOG();
	EPR[m_endpNum].SetRxStatus(EpRxStatus::VALID);

	PBT[m_endpNum].ADDR_RX = m_endpRxBufferStart;
	PBT[m_endpNum].COUNT_RX.SetMaxSize(m_endpRxBufferSize);
}

/*
==================
InterruptPipe::InitIN
==================
*/
void InterruptPipe::InitIN() {
	EPR[m_endpNum].SetType(EpType::INTR);
	EPR[m_endpNum].SetAddress(m_endpNum);
	EPR[m_endpNum].ClearTxDTOG();
	EPR[m_endpNum].SetTxStatus(EpTxStatus::NAK);

	PBT[m_endpNum].ADDR_TX = m_endpTxBufferStart;
}

/*
==================
InterruptPipe::SendIterruptData
==================
*/
void InterruptPipe::SendIterruptData(const uint8_t* data, const int size) {
	WritePMA(data, m_endpTxBufferStart, size < m_endpTxBufferSize ? size : m_endpTxBufferSize);
	PBT[m_endpNum].COUNT_TX = size < m_endpTxBufferSize ? size : m_endpTxBufferSize;
	EPR[m_endpNum].SetTxStatus(EpTxStatus::VALID);
}

/*
==================
InterruptPipe::VOnDataInStage
==================
*/
void InterruptPipe::VOnDataInStage() {

}

/*
==================
InterruptPipe::VOnDataOutStage
==================
*/
void InterruptPipe::VOnDataOutStage() {

}

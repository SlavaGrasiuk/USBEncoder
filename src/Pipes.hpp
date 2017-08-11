#pragma once

#include <cstdint>


struct Pipe {
	virtual void VOnDataInStage() = 0;
	virtual void VOnDataOutStage() = 0;
};


class InterruptPipe : public Pipe {
	const int m_endpNum;
	const int m_endpTxBufferStart, m_endpTxBufferSize;
	const int m_endpRxBufferStart, m_endpRxBufferSize;

public:
	constexpr InterruptPipe(const int endpNum, const int bufStartTx, const int bufSizeTx, const int bufStartRx, const int bufSizeRx) :
		m_endpNum(endpNum), 
		m_endpTxBufferStart(bufStartTx), m_endpTxBufferSize(bufSizeTx),
		m_endpRxBufferStart(bufStartRx), m_endpRxBufferSize(bufSizeRx) {}

	void InitIN();
	void InitOUT();
	void SendIterruptData(const uint8_t* data, const int size);

	virtual void VOnDataInStage() override;
	virtual void VOnDataOutStage() override;
};


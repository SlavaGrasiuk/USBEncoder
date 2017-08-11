#include "procsdef.hpp"
#include "commdef.hpp"
#include <stm32f1xx.h>
#include "usb.hpp"


/*
==================
TimInit

	Init timer 8 to generate interrupt on each encoder tick.
==================
*/
static void TimInit() {
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
	GPIOC->CRL &= ~(GPIO_CRL_CNF6_0 | GPIO_CRL_CNF7_0);		//PC6 and PC7 - input mode
	GPIOC->CRL |= GPIO_CRL_CNF6_1 | GPIO_CRL_CNF7_1;
	GPIOC->ODR |= GPIO_ODR_ODR6 | GPIO_ODR_ODR7;			//Pull-up for PC6 and PC7

	RCC->APB2ENR |= RCC_APB2ENR_TIM8EN;
	TIM8->CCMR1 |= TIM_CCMR1_IC1F | TIM_CCMR1_IC2F;			//Filter 1 and 2 - Fsample = DTS/32, N = 8
	TIM8->CCMR1 |= TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0;		//CC2 and CC1 as input, map IC2 -> TI2, IC1 -> TI1
	TIM8->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC2P);			//Capture is done on a rising edge of IC1 and IC2
	TIM8->SMCR |= TIM_SMCR_SMS_0 | TIM_SMCR_SMS_1;			//Counter counts up/down on both TI1FP1 and TI2FP2 edges
	TIM8->DIER |= TIM_DIER_UIE;								//Enable update interrupt
	NVIC_EnableIRQ(TIM8_UP_IRQn);
	TIM8->ARR = 4;											//Event will be generated on each mechanical tick
	TIM8->CNT = 0;
	TIM8->CR1 |= TIM_CR1_CEN;
}

static OS::message<uint32_t> g_event;

/*
==================
TIM8_UP_IRQHandler

	Here we can analyze encoder ticks, it's up or down?
==================
*/
extern "C" void TIM8_UP_IRQHandler() {
	TIM8->SR &= ~TIM_SR_UIF;

	if (TIM8->CR1 & TIM_CR1_DIR) {
		g_event = 0x00'00'E0'21;			//Volume down AT scancodes (press)
	} else {
		g_event = 0x00'00'E0'32;			//Volume up AT scancodes (press)
	}
	g_event.send_isr();
}


/*
==================
main
==================
*/
int main() {
	TimInit();
	USBInit();

	OS::run();
}

USBCtrlProc g_USBCtrlProc;

/*
==================
USBCtrlProc::exec
==================
*/
template<> void USBCtrlProc::exec() {
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

	GPIOA->CRH |= GPIO_CRH_MODE8_1 | GPIO_CRH_MODE9_1;		//Output mode, max speed 2 MHz
	GPIOA->CRH &= ~(GPIO_CRH_CNF8_0 | GPIO_CRH_CNF9_0);		//General purpose output push-pull
	
	for (;;) {
		g_event.wait();
		if (g_event == 0x00'00'E0'21) {
			GPIOA->ODR ^= GPIO_ODR_ODR8;
		} else if (g_event == 0x00'00'E0'32) {
			GPIOA->ODR ^= GPIO_ODR_ODR9;
		}
	}
}

/*------------------------------ Hardware Excpetion Handlers ------------------------------*/

#pragma region Hardware Exception Handlers
/*
==================
NMI_Handler
==================
*/
extern "C" void NMI_Handler(void) {
	EXCEPT_HNDL()
}

/*
==================
HardFault_Handler
==================
*/
extern "C" void HardFault_Handler() {
	EXCEPT_HNDL()
}

/*
==================
MemManage_Handler
==================
*/
extern "C" void MemManage_Handler() {
	EXCEPT_HNDL()
}

/*
==================
BusFault_Handler
==================
*/
extern "C" void BusFault_Handler() {
	EXCEPT_HNDL()
}

/*
==================
UsageFault_Handler
==================
*/
extern "C" void UsageFault_Handler() {
	EXCEPT_HNDL()
}
#pragma endregion

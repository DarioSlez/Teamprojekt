
#include "main.h"
#include <stm32l073xx.h>


int daten = 0;
static char daten_string[5];
uint8_t messung_fertig = 0;
uint8_t prozent = 0;






void led_init(){
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

	GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE5)
				| ((1u << GPIO_MODER_MODE5_Pos) & GPIO_MODER_MODE5_Msk);

	GPIOA->OTYPER &= ~GPIO_OTYPER_OT_5;

	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD5;



}

void led_toggle(){
	GPIOA->ODR |= GPIO_ODR_OD5;
}

void daten_to_string(int daten) {
	int i = 0;

	if(daten >= 1000)
		{//Temperatur größer gleich 1000?
			daten_string[i++] = (daten/1000)+48;//100er Stelle
			daten_string[i++] = ((daten%1000)/100)+48;//100er Stelle
			daten_string[i++] = (((daten%1000)%100)/10)+48;//10er Stelle
			daten_string[i++] = (daten%10)+48;//1er Stelle
		}
	else if(daten >= 100)
	{//Temperatur größer gleich 100?
		daten_string[i++] = (daten/100)+48;//100er Stelle
		daten_string[i++] = ((daten%100)/10)+48;//10er Stelle
		daten_string[i++] = (daten%10)+48;//1er Stelle
	}
	else if(daten >= 10)
	{//Temperatur größer gleich 10?
		daten_string[i++] = (daten/10)+48;//10er Stelle
		daten_string[i++] = (daten%10)+48;//1er Stelle
	}
	else
	{//wenn nicht größer gleich 10
		daten_string[i++] = (daten%10)+48;// nur 1er Stelle
	}
}

void feuchtigkeit (int daten)
{
	if(daten >= 2780)
	{
		prozent = 0;
	}
	else if (daten >= 1180)
	{
		prozent = (2780-daten)/16;
	}
	else if (daten <= 1180)
	{
		prozent = 100;
	}
}


void daten_an_pc(void)
{

	int8_t Eeprom_data1 = *(uint8_t *)(0x08080000);
	daten = ADC1->DR;
	daten_to_string(daten);
	feuchtigkeit(daten);
	USART2->TDR = prozent;
}

void RTC_IRQHandler(void)
	{

		/*
		ADC1->CR |= ADC_CR_ADSTART;//Kalkulation starten?
		while((ADC1->ISR & ADC_ISR_EOCAL) == 0)
		{
			//warten bis adc daten gelesen hat
		}
		ADC1->ISR |= ADC_ISR_EOCAL;
		while((DMA1->ISR & DMA_ISR_TCIF1) == 0)
		{
			//warten bis dma daten gespeichert hat
		}
		DMA1->IFCR |= DMA_IFCR_CTCIF1;
		*/
		if(EXTI_PR_PIF20 != 0){
			//GPIOA->ODR ^= GPIO_ODR_OD5;		//LED zum testen
			ADC1->CR |= ADC_CR_ADSTART;			//Kalkulation starten
			while((ADC1->ISR & ADC_ISR_EOCAL) == 0){
				//warten bis adc daten gelesen hat
			}
			daten = ADC1->DR;
			daten_to_string(daten);
			feuchtigkeit(daten);
			USART2->TDR = prozent;

			RTC->WPR = 0xCA;				//Write Protection Disable
			RTC->WPR = 0x53;				// s.o.
			RTC->ISR = 0x200;				//Wakeup timer flag reset
			RTC->WPR = 0xCA;				//Write Protection Enable
			RTC->WPR = 0x63;				// s.o.
			EXTI->PR &= EXTI_PR_PIF20;		//Pending interrupt flag on line 20 set
		}

	}

void clock_setup_16MHz(void) {
		// turn on HSI
		RCC->CR |= RCC_CR_HSION;
		while (!(RCC->CR & RCC_CR_HSIRDY)) {
			// wait for HSI ready
		}

		// set prescale factors for peripheral clocks
		RCC->CFGR = (RCC->CFGR
				& ~(RCC_CFGR_HPRE_Msk | RCC_CFGR_PPRE1_Msk | RCC_CFGR_PPRE2_Msk))
				| (RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE1_DIV1 | RCC_CFGR_PPRE2_DIV1);

		// power settings
		RCC->APB1ENR |= RCC_APB1ENR_PWREN; // enable PWR clock
		PWR->CR = (PWR->CR & ~PWR_CR_VOS)
				| ((2u << PWR_CR_VOS_Pos) & PWR_CR_VOS_Msk); // voltage scale 2: medium power
		FLASH->ACR |= FLASH_ACR_LATENCY; // 1 flash wait state

		RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_HSI;

		SystemCoreClockUpdate(); // updates cmsis global state
	}

int main(void) {
	clock_setup_16MHz();
	team_setup();
	//led_init();					//LED zum testen
	//usart_setup();
	sensor_setup();
	//GPIOA->ODR ^= GPIO_ODR_OD5;	//LED zum testen
	auto_calc();
	usart_setup();
	rtc_init();



	go_to_sleep();
	while (1) {




	}
}

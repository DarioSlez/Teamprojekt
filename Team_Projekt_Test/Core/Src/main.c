/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>
#include <string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define PCF8523_Address 0x68//0xD0 // I2C Adresse oder 0x68 ?
#define Control_1 0x00
#define Control_2 0x01
#define Control_3 0x02
#define Tmr_CLKOUT_ctrl 0x0F


#define Tmr_A_freq_ctrl 0x10
#define Tmr_A_reg 0x11


#define PCF8523_TIMER_B_FRCTL 0x12
#define PCF8523_TIMER_B_VALUE 0x13
#define PCF8523_OFFSET 0x0E
#define PCF8523_STATUSREG 0x03


#define RV3028C7_Address 0x52 //oder 0xA4
#define RV_Timer_Val_0 0x0A
#define RV_Timer_Val_1 0x0B
#define RV_Status 0x0E
#define RV_Control_1 0x0F
#define RV_Control_2 0x10
#define RV_EEPROM_Backup 0x37



/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc;

I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

int data = 0;
char msg[20];
uint8_t prozent = 0;
uint8_t reset = 0x00;
uint8_t pcf_reset = 0x58;

/******************************PCF_RTC******************************************/

uint8_t pcf_freq = 0x02;
uint8_t pcf_timr = 0x0A;
uint8_t InEn = 0x02;
uint8_t con = 0xBA;

/* Diese Werte sind für den Modus B!!! Die oberen Werte dann auskommentieren.
*/
/*
uint8_t pcf_freq = 0x32; //0x02=46ms, 0x12=62.5ms, 0x22=78.125ms, 0x32=93.75ms , 0x42=125ms
uint8_t pcf_timr = 0x0A;
uint8_t InEn = 0x01;
uint8_t con = 0x79;
*/

/******************************RV_RTC******************************************/




/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_ADC_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

typedef enum {
	RTC_RV_MODE,
	RTC_PCF_MODE_A,
	RTC_PCF_MODE_B,
	DATA_MODE,
	SLEEP_MODE

} Mode;



void RV3028C7_RTC(uint8_t timer_val)
{
	/*******************Test um zu sehen ob I2C Verbindung vorhanden ist********************/
	  if(HAL_I2C_IsDeviceReady(&hi2c1, RV3028C7_Address << 1, 2, 100) == HAL_OK)
	  {
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
		  HAL_Delay(1000);
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
		  HAL_Delay(1000);
	  }
	  else{
		  	  return;
		  //wenn nicht Verbunden dann Fehler
	  }

	/*********************************RESET******************************************/
	uint8_t TE;
	HAL_I2C_Mem_Read(&hi2c1, RV3028C7_Address << 1, RV_Control_1, 1, &TE, 1, 1000); // lesen von register
	TE &= ~(1 << 2);
	HAL_I2C_Mem_Write(&hi2c1, RV3028C7_Address << 1, RV_Control_1, 1,  &TE, 1, 1000);

	uint8_t TIE;
	HAL_I2C_Mem_Read(&hi2c1, RV3028C7_Address << 1, RV_Control_2, 1, &TIE, 1, 1000); // lesen von register
	TIE &= ~(1 << 4);
	HAL_I2C_Mem_Write(&hi2c1, RV3028C7_Address << 1, RV_Control_2, 1,  &TIE, 1, 1000);

	uint8_t TF;
	HAL_I2C_Mem_Read(&hi2c1, RV3028C7_Address << 1, RV_Status, 1, &TF, 1, 1000); // lesen von register
	TF &= ~(1 << 3);
	HAL_I2C_Mem_Write(&hi2c1, RV3028C7_Address << 1, RV_Status, 1,  &TF, 1, 1000);

	/*********************************INIT******************************************/
	uint8_t TRPT;
	HAL_I2C_Mem_Read(&hi2c1, RV3028C7_Address << 1, RV_Control_1, 1, &TRPT, 1, 1000); // lesen von register
	TRPT |= (1 << 7);
	HAL_I2C_Mem_Write(&hi2c1, RV3028C7_Address << 1, RV_Control_1, 1,  &TRPT, 1, 1000);

	uint8_t TD;
	HAL_I2C_Mem_Read(&hi2c1, RV3028C7_Address << 1, RV_Control_1, 1, &TD, 1, 1000); // lesen von register
	TD |= 0x02;
	HAL_I2C_Mem_Write(&hi2c1, RV3028C7_Address << 1, RV_Control_1, 1,  &TD, 1, 1000);


	HAL_I2C_Mem_Write(&hi2c1, RV3028C7_Address << 1, RV_Timer_Val_0, 1,  &timer_val, 1, 1000);


	uint8_t TIEM;
	HAL_I2C_Mem_Read(&hi2c1, RV3028C7_Address << 1, RV_Control_2, 1, &TIEM, 1, 1000); // lesen von register
	TIEM |= (1 << 4);
	HAL_I2C_Mem_Write(&hi2c1, RV3028C7_Address << 1, RV_Control_2, 1,  &TIEM, 1, 1000);

	uint8_t TEM;
	HAL_I2C_Mem_Read(&hi2c1, RV3028C7_Address << 1, RV_Control_1, 1, &TEM, 1, 1000); // lesen von register
	TEM |= (1 << 2);
	HAL_I2C_Mem_Write(&hi2c1, RV3028C7_Address << 1, RV_Control_1, 1,  &TEM, 1, 1000);
	/*********************************BACKUP******************************************/
	uint8_t backup = 0x00;
	HAL_I2C_Mem_Write(&hi2c1, RV3028C7_Address << 1, RV_EEPROM_Backup, 1,  &backup, 1, 1000); //reset

	backup |= (1 << 2);
	HAL_I2C_Mem_Write(&hi2c1, RV3028C7_Address << 1, RV_EEPROM_Backup, 1,  &backup, 1, 1000);

}


/*
 * Diese Funktion wird benötigt um die PCF8523-RTC zu programmieren. Es wird Hier unterschieden zwischen
 * Modus A und B. Der Vorteil der Interrupt Generierung durch Timer-A ist, dass der Interrupt die
 * geringste Dauer hat. Der durch Timer-B verursachte Interrupt hat jedoch eine variable Dauer die von
 * 46,875ms bis 218,750ms reicht. Für unterschiedliche MCUs und Anwendungsgebiete kann dieser Timer
 * sinnvoll sein. Neben dem Modi werden weitere Parameter wie Frequenz und Anfangswert des Timers
 * um das Interrupt Intervall einzustellen.
 */
void PCF_RTC(uint8_t val, uint8_t freq, uint8_t inter, uint8_t control, Mode mode)
{
	/*******************Test um zu sehen ob I2C Verbindung vorhanden ist********************/
	  if(HAL_I2C_IsDeviceReady(&hi2c1, PCF8523_Address << 1, 2, 100) == HAL_OK)
	  {
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
		  HAL_Delay(1000);
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
		  HAL_Delay(1000);
	  }
	  else{
		  	  return;
		  //wenn nicht Verbunden dann Fehler
	  }

	/*********************************RESET******************************************/
	HAL_I2C_Mem_Write(&hi2c1, PCF8523_Address << 1, Control_1, 1, &pcf_reset, 1, 1000);
	/*********************************Interrupt Enable******************************************/

	HAL_I2C_Mem_Write(&hi2c1, PCF8523_Address << 1, Control_2, 1,  &inter, 1, 1000); // schreiben des registers mit gesetztem bit
	/*********************************Modus******************************************/

	HAL_I2C_Mem_Write(&hi2c1, PCF8523_Address << 1, Tmr_CLKOUT_ctrl, 1,  &control, 1, 1000);


	if (mode == RTC_PCF_MODE_A)
	{
		HAL_I2C_Mem_Write(&hi2c1, PCF8523_Address << 1, Tmr_A_freq_ctrl, 1, &freq, 1, 1000); // schreiben von frequenz register
		HAL_I2C_Mem_Write(&hi2c1, PCF8523_Address << 1, Tmr_A_reg, 1, &val, 1, 1000);
	}
	else if (mode == RTC_PCF_MODE_B)
	{
		HAL_I2C_Mem_Write(&hi2c1, PCF8523_Address << 1, PCF8523_TIMER_B_FRCTL, 1, &freq, 1, 1000); // schreiben von frequenz register
		HAL_I2C_Mem_Write(&hi2c1, PCF8523_Address << 1, PCF8523_TIMER_B_VALUE, 1, &val, 1, 1000);
	}
	else
	{
		HAL_I2C_Mem_Write(&hi2c1, PCF8523_Address << 1, Control_1, 1, &pcf_reset, 1, 1000);
		return;
	}
	/*********************************Batterie******************************************/

	uint8_t battery = 0x00; // Von Hand resetten!
	HAL_I2C_Mem_Write(&hi2c1, PCF8523_Address << 1, Control_3, 1,  &battery, 1, 1000); //reset

	battery |= (1 << 7);
	HAL_I2C_Mem_Write(&hi2c1, PCF8523_Address << 1, Control_3, 1, &battery, 1, 1000); // batterie mode aktiviert

}

/*
 *Go_To_Standby() ist für den Standby-Modus des Stm32 da. Dieser Modus wurde nur für
 *Messungen benutzt um eine Schlussvolgerung von Standby-Modus und STFO-Schaltung zu
 *ziehen. PA0 dient hierbei als Wake-Up-Pin. Der Mikrocontroller geht in den Standby-
 *Modus. Er wacht auf, falls ein Interrupt auf dem Wake-Up-Pin empfangen wird.
 */
void Go_To_Standby()
{

	  if(__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
	  {
		  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);


		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
		  HAL_Delay(500);
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
		  HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1); //disable PA0
	  }

	  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

	  HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);

	  HAL_PWR_EnterSTANDBYMode();
}

void Go_To_Stop()
{


	for (int i=0; i<20; i++)
	{
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
		HAL_Delay(200);
	}

	HAL_SuspendTick();
	HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

	SystemClock_Config();
	HAL_ResumeTick();
	for (int i=0; i<5; i++)
	{
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
		HAL_Delay(2000);
	}


}

/*
 * Die Funktion Feuchtigkeit() wandelt Messdaten des Feuchtigkeitssensors in Prozentuale
 * Messdaten um. Bei neuen Umgebungsbedingungen muss der Sensor neu kalibriert werden.
 * Bei der jetzigen Kalibrierung steht ein Messwert von 2780 für eine Feuchtigkeit von
 * 0% und ein Wert von 1180 für 100%.
 */
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

/*
 * In Messung wird zuerst die Selbsthaltung auf PC0 gesetzt. Danach werden Messdaten
 * über den ADC gesammelt, durch die Funktion feuchtigkeit() weiterverarbeitet und
 * über eine UART-Schnittstelle versendet. Nach dem Verarbeiten der Daten wird der
 * Pin PC0 auf Low gesetzt und somit die Selbsthaltung ausgeschaltet.
 */
void Messung()
{


	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);

	for (int i=0; i<3; i++)
	{
		HAL_ADC_Start(&hadc);
		HAL_ADC_PollForConversion(&hadc, 20);
		data = HAL_ADC_GetValue(&hadc);
		feuchtigkeit(data);
		sprintf(msg, "Feuchtigkeit %hu \r\n", prozent);
		HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
		HAL_Delay(10);
	}

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);


}

/*
 * chooseMode() wird nach der Initialisierung der Peripherie aufgerufen. Hier wird entschieden
 * welche Funktion der MCU ausführen soll. In den beiden RTC-Modi wird jeweils eine RTC
 * programmiert und die Interrupt-Funktion der RTC getestet. Der Sleep-Modus ist nur zum Vergleich
 * von Standby und STFO-Schaltung da. Data-Modus wird benutzt um die Selbsthaltung zu schalten
 * sowie die Daten des Feuchtigkeitssensors auszulesen. Dieser Modus wird benutzt, wenn der MCU
 * mit der STFO-Schaltung verbunden ist.
 */
void chooseMode(Mode m)
{

	  switch (m){

	  case RTC_RV_MODE:
		  RV3028C7_RTC(0x0A);
		  break;

	  case RTC_PCF_MODE_A:
		  PCF_RTC(pcf_timr, pcf_freq, InEn, con, RTC_PCF_MODE_A);
		  break;

	  case RTC_PCF_MODE_B:
		  PCF_RTC(pcf_timr, pcf_freq, InEn, con, RTC_PCF_MODE_B);
		  break;

	  case DATA_MODE:
		  Messung();
		  break;

	  case SLEEP_MODE:
		  Go_To_Standby();
		  break;

	  default:

		  break;

	  }


}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_ADC_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */




 chooseMode(RTC_PCF_MODE_A);




  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_3;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC_Init(void)
{

  /* USER CODE BEGIN ADC_Init 0 */

  /* USER CODE END ADC_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC_Init 1 */

  /* USER CODE END ADC_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc.Instance = ADC1;
  hadc.Init.OversamplingMode = DISABLE;
  hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ContinuousConvMode = DISABLE;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.DMAContinuousRequests = DISABLE;
  hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.LowPowerFrequencyMode = DISABLE;
  hadc.Init.LowPowerAutoPowerOff = DISABLE;
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC_Init 2 */

  /* USER CODE END ADC_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00303D5B;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PC1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

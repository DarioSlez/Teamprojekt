/* All rights reserved.
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
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

uint8_t buffer[6];
uint8_t zwei = 0x02;
uint8_t dreißig = 0x05;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

typedef enum {
	RTC_RV_MODE,
	RTC_PCF_MODE,
	DATA_MODE,
	SLEEP_MODE

} Mode;







void RV3028C7_Backup()
{
	uint8_t backup = 0x00;
	HAL_I2C_Mem_Write(&hi2c1, RV3028C7_Address << 1, RV_EEPROM_Backup, 1,  &backup, 1, 1000); //reset

	backup |= (1 << 2);
	HAL_I2C_Mem_Write(&hi2c1, RV3028C7_Address << 1, RV_EEPROM_Backup, 1,  &backup, 1, 1000);
}

void RV3028C7_Reset()
{
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

}


void RV3028C7_Init(uint8_t timer_val)
{

	uint8_t TRPT;
	HAL_I2C_Mem_Read(&hi2c1, RV3028C7_Address << 1, RV_Control_1, 1, &TRPT, 1, 1000); // lesen von register
	TRPT |= (1 << 7);
	HAL_I2C_Mem_Write(&hi2c1, RV3028C7_Address << 1, RV_Control_1, 1,  &TRPT, 1, 1000);

	uint8_t TD;
	HAL_I2C_Mem_Read(&hi2c1, RV3028C7_Address << 1, RV_Control_1, 1, &TD, 1, 1000); // lesen von register
	TD |= 0x02;
	HAL_I2C_Mem_Write(&hi2c1, RV3028C7_Address << 1, RV_Control_1, 1,  &TD, 1, 1000);


	HAL_I2C_Mem_Write(&hi2c1, RV3028C7_Address << 1, RV_Timer_Val_0, 1,  &timer_val, 1, 1000);


	uint8_t TIE;
	HAL_I2C_Mem_Read(&hi2c1, RV3028C7_Address << 1, RV_Control_2, 1, &TIE, 1, 1000); // lesen von register
	TIE |= (1 << 4);
	HAL_I2C_Mem_Write(&hi2c1, RV3028C7_Address << 1, RV_Control_2, 1,  &TIE, 1, 1000);

	uint8_t TE;
	HAL_I2C_Mem_Read(&hi2c1, RV3028C7_Address << 1, RV_Control_1, 1, &TE, 1, 1000); // lesen von register
	TE |= (1 << 2);
	HAL_I2C_Mem_Write(&hi2c1, RV3028C7_Address << 1, RV_Control_1, 1,  &TE, 1, 1000);



}

void TimerA_Enable_interrupt()
{
	uint8_t EnIn;

	HAL_I2C_Mem_Read(&hi2c1, PCF8523_Address << 1, Control_2, 1, &EnIn, 1, 1000); // lesen von register

	EnIn |= 0x02; // setzen von interrupt enable bit

	HAL_I2C_Mem_Write(&hi2c1, PCF8523_Address << 1, Control_2, 1,  &EnIn, 1, 1000); // schreiben des registers mit gesetztem bit
}


void TimerA_Init(uint8_t val)
{
	uint8_t reset = 0x00;
	HAL_I2C_Mem_Write(&hi2c1, PCF8523_Address << 1, Tmr_CLKOUT_ctrl, 1,  &reset, 1, 1000); //reset


	uint8_t control;
	HAL_I2C_Mem_Read(&hi2c1, PCF8523_Address << 1, Tmr_CLKOUT_ctrl, 1, &control, 1, 1000); // lesen von clock out register
	control |= 0xBA;// clkout deaktiviert, pulsed interrupt für timerA, timerA als countdown timer

	HAL_I2C_Mem_Write(&hi2c1, PCF8523_Address << 1, Tmr_CLKOUT_ctrl, 1,  &control, 1, 1000);


	uint8_t freq = 0x02; // Frequenz 1 Hz
	HAL_I2C_Mem_Write(&hi2c1, PCF8523_Address << 1, Tmr_A_freq_ctrl, 1, &freq, 1, 1000); // schreiben von frequenz register

	uint8_t value = val; // Wert fürs herunterzählen setzen
	HAL_I2C_Mem_Write(&hi2c1, PCF8523_Address << 1, Tmr_A_reg, 1, &value, 1, 1000);

}

void TimerB_Enable_interrupt()
{
	uint8_t EnInB;

	HAL_I2C_Mem_Read(&hi2c1, PCF8523_Address << 1, Control_2, 1, &EnInB, 1, 1000); // lesen von register

	EnInB |= 0x01; // setzen von interrupt enable bit

	HAL_I2C_Mem_Write(&hi2c1, PCF8523_Address << 1, Control_2, 1,  &EnInB, 1, 1000); // schreiben des registers mit gesetztem bit
}

void TimerB_Init(uint8_t val)
{
	uint8_t B_reset = 0x00;
	HAL_I2C_Mem_Write(&hi2c1, PCF8523_Address << 1, Tmr_CLKOUT_ctrl, 1,  &B_reset, 1, 1000); //reset


	uint8_t B_control;
	HAL_I2C_Mem_Read(&hi2c1, PCF8523_Address << 1, Tmr_CLKOUT_ctrl, 1, &B_control, 1, 1000); // lesen von clock out register
	B_control |= 0x79; // clkout deaktiviert, pulsed interrupt für timerA, timerA als countdown timer
	HAL_I2C_Mem_Write(&hi2c1, PCF8523_Address << 1, Tmr_CLKOUT_ctrl, 1,  &B_control, 1, 1000);




	uint8_t B_freq = 0x02; // clkout deaktiviert, pulsed interrupt für timerA, timerA als countdown timer
	HAL_I2C_Mem_Write(&hi2c1, PCF8523_Address << 1, PCF8523_TIMER_B_FRCTL, 1,  &B_freq, 1, 1000);


	uint8_t B_value = val; // Wert fürs herunterzählen setzen
	HAL_I2C_Mem_Write(&hi2c1, PCF8523_Address << 1, PCF8523_TIMER_B_VALUE, 1, &B_value, 1, 1000);

}

void Bat_Mode()
{

	uint8_t battery = 0x00;
	HAL_I2C_Mem_Write(&hi2c1, PCF8523_Address << 1, Control_3, 1,  &battery, 1, 1000); //reset

	//uint8_t battery;
	//HAL_I2C_Mem_Read(&hi2c1, PCF8523_Address << 1, Control_3, 1,  &battery, 1, 1000); //control 3 register lesen

	battery |= (1 << 7);

	HAL_I2C_Mem_Write(&hi2c1, PCF8523_Address << 1, Control_3, 1, &battery, 1, 1000); // batterie mode aktiviert
}


void Go_To_Standby()
{

	  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);



	  HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);





	  HAL_PWR_EnterSTANDBYMode();
}


void Reset_PCF(void)
{
	uint8_t pcf_reset = 0x58;

	HAL_I2C_Mem_Write(&hi2c1, PCF8523_Address << 1, Control_1, 1, &pcf_reset, 1, 1000);
}



void chooseMode(Mode m)
{

	  switch (m){

	  case RTC_RV_MODE:


	  case RTC_PCF_MODE:


	  case DATA_MODE:


	  case SLEEP_MODE:


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

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
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
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */






/*******************************************************************Standby***************************************************************************************/

/*
  if(__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
  {
	  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);


	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
	  HAL_Delay(500);
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	  HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1); //disable PA0


  }

  Go_To_Standby();
*/




  /*******************************************************************Clock***************************************************************************************/



  //Clock_Devider();


  /*******************************************************************RTC***************************************************************************************/

  //PCF8523_Address oder RV3028C7_Address PCF8523_Address
  /*
  if(HAL_I2C_IsDeviceReady(&hi2c1, RV3028C7_Address << 1, 2, 100) == HAL_OK)
  {
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
	  HAL_Delay(1000);
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	  HAL_Delay(1000);
  }

  uint8_t periode = 10;
  RV3028C7_Reset();
  RV3028C7_Init(periode);
  RV3028C7_Backup();
*/
/*
  Reset_PCF();

  TimerA_Enable_interrupt();
  TimerA_Init(0x0A);
  Bat_Mode();
*/

/*
  TimerB_Enable_interrupt();
  TimerB_Init(0x0A);
  Bat_Mode();
*/


  /**********************************************************************************************************************************************************/



  //I2C_Init();
  //Battery_Mode();
  //Set_Time(0);
  uint8_t interrupt;
  //buffer[0] = 0x02;



  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */




	  interrupt = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4);

	  if (interrupt == 0)
	  {



		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
		  HAL_Delay(1000);//1000
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
		  //HAL_Delay(1000);

	  }

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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */

static void MX_I2C1_Init(void)
{


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


  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }


  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }


}


/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */

static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);

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

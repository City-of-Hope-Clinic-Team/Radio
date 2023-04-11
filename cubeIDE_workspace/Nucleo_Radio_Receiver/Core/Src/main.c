/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "string.h"
#include "nRF24L01.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
uint8_t spiReadByte(uint8_t address);
void spiWriteByte(uint8_t address, uint8_t value);
void spiReadNByte(uint8_t reg, uint8_t *buf, uint8_t len);
void spiWriteNByte(uint8_t address, const uint8_t *buf, uint8_t len);
void setupTXsimple(void);
void checkTXsimple(uint8_t *buf, uint8_t *buf2 );
void transmitByte(uint8_t data);
void transmitBytes(uint8_t* txdata, uint8_t len);
void flushTXFIFO(void);
void flushRXFIFO(void);
void setupRXsimple();
uint8_t receiveByte();
void receiveBytes(uint8_t* rxdata, uint8_t len);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  setupRXsimple(); // Configure as a receiver

  HAL_GPIO_WritePin(GPIOA, CSN_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOA, CE_Pin, GPIO_PIN_SET);

  uint8_t receivedDataBuffer[6];
  uint8_t checkStatus;
  int16_t xval, yval, zval;
  char msg[80];
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	HAL_Delay(100);
	checkStatus = spiReadByte(REG_STATUS);
	if (checkStatus & (1 << MASK_RX_DR))
	{
		//receivedByte = receiveByte();
		receiveBytes(receivedDataBuffer, 6);

		// Parse the received acceleration data
		//xval = (receivedDataBuffer[1] << 4) + (receivedDataBuffer[0] >> 4);
		xval = (receivedDataBuffer[1]<<8) | (receivedDataBuffer[0]);
		//alternative
		//
		//yval = (receivedDataBuffer[3] << 4) + (receivedDataBuffer[2] >> 4);
		yval = (receivedDataBuffer[3]<<8) | (receivedDataBuffer[2]);
		//zval = (receivedDataBuffer[5] << 4) + (receivedDataBuffer[4] >> 4);
		zval = (receivedDataBuffer[5]<<8) | (receivedDataBuffer[4]);

		sprintf(msg, "%d,%d,%d\n", xval/16, yval/16, zval/16);
		HAL_UART_Transmit(&huart2, msg, strlen(msg), 100);

		checkStatus = spiReadByte(REG_STATUS);
		// Clear RX_DR flag
		spiWriteByte(REG_STATUS, 0x40); // Assert bit 6 of Status high (clear RX_DR flag)
		// Flush RX FIFO
		flushRXFIFO();
		checkStatus = spiReadByte(REG_STATUS);
		// Stack overflow recommends reconfiguring as receiver here
		// I think it'll be fine, but that's another thing to try.
		// Source: https://stackoverflow.com/questions/51810883/nrf24l01-rx-mode-and-flush
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

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

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
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, CE_Pin|CSN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : CE_Pin CSN_Pin */
  GPIO_InitStruct.Pin = CE_Pin|CSN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
uint8_t spitxbuffer[8];
uint8_t spirxbuffer[8];
uint8_t spiReadByte(uint8_t address)
{
	spitxbuffer[0] = address;
	spitxbuffer[1] = 0;

	HAL_GPIO_WritePin(GPIOA, CSN_Pin, GPIO_PIN_RESET); // put CSN = 0
	HAL_SPI_TransmitReceive(&hspi1, spitxbuffer, spirxbuffer, 2*sizeof(uint8_t), HAL_MAX_DELAY);
	HAL_GPIO_WritePin(GPIOA, CSN_Pin, GPIO_PIN_SET); // put CSN = 0
	return spirxbuffer[1];

}

void spiWriteByte(uint8_t address, uint8_t value)
{
	spitxbuffer[0] = address | 1<<5; // need to set 3 msb to 1 for writes
	spitxbuffer[1] = value;

	HAL_GPIO_WritePin(GPIOA, CSN_Pin, GPIO_PIN_RESET); // put CSN = 0
	HAL_SPI_TransmitReceive(&hspi1, spitxbuffer, spirxbuffer, 2, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(GPIOA, CSN_Pin, GPIO_PIN_SET); // put CSN = 0


}

void spiWriteNByte(uint8_t address, const uint8_t *buf, uint8_t len)
{
	spitxbuffer[0] = address | 1 <<5;
	spitxbuffer[1] = 0;
	//Put CSN low
	HAL_GPIO_WritePin(GPIOA, CSN_Pin, GPIO_PIN_RESET); // put CSN = 0
	//Transmit register address
	HAL_SPI_Transmit(&hspi1, spitxbuffer, 1, HAL_MAX_DELAY);	//Receive data
	HAL_SPI_Transmit(&hspi1, (uint8_t *)buf, len, HAL_MAX_DELAY);
	//Bring CSN high
	HAL_GPIO_WritePin(GPIOA, CSN_Pin, GPIO_PIN_SET); // put CSN = 1
}

void setupTXsimple(void)
{
	spiWriteByte(REG_SETUP_RETR, 0x00); // no retransmission / acknowledgment
	spiWriteByte(REG_EN_AA, 0x00); //no auto acknowledge
	spiWriteByte(REG_SETUP_AW, 0x02); // makes address width 4 bytes
	spiWriteByte(REG_RF_CH, 0x02);// make frequency 2.402 Ghz
	spiWriteByte(REG_RF_SETUP, 0x26); // lowest data rate + highest output pwr

	// What is our TX_ADDRESS
	uint8_t TxAddr[4] = {0x00, 0x11, 0x22, 0x33};
	spiWriteNByte(REG_TX_ADDR, TxAddr, 4);
	// RX address is the same as TX address
	spiWriteNByte(REG_RX_ADDR_P0, TxAddr, 4);

	// Receive Pipes not used
	spiWriteByte(REG_EN_RXADDR, 0x01); // Only enable pipe 0 [bit 0 = 1]
	spiWriteByte(REG_RX_PW_P0, 0x01); // 1 byte payload length on pipe 0

	// no shockburst stuff
	spiWriteByte(REG_DYNPD, 0x00);
	spiWriteByte(REG_FEATURE, 0x00);

	// TX mode
	// Enable error correction, power up, PRIM_RX = 0 [PTX]
	spiWriteByte(REG_CONFIG, 0x0A);
}

void setupRXsimple(void)
{
	spiWriteByte(REG_SETUP_RETR, 0x00); // no retransmission / acknowledgment
	spiWriteByte(REG_EN_AA, 0x00); //no auto acknowledge
	spiWriteByte(REG_SETUP_AW, 0x02); // makes address width 4 bytes
	spiWriteByte(REG_RF_CH, 0x02);// make frequency 2.402 Ghz
	spiWriteByte(REG_RF_SETUP, 0x26); // lowest data rate + highest output pwr

	// Address for the radio that we expect to be receiving data from
	uint8_t TxAddr[4] = {0x00, 0x11, 0x22, 0x33};
	// The unique address for data pipe 0 (see page 39 of datasheet)
	uint8_t Rx_0_Addr[4] = {0x78, 0x78, 0x78, 0x78};

	// Configure RX pipe 0 to the unique address, and pipe 1 to our transmitter address
	spiWriteNByte(REG_RX_ADDR_P0, Rx_0_Addr, 4);
	spiWriteNByte(REG_RX_ADDR_P1, TxAddr, 4);

	// Use channel 1
	// Note: I'm note sure if we need to enable pipe 0 here, since transmission is over pipe 1. Enabling it to be safe.
	spiWriteByte(REG_EN_RXADDR, 0x03); // Enable pipes 0 and 1 [bit 0 and 1 = 1]
	spiWriteByte(REG_RX_PW_P1, 0x06); // 6 byte payload length on pipe 1

	// no shockburst stuff
	spiWriteByte(REG_DYNPD, 0x00);
	spiWriteByte(REG_FEATURE, 0x00);

	// RX mode
	// Enable error correction [bit 3], power up [bit 1], PRIM_RX = 1 [PRX] [bit 0]
	spiWriteByte(REG_CONFIG, 0x0B);
}

void checkTXsimple(uint8_t *buf, uint8_t *buf2 )
{

	buf[0] = spiReadByte(REG_SETUP_RETR);
	buf[13] = spiReadByte(REG_EN_AA); //no auto acknowledge
	buf[1] = spiReadByte(REG_SETUP_AW); // makes address width 4 bytes
	buf[2] = spiReadByte(REG_RF_CH);// make frequency 2.402 Ghz
	buf[3] = spiReadByte(REG_RF_SETUP); // lowest data rate + highest output pwr

	// What is our TX_ADDRESS
	spiReadNByte(REG_TX_ADDR, (uint8_t *)buf2, 4);

	// Receive Pipes not used
	buf[4] =  spiReadByte(REG_RX_PW_P0);
	buf[5] =  spiReadByte(REG_RX_PW_P1);
	buf[6] =  spiReadByte(REG_RX_PW_P2);
	buf[7] =  spiReadByte(REG_RX_PW_P3);
	buf[8] =  spiReadByte(REG_RX_PW_P4);
	buf[9] =  spiReadByte(REG_RX_PW_P5);

	// no shockburst stuff
	buf[10] =  spiReadByte(REG_DYNPD);
	buf[11] =  spiReadByte(REG_FEATURE);
	buf[12] =  spiReadByte(REG_CONFIG);
}

void transmitByte(uint8_t data)
{
	uint8_t txData[2];
	txData[0] = 0xB0; // using the W_TX_PAYLOAD_NOACK cmd
	txData[1] = data;

	HAL_GPIO_WritePin(GPIOA, CSN_Pin, GPIO_PIN_RESET); // put CSN = 0
	// Send W_TX_Command
	HAL_SPI_Transmit(&hspi1, txData, 2, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(GPIOA, CSN_Pin, GPIO_PIN_SET); // put CSN = 1
}

void transmitBytes(uint8_t* txdata, uint8_t len)
{
	uint8_t txPayload = 0xB0;
	HAL_GPIO_WritePin(GPIOA, CSN_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, &txPayload, 1, HAL_MAX_DELAY);
	HAL_SPI_Transmit(&hspi1, txdata, len, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(GPIOA, CSN_Pin, GPIO_PIN_SET);
}

uint8_t receiveByte()
{
	uint8_t txData[2];
	uint8_t rxData[2];
	txData[0] = 0x61; // using the R_RX_PAYLOAD command
	txData[1] = 0x00;
	HAL_GPIO_WritePin(GPIOA, CSN_Pin, GPIO_PIN_RESET); // put CSN = 0
	// Send R_RX_PAYLOAD
	HAL_SPI_TransmitReceive(&hspi1, txData, rxData, 2*sizeof(uint8_t), HAL_MAX_DELAY);
	HAL_GPIO_WritePin(GPIOA, CSN_Pin, GPIO_PIN_SET); // put CSN = 1
	return rxData[1];
}

void receiveBytes(uint8_t* rxdata, uint8_t len)
{
	uint8_t txPayload = 0x61;
	uint8_t txzeros[6];
	txzeros[0] = 0x00; // using the R_RX_PAYLOAD command
	txzeros[1] = 0x00;
	txzeros[2] = 0x00;
	txzeros[3] = 0x00;
	txzeros[4] = 0x00;
	txzeros[5] = 0x00;
	HAL_GPIO_WritePin(GPIOA, CSN_Pin, GPIO_PIN_RESET); // put CSN = 0
	// Send R_RX_PAYLOAD
	HAL_SPI_Transmit(&hspi1, &txPayload, 1, HAL_MAX_DELAY);
	HAL_SPI_TransmitReceive(&hspi1, txzeros, rxdata, len, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(GPIOA, CSN_Pin, GPIO_PIN_SET); // put CSN = 1
}

void flushTXFIFO(void)
{
	uint8_t txData[2];
	txData[0] = 0xE1; // using the FLUSH_TX cmd
	txData[1] = 0x00;
	HAL_GPIO_WritePin(GPIOA, CSN_Pin, GPIO_PIN_RESET); // put CSN = 0
	// Send W_TX_Command
	HAL_SPI_Transmit(&hspi1, txData, 1, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(GPIOA, CSN_Pin, GPIO_PIN_SET); // put CSN = 1
}

void flushRXFIFO(void)
{
	uint8_t txData = 0xE2; // using the FLUSH_RX command (1110 0010)
	HAL_GPIO_WritePin(GPIOA, CSN_Pin, GPIO_PIN_RESET); // put CSN = 0
	// Send W_TX_Command
	HAL_SPI_Transmit(&hspi1, &txData, 1*sizeof(uint8_t), HAL_MAX_DELAY);
	HAL_GPIO_WritePin(GPIOA, CSN_Pin, GPIO_PIN_SET); // put CSN = 1
}
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

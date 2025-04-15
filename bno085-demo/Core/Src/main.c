/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
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
#include "app_fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "sh2.h"
#include "sh2_err.h"
#include "sh2_SensorValue.h"
#include "i2c_ll.h"
#include "string.h"

#include "app_fatfs.h"
#include "sensor_log.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define BNO085_ADDR 0x4A

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define I2C_SPEED_FREQ              100000
#define I2C_BYTE_TIMEOUT_US         ((10^6) / (I2C_SPEED_FREQ / 9) + 1)
#define I2C_BYTE_TIMEOUT_MS         (I2C_BYTE_TIMEOUT_US / 1000 + 1)

#define MILLI_G_TO_MS2 0.0098067 ///< Scalar to convert milli-gs to m/s^2
#define DEGREE_SCALE 0.01        ///< To convert the degree values

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

UART_HandleTypeDef hlpuart1;

SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN PV */

sh2_SensorValue_t sensor_value;

uint8_t bno_buffer[20] = { 0 };

typedef struct BNO08xRVCData {
	float yaw,     ///< Yaw in Degrees
			pitch,     ///< Pitch in Degrees
			roll;      ///< Roll in Degrees
	float x_accel, ///< The X acceleration value in m/s^2
			y_accel,   ///< The Y acceleration value in m/s^2
			z_accel;   ///< The Z acceleration value in m/s^2

} BNO08x_RVC_Data;

BNO08x_RVC_Data data = { 0 };

// Declaring FatFS related objects.; filesystem, file object for general use,
// and result code for FatFS ops.
FATFS fs;
FIL file;
FRESULT res;

// Declaring FatFS related objects.; filesystem, file object for general use,
// and result code for FatFS ops.
FATFS fs;
FIL file;
FRESULT res;

// Creates file object for CSV log
// Object for number of bytes written
FIL csvFile;
UINT bytes_written;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
static void MX_LPUART1_UART_Init(void);
/* USER CODE BEGIN PFP */

int open(sh2_Hal_t *self);
void close(sh2_Hal_t *self);
int read(sh2_Hal_t *self, uint8_t *pBuffer, unsigned len, uint32_t *t_us);
int write(sh2_Hal_t *self, uint8_t *pBuffer, unsigned len);
uint32_t get_time_us(sh2_Hal_t *self);

void eventCallback(void *cookie, sh2_AsyncEvent_t *pEvent);
void sensorCallback(void *cookie, sh2_SensorEvent_t *pEvent);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void myprintf(const char *fmt, ...) {
	static char buffer[256];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	//HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer), HAL_MAX_DELAY);
}

sh2_Hal_t bno_sh = { &open, &close, &read, &write, &get_time_us };
sh2_ProductIds_t bno_id = { 0 };
sh2_ProductIds_t bno_id2 = { 0 };
sh2_SensorConfig_t bno_config = { 0 };
sh2_SensorConfig_t bno_config2 = { 0 };
sh2_SensorId_t reportType = SH2_GAME_ROTATION_VECTOR;

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

	/* USER CODE BEGIN 1 */

	int status = SH2_OK;

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
	MX_SPI1_Init();
	MX_LPUART1_UART_Init();
	MX_FATFS_Init();
	/* USER CODE BEGIN 2 */

	// Link the FatFS driver to SD logical drive
	FATFS_LinkDriver(&USER_Driver, "/SD");

	//-- Mount the filesystem --
	res = f_mount(&fs, "/SD", 1);
	if (res != FR_OK) {
		// If mounting fails print error and halt
		myprintf("f_mount failed (%d)\r\n", res);
		Error_Handler();
	} else {
		// Print if mounting succeeds
		myprintf("SD mounted.\r\n");
	}

	//-- CSV File Setup --

	// Open/create CSV file for writing
	res = f_open(&csvFile, "data.csv", FA_WRITE | FA_CREATE_ALWAYS);
	if (res == FR_OK) {
		// CSV column headers
		const char *header =
				"timestamp,yaw,pitch,roll,x_accel,y_accel,z_accel\r\n";
		res = f_write(&csvFile, header, strlen(header), &bytes_written);
		if (res == FR_OK) {
			myprintf("CSV header written.\r\n");
		} else {
			myprintf("CSV header write failed (%d).\r\n", res);
		}
		// Close file after writing
		f_close(&csvFile);

		//-- Dummy sensor logging test, comment out for flight --
		SensorData test_data; // Struct for fake data
		char csv_line[128];   // Buffer for formatted CSV row

		for (int i = 0; i < 5; i++) {
			// Fake sensor values
			test_data.timestamp = HAL_GetTick();
			test_data.temperature = 20.0f + i; // e.g. 20.0, 21.0, ...
			test_data.pressure = 1000.0f + i * 2;
			test_data.humidity = 50.0f + i * 1.5f;

			// Format CSV line
			format_csv_line(csv_line, sizeof(csv_line), &test_data);

			// Open file in append mode
			res = f_open(&csvFile, "data.csv", FA_OPEN_APPEND | FA_WRITE);
			if (res == FR_OK) {
				UINT bytes_written;
				res = f_write(&csvFile, csv_line, strlen(csv_line),
						&bytes_written);
				f_close(&csvFile);

				// Report success or failure over UART
				if (res == FR_OK) {
					myprintf("Row %d written: %s", i + 1, csv_line);
				} else {
					myprintf("Write failed (%d)\r\n", res);
				}
			} else {
				myprintf("Append open failed (%d)\r\n", res);
			}

			HAL_Delay(1000); // 1 second between entries
		}

	} else {
		// If opening CSV file failed print
		myprintf("Failed to open data.csv (%d).\r\n", res);
	}

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */

	while (1) {

		bno_buffer[0] = 0;
		bno_buffer[1] = 0;

		while (bno_buffer[0] != 0xAA)
			HAL_UART_Receive(&hlpuart1, bno_buffer, 1, 1000);

		while (bno_buffer[1] != 0xAA)
			HAL_UART_Receive(&hlpuart1, bno_buffer + 1, 1, 1000);

		HAL_UART_Receive(&hlpuart1, bno_buffer + 2, 17, 1000);

		if (bno_buffer[0] == 0xAA && bno_buffer[1] == 0xAA) {
			//got data!!!

			uint8_t sum = 0;
			// get checksum ready
			for (uint8_t i = 2; i < 17; i++) {
				sum += bno_buffer[i];
			}

			if (sum != bno_buffer[18]) {
				// data not valid
				;
			}

			// The data comes in endian'd, this solves it so it works on all platforms
			int16_t buffer_16[6];

			for (uint8_t i = 0; i < 6; i++) {

				buffer_16[i] = (bno_buffer[1 + (i * 2)]);
				buffer_16[i] += (bno_buffer[1 + (i * 2) + 1] << 8);
			}
			data.yaw = (float) buffer_16[0] * DEGREE_SCALE;
			data.pitch = (float) buffer_16[1] * DEGREE_SCALE;
			data.roll = (float) buffer_16[2] * DEGREE_SCALE;

			data.x_accel = (float) buffer_16[3] * MILLI_G_TO_MS2;
			data.y_accel = (float) buffer_16[4] * MILLI_G_TO_MS2;
			data.z_accel = (float) buffer_16[5] * MILLI_G_TO_MS2;

		}

		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
	RCC_OscInitStruct.MSIState = RCC_MSI_ON;
	RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_11;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK3 | RCC_CLOCKTYPE_HCLK
			| RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.AHBCLK3Divider = RCC_SYSCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void) {

	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	LL_I2C_InitTypeDef I2C_InitStruct = { 0 };

	LL_GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = { 0 };

	/** Initializes the peripherals clocks
	 */
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
	PeriphClkInitStruct.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
		Error_Handler();
	}

	LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
	/**I2C1 GPIO Configuration
	 PA9   ------> I2C1_SCL
	 PA10   ------> I2C1_SDA
	 */
	GPIO_InitStruct.Pin = LL_GPIO_PIN_9 | LL_GPIO_PIN_10;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_4;
	LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* Peripheral clock enable */
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */

	/** I2C Initialization
	 */
	LL_I2C_EnableAutoEndMode(I2C1);
	LL_I2C_DisableOwnAddress2(I2C1);
	LL_I2C_DisableGeneralCall(I2C1);
	LL_I2C_EnableClockStretching(I2C1);
	I2C_InitStruct.PeripheralMode = LL_I2C_MODE_I2C;
	I2C_InitStruct.Timing = 0x10805D88;
	I2C_InitStruct.AnalogFilter = LL_I2C_ANALOGFILTER_ENABLE;
	I2C_InitStruct.DigitalFilter = 0;
	I2C_InitStruct.OwnAddress1 = 0;
	I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
	I2C_InitStruct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
	LL_I2C_Init(I2C1, &I2C_InitStruct);
	LL_I2C_SetOwnAddress2(I2C1, 0, LL_I2C_OWNADDRESS2_NOMASK);
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

/**
 * @brief LPUART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_LPUART1_UART_Init(void) {

	/* USER CODE BEGIN LPUART1_Init 0 */

	/* USER CODE END LPUART1_Init 0 */

	/* USER CODE BEGIN LPUART1_Init 1 */

	/* USER CODE END LPUART1_Init 1 */
	hlpuart1.Instance = LPUART1;
	hlpuart1.Init.BaudRate = 115200;
	hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
	hlpuart1.Init.StopBits = UART_STOPBITS_1;
	hlpuart1.Init.Parity = UART_PARITY_NONE;
	hlpuart1.Init.Mode = UART_MODE_RX;
	hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	hlpuart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
	hlpuart1.AdvancedInit.AdvFeatureInit =
	UART_ADVFEATURE_RXOVERRUNDISABLE_INIT;
	hlpuart1.AdvancedInit.OverrunDisable = UART_ADVFEATURE_OVERRUN_DISABLE;
	hlpuart1.FifoMode = UART_FIFOMODE_DISABLE;
	if (HAL_UART_Init(&hlpuart1) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_UARTEx_SetTxFifoThreshold(&hlpuart1, UART_TXFIFO_THRESHOLD_1_8)
			!= HAL_OK) {
		Error_Handler();
	}
	if (HAL_UARTEx_SetRxFifoThreshold(&hlpuart1, UART_RXFIFO_THRESHOLD_1_8)
			!= HAL_OK) {
		Error_Handler();
	}
	if (HAL_UARTEx_DisableFifoMode(&hlpuart1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN LPUART1_Init 2 */

	/* USER CODE END LPUART1_Init 2 */

}

/**
 * @brief SPI1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI1_Init(void) {

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
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 7;
	hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
	hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
	if (HAL_SPI_Init(&hspi1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN SPI1_Init 2 */

	/* USER CODE END SPI1_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

	/*Configure GPIO pin : PA4 */
	GPIO_InitStruct.Pin = GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

int open(sh2_Hal_t *self) {
	// auto-gen MX functions run before this is called

	uint8_t softreset_pkt[] = { 5, 0, 1, 0, 1 };
	bool success = false;
	for (uint8_t attempts = 0; attempts < 5; attempts++) {
		if (I2Cx_WriteData(I2C1, BNO085_ADDR, 0x00, 0, softreset_pkt, 5)) {
			success = true;
			break;
		}
		HAL_Delay(30);
	}
	if (!success)
		return -1;
	HAL_Delay(300);
	return 0;
}

void close(sh2_Hal_t *self) {
	// TODO: pull BNO085 reset pin low

	// de-init resource (which I don't think we need to do)
}

int read(sh2_Hal_t *self, uint8_t *pBuffer, unsigned len, uint32_t *t_us) {

	uint8_t header[4];
	if (!I2Cx_ReadData(I2C1, BNO085_ADDR, 0x00, 0, header, 4)) {
		return 0;
	}

	// Determine amount to read
	uint16_t packet_size = (uint16_t) header[0] | (uint16_t) header[1] << 8;
	// Unset the "continue" bit
	packet_size &= ~0x8000;

	size_t i2c_buffer_max = 128;

	if (packet_size > len) {
		// packet wouldn't fit in our buffer
		return 0;
	}
	// the number of non-header bytes to read
	uint16_t cargo_remaining = packet_size;
	uint8_t i2c_buffer[i2c_buffer_max];
	uint16_t read_size;
	uint16_t cargo_read_amount = 0;
	bool first_read = true;

	while (cargo_remaining > 0) {
		if (first_read) {
			read_size = MIN(i2c_buffer_max, (size_t ) cargo_remaining);
		} else {
			read_size = MIN(i2c_buffer_max, (size_t ) cargo_remaining + 4);
		}

		if (!I2Cx_ReadData(I2C1, BNO085_ADDR, 0x00, 0, i2c_buffer, read_size)) {
			return 0;
		}

		if (first_read) {
			// The first time we're saving the "original" header, so include it in the
			// cargo count
			cargo_read_amount = read_size;
			memcpy(pBuffer, i2c_buffer, cargo_read_amount);
			first_read = false;
		} else {
			// this is not the first read, so copy from 4 bytes after the beginning of
			// the i2c buffer to skip the header included with every new i2c read and
			// don't include the header in the amount of cargo read
			cargo_read_amount = read_size - 4;
			memcpy(pBuffer, i2c_buffer + 4, cargo_read_amount);
		}
		// advance our pointer by the amount of cargo read
		pBuffer += cargo_read_amount;
		// mark the cargo as received
		cargo_remaining -= cargo_read_amount;
	}

	return packet_size;
}

int write(sh2_Hal_t *self, uint8_t *pBuffer, unsigned len) {
	size_t i2c_buffer_max = 128;
	uint16_t write_size = MIN(i2c_buffer_max, len);

	if (!I2Cx_WriteData(I2C1, BNO085_ADDR, 0x00, 0, pBuffer, write_size)) {
		return 0;
	}

	return write_size;
}

uint32_t get_time_us(sh2_Hal_t *self) {
	return (uint32_t) SysTick->VAL;
}

void eventCallback(void *cookie, sh2_AsyncEvent_t *pEvent) {
	// do nothing rn
}
void sensorCallback(void *cookie, sh2_SensorEvent_t *pEvent) {
	int rc;

	rc = sh2_decodeSensorEvent(&sensor_value, pEvent);
}

uint8_t pos = 0;

/*
 void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

 if (bno_buffer[0] == )

 HAL_UART_Receive_IT(&hlpuart1, bno_buffer + (pos++ % 19), 1);
 }
 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
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

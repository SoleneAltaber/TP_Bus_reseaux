/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "can.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "BMP280_simple.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef uint32_t BMP280_U32_t;
typedef int32_t BMP280_S32_t;
typedef int64_t BMP280_S64_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//int __io_putchar(int ch1)
//{
//	int ch2;
//	HAL_UART_Transmit(&huart2, (uint8_t *)&ch1, 1, 0xFFFF);
//	HAL_UART_Receive(&huart3, (uint8_t *)&ch2, 1, 0xFFFF);
//	return ch2;
//}
int __io_putchar(int ch)
{
	HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);
	return ch;
}

#define Slave_Add_BMP280 0b1110111

#define ctrl_meas 0xF4
#define status 0xF3
#define ID 0xD0
#define temp_msb 0xFA		//!< 1 octet
#define temp_lsb 0xFB		//!< 1 octet
#define temp_xlsb 0xFC		//!< 4bits
#define addr_dig1 0x88
#define addr_dig3 0x8A
#define addr_dig2 0x8C
#define timeout 50
#define calib_data 0x88

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
CAN_TxHeaderTypeDef pHeader_can1_moteur;

pHeader_can1_moteur.StdId = 0x61;
pHeader_can1_moteur.ExtId = 0;
pHeader_can1_moteur.IDE = CAN_ID_STD;
pHeader_can1_moteur.RTR = CAN_RTR_DATA;
pHeader_can1_moteur.DLC = 2;
pHeader_can1_moteur.TransmitGlobalTime = DISABLE;

uint32_t ptxmailbox;
uint8_t rotpos[2] = {0x5A,0x00};
uint8_t rotneg[2] = {0x5A,0x01};



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
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_CAN1_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  HAL_I2C_Init(&hi2c1);
  uint8_t pData = 0b01010111; 				//!< "010" temp x2 sample "101" pres x16 sample "11" normal mode
  uint8_t Ident = 0;
  uint8_t cal_data[24] = {0};
  uint16_t cal_data_test = 0;
  uint8_t adata1 = 0;
  uint8_t adata2 = 0;
  uint8_t adata3 = 0;
  uint16_t dig_T1 = 0;
  uint16_t dig_T2 = 0;
  uint16_t dig_T3 = 0;
  BMP280_S32_t t_fine;


  // Calibration read & write

  if(HAL_OK != HAL_I2C_Mem_Read(&hi2c1, Slave_Add_BMP280 << 1, ID, 1, &Ident, sizeof(Ident), timeout)){
  	  printf("Error_ID\r\n");
    }
  else{
	  printf("ID: %d\r\n",Ident);
  }
  HAL_Delay(500);
  if(HAL_OK != HAL_I2C_Mem_Write(&hi2c1, Slave_Add_BMP280 << 1, ctrl_meas, 1, &pData, sizeof(pData), timeout)){
	  printf("Error_ctrl_meas\r\n");
  }
  else {
	  printf("All good, ctrl_meas param are set \r\n");
  }

  HAL_Delay(500);
  if(HAL_OK != HAL_I2C_Mem_Read(&hi2c1, Slave_Add_BMP280 << 1, calib_data, 1, &cal_data, sizeof(cal_data), timeout)){
   	  printf("Error Calibration transmission\r\n");
   	  HAL_Delay(2000);
     }
  else {
	  printf("All good, nothing to notice about calibration\r\n");
  }

  HAL_Delay(500);
  HAL_CAN_Init(&hcan1);
  HAL_CAN_Start(&hcan1);

//******************************* Usage des codes fournis*****************************************************
  printf("\r\nChecking for BMP280\r\n");
  BMP280_check();
  printf("\r\nConfigure BMP280\r\n");
  BMP280_init();
  HAL_Delay(500);
//*********************************************************************************************************


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  //******************************* Notre code*****************************************************************
	  HAL_I2C_Mem_Read(&hi2c1, Slave_Add_BMP280 << 1, temp_msb, 1, &adata1, 1, timeout);
	  HAL_I2C_Mem_Read(&hi2c1, Slave_Add_BMP280 << 1, temp_lsb, 1, &adata2, 1, timeout);
	  HAL_I2C_Mem_Read(&hi2c1, Slave_Add_BMP280 << 1, temp_xlsb, 1, &adata3, 1, timeout);

	  HAL_Delay(100);
	  printf("data 0 :%d\r\n",adata1);
	  printf("data 1 :%d\r\n",adata2);
	  printf("data 2 :%d\r\n",adata3);
	  HAL_Delay(100);

//	  uint32_t data_temp = ((adata1 << 11) | (adata1 << 3) | adata3) & 0xFF;  // on les réunit

	//******************************* Usage des codes fournis*****************************************************

	  int temp= BMP280_get_temperature();
	  printf("Température: %d\r\n",temp);
	  int press= BMP280_get_pressure();
	  printf("Pression: %d\r\n",press);
	  HAL_Delay(1000);

	  HAL_I2C_Mem_Read(&hi2c1, Slave_Add_BMP280 << 1, addr_dig1, 1, &dig_T1, 2, timeout);
	  HAL_I2C_Mem_Read(&hi2c1, Slave_Add_BMP280 << 1, addr_dig2, 1, &dig_T2, 2, timeout);
	  HAL_I2C_Mem_Read(&hi2c1, Slave_Add_BMP280 << 1, addr_dig3, 1, &dig_T3, 2, timeout);

	  BMP280_S32_t var1, var2, T;
	  var1 = ((((temp>>3)-((BMP280_S32_t) dig_T1>>1)))*((BMP280_S32_t)dig_T2))>>11;
	  var2 = (((((temp>>4)-((BMP280_S32_t) dig_T1))*((temp>>4)-((BMP280_S32_t)dig_T1)))>>12)*((BMP280_S32_t)dig_T3))>>14;
	  t_fine=var1+var2;
	  T=(t_fine*5+128)>>8;
	  printf("Température compensée: %d\r\n",T);

	  //*********************************************************************************************************

//	  uint8_t rot = data_temp * 360/2^(20);



	  HAL_CAN_AddTxMessage(&hcan1, &pHeader_can1_moteur, rotpos, &ptxmailbox);
	  HAL_Delay(1000);
	  HAL_CAN_AddTxMessage(&hcan1, &pHeader_can1_moteur, rotneg, &ptxmailbox);
	  HAL_Delay(1000);
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 80;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
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

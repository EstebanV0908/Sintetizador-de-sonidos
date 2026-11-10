/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct {
    uint16_t freq;
    uint8_t volume;
} ControlMsg_t;
_Static_assert(sizeof(ControlMsg_t) == 4, "Item Size de las colas debe ser sizeof(ControlMsg_t)");
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

DAC_HandleTypeDef hdac;
DMA_HandleTypeDef hdma_dac1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim6;

/* Definitions for SynthTask */
osThreadId_t SynthTaskHandle;
const osThreadAttr_t SynthTask_attributes = {
  .name = "SynthTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for KeypadTask */
osThreadId_t KeypadTaskHandle;
const osThreadAttr_t KeypadTask_attributes = {
  .name = "KeypadTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for DisplayTask */
osThreadId_t DisplayTaskHandle;
const osThreadAttr_t DisplayTask_attributes = {
  .name = "DisplayTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for keyQueue */
osMessageQueueId_t keyQueueHandle;
const osMessageQueueAttr_t keyQueue_attributes = {
  .name = "keyQueue"
};
/* Definitions for displayQueue */
osMessageQueueId_t displayQueueHandle;
const osMessageQueueAttr_t displayQueue_attributes = {
  .name = "displayQueue"
};
/* USER CODE BEGIN PV */
#define AUDIO_BUF_SIZE 128
uint16_t audio_buffer[AUDIO_BUF_SIZE];

const uint16_t ROW_PINS[4] = {GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3};
const uint16_t COL_PINS[4] = {GPIO_PIN_4, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7};

const uint16_t note_frequencies[4][4] = {
    {262, 277, 294, 311}, // Do4 a Re4#
    {330, 349, 370, 392}, // Mi4 a Sol4
    {415, 440, 466, 494}, // Sol4# a Si4
    {523, 554, 587, 622}  // Do5 a Re5#
};

const uint16_t sine_lut[256] = {
    2048, 2098, 2148, 2198, 2248, 2298, 2347, 2396, 2445, 2493, 2541, 2589, 2636, 2682, 2728, 2773,
    2818, 2862, 2905, 2947, 2989, 3030, 3070, 3109, 3147, 3185, 3221, 3257, 3291, 3325, 3358, 3389,
    3419, 3449, 3477, 3504, 3530, 3555, 3578, 3601, 3622, 3642, 3661, 3678, 3695, 3710, 3724, 3736,
    3748, 3758, 3767, 3774, 3780, 3785, 3789, 3791, 3792, 3791, 3789, 3785, 3780, 3774, 3767, 3758,
    3748, 3736, 3724, 3710, 3695, 3678, 3661, 3642, 3622, 3601, 3578, 3555, 3530, 3504, 3477, 3449,
    3419, 3389, 3358, 3325, 3291, 3257, 3221, 3185, 3147, 3109, 3070, 3030, 2989, 2947, 2905, 2862,
    2818, 2773, 2728, 2682, 2636, 2589, 2541, 2493, 2445, 2396, 2347, 2298, 2248, 2198, 2148, 2098,
    2048, 1997, 1947, 1897, 1847, 1797, 1748, 1699, 1650, 1602, 1554, 1506, 1459, 1413, 1367, 1322,
    1277, 1233, 1190, 1148, 1106, 1065, 1025, 986, 948, 910, 874, 838, 804, 770, 737, 706,
    676, 646, 618, 591, 565, 540, 517, 494, 473, 453, 434, 417, 400, 385, 371, 359,
    347, 337, 328, 321, 315, 310, 306, 304, 303, 304, 306, 310, 315, 321, 328, 337,
    347, 359, 371, 385, 400, 417, 434, 453, 473, 494, 517, 540, 565, 591, 618, 646,
    676, 706, 737, 770, 804, 838, 874, 910, 948, 986, 1025, 1065, 1106, 1148, 1190, 1233,
    1277, 1322, 1367, 1413, 1459, 1506, 1554, 1602, 1650, 1699, 1748, 1797, 1847, 1897, 1947, 1997
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_DAC_Init(void);
static void MX_TIM6_Init(void);
static void MX_I2C1_Init(void);
void StartSynthTask(void *argument);
void StartKeypadTask(void *argument);
void StartDisplayTask(void *argument);

/* USER CODE BEGIN PFP */

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
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_DAC_Init();
  MX_TIM6_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of keyQueue */
  keyQueueHandle = osMessageQueueNew (16, 4, &keyQueue_attributes);

  /* creation of displayQueue */
  displayQueueHandle = osMessageQueueNew (8, 4, &displayQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of SynthTask */
  SynthTaskHandle = osThreadNew(StartSynthTask, NULL, &SynthTask_attributes);

  /* creation of KeypadTask */
  KeypadTaskHandle = osThreadNew(StartKeypadTask, NULL, &KeypadTask_attributes);

  /* creation of DisplayTask */
  DisplayTaskHandle = osThreadNew(StartDisplayTask, NULL, &DisplayTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};
  ADC_InjectionConfTypeDef sConfigInjected = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configures for the selected ADC injected channel its corresponding rank in the sequencer and its sample time
  */
  sConfigInjected.InjectedChannel = ADC_CHANNEL_1;
  sConfigInjected.InjectedRank = 1;
  sConfigInjected.InjectedNbrOfConversion = 1;
  sConfigInjected.InjectedSamplingTime = ADC_SAMPLETIME_3CYCLES;
  sConfigInjected.ExternalTrigInjecConvEdge = ADC_EXTERNALTRIGINJECCONVEDGE_NONE;
  sConfigInjected.ExternalTrigInjecConv = ADC_INJECTED_SOFTWARE_START;
  sConfigInjected.AutoInjectedConv = DISABLE;
  sConfigInjected.InjectedDiscontinuousConvMode = DISABLE;
  sConfigInjected.InjectedOffset = 0;
  if (HAL_ADCEx_InjectedConfigChannel(&hadc1, &sConfigInjected) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief DAC Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC_Init(void)
{

  /* USER CODE BEGIN DAC_Init 0 */

  /* USER CODE END DAC_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC_Init 1 */

  /* USER CODE END DAC_Init 1 */

  /** DAC Initialization
  */
  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT1 config
  */
  sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC_Init 2 */

  /* USER CODE END DAC_Init 2 */

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
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 0;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 2624;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);

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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_SET);

  /*Configure GPIO pins : PD0 PD1 PD2 PD3 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : PD4 PD5 PD6 PD7 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // Envía el valor '1' a SynthTask para decirle que rellene la mitad 1
    xTaskNotifyFromISR(SynthTaskHandle, 1, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// Se llama cuando el DMA termina de reproducir la segunda mitad (búfer completo)
void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // Envía el valor '2' a SynthTask para decirle que rellene la mitad 2
    xTaskNotifyFromISR(SynthTaskHandle, 2, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartSynthTask */
/**
  * @brief  Function implementing the SynthTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartSynthTask */
void StartSynthTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
	HAL_TIM_Base_Start(&htim6);
	  HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)audio_buffer, AUDIO_BUF_SIZE, DAC_ALIGN_12B_R);

	  uint32_t ulNotifiedValue;
	  ControlMsg_t msg = {0, 50};

	  uint16_t phase_acc = 0; // Acumulador de fase (Avanza por la onda)
	  uint16_t phase_inc = 0; // Incremento de fase (Depende de la frecuencia)
	  uint8_t current_vol = 50;

	  for(;;) {
	      // Bloquear tarea (No usa CPU) hasta que el DMA pida la mitad 1 o la mitad 2 del búfer
	      xTaskNotifyWait(0, 0xFFFFFFFF, &ulNotifiedValue, portMAX_DELAY);

	      // Revisamos rápido (sin bloquear) si KeypadTask envió una nueva nota/volumen
	      if (osMessageQueueGet(keyQueueHandle, &msg, NULL, 0) == osOK) {
	          if (msg.freq > 0) {
	              // Fórmula mágica DDS: Inc = (Frecuencia * TamañoTabla) / FrecuenciaMuestreo
	              // Multiplicamos por 256 extra para usar matemáticas enteras precisas (16 bits)
	              phase_inc = (msg.freq * 65536) / 32000;
	          } else {
	              phase_inc = 0;
	          }
	          current_vol = (msg.volume > 100) ? 100 : msg.volume;
	          // Reenviamos a la pantalla
	          osMessageQueuePut(displayQueueHandle, &msg, 0, 0);
	      }

	      // Elegimos qué mitad del búfer rellenar según lo que avisó el DMA
	      uint16_t *buf_ptr;
	      if (ulNotifiedValue == 1) {       // Half-Complete (La mitad 1 ya se reprodujo, podemos sobreescribirla)
	          buf_ptr = &audio_buffer[0];
	      } else if (ulNotifiedValue == 2) { // Full-Complete (La mitad 2 ya se reprodujo)
	          buf_ptr = &audio_buffer[AUDIO_BUF_SIZE / 2];
	      } else {
	          continue;
	      }

	      // Rellenamos la mitad correspondiente (64 muestras)
	      for (int i = 0; i < (AUDIO_BUF_SIZE / 2); i++) {
	              if (phase_inc > 0) {
	                  int32_t centered = (int32_t)sine_lut[phase_acc >> 8] - 2048;
	                  buf_ptr[i] = (uint16_t)(2048 + (centered * current_vol) / 100);
	                  phase_acc += phase_inc;
	              } else {
	                  buf_ptr[i] = 2048;
	                  phase_acc = 0;
	              }
	          }
	  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartKeypadTask */
/**
* @brief Function implementing the KeypadTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartKeypadTask */
void StartKeypadTask(void *argument)
{
  /* USER CODE BEGIN StartKeypadTask */
  /* Infinite loop */
	ControlMsg_t msg = {0, 50};
	  uint16_t last_freq = 0;
	  uint8_t last_vol = 50;

	  for(;;) {
	      // 1. Leer Volumen (Potenciómetro)
	      HAL_ADC_Start(&hadc1);
	      HAL_ADC_PollForConversion(&hadc1, 10);
	      uint8_t vol = (HAL_ADC_GetValue(&hadc1) * 100) / 4095; // Escalar a 0-100%
	      HAL_ADC_Stop(&hadc1);

	      // 2. Escanear teclado (Filas en PD0-PD3, Columnas en PD4-PD7)
	      uint16_t freq = 0;
	      for (int r = 0; r < 4; r++) {
	          // Bajamos una fila a LOW (0V)
	          HAL_GPIO_WritePin(GPIOD, ROW_PINS[r], GPIO_PIN_RESET);

	          for (int c = 0; c < 4; c++) {
	              // Gracias al Pull-Up, si leemos LOW, es que el botón conectó la columna a la fila
	              if (HAL_GPIO_ReadPin(GPIOD, COL_PINS[c]) == GPIO_PIN_RESET) {
	                  freq = note_frequencies[r][c];
	                  break;
	              }
	          }
	          // Regresamos la fila a HIGH antes de pasar a la siguiente
	          HAL_GPIO_WritePin(GPIOD, ROW_PINS[r], GPIO_PIN_SET);
	          if (freq != 0) break;
	      }

	      // 3. Enviar a la cola solo si hay cambios audibles (>2% de cambio de vol, o cambio de nota)
	      if (freq != last_freq || abs(vol - last_vol) > 2) {
	          msg.freq = freq;
	          msg.volume = vol;
	          osMessageQueuePut(keyQueueHandle, &msg, 0, 0);
	          last_freq = freq;
	          last_vol = vol;
	      }

	      osDelay(20); // Retardo anti-rebotes
	  }
  /* USER CODE END StartKeypadTask */
}

/* USER CODE BEGIN Header_StartDisplayTask */
/**
* @brief Function implementing the DisplayTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDisplayTask */
void StartDisplayTask(void *argument)
{
  /* USER CODE BEGIN StartDisplayTask */
  /* Infinite loop */
	ControlMsg_t dMsg = {0, 0};
	  for(;;) {
	      if (osMessageQueueGet(displayQueueHandle, &dMsg, NULL, portMAX_DELAY) == osOK) {
	          if (dMsg.freq > 0) {
	              ITM_SendChar('P'); // Play
	          } else {
	              ITM_SendChar('I'); // Idle
	          }

	          // Aquí puedes agregar la lógica I2C OLED
	          // ssd1306_SetCursor(0,0);
	          // ssd1306_WriteString("Vol: ", ...);
	      }
	  }
  /* USER CODE END StartDisplayTask */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM7 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM7)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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
#ifdef USE_FULL_ASSERT
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

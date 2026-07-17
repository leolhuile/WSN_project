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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include <time.h>
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
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void log_step(char *msg)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
    HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, 100);
}
uint32_t received_ack = 0;
volatile uint32_t tick = 0;
uint32_t rtt_min = UINT32_MAX, rtt_max = 0, rtt_sum = 0;
uint32_t t_send;
volatile uint32_t rtt;
uint8_t buff_test[64];
uint8_t data[64];
uint8_t rx_pc[64];
uint8_t rx_radio[64];
uint16_t lenA = 0;
uint16_t len =0;
uint8_t tag = 0x00;
//uint8_t tab[2] = {tag, data}; pour quelle raison j'aurais envie de faire ça ? 
uint8_t receive_buffer[1024] ;
uint8_t rxA[64] ;
int err_bits =0;
volatile  int start_test = 0;
volatile  int rx_flag =0;
volatile uint8_t can_sleep = 1;
int ack_flag =0;
int cpt =0;
char ack[] = "ACK\r\n";

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size){
    HAL_ResumeTick();
    can_sleep = 1;
      if (huart->Instance == USART1)
    {
        rx_flag = 1;
        receive_buffer[cpt] = *rx_radio;
        
       // HAL_UARTEx_ReceiveToIdle_IT(&huart1, rx_radio, sizeof(rx_radio));
       
        // HC-12 input

    }
    tick = HAL_GetTick();
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    HAL_ResumeTick();
    can_sleep = 1;
    if (huart->Instance == USART2)
    {
      start_test = 1;
      HAL_UART_Receive_IT(&huart2, rx_pc, 1); // rearm the interruption 
        // PC input
         log_step("BEGIN TESTING");
    }

    if (huart->Instance == USART1)
    {
        rx_flag = 1;
        receive_buffer[cpt] = *rx_radio;
        //HAL_UARTEx_ReceiveToIdle_IT(&huart1, rx_radio, sizeof(rx_radio));
        // HC-12 input

    }
    tick = HAL_GetTick();
}


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
    HAL_ResumeTick();
    can_sleep = 1;
    tick = HAL_GetTick();
    if(huart->Instance == USART1)
    {
        rtt = HAL_GetTick() - t_send;
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
      RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1 | RCC_PERIPHCLK_USART2;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_HSI;
    PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_HSI;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  UART_WakeUpTypeDef wakeup;
  //wakeup.WakeUpEvent = UART_WAKEUP_ON_STARTBIT;
  /*
  HAL_UARTEx_StopModeWakeUpSourceConfig(&huart1, wakeup);
  HAL_UARTEx_StopModeWakeUpSourceConfig(&huart2, wakeup);
// double configuration pour config le mode stop 

HAL_UARTEx_EnableStopMode(&huart1);
HAL_UARTEx_EnableStopMode(&huart2);

__HAL_UART_ENABLE_IT(&huart1, UART_IT_WUF);
__HAL_UART_ENABLE_IT(&huart2, UART_IT_WUF);
*/

/*
RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1 | RCC_PERIPHCLK_USART2;
PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_HSI;
PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_HSI;
if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
{
  Error_Handler();
}
  */
wakeup.WakeUpEvent = UART_WAKEUP_ON_STARTBIT;

HAL_UARTEx_StopModeWakeUpSourceConfig(&huart1, wakeup);
HAL_UARTEx_StopModeWakeUpSourceConfig(&huart2, wakeup);
while (__HAL_UART_GET_FLAG(&huart1, USART_ISR_BUSY) == SET);
while (__HAL_UART_GET_FLAG(&huart1, USART_ISR_REACK) == RESET);
while (__HAL_UART_GET_FLAG(&huart2, USART_ISR_BUSY) == SET);
while (__HAL_UART_GET_FLAG(&huart2, USART_ISR_REACK) == RESET);
HAL_UARTEx_EnableStopMode(&huart1);
HAL_UARTEx_EnableStopMode(&huart2);
__HAL_UART_ENABLE_IT(&huart1, UART_IT_WUF);
__HAL_UART_ENABLE_IT(&huart2, UART_IT_WUF);


  log_step("SYSTEM START");
  log_step("WAITING PC INPUT");
  HAL_UART_Receive_IT(&huart2, rx_pc, 1);
  HAL_UARTEx_ReceiveToIdle_IT(&huart1, rx_radio, sizeof(rx_radio));
  data[0]= 0xAA ;
  tick = HAL_GetTick();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    //HAL_GPIO_TogglePin (GPIOA, GPIO_PIN_5);  

    if(can_sleep  && HAL_GetTick() - tick > 5000){ // no activity for X ms 
          log_step("ENTERING SLEEP MODE \n");
          //can_sleep = 0;
          ///* entering sleep section 
          HAL_SuspendTick();
          //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 0);  // Just to indicate that the sleep mode is activated
          HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON,PWR_SLEEPENTRY_WFI);
          HAL_ResumeTick();



          
          
          //end of section */

          //HAL_SuspendTick();
          //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 0);  // Just to indicate that the sleep mode is activated
          //HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON,PWR_SLEEPENTRY_WFI);

          //// enables the UASRT1 and 2 to wake up the mcu 


          /*
          //below stop mode part

          //HAL_UARTEx_StopModeWakeUpSourceConfig(&huart2, wakeup);
          //HAL_UARTEx_EnableStopMode(&huart2);
          //__HAL_UART_ENABLE_IT(&huart2, UART_IT_WUF);
          HAL_SuspendTick();
          //HAL_PWREx_EnterSTOP1Mode(PWR_STOPENTRY_WFI);
          HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
          SystemClock_Config();
          HAL_ResumeTick();
          HAL_GPIO_TogglePin (GPIOA, GPIO_PIN_5); 
          while (__HAL_UART_GET_FLAG(&huart1, USART_ISR_BUSY) == SET){log_step("STUCK HERE");};
          while (__HAL_UART_GET_FLAG(&huart1, USART_ISR_REACK) == RESET){log_step("STUCK HERE");};
          while (__HAL_UART_GET_FLAG(&huart2, USART_ISR_BUSY) == SET){log_step("STUCK HERE");};
          while (__HAL_UART_GET_FLAG(&huart2, USART_ISR_REACK) == RESET){log_step("STUCK HERE");};
          HAL_UARTEx_EnableStopMode(&huart1);
          HAL_UARTEx_EnableStopMode(&huart2);
          // IMPORTANT : réarmement STOP wake 
          //HAL_UART_Init(&huart1);
          //HAL_UART_Init(&huart2);
          //MX_GPIO_Init();
          //MX_USART2_UART_Init();
          //MX_USART1_UART_Init();

          //HAL_UARTEx_StopModeWakeUpSourceConfig(&huart2, wakeup);
           
          log_step("LEAVING STOP MODE MODE \n");
            HAL_UART_Receive_IT(&huart2, rx_pc, 1);
            HAL_UARTEx_ReceiveToIdle_IT(&huart1, rx_radio, sizeof(rx_radio));
          //MX_USART1_UART_Init();
          //MX_USART2_UART_Init();
          // end of section 
          */
          //HAL_UART_Receive_IT(&huart2, rx_pc, 1);
          //HAL_UARTEx_ReceiveToIdle_IT(&huart1, rx_radio, sizeof(rx_radio));
          //tick = HAL_ResumeTick();
          // below common code for sleep or stop mode 
        //
          can_sleep = 0;
          //HAL_GPIO_EXTI_Callback()
    }
    

    // logic is : if something is received via huart2 , turn in start_test case then transmit whatever it has to send with transmit_IT, record the time to send, make an average every 200 sendings, 
    // if you have the receiver flag : check the data taken from interrupt callback (stored in array?), 
    //return the acknowledge if not an ack, if it is then inc the counter, every 200 gives the BER and check after loop if it is correct somehow bitwise 

    if(rx_flag){ // ACK or data received 
      
      // check if ACK 
      if(strncmp((char*)rx_radio, "ACK", 3) == 0){
        // we received an ACK so we count it and write it in the terminal 
        log_step("ACK RECEIVED \n");
        rtt_sum += rtt;
        if (rtt < rtt_min) rtt_min = rtt;
        if (rtt > rtt_max) rtt_max = rtt;
        received_ack++;
        rx_flag = 0;
        //ack_flag = 1;
      }

      else{ // We then need to check if the packet is correct and send an ACK
         if(HAL_UART_GetState(&huart1) == HAL_UART_STATE_READY ){
          rx_flag = 0;
        log_step("BP3: HC12  MESSAGE RECEIVED");
        log_step("DATA : ");
        char buf[10];
        sprintf(buf, "%02X ", rx_radio[0]);
        log_step(buf);
       // HAL_UART_Transmit(&huart2, (uint8_t*)buf, strlen(buf), 100);

       
            HAL_UART_Transmit_IT(&huart1,  (uint8_t*)ack,  strlen(ack));
            HAL_UARTEx_ReceiveToIdle_IT(&huart1, rx_radio, sizeof(rx_radio));
            err_bits += __builtin_popcount(0xAA ^ rx_radio[0]); // returns the number of bit equals to 1 
            char log[64];
            sprintf(log, "ACK SENT and err_bits = %d ",err_bits);
            log_step(log);

        }



      }
      tick = HAL_GetTick(); // update the tick timer 
    }

    if(start_test){ // if user started the test then transmit data and wait for ack before sending again ? 
      //buff_test[0] = HAL_UART_GetState(&huart1) ;
      //log_step(buff_test);
      if( HAL_UART_GetState(&huart1) == HAL_UART_STATE_READY || !cpt || ack_flag ){  //HAL_UART_GetState(&huart1) == HAL_UART_STATE_READY
              t_send = HAL_GetTick();  
              HAL_UART_Transmit_IT(&huart1, data, 2);
              HAL_UARTEx_ReceiveToIdle_IT(&huart1, rx_radio, sizeof(rx_radio));
              
              //rtt = HAL_GetTick() - t_send;
              log_step("AUTO SENDING...");
              cpt ++;
              tick = HAL_GetTick(); // update the tick timer 
      } 
      // do rtt = HAL_GetTick() - t_send; in the transmit_IT callback => last byte of msg sent ? 
      // no HAL_StatusTypeDef status = HAL_UARTEx_ReceiveToIdle(&huart1, rxA, sizeof(rxA), &lenA, 100); 
      // wait x ms for ack after sending to avoid floading ?


      if(cpt == 200){

      log_step("TEST FINISHED");
      char res[64];
      sprintf(res, "ACK RECEIVED: %lu / %d \r\n\r\n",received_ack,cpt);
      log_step(res);
      char mesure[128];
      uint32_t rtt_avg = received_ack ? rtt_sum / received_ack : 0; // prevent div by 0 

      sprintf(mesure, "| RTT min:%lu avg:%lu max:%lu ms", rtt_min, rtt_avg, rtt_max);
      log_step(mesure);
      rtt_avg = 0;
      start_test =0;
      cpt =0;

      }
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
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

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
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
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
  huart2.Init.BaudRate = 9600;
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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PC0 PC1 PC2 PC3
                           PC4 PC5 PC6 PC7
                           PC8 PC9 PC10 PC11
                           PC12 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA4 PA6
                           PA7 PA8 PA11 PA12
                           PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_6
                          |GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_11|GPIO_PIN_12
                          |GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB2 PB10
                           PB11 PB12 PB13 PB14
                           PB15 PB4 PB5 PB6
                           PB7 PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_15|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
                          |GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PD2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

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

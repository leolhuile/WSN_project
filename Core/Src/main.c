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
I2C_HandleTypeDef hi2c1;

LPTIM_HandleTypeDef hlptim1;

UART_HandleTypeDef hlpuart1;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_LPUART1_UART_Init(void);
static void MX_LPTIM1_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void log_step(char *msg)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
    HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, 100);
}


// clear all useless variables 
volatile uint32_t tick = 0;
uint32_t rtt_min = UINT32_MAX, rtt_max = 0, rtt_sum = 0;
volatile uint32_t rtt;
int err_bits =0; // may be usefull for redunduncy (ex : activate hamming code if nb of err bits is 1 or 2)
volatile uint8_t can_sleep = 1;
char ack[] = "ACK\r\n";

uint8_t rx_pc[64];     // PC -> STM32
uint8_t rx_radio[12];   // HC-12 -> STM32  
uint8_t pairing[1];    //  STM32 -> HC-12 (TX uniquement)
uint8_t frame[12];      // STM32 -> HC-12 (TX uniquement)
char AT_phrase[] = "AT+P1\r\n";
char log_rtt[] = "ACK RECEIVED AND RTT =     \n";
volatile uint8_t woken_by_timer = 0;
volatile int wakeup_time = 0;
volatile uint32_t cnt = 0;
int32_t press_raw;
int32_t temp_raw;
typedef struct {
    uint16_t dig_T1;
    int16_t  dig_T2, dig_T3;
    uint16_t dig_P1;
    int16_t  dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
} BMP280_Calib;

BMP280_Calib calib;
int32_t t_fine;

uint8_t calib_buf[24];
volatile int gradient =0;
volatile uint8_t sequence = 0b000001;
volatile uint32_t count = 0;

 // below is receiver parity bits 
      uint8_t pr1 ; 
      uint8_t pr2 ; 
      uint8_t pr4 ; 
      uint8_t pr8 ;
      uint8_t curr_frame;
      uint8_t syndrome;
      uint8_t decoded_frame[6]; 
      volatile uint8_t result[6];
// DONT FORGET TO CHANGE THISDONT FORGET TO CHANGE THISDONT FORGET TO CHANGE THISDONT FORGET TO CHANGE THISDONT FORGET TO CHANGE THISDONT FORGET TO CHANGE THISDONT FORGET TO CHANGE THISDONT FORGET TO CHANGE THIS
const uint8_t node_addr = 0X01; // change accordingly to the address number of your node : DONT FORGET TO CHANGE THIS (server = 00)

// DONT FORGET TO CHANGE THISDONT FORGET TO CHANGE THISDONT FORGET TO CHANGE THISDONT FORGET TO CHANGE THISDONT FORGET TO CHANGE THISDONT FORGET TO CHANGE THISDONT FORGET TO CHANGE THISDONT FORGET TO CHANGE THIS


//DONT FORGET TO CHANGE THISDONT FORGET TO CHANGE THISDONT FORGET TO CHANGE THISDONT FORGET TO CHANGE THISDONT FORGET TO CHANGE THISDONT FORGET TO CHANGE THISDONT FORGET TO CHANGE THISDONT FORGET TO CHANGE THIS
const uint8_t dst_addr = 0X02; // change accordingly to the address number of the node to be reached : DONT FORGET TO CHANGE THIS (server = 00)
//DONT FORGET TO CHANGE THISDONT FORGET TO CHANGE THISDONT FORGET TO CHANGE THISDONT FORGET TO CHANGE THISDONT FORGET TO CHANGE THISDONT FORGET TO CHANGE THISDONT FORGET TO CHANGE THISDONT FORGET TO CHANGE THIS


// Retourne la température en centièmes de °C. Ex: 5123 = 51.23 °C
int32_t bmp280_compensate_T_int32(int32_t adc_T)
{
    int32_t var1, var2, T;

    var1 = ((((adc_T >> 3) - ((int32_t)calib.dig_T1 << 1))) * ((int32_t)calib.dig_T2)) >> 11;

    var2 = (((((adc_T >> 4) - ((int32_t)calib.dig_T1)) *
              ((adc_T >> 4) - ((int32_t)calib.dig_T1))) >> 12) *
            ((int32_t)calib.dig_T3)) >> 14;

    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;

    return T;
}

// Retourne la pression en Pa (entier non signé). Ex: 96386 = 963.86 hPa
// IMPORTANT : doit être appelée APRÈS bmp280_compensate_T_int32 (dépend de t_fine)

uint32_t bmp280_compensate_P_int32(int32_t adc_P)
{
    int32_t var1, var2;
    uint32_t p;

    var1 = (((int32_t)t_fine) >> 1) - (int32_t)64000;

    var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t)calib.dig_P6);
    var2 = var2 + ((var1 * ((int32_t)calib.dig_P5)) << 1);
    var2 = (var2 >> 2) + (((int32_t)calib.dig_P4) << 16);

    var1 = (((calib.dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) +
            ((((int32_t)calib.dig_P2) * var1) >> 1)) >> 18;
    var1 = (((32768 + var1)) * ((int32_t)calib.dig_P1)) >> 15;

    if (var1 == 0)
    {
        return 0; // évite une division par zéro
    }

    p = (((uint32_t)(((int32_t)1048576) - adc_P) - (var2 >> 12))) * 3125;

    if (p < 0x80000000)
    {
        p = (p << 1) / ((uint32_t)var1);
    }
    else
    {
        p = (p / (uint32_t)var1) * 2;
    }

    var1 = (((int32_t)calib.dig_P9) * ((int32_t)(((p >> 3) * (p >> 3)) >> 13))) >> 12;
    var2 = (((int32_t)(p >> 2)) * ((int32_t)calib.dig_P8)) >> 13;

    p = (uint32_t)((int32_t)p + ((var1 + var2 + calib.dig_P7) >> 4));

    return p;
}

uint8_t  decoded_hamming( uint8_t tab1, uint8_t tab2){

    return ((tab1 >> 2) & 1 ) | ((tab1 >> 4) & 1) << 1 | ((tab1 >> 5) & 1) << 2  | ((tab1 >> 6) & 1) << 3  | ((tab2 >> 2) & 1 ) << 4 | ((tab2 >> 4) & 1 )<< 5 | ((tab2 >> 5) & 1 ) << 6 | ((tab2 >> 6) & 1 ) << 7;

}
/// @brief ////////////////////
/// @param huart 
/// @param Size 
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size){
           //int goodput_time = HAL_GetTick();
                        //count = HAL_LPTIM_ReadCounter(&hlptim1);
                        
         cnt = HAL_LPTIM_ReadCounter(&hlptim1);
         //goodput_time = HAL_GetTick() - goodput_time;

          if (huart->Instance == LPUART1)
    {
        wakeup_time = HAL_GetTick();
        
       log_step("MSG RECEIVED :\n");
       // writing every byte in hexa 
        for(int i=0;i<sizeof(rx_radio);i++){
            char buf[10];
            sprintf(buf, "%02X ", rx_radio[i]);
            HAL_UART_Transmit(&huart2, (uint8_t*)buf, strlen(buf), 100);

        }
            // à partir d'ici, décoder le message et corriger les erreurs de transmission
            if(Size== 12){
              log_step("Et après Correction des erreurs :\n");
               for(int i=0;i<sizeof(rx_radio);i++){
                curr_frame = rx_radio[i];

                pr1 = (curr_frame & 1) ^ ((curr_frame >> 2) & 1) ^ ((curr_frame >> 4) & 1) ^ ((curr_frame >> 6) & 1); // p1,d1,d2,d4
                pr2 = ((curr_frame >> 1) & 1) ^ ((curr_frame >> 2) & 1) ^ ((curr_frame >> 5) & 1) ^ ((curr_frame >> 6) & 1); // p2,d1,d3,d4
                pr4 = ((curr_frame >> 3) & 1) ^ ((curr_frame >> 4) & 1) ^ ((curr_frame >> 5) & 1) ^ ((curr_frame >> 6) & 1); // p4,d2,d3,d4
                //pr8 = pr1 ^ pr2 ^ (curr_frame & 1)  ; // p1 ^ p2 ^ d1 ^  ((p4 ^ d2 ^ d3 ^ d4) =0)
                pr8 = 0;
                for (int b = 0; b < 8; b++) pr8 ^= (curr_frame >> b) & 1;
                syndrome = pr1 | pr2 << 1 | pr4 << 2 ;
                if( syndrome != 0 && pr8 == 1){ // one error only 
                  curr_frame ^= (1 << (syndrome-1)); // inversion du bit erroné 
                  rx_radio[i] = curr_frame;
                }
                else if(pr8 == 0 && syndrome != 0 ){ // at least 2 errors so we request a resend/ do nothing until we receive a good msg
                     log_step("error, we should not forward the data received\n");
                     can_sleep =1;
                }

                // else if (syndrome == 0 && pr8 == 1)  error on pr8 so we don't care 

                                //decoded_frame[i%6] = (i<6)? decoded_hamming(rx_radio[2*i],rx_radio[2*i+1]) : decoded_frame[i%6];
                                decoded_frame[i%6] = (i>=6)? decoded_hamming(rx_radio[2*(i-6)],rx_radio[2*(i-6)+1]) : decoded_frame[i%6];

               }
              for(int i=0;i<sizeof(decoded_frame);i++){
                  char buf[10];
                  sprintf(buf, "%02X ", decoded_frame[i]);
                  HAL_UART_Transmit(&huart2, (uint8_t*)buf, strlen(buf), 100);

              }

            }

        if (strncmp((char*)rx_radio, "AT", 2) == 0 ||
            strncmp((char*)rx_radio, "OK", 2) == 0) {
            memset(rx_radio, 0, sizeof(rx_radio));
            HAL_UARTEx_ReceiveToIdle_IT(huart, rx_radio, sizeof(rx_radio));
                    can_sleep =1;
            return;  // on ignore et on réarme
        }



       if((strncmp((char*)rx_radio, "ACK", 3) == 0) || (strncmp((char*)decoded_frame, "ACK", 3) == 0)  ){
         rtt = HAL_GetTick() - rtt ;
           AT_phrase[4] ='1';
           log_step("ACK RECEIVED \n");
           
           sprintf(log_rtt, "%lu", rtt);
           log_step(log_rtt);
           can_sleep =1;
           //increase the number of ack obtained etc
        }
        else if (decoded_frame[1] == node_addr || rx_radio[1] == node_addr)
        {
          
        log_step("  ... SENDING ACK \n");

         // FIX HERE THE FRAME  FIX HERE THE FRAME FIX HERE THE FRAME FIX HERE THE FRAME FIX HERE THE FRAME FIX HERE THE FRAME FIX HERE THE FRAME FIX HERE THE FRAME FIX HERE THE FRAME
         
        HAL_UART_Transmit(huart, (uint8_t*)ack, strlen(ack), 1); // bloquant // no need for hamming code since the message is received, an ack is ok by experience 
        can_sleep =1;
         // FIX HERE THE FRAME  FIX HERE THE FRAME FIX HERE THE FRAME FIX HERE THE FRAME FIX HERE THE FRAME FIX HERE THE FRAME FIX HERE THE FRAME FIX HERE THE FRAME FIX HERE THE FRAME
        tick = HAL_GetTick();
        }
        
        else if (decoded_frame[1] != node_addr || rx_radio[1] != node_addr){
          log_step("WRONG ADRESS BACK TO SLEEP\n");
          if(rx_radio[5] > gradient) {
                        log_step("WE FORWARD THE MESSAGE \n");
                         HAL_UART_Transmit_IT(huart, rx_radio , Size); 
          }      
          can_sleep =1;
        }

         if (Size == 1 && gradient ==0){ // 1st gradient modif and we received a 1 length word so only pairing mode can do this, process data accordingly
          log_step("GRADIENT ATTRIBUTED... FORWARDING NEIGHBOURS"); 
          gradient = pairing[0]++;
          can_sleep =1;
          uint8_t g = (uint8_t)gradient;
          HAL_UART_Transmit_IT(&hlpuart1, &g, 1);

        }
        // HC-12 input        
        can_sleep =0;
        memset(decoded_frame,0,sizeof(decoded_frame));
        memset(rx_radio,0,sizeof(rx_radio));
        HAL_UARTEx_ReceiveToIdle_IT(huart, rx_radio, sizeof(rx_radio));
      }
}

// transform a frame byte into 2Bytes each with 4data bits and 4 parity bits accordingly to hamming code logic 
uint16_t Hamming_code(const uint8_t trame){
      // we construct each new "hamming " byte by halving the frame bytes (8,4) => 4data , 4parity 
      uint16_t res;
      uint8_t d1 = (trame) & 1; // (x >> i) & 1 type
      uint8_t d2 = (trame >> 1) & 1;
      uint8_t d3 = (trame >> 2) & 1;
      uint8_t d4 = (trame >> 3) & 1;
      uint8_t d5 = (trame >> 4) & 1; // for the other byte 
      uint8_t d6 = (trame >> 5) & 1;
      uint8_t d7 = (trame >> 6) & 1;
      uint8_t d8 = (trame >> 7) & 1;


      uint8_t p1 = d1  ^ d2 ^  d4 ; 
      uint8_t p2 =  d1  ^ d3 ^  d4 ; 
      uint8_t p4 = d2  ^ d3 ^  d4 ; 
      uint8_t p8 = p1 ^ p2 ^ d1 ^ p4 ^ d2 ^ d3 ^ d4;


      uint8_t p9 = d5  ^ d6 ^  d8 ; 
      uint8_t p10 =  d5  ^ d7 ^  d8 ; 
      uint8_t p12 = d6  ^ d7 ^  d8 ; 
      uint8_t p16 = p9 ^ p10 ^ d5 ^ p12 ^ d6 ^ d7 ^ d8 ;

      res = (p1 << 0)  |
            (p2 << 1)  |
            (d1 << 2)  |
            (p4 << 3)  |
            (d2 << 4)  |
            (d3 << 5)  |
            (d4 << 6)  |
            (p8 << 7)  |
            (p9 << 8)  |
            (p10 << 9) |
            (d5 << 10) |
            (p12 << 11)|
            (d6 << 12) |
            (d7 << 13) |
            (d8 << 14) |
            (p16 << 15);
      return res;
}
void Send_Sensor_Data(){
    //can_sleep = 0;

    // aquire temperature and pressure

          uint8_t data[6];
          HAL_I2C_Mem_Read(&hi2c1, (0x76<<1), 0xF7, I2C_MEMADD_SIZE_8BIT, data, 6, 100);

          int32_t press_raw = (data[0]<<12) | (data[1]<<4) | (data[2]>>4);
          int32_t temp_raw  = (data[3]<<12) | (data[4]<<4) | (data[5]>>4);

          int32_t T = bmp280_compensate_T_int32(temp_raw);   // met à jour t_fine
          uint32_t P = bmp280_compensate_P_int32(press_raw); // utilise t_fine




        uint16_t frame_part_1 = Hamming_code(node_addr);
        uint16_t frame_part_2 = Hamming_code(dst_addr);
        uint16_t frame_part_3 = Hamming_code((0b01) << 6 |  (sequence));// TYPE of data to be sent (2bits) and SEQUENCE/ index relative to the transfer
        sequence = ((sequence+1)  % 64 ); // problème : dès qu'on dépasse 63 transfert ce qui est quasi certain on reboucle et on perds l'info sur la durabilité du capteur ? sauf si ceux qui ont l'info la conserve   
        uint16_t frame_part_4 = Hamming_code(( T >> 8 ) & 0xff); // data temperature  MSB 
        uint16_t frame_part_5 = Hamming_code( T & 0xff);// data temperature  LSB 
        uint16_t frame_part_6 = Hamming_code(gradient);

         // send data frame 
          frame[0] = frame_part_1;
          frame[1] = frame_part_1 >> 8;
          frame[2] = frame_part_2;
          frame[3] = frame_part_2 >> 8;
          frame[4] = frame_part_3;
          frame[5] = frame_part_3 >> 8;
          frame[6] = frame_part_4 ;
          frame[7] = frame_part_4 >> 8;
          frame[8] = frame_part_5;
          frame[9] = frame_part_5 >> 8;
          frame[10] = frame_part_6;
          frame[11] = frame_part_6 >> 8;
         // info : 2 complement so to read properly you need to reassemble the int-16 signed
         // incrémenter un compteur de nombre de transfert et envoyer cette information pour estimer l'autonomie restante => ça correspond au transfert 
         rtt = HAL_GetTick() ;
        //count = HAL_LPTIM_ReadCounter(&hlptim1);
        uint32_t start = DWT->CYCCNT;

        char goodput[64];
         //HAL_UART_Transmit_IT(&hlpuart1, frame, 6);
         HAL_UART_Transmit(&hlpuart1, frame, 12, 20);

        uint32_t cycles = DWT->CYCCNT - start;
        uint32_t us = cycles / (SystemCoreClock / 1000000);

         sprintf(goodput,"MSG SENT and Tick = %ld \n temperature calculated is = ",us);
         log_step(goodput);
            char msg[64];
          for(int i=0;i<sizeof(frame);i++){
                  char buf[10];
                  sprintf(buf, "%02X ", frame[i]);
                  HAL_UART_Transmit(&huart2, (uint8_t*)buf, strlen(buf), 100);

              }
            // en commentaire : tests de la conversion de temperature en hexa 
                  //char buf[10];
                  
                  //sprintf(buf, "%04lX ", (unsigned long)T);
                  //HAL_UART_Transmit(&huart2, (uint8_t*)buf, strlen(buf), 100);
sprintf(msg, "T=%ld.%02ld°C\r\n",
        T/100, T%100);
HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
        cnt = 0;

 
        
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){

    tick = HAL_GetTick();
}

void HAL_LPTIM_CompareMatchCallback(LPTIM_HandleTypeDef *hlptim)
{
  // Do whatever you want
  woken_by_timer = 1;
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

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
DWT->CYCCNT = 0;
DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  MX_LPUART1_UART_Init();
  MX_LPTIM1_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */

  UART_WakeUpTypeDef wakeup;
  wakeup.WakeUpEvent = UART_WAKEUP_ON_STARTBIT;

  HAL_UARTEx_StopModeWakeUpSourceConfig(&hlpuart1, wakeup);
  HAL_UARTEx_StopModeWakeUpSourceConfig(&huart2, wakeup);
  while (__HAL_UART_GET_FLAG(&hlpuart1, USART_ISR_BUSY) == SET);
  while (__HAL_UART_GET_FLAG(&hlpuart1, USART_ISR_REACK) == RESET);
  while (__HAL_UART_GET_FLAG(&huart2, USART_ISR_BUSY) == SET);
  while (__HAL_UART_GET_FLAG(&huart2, USART_ISR_REACK) == RESET);
  HAL_UARTEx_EnableStopMode(&hlpuart1);
  HAL_UARTEx_EnableStopMode(&huart2);
  __HAL_UART_ENABLE_IT(&hlpuart1, UART_IT_WUF);
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_WUF);

HAL_I2C_Mem_Read(&hi2c1, (0x76<<1), 0x88, I2C_MEMADD_SIZE_8BIT, calib_buf, 24, 100);

calib.dig_T1 = (uint16_t)(calib_buf[1]<<8 | calib_buf[0]);
calib.dig_T2 = (int16_t) (calib_buf[3]<<8 | calib_buf[2]);
calib.dig_T3 = (int16_t) (calib_buf[5]<<8 | calib_buf[4]);
calib.dig_P1 = (uint16_t)(calib_buf[7]<<8 | calib_buf[6]);
calib.dig_P2 = (int16_t) (calib_buf[9]<<8 | calib_buf[8]);
calib.dig_P3 = (int16_t) (calib_buf[11]<<8 | calib_buf[10]);
calib.dig_P4 = (int16_t) (calib_buf[13]<<8 | calib_buf[12]);
calib.dig_P5 = (int16_t) (calib_buf[15]<<8 | calib_buf[14]);
calib.dig_P6 = (int16_t) (calib_buf[17]<<8 | calib_buf[16]);
calib.dig_P7 = (int16_t) (calib_buf[19]<<8 | calib_buf[18]);
calib.dig_P8 = (int16_t) (calib_buf[21]<<8 | calib_buf[20]);
calib.dig_P9 = (int16_t) (calib_buf[23]<<8 | calib_buf[22]);

uint8_t ctrl_meas = (0b001 << 5) | (0b001 << 2) | 0b01; // osrs_t=x1, osrs_p=x1, forced mode
HAL_I2C_Mem_Write(&hi2c1, (0x76<<1), 0xF4, I2C_MEMADD_SIZE_8BIT, &ctrl_meas, 1, 100);


  log_step("SYSTEM START");
  log_step("WAITING PC INPUT");
  HAL_UART_Receive_IT(&huart2, rx_pc, 1);
  HAL_UARTEx_ReceiveToIdle_IT(&hlpuart1, rx_radio, sizeof(rx_radio));
  HAL_UARTEx_ReceiveToIdle_IT(&hlpuart1, pairing, sizeof(pairing));
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);
  tick = HAL_GetTick();
          // HAL_LPTIM_TimeOut_Start_IT(&hlptim1, 5240, 5240-cnt);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    // noeud serveur : envoi uniquement une seule fois à ses voisins son adresse 00
    if(can_sleep ){ // no activity for X ms 
          log_step("ENTERING SLEEP MODE \n");
          // entering stop section 
          HAL_LPTIM_TimeOut_Stop_IT(&hlptim1);
          HAL_LPTIM_TimeOut_Start_IT(&hlptim1, 5240, 5240-cnt);
          
          //uint32_t cnt = hlptim1.Instance->CNT;
          HAL_SuspendTick();

          HAL_PWREx_EnterSTOP2Mode(PWR_SLEEPENTRY_WFI);
          SystemClock_Config();

          HAL_ResumeTick();
          HAL_Delay(50);  
          //can_sleep = 0; 
          tick = HAL_GetTick();
    

    }

    // X seconds elapsed so we take the sensor data and send it asap ( copy usart2 in a function )
    /*
      if( HAL_GetTick() - tick > 1000  && !can_sleep){ // if nothing is received in 100ticks in this mode then resend . This does not conflict if an ack is received because the can_sleep flag would be on before 100ticks 
          if(AT_phrase[4] != '8') {

          AT_phrase[4] = ((AT_phrase[4] - '1' + 1 ) % 8 )  + '1';
          char text[50];
          snprintf(text, sizeof(text), "POWER UP THE TX %c\n", AT_phrase[4]);

          log_step(text);
                                                                                                                                    
          HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET);  // toggle the set pin to low so we can change the power 
          HAL_Delay(100); // delay to shorten                                                                                                                         
          HAL_UART_Transmit_IT(&huart1, (uint8_t*)(AT_phrase), strlen(AT_phrase));
          HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);   // toggle the set pin to high so we can work again
          HAL_Delay(100); // delay to shorten 
          HAL_UART_Transmit_IT(&huart1, frame,  3); // ok ? 
          }
          else{
          HAL_UART_Transmit_IT(&huart1, frame , 3);
          }
          tick = HAL_GetTick();

        }*/
    if(woken_by_timer ){
    Send_Sensor_Data();
            uint8_t ctrl_meas = (0b001 << 5) | (0b001 << 2) | 0b01; // osrs_t=x1, osrs_p=x1, forced mode
        HAL_I2C_Mem_Write(&hi2c1, (0x76<<1), 0xF4, I2C_MEMADD_SIZE_8BIT, &ctrl_meas, 1, 100);
      woken_by_timer =0;

        tick = HAL_GetTick();

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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
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
  hi2c1.Init.Timing = 0x10D19CE4;
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
  * @brief LPTIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_LPTIM1_Init(void)
{

  /* USER CODE BEGIN LPTIM1_Init 0 */

  /* USER CODE END LPTIM1_Init 0 */

  /* USER CODE BEGIN LPTIM1_Init 1 */

  /* USER CODE END LPTIM1_Init 1 */
  hlptim1.Instance = LPTIM1;
  hlptim1.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
  hlptim1.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV32;
  hlptim1.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
  hlptim1.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;
  hlptim1.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
  hlptim1.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
  hlptim1.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
  hlptim1.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
  if (HAL_LPTIM_Init(&hlptim1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPTIM1_Init 2 */

  /* USER CODE END LPTIM1_Init 2 */

}

/**
  * @brief LPUART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_LPUART1_UART_Init(void)
{

  /* USER CODE BEGIN LPUART1_Init 0 */

  /* USER CODE END LPUART1_Init 0 */

  /* USER CODE BEGIN LPUART1_Init 1 */

  /* USER CODE END LPUART1_Init 1 */
  hlpuart1.Instance = LPUART1;
  hlpuart1.Init.BaudRate = 9600;
  hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
  hlpuart1.Init.StopBits = UART_STOPBITS_1;
  hlpuart1.Init.Parity = UART_PARITY_NONE;
  hlpuart1.Init.Mode = UART_MODE_TX_RX;
  hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPUART1_Init 2 */

  /* USER CODE END LPUART1_Init 2 */

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

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PC2 PC3 PC4 PC5
                           PC6 PC7 PC8 PC9
                           PC11 PC12 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9
                          |GPIO_PIN_11|GPIO_PIN_12;
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
                           PB7 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_15|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
                          |GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PC10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

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

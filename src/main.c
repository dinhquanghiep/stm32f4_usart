/**
  ******************************************************************************
  * @file     main.c
  * @author   hiepdq
  * @version  xxx
  * @date     26.09.2018
  * @brief    This file provides main firmware functions for MCU 
  *           Try to using all peripheral and standard coding style    
  *           Sử dụng DAC tạo sóng sin và sóng vuông với tần số bất kỳ
  *           Sử dụng ADC để đọc lại giá trị của DAC                           
 ===============================================================================      
                       ##### How to use this driver #####
 ===============================================================================
  
  ******************************************************************************
  * @attention
  *
  ******************************************************************************  
  */ 

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stm32f4xx.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_dma.h>
#include <stm32f4xx_usart.h>
#include <misc.h>
#include <string.h>
#if defined (__GNUC__)
#include <malloc.h>
#elif defined (__ICCARM__)

#endif

/* Private macro -------------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define LED_GREEN   GPIO_Pin_12
#define LED_ORANGE  GPIO_Pin_13
#define LED_RED     GPIO_Pin_14
#define LED_BLUE    GPIO_Pin_15
#define USER_BUTTON GPIO_Pin_0
/* Private typedef -----------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
volatile uint32_t time_ms = 0;
/* Private variables ---------------------------------------------------------*/
static uint8_t buff_recv[100];
static uint8_t buff_send[] = "Dinh Quang Hiep";
static uint8_t buffer_count = 0;
/* Private function prototypes -----------------------------------------------*/
static void rcc_config(void);
void delay_ms(uint32_t ms);
static void gpio_config(void);
static void dma_config(void);
static void usart_config(void);
static void nvic_config(void);
/* Public functions ----------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/** @brief  Config the clocks for system
  * @param  None
  * 
  * @retval None
  */
static void rcc_config(void) {
  /* Switch systemclock to HSI */
  RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);
  // RCC_HSEConfig(RCC_HSE_ON);
  // while (RCC_WaitForHSEStartUp() == ERROR) {
  //   /* Waitng for HSE config  */
  // }
  /* PLL must be disabled before config */
  RCC_PLLCmd(DISABLE);
  RCC_PLLConfig(RCC_PLLSource_HSE, 8, 336, 2, 7);
  RCC_PLLCmd(ENABLE);
  while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {
    /* Waitng for PLL enable  */
  }
  /* Switch the systemclock to PLLCLK */
  RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
  /* Ensure the systemclock is switched to PLL then disable HSI */
  if (RCC_GetSYSCLKSource() == 0x08) {
    RCC_HSICmd(DISABLE);
  }
  RCC_ClockSecuritySystemCmd(DISABLE);
  RCC_HCLKConfig(RCC_SYSCLK_Div1);
  RCC_PCLK1Config(RCC_HCLK_Div4);
  RCC_PCLK2Config(RCC_HCLK_Div2);

  SystemCoreClockUpdate();
  SysTick_Config(167999); 
}

/** @brief  Delay in ms
  * @param  the time to delay(unit: ms)
  * 
  * @retval None
  */
void delay_ms(uint32_t ms) {
  uint32_t curr_time_ms = time_ms;
  while (ms) {
    if (curr_time_ms != time_ms) {
      ms--;
      curr_time_ms = time_ms;
    }
  }
}

/** @brief  Config the clocks for system
  * @param  None
  * 
  * @retval None
  */
static void gpio_config(void) {
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

  GPIO_DeInit(GPIOA);
  GPIO_DeInit(GPIOD);

  GPIO_InitTypeDef GPIO_InitStruct;

  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_Pin = USER_BUTTON;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Pin = LED_BLUE | LED_GREEN | LED_RED | LED_ORANGE;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOD, &GPIO_InitStruct);

  GPIO_WriteBit(GPIOD, LED_BLUE, Bit_SET);

  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

}

/** @brief  Config the clocks for system
  * @param  None
  * 
  * @retval None
  */
static void dma_config(void) {
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

  DMA_DeInit(DMA1_Stream6);
  DMA_InitTypeDef DMA_InitStruct;
  DMA_StructInit(&DMA_InitStruct);
  DMA_InitStruct.DMA_Channel = DMA_Channel_4;
  DMA_InitStruct.DMA_PeripheralBaseAddr = USART2_BASE + 0x04;
  DMA_InitStruct.DMA_Memory0BaseAddr = buff_send;
  DMA_InitStruct.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStruct.DMA_BufferSize = strlen(buff_send);
  DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStruct.DMA_Priority = DMA_Priority_High;
  DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStruct.DMA_FIFOThreshold = DMA_FIFOStatus_1QuarterFull;
  DMA_InitStruct.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStruct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA1_Stream6, &DMA_InitStruct);


  DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStruct.DMA_BufferSize = 100;
  DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStruct.DMA_Memory0BaseAddr = buff_recv;
  DMA_Init(DMA1_Stream5, &DMA_InitStruct);
  // DMA_ITConfig(DMA1_Stream6, DMA_IT_TC, ENABLE);

  DMA_Cmd(DMA1_Stream6, ENABLE);
  DMA_Cmd(DMA1_Stream5, ENABLE);
}
/** @brief  Config the UASRT2
  * @param  None
  * 
  * @retval None
  */
static void usart_config(void) {
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
  USART_InitTypeDef USART_InitStruct;
  USART_StructInit(&USART_InitStruct);
  USART_InitStruct.USART_BaudRate = 115200;
  USART_InitStruct.USART_WordLength = USART_WordLength_9b;
  USART_InitStruct.USART_StopBits = USART_StopBits_1;
  USART_InitStruct.USART_Parity = USART_Parity_Even;
  USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_OverSampling8Cmd(USART2, DISABLE);
  USART_Init(USART2, &USART_InitStruct);

  // USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

  USART_Cmd(USART2, ENABLE);
  USART_DMACmd(USART2, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);
}

/** @brief  Config the NVIC
  * @param  None
  * 
  * @retval None
  */
static void nvic_config(void) {
  NVIC_InitTypeDef NVIC_InitStruct;
  NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);

  NVIC_InitStruct.NVIC_IRQChannel = DMA1_Stream6_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);
}
/* Main source ---------------------------------------------------------------*/
int main(void) {
  rcc_config();
  gpio_config();
  dma_config();
  usart_config();
  nvic_config();
  uint8_t chuoi[] = "Chao mung cac ban den voi phan quan ly su dung command line\n"
"    + nhap vao ki tu, ket thuc bang dau cham\n";
/* Toc do baud 115200 truye chuoi tren mat 12ms */
  while (1) {
    // delay_ms(500);
    while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET) {
      /* Wait until Transmistion complete */
    }
    while (DMA_GetFlagStatus(DMA1_Stream6, DMA_FLAG_TCIF6) == RESET) {
      /* Wait until Transmistion complete */
    }
    for (uint16_t len = 0; len < strlen(chuoi); len++) {
    while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET) {
      /* Wait until Transmistion complete */
    }
      USART_SendData(USART2, chuoi[len]);
    }

    // USART_DMACmd(USART2, USART_DMAReq_Tx, DISABLE);
    // DMA_Cmd(DMA1_Stream6, DISABLE);
    USART_ClearFlag(USART2, USART_FLAG_TC);
    DMA_ClearFlag(DMA1_Stream6, DMA_FLAG_TCIF6);
    DMA_Cmd(DMA1_Stream6, ENABLE);
    // USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
    GPIO_ToggleBits(GPIOD, LED_BLUE);
    delay_ms(500);
  }
  return 0;
}
/**
  * @brief  This function handles EXTI0_IRQHandler interrupt request.
  * @param  None
  * @retval None
  */
void USART2_IRQHandler(void) {
  if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
    uint8_t tmp = (uint8_t)USART_ReceiveData(USART2);
    if (tmp != '\n') {
      buff_recv[buffer_count++] = tmp;
    } else {
      const uint8_t chuoi_tmp[] = "\nBan da nhap vao chuoi: ";
      for (uint8_t tmp = 0; tmp < strlen(chuoi_tmp); tmp++) {
        while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET) {
          /* waiting for transfer */
        }
        USART_SendData(USART2, chuoi_tmp[tmp]);
      }
      for (uint8_t tmp = 0; tmp < buffer_count; tmp++) {
        while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET) {
          /* waiting for transfer */
        }
        USART_SendData(USART2, buff_recv[tmp]);
      }
      buffer_count = 0;
    }
  }
}

void DMA1_Stream4_IRQHandler(void) {

}
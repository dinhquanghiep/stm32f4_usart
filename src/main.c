/**
  ******************************************************************************
  * @file     main.c
  * @author   hiepdq
  * @version  xxx
  * @date     06.09.2018
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
#include <misc.h>
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
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void rcc_config(void);
static void gpio_config(void);
/* Public functions ----------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/** @brief  Config the clocks for system
  * @param  None
  * 
  * @retval None
  */
static void rcc_config(void) {
  RCC_DeInit();
  RCC_HSEConfig(RCC_HSE_ON);
  while (RCC_WaitForHSEStartUp() == ERROR) {
    /* Waitng for HSE config  */
  }
  RCC_HSICmd(DISABLE);
  RCC_PLLConfig(RCC_PLLSource_HSE, 4, 168, 2, 4);
  RCC_PLLCmd(ENABLE);
  RCC_ClockSecuritySystemCmd(DISABLE);
  RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
  RCC_HCLKConfig(RCC_SYSCLK_Div1);
  RCC_PCLK1Config(RCC_HCLK_Div4);
  RCC_PCLK2Config(RCC_HCLK_Div2);
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
}
/* Main source ---------------------------------------------------------------*/
int main(void) {
  rcc_config();
  gpio_config();
  return 0;
}
/**
  * @brief  This function handles EXTI0_IRQHandler interrupt request.
  * @param  None
  * @retval None
  */
// void Interrupt_IRQHandler(void) {

// }
#include "stm32f10x_it.h"

void EXTI15_10_IRQHandler(void)
{
if(EXTI_GetITStatus(EXTI_Line13) != RESET)
	{
	/* Clear the EXTI line pending bit */
	EXTI_ClearITPendingBit(EXTI_Line13);
		// dao trang thai led moi lan co canh xuong
	GPIO_WriteBit(GPIOB,GPIO_Pin_9,(BitAction)(1));
	}
}
void EXTI9_5_IRQHandler(void)
{
if(EXTI_GetITStatus(EXTI_Line8) != RESET)
	{
	/* Clear the EXTI line pending bit */
	EXTI_ClearITPendingBit(EXTI_Line8);
		// dao trang thai led moi lan co canh xuong
	GPIO_WriteBit(GPIOB,GPIO_Pin_9,(BitAction)(0));
	}
}

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
}

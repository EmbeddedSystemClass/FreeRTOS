/*
 * ADC_Task.c
 *
 *  Created on: 03-03-2013
 *      Author: Grzybek
 */
#include <stdio.h>
#include <string.h>
#include "ADC_Task.h"
#include "LcdTask.h"

#define ADC1_DR_Address    ((uint32_t)0x4001244C)
static __IO uint16_t ADCConvertedValue;

portBASE_TYPE vStartAdcTask(unsigned portBASE_TYPE uxPriority)
{
    portBASE_TYPE result = pdTRUE;

    // Create a task
    if(pdPASS == xTaskCreate( AdcTask, ( signed portCHAR * ) "ADC", ADC_STACK_SIZE,
            NULL, uxPriority, NULL))
    {
        result = pdTRUE;
    }
    else
    {
        result = pdFALSE;
    }

    return result;
}

portTASK_FUNCTION(AdcTask,parameters)
{
    portTickType xLastWakeTime;
    const portTickType freq = 100;
    char buff[20];

    memset(buff,0x00,20);

    ADCConvertedValue = 0;

    AdcConfig();

    /* Start ADC1 Software Conversion */
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

/*    sprintf(buff,"ADC = %d",ADCConvertedValue);
    LCD_Cmd(DISPLAY_LINE,buff,6);
    vTaskDelay(10);*/

    xLastWakeTime = xTaskGetTickCount();
    while (1)
    {
        //vTaskDelayUntil(&xLastWakeTime, freq);

        //printf("ADC=%d\n\r",ADCConvertedValue);
        //memset(buff,0x00,20);
        sprintf(buff,"ADC = %d   ",ADCConvertedValue);

        LCD_Cmd(DISPLAY_LINE_COLUMN,buff,6,6,strlen(buff));
        //vTaskDelay(1000);
        vTaskDelayUntil(&xLastWakeTime, freq);
    }
}

void AdcConfig(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    /* ADCCLK = PCLK2/4 */
    RCC_ADCCLKConfig(RCC_PCLK2_Div4 );

    /* Enable peripheral clocks ------------------------------------------------*/
    /* Enable DMA1 clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    /* Enable ADC1 and GPIOC clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC, ENABLE);

    /* Configure PC.04 (ADC Channel13) as analog input -------------------------*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* DMA1 channel1 configuration ----------------------------------------------*/
    DMA_DeInit(DMA1_Channel1 );
    DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) &ADCConvertedValue;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = 1;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);

    /* Enable DMA1 channel1 */
    DMA_Cmd(DMA1_Channel1, ENABLE);

    /* ADC1 configuration ------------------------------------------------------*/
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    /* ADC1 regular channel14 configuration */
    ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 1,
            ADC_SampleTime_55Cycles5 );

    /* Enable ADC1 DMA */
    ADC_DMACmd(ADC1, ENABLE);

    /* Enable ADC1 */
    ADC_Cmd(ADC1, ENABLE);

    /* Enable ADC1 reset calibration register */
    ADC_ResetCalibration(ADC1 );
    /* Check the end of ADC1 reset calibration register */
    while (ADC_GetResetCalibrationStatus(ADC1 ))
        ;

    /* Start ADC1 calibration */
    ADC_StartCalibration(ADC1 );
    /* Check the end of ADC1 calibration */
    while (ADC_GetCalibrationStatus(ADC1 ))
        ;
}

/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include "gd32f1x0.h"
#include "colibri_bsp_led.h"

/*************************************************************************************************
 *  ���ܣ���ʼ���û�Led�豸                                                                      *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void EvbLedConfig(void)
{
    GPIO_InitPara GPIO_InitStructure;
	
	    /* Enable the LED Clock */
    RCC_AHBPeriphClock_Enable(RCC_AHBPERIPH_GPIOB,ENABLE);

    /* Configure the LED pin */
    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_8 | GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_MODE_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OTYPE_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PUPD_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_SPEED_50MHZ;
    GPIO_Init(GPIOB,&GPIO_InitStructure);
}



/*************************************************************************************************
 *  ���ܣ�����Led�ĵ�����Ϩ��                                                                    *
 *  ������(1) index Led�Ʊ��                                                                    *
 *        (2) cmd   Led�Ƶ�������Ϩ�������                                                      *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void EvbLedControl(int index, int cmd)
{
    switch (index)
    {
        case LED1:
        {
            if (cmd == LED_ON)
            {
                GPIO_SetBits(GPIOB,GPIO_PIN_8);  /*����Led1��*/
            }
            else
            {
                GPIO_ResetBits(GPIOB,GPIO_PIN_8);  /*Ϩ��Led1��*/
            }
            break;
        }
        case LED2:
        {
            if (cmd == LED_ON)
            {
                GPIO_SetBits(GPIOB,GPIO_PIN_9);  /*����Led2��*/
            }
            else
            {
                GPIO_ResetBits(GPIOB,GPIO_PIN_9);  /*Ϩ��Led2��*/
            }
            break;
        }
        case LED3:
        {
            if (cmd == LED_ON)
            {
                GPIO_SetBits(GPIOB,GPIO_PIN_10);  /*����Led3��*/
            }
            else
            {
                GPIO_ResetBits(GPIOB,GPIO_PIN_10);  /*Ϩ��Led3��*/
            }
            break;
        }
        default:
        {
            if (cmd == LED_ON)
            {
                GPIO_SetBits(GPIOB,GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10); /*�������еĵ�*/
            }
            else
            {
                GPIO_ResetBits(GPIOB,GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10); /*Ϩ�����еĵ�*/
            }
            break;
        }
    }
}


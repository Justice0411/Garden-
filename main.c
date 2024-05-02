#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "lcd_i2c.h"
#include <stdio.h>
#include "stm32f10x_adc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "adc.h"
#include "delay.h"
#include "dht11.h"


#define LIGHT_SENSOR_PIN        GPIO_Pin_0      //pin A0
#define ADC_CHANNEL             ADC_Channel_0
#define THRESHOLD               30
#define I2C_DEVICE_ADDRESS      0x50

#define relay_GPIO_PORT         GPIOB
#define relay_GPIO_PIN          GPIO_Pin_8  // Example pin for the relay
#define LED_GPIO_PORT           GPIOA
#define LED_GPIO_PIN            GPIO_Pin_12 // Example pin for the led 
#define BUTTON1_GPIO_PORT       GPIOA
#define BUTTON1_GPIO_PIN        GPIO_Pin_1  // Example pin for Button 1
#define BUTTON2_GPIO_PORT       GPIOA
#define BUTTON2_GPIO_PIN        GPIO_Pin_2  // Example pin for Button 2
#define BUTTON3_GPIO_PORT       GPIOA
#define BUTTON3_GPIO_PIN        GPIO_Pin_4  // Example pin for Button 4 
xSemaphoreHandle xMutex;
	
int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;// Ánh x? giá tr? x t? kho?ng [in_min, in_max] sang kho?ng [out_min, out_max]
}
const int air = 3423;//4050;
const int water = 1845;//2050;2460;//
int value = 0;
 
void vTask_SoilMoistureSensor(void *p);
void vTask_DHT11(void *p);
void vTask_ControlRelay(void *p);

typedef enum {
    AUTOMATIC_MODE,   
    MANUAL_MODE
} ControlMode;

ControlMode control_mode = AUTOMATIC_MODE;
//uint16_t luxvalue = 0; // bien toan cuc
//int light_percentage = 0 ;

ControlMode previous_control_mode = AUTOMATIC_MODE; //Bien luu tr?ng thái tru?c dó c?a ch? d?

void vTask_ReadLightSensor(void *p);
void vTask_CheckButtons(void *p);
void GPIO_config();
//void vTask_UART(void *p);
	
char tmp[50];
char doam[20];
int main(void) {
    SystemInit();
    SystemCoreClockUpdate();
    xMutex = xSemaphoreCreateMutex();
    lcd_init();      
    adc_1_init();
	  GPIO_config();
	  xTaskCreate(vTask_ReadLightSensor, "ReadLight", 128, NULL, 2, NULL);
    xTaskCreate(vTask_CheckButtons, "CheckButtons", 128, NULL, 1, NULL);
	  adc_2_init();
	  DHT11_Start();
	  xTaskCreate(vTask_SoilMoistureSensor, "Soid MoisTure Sensor", 128, NULL, 2, NULL);
	  xTaskCreate(vTask_DHT11, "DHT11", 128, NULL, 2, NULL);
    xTaskCreate(vTask_ControlRelay, "Controlrelay", 128, NULL, 2, NULL);
//		xTaskCreate(vTask_UART, "UART", 128, NULL,2,NULL);
    vTaskStartScheduler(); // Start the scheduler
    return 0;
}
void GPIO_config(void) { // ly
    // Configure relay GPIO
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStruct.GPIO_Pin = relay_GPIO_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;                         //8B
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(relay_GPIO_PORT, &GPIO_InitStruct);
    
	  // Configure Led GPIO
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStruct.GPIO_Pin = LED_GPIO_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;                         //12A
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);
    // Configure Button 1 GPIO
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStruct.GPIO_Pin = BUTTON1_GPIO_PIN;                              //1A va 4A
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;  // Input Pull-up
    GPIO_Init(BUTTON1_GPIO_PORT, &GPIO_InitStruct);
//    // Configure Button 2 GPIO
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStruct.GPIO_Pin = BUTTON2_GPIO_PIN;                              //1A va 4A
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;  // Input Pull-up
    GPIO_Init(BUTTON2_GPIO_PORT, &GPIO_InitStruct);
		
		//Configure Button 3 GPIO
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStruct.GPIO_Pin = BUTTON3_GPIO_PIN;                              //1A va 4A
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;  // Input Pull-up
    GPIO_Init(BUTTON3_GPIO_PORT, &GPIO_InitStruct);
}
uint16_t light_percentage;
void vTask_ReadLightSensor(void *pvParameters) { // ly
	  uint16_t max_light = 4095;
	  uint16_t min_light = 0;
    while (1) {
        xSemaphoreTake(xMutex, portMAX_DELAY);   // Read data from light sensor  ADC
        lcd_gotoxy(9,0);
        ADC_SoftwareStartConvCmd(ADC1, ENABLE);
        while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);  
			  uint16_t luxvalue = ADC_GetConversionValue(ADC1);  
			  //light_percentage = (luxvalue - min_light) * 100 / (max_light - min_light);
				light_percentage = map(luxvalue, 4095, 0, 0, 100);
        sprintf(tmp,"Li:%d", light_percentage);                                                    // 0A
        lcd_send_string(tmp);  
				lcd_gotoxy(14,0);		
				lcd_send_string("%");  
        xSemaphoreGive(xMutex);  
        vTaskDelay(600);
    }  
}

void vTask_SoilMoistureSensor(void *p)//khoi
{
	while(1)
	{
		xSemaphoreTake(xMutex, portMAX_DELAY);
		lcd_gotoxy(0,0);
		value = map(adc_2_get_value(), air, water, 0, 100);
		sprintf(doam,"Hum:%d%%", value);
		lcd_send_string(doam);   //1B
		xSemaphoreGive(xMutex);
		vTaskDelay(500);
	}
}
float temp = 0;
char t[20];
void vTask_DHT11(void *p)//khoi
{
                                        //3a
	while(1)
	{
		xSemaphoreTake(xMutex, portMAX_DELAY);
		DHT11_ReadData();
		temp = DHT11_GetTemp()/256;
		sprintf(t,"T:%.1f", temp);
		lcd_gotoxy(0,1);
		lcd_send_string(t);                           
		xSemaphoreGive(xMutex);
		vTaskDelay(500);
	}
}

typedef enum {
    BUTTON_NOT_PRESSED,   
    BUTTON_PRESSED
} ButtonState;

ButtonState button1_state = BUTTON_NOT_PRESSED;
ButtonState button2_state = BUTTON_NOT_PRESSED;
ButtonState button3_state = BUTTON_NOT_PRESSED;

void vTask_CheckButtons(void *p) {
		char mode[20];
    while(1) {
        // Check if BUTTON1 is pressed
        if(GPIO_ReadInputDataBit(BUTTON1_GPIO_PORT, BUTTON1_GPIO_PIN) == Bit_RESET) {
            // Toggle control mode
            if(control_mode == AUTOMATIC_MODE) {
                control_mode = MANUAL_MODE;
								sprintf(mode, "Mode: MA");
            } else {
                control_mode = AUTOMATIC_MODE;
								sprintf(mode, "Mode: AU");
            }
						xSemaphoreTake(xMutex, portMAX_DELAY);
            lcd_gotoxy(8, 1); // Go to the 3rd line of the LCD
            lcd_send_string(mode);
            xSemaphoreGive(xMutex);
            // Debounce delay
            vTaskDelay(200);
        }
        // Check if BUTTON2 is pressed in MANUAL_MODE
        if(control_mode == MANUAL_MODE && GPIO_ReadInputDataBit(BUTTON2_GPIO_PORT, BUTTON2_GPIO_PIN) == Bit_RESET) {
            // Toggle LED
            GPIO_WriteBit(LED_GPIO_PORT, LED_GPIO_PIN, (BitAction)(1 - GPIO_ReadOutputDataBit(LED_GPIO_PORT, LED_GPIO_PIN)));
            // Debounce delay
            vTaskDelay(200);
        }
        // Check if BUTTON3 is pressed in MANUAL_MODE
        if(control_mode == MANUAL_MODE && GPIO_ReadInputDataBit(BUTTON3_GPIO_PORT, BUTTON3_GPIO_PIN) == Bit_RESET) {
            // Toggle Relay
            GPIO_WriteBit(relay_GPIO_PORT, relay_GPIO_PIN, (BitAction)(1 - GPIO_ReadOutputDataBit(relay_GPIO_PORT, relay_GPIO_PIN)));
            // Debounce delay
            vTaskDelay(200);
        }
        vTaskDelay(10);
    }
}

void vTask_ControlRelay(void *p) {
    while(1) {
        if(control_mode == AUTOMATIC_MODE) {
            // If soil moisture < 45%, turn on relay
            if(value < 45) {
                GPIO_SetBits(relay_GPIO_PORT, relay_GPIO_PIN);
            } else {
                GPIO_ResetBits(relay_GPIO_PORT, relay_GPIO_PIN);
            }
            // If light intensity < 40%, turn on LED
            if(light_percentage < 40) {
                GPIO_SetBits(LED_GPIO_PORT, LED_GPIO_PIN);
            } else {
                GPIO_ResetBits(LED_GPIO_PORT, LED_GPIO_PIN);
            }
        }
        vTaskDelay(100);
    }
}
//void vTask_UART(void *p)
//{
//	uart_speed(115200);
//	while(1)
//	{
//		xSemaphoreTake(xMutex, portMAX_DELAY);
//		uart_send_string(tmp);
//		uart_send_string(doam);
//		uart_send_string(t);
//		xSemaphoreGive(xMutex);
//		vTaskDelay(1000);
//	}
//}

#ifndef __DS3231_H
#define __DS3231_H
#include "stm32f10x.h"
#include "stm32f10x_i2c.h"
#include "usart.h"
#include <stdio.h>

//#define I2C_FLAG_TIMEOUT 			((uint32_t)0x1000)
#define DS3231_WriteSlaveAddress  	0xD1
#define DS3231_ReadSlaveAddress 	0xD0

#define REG_SEC						0x00
#define REG_MIN						0x01
#define REG_HOUR					0x02
#define REG_DAY						0x03
#define REG_DATE					0x04
#define REG_MON						0x05
#define REG_YEAR					0x06

#define REG_A1M1					0x07
#define REG_A1M2					0x08
#define REG_A1M3					0x09
#define REG_A1M4					0x0A
#define REG_CONTROL 			0x0E
#define REG_STATUS        0x0F

typedef struct 
{
	char Sec;
	char Min;
	char Hour;
	char Day;
	char Date;
	char Month;
	char Year;
}DS3231_InitTypeDef;

typedef struct {
	char Seconds;
	char Minutes;
	char Hour;
	char Day;//Day;// Range 1-7
//	char Date;// Range 1-31
}DS3231_Alarm1TypeDef;

// DS3231_InitTypeDef DS3231_WriteDate;
// DS3231_InitTypeDef DS3231_ReadDate;
// DS3231_InitTypeDef DS3231_printf;
		
void I2C_ENABLE_Init(void);
void Start(I2C_TypeDef * I2Cx);
void SendSlaveAddress_Write(I2C_TypeDef* I2Cx);
void SendData(I2C_TypeDef* I2Cx,uint8_t);
void Stop(I2C_TypeDef* I2Cx);
uint8_t DS3231_DecToBCD(uint8_t value);
uint8_t DS3231_BCDToDec(uint8_t value);
void DS3231_SetDataTime(I2C_TypeDef* I2C,DS3231_InitTypeDef* DS3231_InitStructure);

void SendAddressRead(I2C_TypeDef* I2Cx);
uint8_t ReceiveData(I2C_TypeDef* I2Cx);
void DS3231_GetDateTime(I2C_TypeDef* I2Cx,DS3231_InitTypeDef* DS3231_InitStructure);
void SetAlarmOne (I2C_TypeDef* I2Cx,DS3231_Alarm1TypeDef* Alarm1Structure);
void GetAlarmOne(I2C_TypeDef* I2Cx,DS3231_Alarm1TypeDef* Alarm1Structure);
void SetCtrlRegister(I2C_TypeDef* I2Cx);
uint8_t DS3231_GetFlag(I2C_TypeDef* I2Cx);
void DS3231_ClearFlag(I2C_TypeDef* I2Cx);
uint8_t GetCotrolRegister(I2C_TypeDef* I2Cx);




#endif


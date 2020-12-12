#include "ds3231.h"
#include "stm32f10x_i2c.h"


//typedef struct 
//{
//	uint8_t Sec;
//	uint8_t Min;
//	uint8_t Hour;
//	uint8_t Day;
//	uint8_t Date;
//	uint8_t Month;
//	uint8_t Year;
//}DS3231_InitTypeDef;
//#define REG_A1M1					0x07
//#define REG_A1M2					0x08
//#define REG_A1M3					0x09
//#define REG_A1M4					0x0A
//#define REG_CONTROL 			0x0E
//#define REG_STATUS        0x0F
 

static void I2C_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
 // GPIO_PinRemapConfig(GPIO_Remap_I2C1,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	//GPIO_SetBits(GPIOB,GPIO_Pin_6 |GPIO_Pin_7);
}
static void I2C_Mode_Config(void)
{
	I2C_InitTypeDef I2C_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,ENABLE);
	
	I2C_AcknowledgeConfig(I2C1,ENABLE);//接受到一个字节以后返回一个应答ACK
	
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0x00;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = 40000;
	I2C_Init(I2C1,&I2C_InitStructure);
	I2C_Cmd(I2C1,ENABLE);
}
void I2C_ENABLE_Init(void)
{
	I2C_GPIO_Config();
	I2C_Mode_Config();	
	
}


 void Start(I2C_TypeDef * I2Cx)                                            //主机发送开始信号
{   
	I2C_GenerateSTART (I2Cx,ENABLE);                                   
	while(!I2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_MODE_SELECT));              //检测EV5
}

void Stop(I2C_TypeDef* I2Cx)                                              //主机发送停止信号
{
	I2C_GenerateSTOP(I2Cx,ENABLE);
	while(I2C_GetFlagStatus(I2Cx,I2C_FLAG_STOPF));
}

void SendSlaveAddress_Write(I2C_TypeDef* I2Cx)                             //主机发送从机地址
{
	I2C_Send7bitAddress(I2Cx,0xD0,I2C_Direction_Transmitter);                //主机发送7位从地址
	while(!I2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)); //检测EV6
}                            

void SendData(I2C_TypeDef* I2Cx,uint8_t data)                              //发送寄存器的地址或者寄存器中写入值
{
	I2C_SendData(I2Cx,data);
	while(!I2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTED));	
};



void SendAddressRead(I2C_TypeDef* I2Cx)
{
	I2C_Send7bitAddress(I2Cx, 0xD1, I2C_Direction_Receiver);// send Address DS3231(0xD0)
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));// wait send address
};

uint8_t ReceiveData(I2C_TypeDef* I2Cx)
{
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED));// wait receive a byte
	return I2C_ReceiveData(I2Cx);
};


uint8_t DS3231_DecToBCD(uint8_t value)
{
	return ((value / 10) << 4) + (value % 10);// convert decimal to BCD
}
uint8_t DS3231_BCDToDec(uint8_t value)
{
	return ((value >> 4) * 10) + (value & 0x0F);// convert BCD(Binary Coded Decimal) to Decimal
};




void DS3231_SetDataTime(I2C_TypeDef* I2Cx , DS3231_InitTypeDef* DS3231_InitStructure)
{
	while (I2C_GetFlagStatus(I2Cx,I2C_FLAG_BUSY));                      //
	I2C_AcknowledgeConfig(I2Cx,ENABLE);                                 //
	
 	Start(I2Cx);                                                        //产生起始信号，并检测Ev5
	
	SendSlaveAddress_Write(I2Cx);                                       //发送从机地址，并检测Ev6
	
	SendData(I2Cx,0x00);                                                //发送寄存器地址，并检测Ev8
	SendData(I2Cx,DS3231_DecToBCD(DS3231_InitStructure->Sec));          //发送Sec的值        
	SendData(I2Cx,DS3231_DecToBCD(DS3231_InitStructure->Min));          
	SendData(I2Cx,DS3231_DecToBCD(DS3231_InitStructure->Hour));
	SendData(I2Cx,DS3231_DecToBCD(DS3231_InitStructure->Day));
	SendData(I2Cx,DS3231_DecToBCD(DS3231_InitStructure->Date));
	SendData(I2Cx,DS3231_DecToBCD(DS3231_InitStructure->Month));
	SendData(I2Cx,DS3231_DecToBCD(DS3231_InitStructure->Year));
	
	Stop(I2Cx);
}

void DS3231_GetDateTime(I2C_TypeDef* I2Cx,DS3231_InitTypeDef* DS3231_InitStructure)
{
	while(I2C_GetFlagStatus(I2Cx,I2C_FLAG_BUSY));													//
	I2C_AcknowledgeConfig(I2Cx,ENABLE);																		//
	
	Start(I2Cx);																													//主机发送起始信号，并检测Ev5
	SendSlaveAddress_Write(I2Cx);																					//主机发送从机地址+写信号，并检测Ev6
	SendData(I2Cx,0x00);																									//发送寄存器地址，并检测Ev8
	Stop(I2Cx);																														//主机发送结束信号
	
	Start(I2Cx);                                                          //主机发送起始信号
	SendAddressRead(I2Cx); 																								//主机发送从机地址+读信号，并检测Ev7
	
	DS3231_InitStructure->Sec = DS3231_BCDToDec(ReceiveData(I2Cx));       //
	DS3231_InitStructure->Min = DS3231_BCDToDec(ReceiveData(I2Cx));
	DS3231_InitStructure->Hour = DS3231_BCDToDec(ReceiveData(I2Cx));
	DS3231_InitStructure->Day = DS3231_BCDToDec(ReceiveData(I2Cx));
	DS3231_InitStructure->Date = DS3231_BCDToDec(ReceiveData(I2Cx));
	DS3231_InitStructure->Month = DS3231_BCDToDec(ReceiveData(I2Cx));
	DS3231_InitStructure->Year = DS3231_BCDToDec(ReceiveData(I2Cx));
	I2C_NACKPositionConfig(I2Cx, I2C_NACKPosition_Current);               //
	I2C_AcknowledgeConfig(I2Cx, DISABLE);                                 //失能应答信号
	
	Stop(I2Cx);																														//主机发送停止信号
	
}

void SetAlarmOne (I2C_TypeDef* I2Cx,DS3231_Alarm1TypeDef* Alarm1Structure)
{
	while (I2C_GetFlagStatus(I2Cx,I2C_FLAG_BUSY));                      //
	I2C_AcknowledgeConfig(I2Cx,ENABLE);                                 //
	
 	Start(I2Cx);                                                        //产生起始信号，并检测Ev5
	
	SendSlaveAddress_Write(I2Cx); 
	SendData(I2Cx,0x07);
	SendData(I2Cx, DS3231_DecToBCD(Alarm1Structure->Seconds));// send second
	SendData(I2Cx, DS3231_DecToBCD(Alarm1Structure->Minutes));// send Minutes
	SendData(I2Cx, DS3231_DecToBCD(Alarm1Structure->Hour));// send Hour
	SendData(I2Cx, DS3231_DecToBCD(Alarm1Structure->Day | (0x80)));// send Day
	//DS3231_SendData(I2Cx, DS3231_DecToBCD(Alarm1Structure->Date));// send Date
	Stop(I2Cx);
}
void GetAlarmOne(I2C_TypeDef* I2Cx,DS3231_Alarm1TypeDef* Alarm1Structure)
{
	while(I2C_GetFlagStatus(I2Cx,I2C_FLAG_BUSY));													
	I2C_AcknowledgeConfig(I2Cx,ENABLE);																		
	
	Start(I2Cx);																													//主机发送起始信号，并检测Ev5
	SendSlaveAddress_Write(I2Cx);																					//主机发送从机地址+写信号，并检测Ev6
	SendData(I2Cx,0x07);																									//发送闹钟1寄存器地址，并检测Ev8
	Stop(I2Cx);																														//主机发送结束信号
	
	Start(I2Cx);                                                          //主机发送起始信号
	SendAddressRead(I2Cx); 
	
	Alarm1Structure->Seconds = DS3231_BCDToDec(ReceiveData(I2Cx) & 0x7F);// receive seconds
	Alarm1Structure->Minutes = DS3231_BCDToDec(ReceiveData(I2Cx) & 0x7F);// receive Month
	Alarm1Structure->Hour = DS3231_BCDToDec(ReceiveData(I2Cx) & 0x3F);// receive Hour
	Alarm1Structure->Day = DS3231_BCDToDec(ReceiveData(I2Cx) & 0x3F);// receive Day or Date
	//Alarm1Structure->Date = DS3231_BCDToDec(DS3231_ReceiveData(I2Cx) & 0x7F);// receive Date
	I2C_NACKPositionConfig(I2Cx, I2C_NACKPosition_Current);// generate not acknowledge
	I2C_AcknowledgeConfig(I2Cx, DISABLE);// disable acknowledge
	Stop(I2Cx);
	I2C_AcknowledgeConfig(I2Cx, ENABLE);// enable acknowledge
	
	
}

//设置控制寄存器
//开启Alarm1 开启中断 开启EOSC 
void SetCtrlRegister(I2C_TypeDef* I2Cx)
{
	while(I2C_GetFlagStatus(I2Cx,I2C_FLAG_BUSY));
	I2C_AcknowledgeConfig(I2Cx,ENABLE);										//使能响应
	
	Start(I2Cx);																					//开始信号
	SendSlaveAddress_Write(I2Cx);													//发送从机写地址
	SendData(I2Cx,REG_CONTROL);														//发送控制寄存器地址0x0E
	SendData(I2Cx,0x07);												          //0b10000101 使能bit0 bit2 bit7位。闹钟1 中断 振荡器
	Stop(I2Cx);																						//停止信号
	
}

/*获取控制寄存器值*/
uint8_t GetCotrolRegister(I2C_TypeDef* I2Cx)
{
	uint8_t readCtrlReg;
	while(I2C_GetFlagStatus(I2Cx,I2C_FLAG_BUSY));
	I2C_AcknowledgeConfig(I2Cx,ENABLE);											//使能响应					
	
	Start(I2Cx);                                            //开始信号
	SendSlaveAddress_Write(I2Cx); 													//发送从机地址
	SendData(I2Cx,0x0E);															//发送控制寄存器地址0x0E
	Stop(I2Cx);                                         		//停止信号
	
	Start(I2Cx);																						//开始信号
	SendAddressRead(I2Cx); 															//发送控制寄存器地址
	
	readCtrlReg = ReceiveData(I2Cx);								//获取数据
	I2C_NACKPositionConfig(I2Cx,I2C_NACKPosition_Current);  //
	I2C_AcknowledgeConfig(I2Cx,DISABLE);											//
	Stop(I2Cx);																							//
	                    //
	
	return readCtrlReg;

}


//获取闹钟1的状态（主机读取数据）
/*return @ calculate
calculate = 1;表示闹钟触发
printf ...;
*/
uint8_t DS3231_GetFlag(I2C_TypeDef* I2Cx)
{ 
	uint8_t calculate = 0;
	while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));
	I2C_AcknowledgeConfig(I2Cx, ENABLE);                                   
	
	Start(I2Cx);																													//主机发送起始信号，并检测Ev5
	SendSlaveAddress_Write(I2Cx);																					//主机发送从机地址+写信号，并检测Ev6
	SendData(I2Cx,0x0F);																						//发送状态寄存器地址，并检测Ev8
	Stop(I2Cx);
	
	Start(I2Cx);
	SendAddressRead(I2Cx);                                                //send address DS3231(address: 0xD0)
	calculate = (ReceiveData(I2Cx) & 0x01);                       //读状态寄存器bit0 闹钟1状态位  若bit0=1->表示已触发闹钟
	I2C_NACKPositionConfig(I2Cx, I2C_NACKPosition_Current);
	I2C_AcknowledgeConfig(I2Cx, DISABLE);
	Stop(I2Cx);
	
	return calculate;
}
//清除状态寄存器bit0位
//闹钟响应之后清除状态，让其下一次继续触发。
void DS3231_ClearFlag(I2C_TypeDef* I2Cx)
{
	while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));
	I2C_AcknowledgeConfig(I2Cx, ENABLE);                                  // 使能响应
	Start(I2Cx);																													//主机发送起始信号，并检测Ev5
	SendSlaveAddress_Write(I2Cx);																					//主机发送从机地址+写信号，并检测Ev6
	SendData(I2Cx,REG_STATUS);																						//发送状态寄存器地址，并检测Ev8
	
	I2C_SendData(I2Cx, ~0x01);                                            //清除状态寄存器bit0位，
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));      
	Stop(I2Cx);
}
























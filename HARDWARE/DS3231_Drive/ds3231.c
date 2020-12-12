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
	
	I2C_AcknowledgeConfig(I2C1,ENABLE);//���ܵ�һ���ֽ��Ժ󷵻�һ��Ӧ��ACK
	
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


 void Start(I2C_TypeDef * I2Cx)                                            //�������Ϳ�ʼ�ź�
{   
	I2C_GenerateSTART (I2Cx,ENABLE);                                   
	while(!I2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_MODE_SELECT));              //���EV5
}

void Stop(I2C_TypeDef* I2Cx)                                              //��������ֹͣ�ź�
{
	I2C_GenerateSTOP(I2Cx,ENABLE);
	while(I2C_GetFlagStatus(I2Cx,I2C_FLAG_STOPF));
}

void SendSlaveAddress_Write(I2C_TypeDef* I2Cx)                             //�������ʹӻ���ַ
{
	I2C_Send7bitAddress(I2Cx,0xD0,I2C_Direction_Transmitter);                //��������7λ�ӵ�ַ
	while(!I2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)); //���EV6
}                            

void SendData(I2C_TypeDef* I2Cx,uint8_t data)                              //���ͼĴ����ĵ�ַ���߼Ĵ�����д��ֵ
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
	
 	Start(I2Cx);                                                        //������ʼ�źţ������Ev5
	
	SendSlaveAddress_Write(I2Cx);                                       //���ʹӻ���ַ�������Ev6
	
	SendData(I2Cx,0x00);                                                //���ͼĴ�����ַ�������Ev8
	SendData(I2Cx,DS3231_DecToBCD(DS3231_InitStructure->Sec));          //����Sec��ֵ        
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
	
	Start(I2Cx);																													//����������ʼ�źţ������Ev5
	SendSlaveAddress_Write(I2Cx);																					//�������ʹӻ���ַ+д�źţ������Ev6
	SendData(I2Cx,0x00);																									//���ͼĴ�����ַ�������Ev8
	Stop(I2Cx);																														//�������ͽ����ź�
	
	Start(I2Cx);                                                          //����������ʼ�ź�
	SendAddressRead(I2Cx); 																								//�������ʹӻ���ַ+���źţ������Ev7
	
	DS3231_InitStructure->Sec = DS3231_BCDToDec(ReceiveData(I2Cx));       //
	DS3231_InitStructure->Min = DS3231_BCDToDec(ReceiveData(I2Cx));
	DS3231_InitStructure->Hour = DS3231_BCDToDec(ReceiveData(I2Cx));
	DS3231_InitStructure->Day = DS3231_BCDToDec(ReceiveData(I2Cx));
	DS3231_InitStructure->Date = DS3231_BCDToDec(ReceiveData(I2Cx));
	DS3231_InitStructure->Month = DS3231_BCDToDec(ReceiveData(I2Cx));
	DS3231_InitStructure->Year = DS3231_BCDToDec(ReceiveData(I2Cx));
	I2C_NACKPositionConfig(I2Cx, I2C_NACKPosition_Current);               //
	I2C_AcknowledgeConfig(I2Cx, DISABLE);                                 //ʧ��Ӧ���ź�
	
	Stop(I2Cx);																														//��������ֹͣ�ź�
	
}

void SetAlarmOne (I2C_TypeDef* I2Cx,DS3231_Alarm1TypeDef* Alarm1Structure)
{
	while (I2C_GetFlagStatus(I2Cx,I2C_FLAG_BUSY));                      //
	I2C_AcknowledgeConfig(I2Cx,ENABLE);                                 //
	
 	Start(I2Cx);                                                        //������ʼ�źţ������Ev5
	
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
	
	Start(I2Cx);																													//����������ʼ�źţ������Ev5
	SendSlaveAddress_Write(I2Cx);																					//�������ʹӻ���ַ+д�źţ������Ev6
	SendData(I2Cx,0x07);																									//��������1�Ĵ�����ַ�������Ev8
	Stop(I2Cx);																														//�������ͽ����ź�
	
	Start(I2Cx);                                                          //����������ʼ�ź�
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

//���ÿ��ƼĴ���
//����Alarm1 �����ж� ����EOSC 
void SetCtrlRegister(I2C_TypeDef* I2Cx)
{
	while(I2C_GetFlagStatus(I2Cx,I2C_FLAG_BUSY));
	I2C_AcknowledgeConfig(I2Cx,ENABLE);										//ʹ����Ӧ
	
	Start(I2Cx);																					//��ʼ�ź�
	SendSlaveAddress_Write(I2Cx);													//���ʹӻ�д��ַ
	SendData(I2Cx,REG_CONTROL);														//���Ϳ��ƼĴ�����ַ0x0E
	SendData(I2Cx,0x07);												          //0b10000101 ʹ��bit0 bit2 bit7λ������1 �ж� ����
	Stop(I2Cx);																						//ֹͣ�ź�
	
}

/*��ȡ���ƼĴ���ֵ*/
uint8_t GetCotrolRegister(I2C_TypeDef* I2Cx)
{
	uint8_t readCtrlReg;
	while(I2C_GetFlagStatus(I2Cx,I2C_FLAG_BUSY));
	I2C_AcknowledgeConfig(I2Cx,ENABLE);											//ʹ����Ӧ					
	
	Start(I2Cx);                                            //��ʼ�ź�
	SendSlaveAddress_Write(I2Cx); 													//���ʹӻ���ַ
	SendData(I2Cx,0x0E);															//���Ϳ��ƼĴ�����ַ0x0E
	Stop(I2Cx);                                         		//ֹͣ�ź�
	
	Start(I2Cx);																						//��ʼ�ź�
	SendAddressRead(I2Cx); 															//���Ϳ��ƼĴ�����ַ
	
	readCtrlReg = ReceiveData(I2Cx);								//��ȡ����
	I2C_NACKPositionConfig(I2Cx,I2C_NACKPosition_Current);  //
	I2C_AcknowledgeConfig(I2Cx,DISABLE);											//
	Stop(I2Cx);																							//
	                    //
	
	return readCtrlReg;

}


//��ȡ����1��״̬��������ȡ���ݣ�
/*return @ calculate
calculate = 1;��ʾ���Ӵ���
printf ...;
*/
uint8_t DS3231_GetFlag(I2C_TypeDef* I2Cx)
{ 
	uint8_t calculate = 0;
	while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));
	I2C_AcknowledgeConfig(I2Cx, ENABLE);                                   
	
	Start(I2Cx);																													//����������ʼ�źţ������Ev5
	SendSlaveAddress_Write(I2Cx);																					//�������ʹӻ���ַ+д�źţ������Ev6
	SendData(I2Cx,0x0F);																						//����״̬�Ĵ�����ַ�������Ev8
	Stop(I2Cx);
	
	Start(I2Cx);
	SendAddressRead(I2Cx);                                                //send address DS3231(address: 0xD0)
	calculate = (ReceiveData(I2Cx) & 0x01);                       //��״̬�Ĵ���bit0 ����1״̬λ  ��bit0=1->��ʾ�Ѵ�������
	I2C_NACKPositionConfig(I2Cx, I2C_NACKPosition_Current);
	I2C_AcknowledgeConfig(I2Cx, DISABLE);
	Stop(I2Cx);
	
	return calculate;
}
//���״̬�Ĵ���bit0λ
//������Ӧ֮�����״̬��������һ�μ���������
void DS3231_ClearFlag(I2C_TypeDef* I2Cx)
{
	while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));
	I2C_AcknowledgeConfig(I2Cx, ENABLE);                                  // ʹ����Ӧ
	Start(I2Cx);																													//����������ʼ�źţ������Ev5
	SendSlaveAddress_Write(I2Cx);																					//�������ʹӻ���ַ+д�źţ������Ev6
	SendData(I2Cx,REG_STATUS);																						//����״̬�Ĵ�����ַ�������Ev8
	
	I2C_SendData(I2Cx, ~0x01);                                            //���״̬�Ĵ���bit0λ��
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));      
	Stop(I2Cx);
}
























#include "bsp_ap3216c.h"
#include "bsp_i2c.h"
#include "bsp_gpio.h"
#include "bsp_delay.h"
#include "stdio.h"

/* 初始化AP3216C */
void ap3216c_init(void)
{
	unsigned char value = 0;
    /* IO初始化 */
	IOMUXC_SetPinMux(IOMUXC_UART4_TX_DATA_I2C1_SCL,1);          /* 复用为I2C1_SCL */
	IOMUXC_SetPinMux(IOMUXC_UART4_RX_DATA_I2C1_SDA,1);          /* 复用为I2C1_SDA */

	IOMUXC_SetPinConfig(IOMUXC_UART4_TX_DATA_I2C1_SCL,0x70b0);  /* 复用为I2C1_SDA */
	IOMUXC_SetPinConfig(IOMUXC_UART4_RX_DATA_I2C1_SDA,0x70b0);  /* 复用为I2C1_SDA */
    
	/* I2C接口初始化 */
	i2c_init(I2C1);

    /* AP3216C传感器初始化 */
	ap3216c_writeonebyte(AP3216C_ADDR, AP3216C_SYSTEMCONG, 0x4);	/* 复位 */
	delayms(50);
	ap3216c_writeonebyte(AP3216C_ADDR, AP3216C_SYSTEMCONG, 0x3);	/* 复位 */
	value = ap3216c_readonebyte(AP3216C_ADDR, AP3216C_SYSTEMCONG);
	printf("AP3216C_SYSTEMCONG reg=%#x\r\n", value);
}

/* AP3216C读一个字节数据,返回值就是读取寄存器的值 */
unsigned char ap3216c_readonebyte(unsigned char addr, unsigned char reg)
{
	unsigned char val = 0;

	struct i2c_transfer masterXfer;
	masterXfer.slaveAddress   = addr;
	masterXfer.direction      = kI2C_Read;
	masterXfer.subaddress 	  = reg;
	masterXfer.subaddressSize = 1; 
	masterXfer.data			  = &val;
	masterXfer.dataSize 	  = 1;
	i2c_master_transfer(I2C1, &masterXfer);
	return val;
}

/* AP3216C写一个字节数据，返回值就是错误类型 */
unsigned char ap3216c_writeonebyte(unsigned char addr, unsigned char reg, unsigned char data)
{
	unsigned char writedata = data;
	unsigned char status = 0;

	struct i2c_transfer masterXfer;
	masterXfer.slaveAddress   = addr;
	masterXfer.direction      = kI2C_Write;
	masterXfer.subaddress 	  = reg;
	masterXfer.subaddressSize = 1; 
	masterXfer.data			  = &writedata;
	masterXfer.dataSize 	  = 1;

	if (i2c_master_transfer(I2C1, &masterXfer))
		status = 1;
	return status;
}

/* AP3216C数据读取 */
void ap3216c_readdata(unsigned short *ir, unsigned short *ps, unsigned short *als)
{
	unsigned char buf[6];
	unsigned char i = 0;
	/* 循环读取数据 */
	for (i = 0; i < 6; i++)
	{
		buf[i] = ap3216c_readonebyte(AP3216C_ADDR, AP3216C_IRDATALOW + i);
	}

	if (buf[0] & 0x80)	/* 为真表示IR和PS数据无效 */
	{
		*ir = 0;
		*ps = 0;
	}
	else
	{
		*ir = ((unsigned short)buf[1] << 2) | (buf[0] & 0x03);
		*ps = (((unsigned short)buf[5] & 0x3f) << 4) | (buf[4] & 0x0f);
	}
	*als = ((unsigned short)buf[3] << 8) | buf[2] ;

}
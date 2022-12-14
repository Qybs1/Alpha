#include "bsp_lcd.h"
#include "bsp_gpio.h"
#include "stdio.h"
#include "bsp_delay.h"

/* 屏幕参数结构体变量 */
struct tftlcd_typedef tftlcd_dev;

/* lcd初始化 */
void lcd_init(void) 
{
    unsigned short lcdid = 0;

    lcdid = lcd_read_panelid();
    tftlcd_dev.id = lcdid;
    printf("LCD ID = %#X\r\n", lcdid);

    /* 初始化屏幕IO */
    lcdgpio_init();
    lcd_reset();
    delayms(10);
    lcd_noreset();

    /* 根据不同的屏幕id来设置参数 */
    if(lcdid == ATK4342) 
    {
		tftlcd_dev.height = 272;	
		tftlcd_dev.width = 480;
		tftlcd_dev.vspw = 1;
		tftlcd_dev.vbpd = 8;
		tftlcd_dev.vfpd = 8;
		tftlcd_dev.hspw = 1;
		tftlcd_dev.hbpd = 40;
		tftlcd_dev.hfpd = 5; 	
		lcdclk_init(27, 8, 8);	/* 初始化LCD时钟 10.1MHz */
	} 
    else if(lcdid == ATK4384) 
    {
		tftlcd_dev.height = 480;	
		tftlcd_dev.width = 800;
		tftlcd_dev.vspw = 3;
		tftlcd_dev.vbpd = 32;
		tftlcd_dev.vfpd = 13;
		tftlcd_dev.hspw = 48;
		tftlcd_dev.hbpd = 88;
		tftlcd_dev.hfpd = 40;
		lcdclk_init(42, 4, 8);	/* 初始化LCD时钟 31.5MHz */
	} 
    else if(lcdid == ATK7084) 
    {
		tftlcd_dev.height = 480;	
		tftlcd_dev.width = 800;
		tftlcd_dev.vspw = 1;
		tftlcd_dev.vbpd = 23;
		tftlcd_dev.vfpd = 22;
		tftlcd_dev.hspw = 1;
		tftlcd_dev.hbpd = 46;
		tftlcd_dev.hfpd = 210;	
		lcdclk_init(30, 3, 7);	/* 初始化LCD时钟 34.2MHz */
	} 
    else if(lcdid == ATK7016) 
    {
		tftlcd_dev.height = 600;	
		tftlcd_dev.width = 1024;
		tftlcd_dev.vspw = 3;
		tftlcd_dev.vbpd = 20;
		tftlcd_dev.vfpd = 12;
		tftlcd_dev.hspw = 20;
		tftlcd_dev.hbpd = 140;
		tftlcd_dev.hfpd = 160;
		lcdclk_init(32, 3, 5);	/* 初始化LCD时钟 51.2MHz */
	} 
    else if(lcdid == ATK1018) 
    {
		tftlcd_dev.height = 800;	
		tftlcd_dev.width = 1280;
		tftlcd_dev.vspw = 3;
		tftlcd_dev.vbpd = 10;
		tftlcd_dev.vfpd = 10;
		tftlcd_dev.hspw = 10;
		tftlcd_dev.hbpd = 80;
		tftlcd_dev.hfpd = 70;
		lcdclk_init(35, 3, 5);	/* 初始化LCD时钟 56MHz */
	} 
    else if(lcdid == ATKVGA) 
    {  
		tftlcd_dev.height = 768;	
		tftlcd_dev.width = 1366;
		tftlcd_dev.vspw = 3;
		tftlcd_dev.vbpd = 24;
		tftlcd_dev.vfpd = 3;
		tftlcd_dev.hspw = 143;
		tftlcd_dev.hbpd = 213;
		tftlcd_dev.hfpd = 70;
		lcdclk_init(32, 3, 3);	/* 初始化LCD时钟 85MHz */
	}
    
    tftlcd_dev.pixsize = 4;     /* 每个像素4字节 */
    tftlcd_dev.framebuffer = LCD_FRAMEBUFFER_ADDR;
    tftlcd_dev.backcolor = LCD_WHITE;
    tftlcd_dev.forecolor = LCD_BLACK;

    /* 配置LCDIF控制器接口 */
    LCDIF->CTRL |= (1 << 19) | (1 << 17) | (0 << 14) | (0 << 12) |
	 				(3 << 10) | (3 << 8) | (1 << 5) | (0 << 1);

    LCDIF->CTRL1 = 0X7 << 16; 

    LCDIF->TRANSFER_COUNT = (tftlcd_dev.height << 16) | (tftlcd_dev.width << 0);

    LCDIF->VDCTRL0 = 0;
    LCDIF->VDCTRL0 = (0 << 29) | (1 << 28) | (0 << 27) |
					 (0 << 26) | (0 << 25) | (1 << 24) |
					 (1 << 21) | (1 << 20) | (tftlcd_dev.vspw << 0);

    LCDIF->VDCTRL1 = tftlcd_dev.vspw + tftlcd_dev.vbpd + tftlcd_dev.height + tftlcd_dev.vfpd;

    LCDIF->VDCTRL2 = (tftlcd_dev.hspw + tftlcd_dev.hbpd + tftlcd_dev.width + tftlcd_dev.hfpd) | 
                    (tftlcd_dev.hspw << 18);

    LCDIF->VDCTRL3 = (tftlcd_dev.vspw + tftlcd_dev.vbpd) | 
                    ((tftlcd_dev.hspw + tftlcd_dev.hbpd) << 16);

    LCDIF->VDCTRL4 = (1 << 18) | (tftlcd_dev.width);

    LCDIF->CUR_BUF  = (unsigned int)tftlcd_dev.framebuffer;
    LCDIF->NEXT_BUF = (unsigned int)tftlcd_dev.framebuffer;

    lcd_enable();
    delayms(20);
    lcd_clear(LCD_WHITE);
}

/* 像素时钟初始化 
 * loopDive 设置DIV_SELECT，范围27-54
 * predive 设置为1~8
 * div 设置为1~8
 * LCD_CLK = 24 * loopDiv / prediv / div
 */
void lcdclk_init(unsigned char loopDiv, unsigned char prediv, unsigned char div)
{
    /* 不使用小数分频器 */
    CCM_ANALOG->PLL_VIDEO_NUM = 0;
    CCM_ANALOG->PLL_VIDEO_DENOM = 0;

    CCM_ANALOG->PLL_VIDEO = (1 << 13) | (2 << 19) | (loopDiv << 0);
    
    CCM_ANALOG->MISC2 &= ~(3 << 30);
    CCM_ANALOG->MISC2 = 0 << 30;

    CCM->CSCDR2 &= ~(7 << 15);
    CCM->CSCDR2 |=  (2 << 15);

    CCM->CSCDR2 &= ~(7 << 12);
    CCM->CSCDR2 |=  (prediv - 1) << 12;

    CCM->CBCMR  &= ~(7 << 23);
    CCM->CBCMR  |=  (div - 1) << 23;

    CCM->CSCDR2 &= ~(7 << 9);
    CCM->CSCDR2 |=  (0 << 9);
}

/* 复位LCD控制器 */
void lcd_reset(void) 
{
    LCDIF->CTRL = 1 << 31; /* 复位 */
}

/* 停止复位 */
void lcd_noreset(void)
{
    LCDIF->CTRL = 0 << 31; /* 停止复位 */
}

/* 使能lcd控制器 */
void lcd_enable(void) 
{
    LCDIF->CTRL |= 1 << 0; /* 使能 */
}

/*
 * 读取屏幕ID，
 * 描述：LCD_DATA23=R7(M0);LCD_DATA15=G7(M1);LCD_DATA07=B7(M2);
 * 		M2:M1:M0
 *		0 :0 :0	//4.3寸480*272 RGB屏,ID=0X4342
 *		0 :0 :1	//7寸800*480 RGB屏,ID=0X7084
 *	 	0 :1 :0	//7寸1024*600 RGB屏,ID=0X7016
 *  	1 :0 :1	//10.1寸1280*800,RGB屏,ID=0X1018
 *		1 :0 :0	//4.3寸800*480 RGB屏,ID=0X4384
 * @param 		: 无
 * @return 		: 屏幕ID
 */
unsigned short lcd_read_panelid(void)
{
    unsigned char idx = 0;

    /* 打开模拟开关,设置LCD_VSYNC为高电平 */
    gpio_pin_config_t lcdio_foncifg;
    IOMUXC_SetPinMux(IOMUXC_LCD_VSYNC_GPIO3_IO03,0);	/* 复用为GPIO3的IO3 */	
	IOMUXC_SetPinConfig(IOMUXC_LCD_VSYNC_GPIO3_IO03,0X10B0);

	/* 初始化GPIO */
    lcdio_foncifg.direction = kGPIO_DigitalOutput;
	lcdio_foncifg.outputLogic = 1;
    gpio_init(GPIO3, 3, &lcdio_foncifg);

    /* 读取屏幕id */
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA07_GPIO3_IO12,0);	/* B7(M2) */	
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA15_GPIO3_IO20,0);	/* G7(M1) */
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA23_GPIO3_IO28,0);	/* R7(M0) */

    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA07_GPIO3_IO12,0xF080);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA15_GPIO3_IO20,0xF080);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA23_GPIO3_IO28,0xF080);

    /* 设置io功能 */
    lcdio_foncifg.direction = kGPIO_DigitalInput;
    gpio_init(GPIO3, 12, &lcdio_foncifg);
    gpio_init(GPIO3, 20, &lcdio_foncifg);
    gpio_init(GPIO3, 28, &lcdio_foncifg);

    idx  = (unsigned char )gpio_pinread(GPIO3, 28);           /* M0 */
    idx |= (unsigned char )gpio_pinread(GPIO3, 20) << 1;      /* M1 */
    idx |= (unsigned char )gpio_pinread(GPIO3, 12) << 2;      /* M2 */

    if (idx == 0)       return ATK4342;
    else if (idx == 1)  return ATK7084;
    else if (idx == 2)  return ATK7016;
    else if (idx == 4)  return ATK4384;
    else if (idx == 5)  return ATK1018;
    else                return 0;
}

/* 屏幕io初始化 */
void lcdgpio_init(void)
{
    gpio_pin_config_t gpio_config;
	
	/* 1、IO初始化复用功能 */
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA00_LCDIF_DATA00,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA01_LCDIF_DATA01,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA02_LCDIF_DATA02,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA03_LCDIF_DATA03,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA04_LCDIF_DATA04,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA05_LCDIF_DATA05,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA06_LCDIF_DATA06,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA07_LCDIF_DATA07,0);
	
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA08_LCDIF_DATA08,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA09_LCDIF_DATA09,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA10_LCDIF_DATA10,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA11_LCDIF_DATA11,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA12_LCDIF_DATA12,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA13_LCDIF_DATA13,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA14_LCDIF_DATA14,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA15_LCDIF_DATA15,0);

	IOMUXC_SetPinMux(IOMUXC_LCD_DATA16_LCDIF_DATA16,0);
	
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA17_LCDIF_DATA17,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA18_LCDIF_DATA18,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA19_LCDIF_DATA19,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA20_LCDIF_DATA20,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA21_LCDIF_DATA21,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA22_LCDIF_DATA22,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA23_LCDIF_DATA23,0);

	IOMUXC_SetPinMux(IOMUXC_LCD_CLK_LCDIF_CLK,0);	
	IOMUXC_SetPinMux(IOMUXC_LCD_ENABLE_LCDIF_ENABLE,0);	
	IOMUXC_SetPinMux(IOMUXC_LCD_HSYNC_LCDIF_HSYNC,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_VSYNC_LCDIF_VSYNC,0);

	IOMUXC_SetPinMux(IOMUXC_GPIO1_IO08_GPIO1_IO08,0);			/* 背光BL引脚      */
	IOMUXC_SetPinConfig(IOMUXC_UART1_CTS_B_GPIO1_IO18,0xF080);
					
	/* 2、配置LCD IO属性 */
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA00_LCDIF_DATA00,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA01_LCDIF_DATA01,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA02_LCDIF_DATA02,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA03_LCDIF_DATA03,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA04_LCDIF_DATA04,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA05_LCDIF_DATA05,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA06_LCDIF_DATA06,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA07_LCDIF_DATA07,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA08_LCDIF_DATA08,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA09_LCDIF_DATA09,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA10_LCDIF_DATA10,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA11_LCDIF_DATA11,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA12_LCDIF_DATA12,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA13_LCDIF_DATA13,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA14_LCDIF_DATA14,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA15_LCDIF_DATA15,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA16_LCDIF_DATA16,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA17_LCDIF_DATA17,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA18_LCDIF_DATA18,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA19_LCDIF_DATA19,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA20_LCDIF_DATA20,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA21_LCDIF_DATA21,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA22_LCDIF_DATA22,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA23_LCDIF_DATA23,0xB9);

	IOMUXC_SetPinConfig(IOMUXC_LCD_CLK_LCDIF_CLK,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_ENABLE_LCDIF_ENABLE,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_HSYNC_LCDIF_HSYNC,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_VSYNC_LCDIF_VSYNC,0xB9);

	IOMUXC_SetPinConfig(IOMUXC_GPIO1_IO08_GPIO1_IO08,0xB9);	/* 背光BL引脚 		*/

	/* GPIO初始化 */
	gpio_config.direction = kGPIO_DigitalOutput;			/* 输出 			*/
	gpio_config.outputLogic = 1; 							/* 默认关闭背光 */
	gpio_init(GPIO1, 8, &gpio_config);						/* 背光默认打开 */
	gpio_pinwrite(GPIO1, 8, 1);								/* 打开背光     */
}

/* 画点函数 */
inline void lcd_drawpoint(unsigned short x, unsigned short y, unsigned int color)
{
    *(unsigned int *)((unsigned int)tftlcd_dev.framebuffer + 
                    tftlcd_dev.pixsize * (tftlcd_dev.width *y + x)) = color;
}

/* 读点函数 */
inline unsigned int lcd_readpoint(unsigned short x, unsigned short y)
{
    return *(unsigned int *)((unsigned int)tftlcd_dev.framebuffer + 
                    tftlcd_dev.pixsize * (tftlcd_dev.width *y + x));
}

/* 清屏 */
void lcd_clear(unsigned int color)
{
    unsigned int num;
    unsigned int i = 0;

    unsigned int *startaddr = (unsigned int *)tftlcd_dev.framebuffer;

    num = (unsigned int)tftlcd_dev.height * tftlcd_dev.width;

    for (i = 0; i < num; i++)
    {
        startaddr[i] = color;
    }
}

/* 填充函数 */
void lcd_fill(unsigned    short x0, unsigned short y0, 
                 unsigned short x1, unsigned short y1, unsigned int color)
{ 
    unsigned short x, y;

	if(x0 < 0) x0 = 0;
	if(y0 < 0) y0 = 0;
	if(x1 >= tftlcd_dev.width) x1 = tftlcd_dev.width - 1;
	if(y1 >= tftlcd_dev.height) y1 = tftlcd_dev.height - 1;
	
    for(y = y0; y <= y1; y++)
    {
        for(x = x0; x <= x1; x++)
			lcd_drawpoint(x, y, color);
    }
}
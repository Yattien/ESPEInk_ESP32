/**
  ******************************************************************************
  * @file    edp2in9.h
  * @author  Waveshare Team
  * @version V1.0.0
  * @date    23-January-2018
  * @brief   This file describes initialisation of 2.9 and 2.9b e-Papers
  *
  ******************************************************************************
  */
int EPD_Init_2in9() 
{
    EPD_Reset();
    EPD_Send_3(0x01, 39, 1, 0);//DRIVER_OUTPUT_CONTROL:LO(EPD_HEIGHT-1),HI(EPD_HEIGHT-1), GD = 0; SM = 0; TB = 0;
    EPD_Send_3(0x0C, 0xD7, 0xD6, 0x9D);//BOOSTER_SOFT_START_CONTROL
    EPD_Send_1(0x2C, 0xA8);//WRITE_VCOM_REGISTER:VCOM 7C
    EPD_Send_1(0x3A, 0x1A);//SET_DUMMY_LINE_PERIOD: 4 dummy lines per gate
    EPD_Send_1(0x3B, 0x08);//SET_GATE_TIME: 2us per line
    EPD_Send_1(0x11, 0x03);//DATA_ENTRY_MODE_SETTING: X increment; Y increment
 
    EPD_Send_2(0x44, 0, 15);//SET_RAM_X_ADDRESS_START_END_POSITION: LO(x >> 3), HI ((w-1) >> 3)
    EPD_Send_4(0x45, 0, 0, 45, 1);//SET_RAM_Y_ADDRESS_START_END_POSITION: LO(y), HI(y), LO(h - 1),HI(h - 1)

    EPD_Send_1(0x4E, 0);//SET_RAM_X_ADDRESS_COUNTER: LO(x >> 3)
    EPD_Send_2(0x4F, 0, 0);//SET_RAM_Y_ADDRESS_COUNTER: LO(y), HI(y)

    EPD_lut(0x32, 30, &lut_full_mono[0]);

    EPD_SendCommand(0x24);//WRITE_RAM
    delay(2);
    return 0;
}

int EPD_Init_2in9b() 
{
    EPD_Reset();
    EPD_Send_4(0x01, 0x07, 0x00, 0x08, 0x00);//POWER_SETTING
    EPD_Send_3(0x06, 0x17, 0x17, 0x17);//BOOSTER_SOFT_START
    EPD_SendCommand(0x04);//POWER_ON
    EPD_WaitUntilIdle();
    
    EPD_Send_1(0x00, 0x8F);//PANEL_SETTING
    EPD_Send_1(0x50, 0x77);//VCOM_AND_DATA_INTERVAL_SETTING;
    EPD_Send_1(0x30, 0x39);//PLL_CONTROL
    EPD_Send_3(0x61, 0x80, 0x01, 0x28);//TCON_RESOLUTION
    EPD_Send_1(0x82, 0x0A);//VCM_DC_SETTING_REGISTER

    EPD_SendCommand(0x10);//DATA_START_TRANSMISSION_1  
    delay(2);
    return 0;
}

/************************************************************************************/
#define UBYTE   uint8_t
#define UWORD   uint16_t
#define UDOUBLE uint32_t


#define EPD_2IN9D_WIDTH   128
#define EPD_2IN9D_HEIGHT  296

const unsigned char EPD_2IN9D_lut_vcomDC[] = {
    0x00, 0x08, 0x00, 0x00, 0x00, 0x02,
    0x60, 0x28, 0x28, 0x00, 0x00, 0x01,
    0x00, 0x14, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x12, 0x12, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00,
};
const unsigned char EPD_2IN9D_lut_ww[] = {
    0x40, 0x08, 0x00, 0x00, 0x00, 0x02,
    0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
    0x40, 0x14, 0x00, 0x00, 0x00, 0x01,
    0xA0, 0x12, 0x12, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
const unsigned char EPD_2IN9D_lut_bw[] = {
    0x40, 0x17, 0x00, 0x00, 0x00, 0x02,
    0x90, 0x0F, 0x0F, 0x00, 0x00, 0x03,
    0x40, 0x0A, 0x01, 0x00, 0x00, 0x01,
    0xA0, 0x0E, 0x0E, 0x00, 0x00, 0x02,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
const unsigned char EPD_2IN9D_lut_wb[] = {
    0x80, 0x08, 0x00, 0x00, 0x00, 0x02,
    0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
    0x80, 0x14, 0x00, 0x00, 0x00, 0x01,
    0x50, 0x12, 0x12, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
const unsigned char EPD_2IN9D_lut_bb[] = {
    0x80, 0x08, 0x00, 0x00, 0x00, 0x02,
    0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
    0x80, 0x14, 0x00, 0x00, 0x00, 0x01,
    0x50, 0x12, 0x12, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

void EPD_SetFullReg(void)
{
    EPD_SendCommand(0X50);			//VCOM AND DATA INTERVAL SETTING
    EPD_SendData(0xb7);		//WBmode:VBDF 17|D7 VBDW 97 VBDB 57		WBRmode:VBDF F7 VBDW 77 VBDB 37  VBDR B7

    unsigned int count;
    EPD_SendCommand(0x20);
    for(count=0; count<44; count++) {
        EPD_SendData(EPD_2IN9D_lut_vcomDC[count]);
    }

    EPD_SendCommand(0x21);
    for(count=0; count<42; count++) {
        EPD_SendData(EPD_2IN9D_lut_ww[count]);
    }

    EPD_SendCommand(0x22);
    for(count=0; count<42; count++) {
        EPD_SendData(EPD_2IN9D_lut_bw[count]);
    }

    EPD_SendCommand(0x23);
    for(count=0; count<42; count++) {
        EPD_SendData(EPD_2IN9D_lut_wb[count]);
    }

    EPD_SendCommand(0x24);
    for(count=0; count<42; count++) {
        EPD_SendData(EPD_2IN9D_lut_bb[count]);
    }
}

void EPD_2IN9D_ReadBusy(void)
{
  Serial.print("\r\ne-Paper busy");
    UBYTE busy;
    do {
        EPD_SendCommand(0x71);
        busy = digitalRead(PIN_SPI_BUSY);
        busy = !(busy & 0x01);
    } while(busy);
    delay(200);
    Serial.print("\r\ne-Paper busy free");
}

void EPD_2IN9D_Show(void)
{
  Serial.print("\r\nEPD_2IN9D_Show");
    EPD_SetFullReg();
    EPD_SendCommand(0x12);		 //DISPLAY REFRESH
    delay(10);     //!!!The delay here is necessary, 200uS at least!!!

    EPD_2IN9D_ReadBusy();

    // Sleep
    EPD_SendCommand(0X50);
    EPD_SendData(0xf7);
    EPD_SendCommand(0X02);  	//power off
    EPD_2IN9D_ReadBusy();
    EPD_SendCommand(0X07);  	//deep sleep
    EPD_SendData(0xA5);
}

void EPD_2IN9D_Clear(void)
{
    UWORD Width, Height;
    Width = (EPD_2IN9D_WIDTH % 8 == 0)? (EPD_2IN9D_WIDTH / 8 ): (EPD_2IN9D_WIDTH / 8 + 1);
    Height = EPD_2IN9D_HEIGHT;

    EPD_SendCommand(0x10);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            EPD_SendData(0x00);
        }
    }

    EPD_SendCommand(0x13);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            EPD_SendData(0xFF);
        }
    }

    // EPD_2IN9D_Show();
    EPD_SendCommand(0x13);
}

int EPD_Init_2in9d() 
{
    EPD_Reset();

    EPD_SendCommand(0x01);	//POWER SETTING
    EPD_SendData(0x03);
    EPD_SendData(0x00);
    EPD_SendData(0x2b);
    EPD_SendData(0x2b);
    EPD_SendData(0x03);

    EPD_SendCommand(0x06);	//boost soft start
    EPD_SendData(0x17);     //A
    EPD_SendData(0x17);     //B
    EPD_SendData(0x17);     //C

    EPD_SendCommand(0x04);
    EPD_2IN9D_ReadBusy();

    EPD_SendCommand(0x00);	//panel setting
    EPD_SendData(0xbf);     //LUT from OTP，128x296
    EPD_SendData(0x0e);     //VCOM to 0V fast

    EPD_SendCommand(0x30);	//PLL setting
    EPD_SendData(0x3a);     // 3a 100HZ   29 150Hz 39 200HZ	31 171HZ

    EPD_SendCommand(0x61);	//resolution setting
    EPD_SendData(EPD_2IN9D_WIDTH);
    EPD_SendData((EPD_2IN9D_HEIGHT >> 8) & 0xff);
    EPD_SendData(EPD_2IN9D_HEIGHT & 0xff);

    EPD_SendCommand(0x82);	//vcom_DC setting
    EPD_SendData(0x28);
    delay(2);

    EPD_2IN9D_Clear();

    return 0;
}


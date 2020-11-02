/*****************************************************************************
* | File      	:  	EPD_2in66.c
* | Author      :   Waveshare team
* | Function    :   2.66inch e-paper
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2020-08-10
* | Info        :
* -----------------------------------------------------------------------------
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/

// Display resolution
#define EPD_2IN66_WIDTH       152
#define EPD_2IN66_HEIGHT      296

/******************************************************************************
function :	Wait until the busy_pin goes LOW
parameter:
******************************************************************************/
static void EPD_2IN66_ReadBusy(void)
{
    Serial.print("e-Paper busy\r\n");
    delay(100);
    while(digitalRead(PIN_SPI_BUSY) == 1) {      //LOW: idle, HIGH: busy
        delay(100);
    }
    delay(100);
    Serial.print("e-Paper busy release\r\n");
}


/******************************************************************************
function :	Turn On Display
parameter:
******************************************************************************/
static void EPD_2IN66_Show(void)
{
    EPD_SendCommand(0x20);
    EPD_2IN66_ReadBusy();
	Serial.print("EPD_2IN66_Show END\r\n");
	
	EPD_SendCommand(0x10);//sleep
    EPD_SendData(0x01); 
}

/******************************************************************************
function :	Initialize the e-Paper register
parameter:
******************************************************************************/
int EPD_2IN66_Init(void)
{
    EPD_Reset();
    EPD_2IN66_ReadBusy();
    EPD_SendCommand(0x12);//soft  reset
    EPD_2IN66_ReadBusy();
	/*	Y increment, X increment	*/
	EPD_SendCommand(0x11);
	EPD_SendData(0x03);
	/*	Set RamX-address Start/End position	*/
	EPD_SendCommand(0x44);
	EPD_SendData(0x01);	
	EPD_SendData((EPD_2IN66_WIDTH % 8 == 0)? (EPD_2IN66_WIDTH / 8 ): (EPD_2IN66_WIDTH / 8 + 1) );
	/*	Set RamY-address Start/End position	*/
	EPD_SendCommand(0x45);
	EPD_SendData(0);
	EPD_SendData(0);
	EPD_SendData((EPD_2IN66_HEIGHT&0xff));
	EPD_SendData((EPD_2IN66_HEIGHT&0x100)>>8);

	EPD_2IN66_ReadBusy();
	
	EPD_SendCommand(0x24);//show
	
	return 0;
}


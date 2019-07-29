/*
 * 
 * Gateway Module
 * UART Communication between TORCS(PC) and Gateway
 * CAN communication between Gateway and Electronic Control Unit(ECU/Hercules) 
 *
 */

#include "CLK.h"
#include "UART.h"
#include <stdio.h>
#include "typedefs.h"
#include "msCANdrv.h"
#include "derivative.h" /* include peripheral declarations */
#include "MSCAN_Module.h"

void Led_init();
void Led_toggle();
void Initialize_UART();
uint8 Initialize_CAN();
void delay_ms(UINT16 delay);
void Uart2_Receive(UINT8 rxBuffer);
void Enable_Interrupt(UINT8 vector_number);
void Disable_Interrupt(UINT8 vector_number);

//static uint8 buffer[100]={0};

int main(void)
{
	uint8 indx = 0;

	Clk_Init();
	Led_init();
	Initialize_UART();
	Initialize_CAN();

	/* Start up: Toggle LED's */
	for(indx = 0; indx < 16; indx++) {
		Led_toggle();
		delay_ms(100);
	}
	
	Enable_Interrupt(INT_UART2); //Enable UART interrupt
	//EnableInterrupts; //Enable CAN interrupt

	/* CAN Transmission is carried out in Interrupt Handler; */
	
	while(1);
	
#if 0	
	while (1) 
	{
	    Check_CAN_MB_Status(0, 0, buffer_status);
	    while (buffer_status[0] != NEWDATA) //Wait for the Receive ISR to finish and change the buffer status, NEWDATA indicates that the buffer has receive a new data
		{	
			Check_CAN_MB_Status(0, 0, buffer_status);
		}
		Read_CAN_MB_Data(0, 0, rx_data);

		indx = 1;
		
		//printf("Rx <-----");
		//for(indx = 0; indx < 9; indx++)
		//{
			Uart_SendChar(rx_data[indx]);
//			Led_toggle();
//			delay_ms(4);
			Enable_Interrupt(INT_UART2);
		//}
	}
#endif
	
	return 0;
}

/* Uart2 Recieve interrupt handler */
void Uart2_Receive(UINT8 rxBuffer)
{
	/* MATLAB sends float data (4 bytes) in BIG endian format */
/*	static int i=0;
	buffer[i] = rxBuffer;
	i++;
	if (i == 100){
		i = 0;
	}*/
	static uint8 countNthrow = 0;
	countNthrow++;
	static uint8 transmit = 1; 
	static uint8 dataset = 0;
	
	uint8 indx = 0;
	static uint8 can_data_len = 0;
	static uint8 data[9] = {8, 0, 0, 0, 0, 0, 0, 0, 0};
	can_data_len++;
	data[can_data_len] = rxBuffer;
	
	if(can_data_len == 8)
	{
		Disable_Interrupt(INT_UART2);
		
		if(transmit == 1)
		{
		
			if(dataset == 0)
			{
				Abort_CAN_MB (0, 1);
				if(Load_CAN_MB (0, 1, data) == ERR_OK)//Load the data to the buffer of Channel 0 buffer 1 0x81
				{
					Transmit_CAN_MB(0, 1);//Transmit the frame of Channel 0 buffer 1;
					Led_toggle();
				}
			}
			if(dataset == 1)
			{
				Abort_CAN_MB (0, 2);
				if (Load_CAN_MB (0, 2, data) == ERR_OK)//Load the data to the buffer of Channel 0 buffer 2 0x82
				{
					Transmit_CAN_MB(0, 2);//Transmit the frame of Channel 0 buffer 2;
					Led_toggle();
				}
				
			}
			if(dataset == 2)
			{
				Abort_CAN_MB (0, 3);
				if (Load_CAN_MB (0, 3, data) == ERR_OK) //Load the data to the buffer of Channel 0 buffer 3 0x83
				{
					Transmit_CAN_MB(0, 3);//Transmit the frame of Channel 0 buffer 3;
					Led_toggle();
				}
			}
			if(dataset == 3)
			{
				Abort_CAN_MB (0, 4);
				if (Load_CAN_MB (0, 4, data) == ERR_OK)//Load the data to the buffer of Channel 0 buffer 4 0x84
				{
					Transmit_CAN_MB(0, 4);//Transmit the frame of Channel 0 buffer 4;
					Led_toggle();
				}
				
			}
			if(dataset == 4)
			{
				Abort_CAN_MB (0, 5);
				if (Load_CAN_MB (0, 5, data) == ERR_OK)//Load the data to the buffer of Channel 0 buffer 5 0x85
				{
					Transmit_CAN_MB(0, 5);//Transmit the frame of Channel 0 buffer 5;
					Led_toggle();
				}
				
			}
			
//			if(dataset == 5)
//			{
//				Abort_CAN_MB (0, 6);
//				if (Load_CAN_MB (0, 6, data) == ERR_OK)//Load the data to the buffer of Channel 0 buffer 6 0x86
//				{
//					Transmit_CAN_MB(0, 6);//Transmit the frame of Channel 0 buffer 6;
//					Led_toggle();
//				}
//			}
//			if(dataset == 6)
//			{
//				Abort_CAN_MB (0, 7);
//				if (Load_CAN_MB (0, 7, data) == ERR_OK) //Load the data to the buffer of Channel 0 buffer 7 0x87
//				{
//					Transmit_CAN_MB(0, 7);//Transmit the frame of Channel 0 buffer 1;
//					Led_toggle();
//				}
//				
//			}
//			if(dataset == 7)
//			{
//				Abort_CAN_MB (0, 8);
//				//Load_CAN_MB (0, 8, data);//Load the data to the buffer of Channel 0 buffer 8 0x88
//				
//				if (Load_CAN_MB (0, 8, data) == ERR_OK)//Load the data to the buffer of Channel 0 buffer 8 0x88
//				{
//					Transmit_CAN_MB(0, 8);//Transmit the frame of Channel 0 buffer 8;
//					Led_toggle();
//				}
//				
//			}
//			if(dataset == 8)
//			{
//				Abort_CAN_MB (0, 9);
//				if (Load_CAN_MB (0, 9, data) == ERR_OK)//Load the data to the buffer of Channel 0 buffer 9 0x89
//				{
//					Transmit_CAN_MB(0, 9);//Transmit the frame of Channel 0 buffer 9;
//					Led_toggle();
//				}
//				
//			}
	
			
			dataset++;
			
			if(dataset == 5)
			{
				dataset = 0;
			}
		}
	
	
		for(indx = 1; indx < 9; indx++)
		{
			data[indx] = 0;
		}
		can_data_len = 0;
		Enable_Interrupt(INT_UART2);
	}
	
	if (countNthrow == 80)
	{
		can_data_len = 0;
		countNthrow = 0;
		transmit = 1 - transmit;
	}
	
}

/* Initializing UART and  interrupt handler*/ 
void Initialize_UART()
{
	UART_Init();
	Uart_SetCallback(Uart2_Receive); 
}

/* Initializing CAN and interrupt handler*/
uint8 Initialize_CAN()
{
	uint8 CAN_status[3];
	uint8 err_status;
	
	MSCAN_ModuleEn();
	GPIOB_PDDR |= (1<<16); //(Workaround for new kinteis revision(schematic D))to enable STB mode in MC33901 (CAN controller chip)
	GPIOB_PCOR = (1<<16);

	Init_CAN(0, CMPTX); //Initialize msCAN channel 0 and switch without completing current transmission(FAST)
	 
	Config_CAN_MB (0, 1, TXDF, 1); //Channel: 0, Buffer/Message object: 1, ID: 0x81, TX ----> ECU
	Config_CAN_MB (0, 2, TXDF, 2); //Channel: 0, Buffer/Message object: 2, ID: 0x82, TX ----> ECU
	Config_CAN_MB (0, 3, TXDF, 3); //Channel: 0, Buffer/Message object: 2, ID: 0x83, TX ----> ECU
	Config_CAN_MB (0, 4, TXDF, 4); //Channel: 0, Buffer/Message object: 2, ID: 0x84, TX ----> ECU
	Config_CAN_MB (0, 5, TXDF, 5); //Channel: 0, Buffer/Message object: 2, ID: 0x85, TX ----> ECU
	
//	Config_CAN_MB (0, 6, TXDF, 6); //Channel: 0, Buffer/Message object: 2, ID: 0x86, TX ----> ECU
//	Config_CAN_MB (0, 7, TXDF, 7); //Channel: 0, Buffer/Message object: 2, ID: 0x87, TX ----> ECU
//	Config_CAN_MB (0, 8, TXDF, 8); //Channel: 0, Buffer/Message object: 2, ID: 0x88, TX ----> ECU
//	Config_CAN_MB (0, 9, TXDF, 9); //Channel: 0, Buffer/Message object: 2, ID: 0x89, TX ----> ECU
	
//	Config_CAN_MB (1, 5, TXDF, 5); //Channel: 0, Buffer/Message object: 2, ID: 0x85, TX ----> ECU
//	Config_CAN_MB (1, 6, TXDF, 6); //Channel: 0, Buffer/Message object: 2, ID: 0x86, TX ----> ECU
//	Config_CAN_MB (1, 7, TXDF, 7); //Channel: 0, Buffer/Message object: 2, ID: 0x87, TX ----> ECU
//	Config_CAN_MB (1, 8, TXDF, 8); //Channel: 0, Buffer/Message object: 2, ID: 0x88, TX ----> ECU
//	Config_CAN_MB (1, 9, TXDF, 9); //Channel: 0, Buffer/Message object: 2, ID: 0x89, TX ----> ECU
	
	//Config_CAN_MB (0, 0, RXDF, 0); //Channel: 0, Buffer/Message object: 0, ID: 0x80, RX <---- ECU
	
    while ((CAN_status[0] & SYNCH) == 0) //Wait for msCAN channel 0 synchronized to CAN Bus
    {	
        err_status = Check_CAN_Status(0, CAN_status);
    }
    if(err_status == ERR_OK)
    	return ERR_OK;
    else 
    	return err_status;
    
    while ((CAN_status[0] & SYNCH) == 0) //Wait for msCAN channel 0 synchronized to CAN Bus
        {	
            err_status = Check_CAN_Status(1, CAN_status);
        }
        if(err_status == ERR_OK)
        	return ERR_OK;
        else 
        	return err_status;
}

/* Enable UART interrupt */
void Enable_Interrupt(UINT8 vector_number)
{
	vector_number= vector_number - 16;
	/* Set the ICPR and ISER registers accordingly */
	NVIC_ICPR |= 1 << (vector_number%32);
	NVIC_ISER |= 1 << (vector_number%32);
}

/* Disable UART interrupt */
void Disable_Interrupt(UINT8 vector_number)
{
	vector_number= vector_number-16;
	
	/* Clear the ICPR and ISER registers accordingly */
	NVIC_ICPR &= 1 << ~(vector_number%32);
	NVIC_ISER &= 1 << ~(vector_number%32);

}

void delay_ms(UINT16 delay)
{
	UINT32 i=0;
	while (i<delay*1333)
	{ i++; }	
}

void Led_init()
{
	GPIOA_PDDR|=1 <<16; /* Set pin in port PTC0 as output*/
	GPIOA_PDDR|=1 <<17;
	GPIOA_PDDR|=1 <<18;
	GPIOA_PDDR|=1 <<19;
}

void Led_toggle()
{
	static int i = 16;
	GPIOA_PDOR ^= 1<<i++;
	if(i == 20)
		i = 16;
}
void Led_one()
{
	static int i = 16;
	GPIOA_PDOR ^= 1<<16;
}
void Led_two()
{
	static int i = 16;
	GPIOA_PDOR ^= 1<<17;
}
void Led_three()
{
	static int i = 16;
	GPIOA_PDOR ^= 1<<18;
}

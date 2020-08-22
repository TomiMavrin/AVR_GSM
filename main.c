#define F_CPU 7372800UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lcd.h"


#define FOSC 7372800UL// Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1 // Ne radi -> ovo iznosi 47 -> 51 radi

void USART_Init( unsigned int ubrr){
	/* Set baud rate */
	UBRRH = (unsigned char)(ubrr>>8);
	UBRRL = (unsigned char)ubrr;
	/* Enable receiver and transmitter */
	UCSRB = (1 << RXEN) | (1 << TXEN);
	/* Set frame format: 8data, 2stop bit */
	UCSRC = (1 << URSEL) | (1 << USBS) | (3 << UCSZ0);
}

unsigned char USART_Receive( void ){

	/* Wait for data to be received */
	while ( !(UCSRA & (1<<RXC)) )
	;
	/* Get and return received data from buffer */
	return UDR;
}

void USART_Receive_Line(char data[] ){
	unsigned int i = 0;
	while(data[i-1] != 13){
		data[i] = USART_Receive();
		i++;
	}
	data[i] = USART_Receive();
	data[i-1] = '\0';
}

void USART_Transmit( unsigned char data ){
	
	/* Wait for empty transmit buffer */
	while ( !( UCSRA & (1<<UDRE)) )
	;
	/* Put data into buffer, sends the data */
	UDR = data;
	
}

void USART_Transmits(char data[] ) {
	int i;

	for(i = 0; i < strlen(data); i++) {
		USART_Transmit(data[i]);
		_delay_ms(25);
	}
}


void USART_Wait_For(char data[]){
	char line[64];
	
	int row=0;
	int compare = 1;
	while(compare != 0){
		USART_Receive_Line(line);
		compare = strncmp(data, line, strlen(data));
	
		lcd_puts(line);
		lcd_puts("\n");
		_delay_ms(2000);
		lcd_clrscr();
	}
}

void USART_Retry_Until(char command[], char message[]){
	char line[64];
	USART_Transmits(command);
	
	int compare = 1;
	while(compare != 0){
		USART_Receive_Line(line);
		compare = strncmp(message, line, strlen(message));
		
		if(compare == 0){
			break;
		} else {
			_delay_ms(1000);
			USART_Transmits(command);
		}
	}
}

void GSM_Read_Msg(){
	char message1[128];
	char message2[128];
	
	USART_Receive_Line(message1); // get first line in message ignore that
	USART_Receive_Line(message2); // get the actual message
	
	lcd_puts(message1);
	_delay_ms(5000);
	
	lcd_clrscr();
	
	lcd_puts(message2);

	//USART_Wait_For("OK"); // last one should be "OK"
}


int main( void ){
	DDRD = _BV(4);
	USART_Init ( 51 );
	lcd_init(LCD_DISP_ON);
	lcd_clrscr();

	_delay_ms(2000);
	
	lcd_puts("Booting...");
	
	/*	
	char sind11[] = "+SIND: 11";
	_delay_ms(1000);
	USART_Wait_For(sind11);
	lcd_clrscr();
	lcd_puts("Success!\nConnected!\n");

	_delay_ms(5000);
	
	lcd_puts("SENDING SMS");
	USART_Retry_Until("AT+CMGF=1\r", "OK");
	
	lcd_clrscr();
	_delay_ms(2000);



	USART_Transmits("AT+CMGS=\"0998304164\"\r");
	
	_delay_ms(2000);
	
	USART_Transmits("Posalji nest nazaj npr 20 c\x1A\r");
	
	USART_Wait_For("OK");
	
	lcd_clrscr();

	
	lcd_puts("Waiting 30s for\nresponse");
	
	_delay_ms(30000);
	*/
	
	lcd_clrscr();
	
	USART_Retry_Until("AT+CMGF=1\r", "OK");
	_delay_ms(1000);
	
	lcd_clrscr();
	USART_Transmits("AT+CMGR=4\r");
	
	
	GSM_Read_Msg();
	
	while(1){
		_delay_ms(500);
	}
	 
	return 0;
}
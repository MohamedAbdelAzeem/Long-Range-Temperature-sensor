/*
 * GccApplication1.c
 *
 * Created: 5/12/2020 7:41:17 PM
 * Author : ZAMALA
 */ 
#define F_CPU 1000000UL   //to tell the compiler how fast your micro controller is which is in the data sheet
#include <avr/io.h>
#include <util/delay.h>
#include <math.h>
#include <stdbool.h>	//to define the boolean data type

#define LCD_DATA PORTB    // define that port B is connected to data input of the LCD
#define en PIND4	      // define that Enable Pin is connected to PIN4 in PORTD  D4
#define rw PIND5          // define that read/write pin is connected to PIN4 in port D
#define rs PIND6          // define that Reset pin is connected to PIN6 in port D

/********************** LCD Functions Prototype ******************************/
void init_LCD(void);					//LCD initialization 
void LCD_cmd(unsigned char cmd);		//Sending commands to LCD  
void LCD_write(unsigned char data);		//Writing an character on the LCD
void LCD_write_str(unsigned char *str);	//Writing an entire String on the LCD
void cursor_position(unsigned char x_pos , unsigned char y_pos); //Moving the cursor position
/****************************** ADC settings *****************************/
void init_ADC(void);					//ADC initialization 
uint16_t ADC_Read(uint8_t channel);		//ADC reading form a specific channel(pin)
/***************************** Temperature Functions *******************/
void temp_avg(int *readings,  int *temp_sum); //Printing the temp avg on the LCD




int main(void)
{
	DDRB = 0xFF; // Port B set as output
	DDRD = 0xFF; // port D set as output
	DDRC = 0X00;  // PORT C set as input
		
	float data_read;
	double temp_volt;
	int temp_sum = 0 ;
	int temp;
	int readings[60]={0};
	int seconds = 0;
	bool flag = false;
	
	init_LCD();   // initialize LCD 
	init_ADC();   // initialize ADC
	_delay_ms(100);
	
	LCD_cmd(0x0E);  // Display ON , Cursor ON
	_delay_ms(100);
	

    while (1) 
    {		
		char num[8] ={0};
		// taking reading from Pin 0 through the ADC.
		data_read = ADC_Read(0);   //this var should take a value from 0 to 1023 (10 bit ADC).
		temp_volt = data_read*5/1023;  // this value is a voltage in range (0-5) corresponding to a certain temp 
		temp = temp_volt * (27000/481) + (37080/481)+1; 
		// alarm detection
		if( temp >= 350)
		{
			PORTD |= (1<<PIND2); //Put 1 on the output pin: Port D pin 2
		}
		else
		{
			PORTD &= ~(1 <<PIND2);//Put 0 on the output pin: Port D pin 2
		}
		if( seconds ==0){
			temp++;
		}
		/* first line in the LCD */
		
		LCD_write_str("Current:");  // data sent converted to ascii code  total delay= 102ms * num of character
		itoa(temp, num , 10) ;      // convert the temp to an array of characters
		LCD_write_str(num);		//print the string on the LCD
		LCD_write(' ');		
		itoa(seconds, num , 10) ;
		LCD_write_str(num);			//Printing seconds indicator 
				
		// move to the second line
		cursor_position(0,1);
		if(seconds == 59)
		{
			// calculate the avg and print it
			readings[seconds]= temp ;
			seconds = 0;
			flag = true;
			temp_avg(readings, &temp_sum);
		}
		else if (flag)
		{
			readings[seconds]= temp;
			seconds++;
			temp_avg(readings, &temp_sum);
		}
		else if (flag == false)
		{
			readings[seconds]= temp;
			seconds++;
		}
		temp_sum = 0;
		LCD_cmd(0x0E);  // Display ON
		_delay_ms(1000); 
		LCD_cmd(0x01); // LCD clear

    }
}





/*******************************Functions**********************************/
void init_LCD()
{
	//Any address need to be sent to LCD must go through 3 steps
	/*
      1.Reset
	  2.Write
	  3. reactive the ENABLE  high then low
	  
	  these 3 steps will be written in LCD_cmd Function. 
	*/
	_delay_ms(30);  // 30ms before doing anything
	
	LCD_cmd(0x38);    // 0X38 is the address required to initialize the LCD in 8bit mode 
	_delay_ms(1);
	LCD_cmd(0x01);   //0x01 is the address to clear the screen
	_delay_ms(1);
	LCD_cmd(0x02);   //Return HOME
	_delay_ms(1);
	LCD_cmd(0x06);   //this address make the LCD increment in cursor
	_delay_ms(1);
	LCD_cmd(0x80);     // Go to the first position at the first line
	_delay_ms(1);	
}


void LCD_cmd(unsigned char cmd)
{
	/*This Function do 3 steps mentioned above */
	LCD_DATA = cmd ; //put the address cmd on PORTB which is LCD_DATA
	// 1. Reset = 0 
	PORTD &= ~(1 << rs);
	// 2. write = 0 
	PORTD &= ~(1<< rw);
	// 2.Enable = high
	PORTD |= (1 << en);
	_delay_ms(2);
	// Then LOW
	PORTD &= ~(1 << en);
}

void LCD_write(unsigned char data)
{
	/*
	to make the LCD know if the sent data is DATA NOT address 
	we put reset = 0  if we send address 
	we put reset = 1   if we send DATA 
	
	and it is same as LCD_cmd but the difference is Reset
	*/
	LCD_DATA = data ; //put the address cmd on PORTB which is LCD_DATA
	// 1. Reset = 1
	PORTD |= (1 << rs);
	// 2. write = 0
	PORTD &= ~(1<< rw);
	// 2.Enable = high
	PORTD |= (1 << en);
	_delay_ms(2);
	// Then LOW
	PORTD &= ~(1 << en);
}


void LCD_write_str(unsigned char *str)
{
	int i;
	for(i=0;str[i]!=0;i++)  /* send each char of string till the NULL */
	{
		LCD_write(str[i]);  /* call LCD data write */
	}
	
}

void cursor_position(unsigned char x_pos , unsigned char y_pos)
{
	/*
	x_pos for x_axis 0->16
	y_pos for y_axis 0-1
	*/
	// line 1 : col1-> 0x80 , col2-> 0x81 ,..etc 
	// line 2 : col1 -> 0xc0, col2-> 0xc1 ...etc
	uint8_t address =  0 ;
	if (y_pos == 0)
	{
		address =  0x80;
	}
	else if( y_pos == 1)
	{
		address = 0xC0;
	}
	if(x_pos < 16)
	{
		address += x_pos ;
	}
	LCD_cmd(address);
}

void init_ADC(void){
	ADMUX |= (1<<REFS0);    //setting voltage ref = Vcc 
	ADCSRA |= (1<<ADEN);    // Turn ADC ON
	ADCSRA |= (1<<ADSC) ; 
	ADCSRA |= (1<<ADPS2) |(1<<ADPS1)|(1<<ADPS0);
}

uint16_t ADC_Read(uint8_t channel){
	ADMUX &= 0xF0;     // clear the older channel before any new conversion 
	ADMUX |= channel    ;     // defines the new ADC channel for the new reading
	ADCSRA |= (1<<ADSC);    // ADC must be Enabled  for every single conversion  (start new conversion)
	while(ADCSRA & (1<<ADSC));  //WAIt until conversion is done
	return ADC;
}

void temp_avg(int *readings, int *temp_sum)
{
	int temp_avg = 0;
	char num2[8] = {0};  //An array to hold the characters of the num
	for (uint8_t i= 0 ; i <= 59 ;i++)
	{
		*temp_sum += readings[i];
	}
	temp_avg = *temp_sum / 60;
	//Printing the avg
	LCD_write_str("Temp_avg:");
	itoa((int)temp_avg, num2 , 10);
	LCD_write_str(num2);
	
}
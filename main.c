#include <8051.h>
#define LCD_INST 0xFFC2
#define LCD_DATA 0xFFD2
//#define LCD_BUSY 0xFFE2
//#define DISPLAY_R 0xFFF2

// pointers to LCD registers 
static char far at LCD_DATA dado;
static char far at LCD_INST inst;

// map of the LCD address to make more easy to work
unsigned char lcdAddressMap[4][16] = {
{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
{0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F},
{0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F},
{0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F}};

#define LCD_ROWS 4
#define LCD_COLUMNS 16

bit rowDir = 1;  // row direction flag
bit columnDir = 1; // collumn direction flag
unsigned int column = 0; // collumn position of the character
unsigned int row = 0; // row position of the character
unsigned char character; // the character!
bit move = 1; // movement flag

unsigned int Delay = 200; // Prestes a sumir
unsigned int Tm0 = 0;

char rcvd = 0;

// Prestes a sumir
void delay(unsigned int period){ 
	int i = 0;
	while (period--) 
		for ( i = 0; i < 255; i++);
}

// send a command to the LCD
void lcdCMD(unsigned char cmd) {
	delay(2); // wait lcd
	inst = cmd; // Write the instruction
	delay(2); // wait lcd
}

// clear the LCD
void lcdClear(){
	lcdCMD(0x01); // send command to clear
}

// send the LCD cursor to home
void lcdHome(){
	lcdCMD(0x02);  // send command to move the lcd cursor to home
}

// move the cursor to a row and a column
void lcdSetCursor(unsigned char row, unsigned char column) {
	lcdCMD(0x80 +  lcdAddressMap[row][column]);
}

// write a character to the LCD
void lcdSend(unsigned char c) {
	delay(2); // wait lcd
	dado = c; // write the data
	delay(2); // wait lcd
}

// write a character at a row and column positon
void lcdWrite(int row, int column, unsigned char c) {
	lcdSetCursor(row, column);
	lcdSend(c);
}

// send a character 
void serialSend(unsigned char c) {
	SBUF = c; // write the character
	while(!TI); // wait the send 
	TI = 0; // clear the flag
}

void incRow() {
	if (row < LCD_ROWS - 1) row++;
}

void decRow() {
	if (row > 0) row--;
}

void incColumn() {
	if (column < LCD_COLUMNS - 1)  column++;
}

void decColumn() {
	if (column > 0) column--;
}

void prog(){
	IE = 0x92;// 10000010 Serial e timer 0
  	TMOD = 0x21;// timer 0 habilitado
  	SCON = 0x50;// Serial
  	// Timers
	TH0 = 0; // Timer 0 HIGH
	TL0 = 0; // Timer 0 LOW
	TH1 = 0;
	TF0 = 0; // Flag de overflow do Timer 0
	TR0 = 1; // Inicia (1) ou para (0) Timer 0
	TF1 = 0; // Flag de overflow do Timer 0
	TR1 = 1; // Inicia (1) ou para (0) Timer 0
	TH0 = 0x4B;
	TL0 = 0xFF;
	TH1 = 0xFD;
}

void progS(){
	TMOD = 0x20;//somente timer 1 habilitado
	TH1 = 0xFD;
	TL1 = 0xFD;
	TR1 = 1;
	SCON = 0x40;//Serial modo 1
	REN = 1;
//	TF1 = 0;
}

void main(void){

	//prog();
	progS();
	
	lcdCMD(0x38);	// Setup inicial
	lcdCMD(0x06);	// Setup inicial
	lcdCMD(0x0C);	// Setup inicial
	
	character = '!';
	
	while(1) {


		if(RI) { // if receveid some data from serial
		
			RI = 0; // clear the flag
			
			rcvd = SBUF; 
			
			switch (rcvd) { 
				
				case '5': move = ! move; break;
				
				case '-': if (move) Delay = Delay + 20; break;
				
				case '+': if (move) Delay = Delay > 20 ? Delay - 20 : Delay; break;
				
				case '2': incRow(); break;
				
				case '8': decRow(); break;
				
				case '4': decColumn(); break;
				
				case '6': incColumn(); break;
				
				default: character = rcvd; break;	
			}
			
			lcdClear();
			lcdWrite(row, column, character);
			
		}
		
		if (move) {
		
			lcdClear();
			
			lcdWrite(row, column, character);
			
			delay(Delay);
			
			rowDir ? incRow() : decRow();
			columnDir ? incColumn() : decColumn();
			
			if (row <= 0 || row >= LCD_ROWS - 1) 
				rowDir = ! rowDir;
			
			if (column <= 0 || column >= LCD_COLUMNS - 1) 
				columnDir = ! columnDir;
				
		}
	
		if(Tm0 >= 20){
      			Tm0 = 0;
     	 		P1_0 = !P1_0;
    		}
    		
	}
	
}

//void Timer_0() interrupt 1{
//  	Tm0++;
//  	TH0 = 0x4B;
//	TL0 = 0xFF;
//}

//void ChangeAni() interrupt 4{
//	RI = 0;//Preparado para mais um recebimento
//	Ani = SBUF;//Passa o dado recebido para a variavel
//	SBUF = 65;//ACK
//  	while(!TI);//Transmite ACK
 // 	TI = 0;//Preparado para mais uma transmissao
//}
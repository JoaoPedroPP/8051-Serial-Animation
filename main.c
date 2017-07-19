#include <8051.h>
#define LCD_INST 0xFFC2
#define LCD_DADO 0xFFD2
#define LCD_OCUPADO 0xFFE2
#define DISPLAY_R 0xFFF2

static char far at LCD_DADO dado;
static char far at LCD_INST inst;
//static char far at DISPLAY_IR Display_dado ;
//static char far at DISPLAY_IR Display_dado ;
unsigned int Tm0 = 0;
bit H = 0;//Flag (0)indica que o cursor deve se mover para baixo, (1) para cima
bit L = 0;//Flag (0)indica que o cursor deve se mover para a direita, (1) para a esquerda
unsigned int D = 0;
unsigned int A = 0;
unsigned char Ani;
unsigned int Delay = 500;

unsigned char lcdAddress[4][16] = {
{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
{0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F},
{0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F},
{0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F}};

void delay(unsigned int period){ 
	int i = 0;
	while (period--) 
		for ( i = 0; i < 255; i++);
}

void lcdCMD(unsigned char cmd) {
	delay(2);
	inst = cmd;
	delay(2);
}

void lcdWrite(unsigned char c) {
	delay(2);
	dado = c;
	delay(2);
}

void lcdClear(){
	lcdCMD(0x01);
}

void lcdHome(){
	lcdCMD(0x02);
}

void lcdSetCursor(unsigned char row, unsigned char collumn) {
	lcdCMD(0x80 + lcdAddress[row][collumn]);
}

//void lcdWrite(int row, int collumn, unsigned char c) {
	
//}
//void lcdPrint(unsigned char s[]) {
//	int i;
//	for (i = 0; s[i] != 0; lcdWrite(s[i]);
//}

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

/*void EscreveInst(){
	P2_3 = 0;
	P2_2 = 1;
	P2_2 = 0;
	if(!TF0);
}

void Display_Inicia(){
	Display_dado = 0x02;
}
*/
void main(void){
	//prog();
	progS();
	lcdCMD(0x38);//Setup inicial
	lcdCMD(0x06);//Setup inicial
	lcdCMD(0x0C);//Setup inicial
	
	Ani = 'A';
	/*
	lcdHome();//Posiciona o cursor na posição [0][0]
	lcdClear();//Limpa o conteudo da posição
	lcdSetCursor(2,6);//Coloca o cursor na posição desejada
	lcdWrite('A');//Escreve no cursor a letra desejada
	delay(1000);
	lcdSetCursor(3, 15);
	lcdWrite('B');
	delay(1000);
	lcdCMD(0x01);
	lcdWrite('C');
	delay(1000);
	lcdCMD(0x01);
	lcdCMD(0x02);
	lcdWrite('D');
	delay(1000);*/
	
	while(1){
		//REN = 1;
		if(RI){
			RI = 0;
			switch (SBUF){
				case '-':
					Delay = Delay + 50;
					break;
				case '+':
					Delay = Delay - 50;
					break;
				default:
					Ani = SBUF;
					break;
			}/*
			if(SBUF == 'A')	{while(!RI); RI = 0; Ani = SBUF;}
			if(SBUF == '+') Delay = Delay + 50;
			if(SBUF == '-') Delay = Delay - 50;
			//if(SBUF == 'D') {if(A < 2){A++;}};
			//if(SBUF == 'E') {if(A > 1){A--;}};
			//if(SBUF == 'H') A = 0; D = 0;*/
			SBUF = 'A';
			while(!TI);
			TI = 0;
		}
		lcdClear();
		lcdSetCursor(A, D);
		lcdWrite(Ani);
		delay(Delay);
		lcdClear();
		if(!H) A++;
		if(H) A--;
		if(A == 3) H = !H;
		if(A == 0) H = !H;
		if(!L) D++;
		if(L) D--;
		if(D == 15) L = !L;
		if(D == 0) L = !L;
	
	
		if(Tm0 >= 20){
      			Tm0 = 0;
     	 		P1_0 = !P1_0;
    		}
	}
}

void Timer_0() interrupt 1{
  	Tm0++;
  	TH0 = 0x4B;
	TL0 = 0xFF;
}

void ChangeAni() interrupt 4{
	RI = 0;//Preparado para mais um recebimento
	Ani = SBUF;//Passa o dado recebido para a variavel
	SBUF = 65;//ACK
  	while(!TI);//Transmite ACK
  	TI = 0;//Preparado para mais uma transmissao
}
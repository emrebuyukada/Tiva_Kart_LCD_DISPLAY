#include <stdint.h>//standart kütüphanemiz
#include <stdlib.h>
#include <stdbool.h>//true or false için gerekli kutuphane
#include "inc/tm4c123gh6pm.h"//tiva kartının kütüphanesi

#include "driverlib/rom_map.h"//bu kütüphaneyi internette gördüğümden ekledim ne olur ne olmaz
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h" //lcd deki pinler için gerekli
#include "driverlib/sysctl.h"

//http://mostlyanalog.blogspot.com.tr/2015/07/lcd-display-library-for-tiva-and.html
//bu internet sitesinden yararlandım

//bu port açma yerleri tamamıyla yap benzet sayfasından
// derstede uyguladıgımız gibi oradan kopyaladım ve istediğim port açma
//ayarlarınıkendim yaptım
void init_port_C()
{
	//yap benzetten aldıgım  kısım port C nın sadece pC4 ve pC5 i aktifleştirme kısmını değiştirdim
   volatile unsigned long tmp;
   SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOC;
   tmp = SYSCTL_RCGCGPIO_R;    	// allow time for clock to start
   GPIO_PORTC_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port E
   GPIO_PORTC_CR_R = 0x30;
   GPIO_PORTC_AMSEL_R = 0x00;
   GPIO_PORTC_PCTL_R = 0x00000000;
   GPIO_PORTC_DIR_R = 0x30;      	//port C nin   4 üncü 5 inci birini 1 olarak ayarladım
   GPIO_PORTC_AFSEL_R = 0x00;    	//yani port c nin 4 ve 5in 1 inden çıkış yapabilirim
   GPIO_PORTC_PUR_R = 0x30;
   GPIO_PORTC_DEN_R = 0x30;
   }
void init_port_D() {//sadece kullandıgım pinleri aktıif ettim
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOD; // Port D’yi aktiflestir
	delay = SYSCTL_RCGC2_R;  	// zaman gecirmek icin
	GPIO_PORTD_DIR_R |= 0x0F;	// PD in   0 1 2 3 uncu  pinlerinden cikis yap
	 GPIO_PORTD_AFSEL_R &= ~0x0F; //GPIO_PORTD_AFSEL_R & = ~0x0F; // PD 3,2,1,0 pinlerini alternatif fonksinunu 0 yap
	GPIO_PORTD_DEN_R |= 0x0F;	// PD 3,2,1,0 pinlerini aktiflestir
}
void init_port_A() {
    volatile unsigned long delay;
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA;
    delay = SYSCTL_RCGC2_R;
    GPIO_PORTA_DIR_R |= 0xFF; // 0xff == 0b11111111, hepsini cikis olarak ayarla
    GPIO_PORTA_AFSEL_R &= ~0xFF; // alternatif fonksiyo kapali
    GPIO_PORTA_DEN_R |= 0xFF; // hepsi aktif
}

//0 lar giriş 1 ler çıkıştır.
#define Register_select 0x00000010 // Lcd displaydeki RS pinini 4 portu için  GPIO_PIN_4
#define Enable 0x00000020 // lcd deki En pinini herhangi bir portun 5 incisine atar
#define D4 0x00000001 // lcd displaydeki d4 ü port üzrindeki herhangi bir pinin 0 ıncısına  atamak için
#define D5 0x00000002 // lcd displaydeki d5 ü port üzrindeki herhangi bir pinin 1 ıncısına  atamak için
#define D6 0x00000004 //  lcd displaydeki d6 üport üzrindeki herhangi bir pinin 2 ıncısına  atamak için
#define D7 0x00000008 //  lcd displaydeki d7 ü port üzrindeki herhangi bir pinin 3 ıncısına  atamak için


#define Tum_pin_aktif  D7 | D6 | D5 | D4 //tüm pinleri veyalayıp hepsini 1 yapmak için
#define RS_ve_EN_cikis_yap Register_select | Enable

#define base_D GPIO_PORTD_BASE //port D nin kullanımı devreye alır
#define base_E GPIO_PORTC_BASE //port C nin kullanımı devreye alır
#define D_veri SYSCTL_PERIPH_GPIOD //zamanda veri alımı için
#define E_veri SYSCTL_PERIPH_GPIOC


#define portd GPIO_PORTD_DATA_R//portd için
#define porte GPIO_PORTC_DATA_R//port c için uzun uzun yazmamak için
#define portA GPIO_PORTA_DATA_R //port A için uzun uzun yazmamak için

//bütün portları giriş ve çıkışları için
//sabit degerler atandı aynı şeyleri tekrar tekrar yazmamak için
#define P0 0b0001
#define P1 0b0010
#define P2 0b0100
#define P3 0b1000
#define P4 0b10000
#define P5 0b100000
#define P6 0b1000000

#define beklet SysCtlDelay //zaman geçirme fonk. anlaşılır olması için yazıldı

#define lcdyaz GPIOPinWrite

void pulseLCD()
{
	lcdyaz(base_E, Enable, 0);//gücü düşükte calışmak için
	lcdyaz(base_E, Enable, Enable); //gücü yüksek almak için
	lcdyaz(base_E, Enable, 0);//tekrar düşük
}

void setCmd() {
	lcdyaz(base_E, Register_select,0);
	//false durumunda devreye girer
}

void setData() {
	lcdyaz(base_E, Register_select,Register_select);
}
void sendByte(char gonderbyt, int True_or_False)
{
	if (True_or_False)//ekrana yazarken sadece true olur
		setData();
	else //diğer şartlarda hep false
		setCmd();
	beklet(400);
	lcdyaz(base_D, Tum_pin_aktif, gonderbyt >>4);
	pulseLCD();
	lcdyaz(base_D, Tum_pin_aktif, gonderbyt);
	pulseLCD();
}

void LCD_Baslama_yeri(char satir, char sutun)
{
	char konum=0;

	if (satir == 0)//birinci satir sa 0 dir
		konum = 0;
	else if (satir==1)//2. satir ise  1 dir
		konum = 0x40;
	konum |= sutun;//o satırın kaçıncı sütünundan başması için yazılmış
	sendByte(0x80 | konum, false);
}


void clearLCD(void)
{
	sendByte(0x01, false); // ekranı temizler
	sendByte(0x02, false); // temizledikten snra tekrar başa döner
	beklet(30000);
}
void cursorOffLCD(void) {
	sendByte(0x0C, false);//cursoru kaybediyor
}
void initLCD(void)
{
	SysCtlPeripheralEnable(D_veri);
	SysCtlPeripheralEnable(E_veri);
	GPIOPinTypeGPIOOutput(base_D,  Tum_pin_aktif);
	GPIOPinTypeGPIOOutput(base_E, RS_ve_EN_cikis_yap);
	lcdyaz(base_D, Tum_pin_aktif ,0);
	lcdyaz(base_E, RS_ve_EN_cikis_yap ,0);

	beklet(10000);

	setCmd();
	beklet(15000);
	lcdyaz(base_D, Tum_pin_aktif, P1);
	pulseLCD();
	lcdyaz(base_D, Tum_pin_aktif, P1);
	pulseLCD();
	sendByte(0x28,false);  // lcd de 2. satırı oluşturmak için
	cursorOffLCD();//cursoru gecici olarak kaldırıyor
	sendByte(0x06, false); //  tekrardan lcdye ekleme moduna geçmek için yazılmış
	clearLCD();
}

void printLCD(char *text)
{
	char *c;//değişkenlerimize  ve gelen değeri bu pointere atama yapılmış
	c = text;
	while ((c != 0) && (*c != 0))//lcd displayde 0 hata kodu gibi birşey oluyor
	{
		sendByte(*c, true);
		c++;
	}
}
void sola_kaydir(){
	//http://ccspic.com/lcd-kayan-yazi/ bu adresten buldum
	sendByte(0x18,false);//internetten buldugum kod
}
void saga_kaydir(){
   sendByte(0x1E,false);//saga dogru kayar üstte kısımda belirttim nereden buldugumu
}


void portAC(){
	init_port_D();
	init_port_C();
	init_port_A();
		portd |= P0; // PD0 aktif etmek için
		portd |= P1; // PD1 1aktif yap
		portd |= P2; // PD2’i aktif etmek için
		portd |= P3; // PD3’i  aktif etmek için

		porte |= P4; // PE4’i aktif etmek için
		porte |= P5; // PE5’i aktif etmek için
}
void birinci(){
	printLCD("Kocaeli Uni ");
	LCD_Baslama_yeri(1,0);
	beklet(12000000);//ortalama 2 sn  kadar bekler
	    		printLCD("Emre Buyukada");
}
void ikinci(){
	LCD_Baslama_yeri(1,0);//ilk olarak alt satırı yazması için
		printLCD("Emre BUYUKADA");
		LCD_Baslama_yeri(0,0);//üst satırın birinci sütündan yazmaya başlar
		beklet(12000000);//ortalama 2 sn  kadar bekler
		    		printLCD("Kocaeli Uni");
}
void ucuncu(){
	int i;
		LCD_Baslama_yeri(0,0);//üst satırın birinci sütündan yazmaya başlar
		printLCD("Kocaeli Uni");
		for(i = 0; i < 40; i++)  //40 karakter için kaydırma
		      {
		       saga_kaydir();
		       beklet(400000);
		      }
		LCD_Baslama_yeri(1,0);//ilk olarak alt satırı yazması için
		printLCD("Emre BUYUKADA");
		for(i = 0; i < 40; i++)  //40 karakter için kaydırma
				      {
				       saga_kaydir();
				       beklet(400000);   // Kayma hızı
				      }
}
void dorduncu(){
	int i;
		LCD_Baslama_yeri(0,5);//üst satırın birinci sütündan yazmaya başlar
		printLCD("Kocaeli Uni");
		for(i = 0; i < 40; i++)  //40 karakter için kaydırma
		      {
		       sola_kaydir();
		       beklet(400000);//rastgele deneyerek urettiğim sayılar hesaplama kullanmadım
		      }
		LCD_Baslama_yeri(1,3);//ilk olarak alt satırı yazması için
		printLCD("Emre BUYUKADA");
			for(i = 0; i < 40; i++)  //40 karakter için kaydırma
				      {
				       sola_kaydir();
				       beklet(400000);   // Kayma hızı
				      }
}
int main()
{
	portAC();
    SysCtlClockSet(SYSCTL_SYSDIV_8|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
    initLCD();//lcd fonksiyonundaki bilgileri çekmek için
    while (1) {
    	int rastgele=rand()%4;//rastgele deger olşturması için
    				if(rastgele==0)
    				{
    					portA |= P2;//1. için Led için PA2 aktif
    					birinci();
    					beklet(16000000);//ortalama 3 sn  kadar bekler
    					portA &= ~(P2);
    				}
    				if(rastgele==1)
    				{
						portA |=P3;//2. led için PA3 portu aktif
						ikinci();
						beklet(16000000);//ortalama 3 sn  kadar bekler
						portA &= ~(P3);//2. ledin sönmesi için PA3 portu pasif
    				}
       	 	 	 	 if(rastgele==2)
       	 	 	 	 {
						 portA |= P4;
						 ucuncu();
						 beklet(8000000);//ortalama 1,5 sn  kadar bekler
						 portA &= ~(P4);
       	 	 	 	 }
						 if(rastgele==3)
						 {
							portA |= P5;
							dorduncu();
							beklet(8000000);//ortalama 1,5 sn  kadar bekler
							portA &= ~(P5);
						 }
       	 	 	 clearLCD();//her bir işlem sonunda silme işlemi gerçekleştirmek için
       }
}

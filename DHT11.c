#include <mega16.h>
#include <delay.h>
#include <stdlib.h>       
#include <math.h>     
#include <alcd.h>        
 
unsigned char Bit[40];
unsigned char Bitdata=0;
unsigned int Byte0,Byte1,Byte2,Byte3,Byte4,Sum;
char Humidity[16];
char Temperature[16]; 
int Htc=0;   // Hi time count   (80us ,26-28us,70us)
int Ltc=0;   // Low time count   (80us,50us)
int h;
int t;
int i=0;

void main(void) {
   
// Watchdog Timer Prescaler: OSC/2048k
WDTCR=(0<<WDTOE) | (1<<WDE) | (1<<WDP2) | (1<<WDP1) | (1<<WDP0); 

lcd_init(16);
lcd_clear();
lcd_putsf("Hello           Wellcome");

//step1:Delay Time Sensor Stability to Ready Power  = 1(S) 
      
delay_ms(2000);                                   

//reset WDT */
WDTCR=(1<<WDTOE) | (1<<WDE);
WDTCR=0x00;

//step2: Send  Signal (18ms Low) From MCU On DataBusLine = I/O B.2 
 
 lable1:
// Watchdog Timer Prescaler: OSC/2048k
WDTCR=(0<<WDTOE) | (1<<WDE) | (1<<WDP2) | (1<<WDP1) | (1<<WDP0); 
  
DDRB=(1<<DDB0);    
PORTB.0=0;
delay_ms(20);
PORTB.0=1;
delay_us(30);

//step3: Wait For Recieve Signal (80us Hi ,80us low) From Sensor  On DataBusLine = I/O B.0 
  
DDRB=(0<<DDB0);  

    if (PINB.0==1) {
       
       // No Sensor Connect 
       lcd_clear();
       lcd_gotoxy(0,0);  
       lcd_putsf("Error Sensor");
       lcd_gotoxy(0,1);  
       lcd_putsf("Not Connected");
       delay_ms(1000);
              
    //reset WDT */
    WDTCR=(1<<WDTOE) | (1<<WDE);
    WDTCR=0x00;
     
    goto lable1;
        }  
   
 lable2:

    if (PINB.0==0) {
    Ltc++;   
    delay_us(1);
    goto lable2;
        }  
          
 lable3:
         
    if (PINB.0==1) { 
       delay_us(1);
       Htc++;  
       goto lable3;
          } 
    
        if (Htc>=Ltc) {   
       // Link Ok   
       Htc=0;
       Ltc=0;
       goto lable4;
       }  
    
       else  { 
       // Link faild  
       lcd_clear();
       lcd_gotoxy(0,0);
       lcd_putsf("Error Sensor link");
       delay_ms(1000);

       Htc=0;
       Ltc=0;
       goto lable1;          
        } ;
         
 lable4:
 
 //step3: Wait For Recieve 40 Bits  From Sensor  On DataBusLine = I/O B.2 
  
 //Read bit[i]

  for (i=0;i<40;i++) {

   lable5:

       if (PINB.0==0) { 
       delay_us(1);
       Ltc++;      
       goto lable5;
          } 
            
       lable6:
   
       if (PINB.0==1) { 
       delay_us(1);
       Htc++; 
       goto lable6;          

          } 
      
       if (Htc<Ltc) { 
       Bitdata=0; 
       Bit[i]=Bitdata; 
       Htc=0;
       Ltc=0;

       }  
    
       else  { 
       Bitdata=1; 
       Bit[i]=Bitdata; 
       Htc=0;
       Ltc=0;
        }
           
   };    
   
          
DDRB=(1<<DDB0);  
                         
   Byte0 =Bit[0] *pow(2,7)+Bit[1] *pow(2,6)+Bit[2] *pow(2,5)+Bit[3] *pow(2,4)+Bit[4] *pow(2,3)+Bit[5] *pow(2,2)+Bit[6] *pow(2,1)+Bit[7] *pow(2,0);    //  Humidity      (MSB)
   Byte1 =Bit[8] *pow(2,7)+Bit[9] *pow(2,6)+Bit[10]*pow(2,5)+Bit[11]*pow(2,4)+Bit[12]*pow(2,3)+Bit[13]*pow(2,2)+Bit[14]*pow(2,1)+Bit[15]*pow(2,0);   //  Humidity      (LSB)
   Byte2 =Bit[16]*pow(2,7)+Bit[17]*pow(2,6)+Bit[18]*pow(2,5)+Bit[19]*pow(2,4)+Bit[20]*pow(2,3)+Bit[21]*pow(2,2)+Bit[22]*pow(2,1)+Bit[23]*pow(2,0);   //  Temperature   (MSB)
   Byte3 =Bit[24]*pow(2,7)+Bit[25]*pow(2,6)+Bit[26]*pow(2,5)+Bit[27]*pow(2,4)+Bit[28]*pow(2,3)+Bit[29]*pow(2,2)+Bit[30]*pow(2,1)+Bit[31]*pow(2,0);   //  Temperature   (LSB)
   Byte4 =Bit[32]*pow(2,7)+Bit[33]*pow(2,6)+Bit[34]*pow(2,5)+Bit[35]*pow(2,4)+Bit[36]*pow(2,3)+Bit[37]*pow(2,2)+Bit[38]*pow(2,1)+Bit[39]*pow(2,0);   //  CRC
   
   Sum=Byte0+Byte1+Byte2+Byte3;  
     
     if (Sum==Byte4) {
      //CheckSum OK
              
      h=Byte0;
      t=Byte2;
         
      ftoa(h,0,Humidity);
      lcd_clear();
      lcd_gotoxy(0,0);
      lcd_putsf("Humidity   =");
      lcd_puts(Humidity);   
      lcd_gotoxy(15,0); 
      lcd_putsf("%");   
                            
      ftoa(t,0,Temperature);
      lcd_gotoxy(0,1);
      lcd_putsf("Temperature=");
      lcd_puts(Temperature);   
      lcd_gotoxy(15,1); 
      lcd_putsf("C");   
      delay_ms(1000); 
      h=0;
      t=0;
      
     }

     else  { 
      //CheckSum Not OK 
      lcd_clear();
      lcd_gotoxy(0,0);
      lcd_putsf("CheckSum Error");
      delay_ms(1000);

           }
     
   h = 0 ;
   t = 0 ;
   Byte0 = 0 ;
   Byte1 = 0 ;
   Byte2 = 0 ;
   Byte3 = 0 ;
   Byte4 = 0 ;
 
//reset WDT */
WDTCR=(1<<WDTOE) | (1<<WDE);
WDTCR=0x00;
        
goto lable1;
  
}



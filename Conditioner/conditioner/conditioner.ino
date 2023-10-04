#include <EEPROM.h>


//define pins:
#define LED1 2
#define LED2 4
#define LED3 6
#define LED4 8
#define BTN_int 3
#define BTN_ext 5
#define LED_ext A0

const int on_seconds[]={0,120,300,600,600,0}; //conditioner on in seconds for mode  1 2 3 4 //exclude mode 0 and 5 //1-minimum cold
const int off_seconds[]={0,600,600,300,120,0}; //conditioner off in seconds for mode 1 2 3 4 //exclude mode 0 and 5 //4-maximum cold

bool timer_flag=false;
int mode=0;//temperature mode 0-off 1-min ... 4-max 5-always on
bool power_on=true; //conditioner power
bool btn_pressed=false; //btn was pressed flag
int seconds=0; //count second for timer
  

void setup() {
  // put your setup code here, to run once:

  analogRead(LED_ext);//initialisation
  
  Serial.begin(115200);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  pinMode(BTN_int, INPUT_PULLUP);
  pinMode(BTN_ext, OUTPUT);
  pinMode(LED_ext, INPUT);

  clr();

  EEPROM.get(0, mode);
  show_mode();
  set_seconds();
  digitalWrite(BTN_ext,LOW);


  // инициализация Timer1
  cli(); // отключить глобальные прерывания
  TCCR1A = 0; // установить регистры в 0
  TCCR1B = 0; 

  OCR1A = 15624; // установка регистра совпадения //15624 с делителем = 1сек
  TCCR1B |= (1 << WGM12); // включение в CTC режим

  // Установка битов CS10 и CS12 на коэффициент деления 1024
  TCCR1B |= (1 << CS10);
  TCCR1B |= (1 << CS12);

  TIMSK1 |= (1 << OCIE1A);  // включение прерываний по совпадению
  sei(); // включить глобальные прерывания

}

void loop() {
  // put your main code here, to run repeatedly:

//debug:
//  delay(100);
//  if(digitalRead(BTN_int)==LOW)digitalWrite(BTN_ext,HIGH);//button pressed
//  if(digitalRead(BTN_int)==HIGH)digitalWrite(BTN_ext,LOW);//button not pressed
//
//  if(led_is_on()==true)digitalWrite(LED1,HIGH); //LED is on  //ext led measured: 1023>off>900 700>on>600
//  if(led_is_on()==false)digitalWrite(LED1,LOW);  //LED is off 

//  if(digitalRead(BTN_int)==LOW){digitalWrite(BTN_ext,LOW);delay(200);digitalWrite(BTN_ext,HIGH);digitalWrite(LED1,HIGH);delay(100);digitalWrite(BTN_ext,LOW);delay(100);}//button pressed

//debug

  
  delay (50);
    
  if(digitalRead(BTN_int)==LOW) btn_pressed=true;//button pressed
  delay(10);
  if((digitalRead(BTN_int)==HIGH)and(btn_pressed==true))//button released
  {
    change_mode();
    btn_pressed=false;
  }
  
  if(mode==0)//always off
    {
      power_on=false;
      if(led_is_on()==true)correcting();  //correcting power correscponding mode
    };
    
  if(mode==5)//always on
    {
      power_on=true;
      if(led_is_on()==false)correcting(); //correcting power correscponding mode
    };

    if(timer_flag==true)
    {
        on_off(); //change power on/off
        set_seconds();
        timer_flag=false;
    }

 


}

void change_mode() //change mode (0-1-2-3-4-5)
{
  mode=mode+1;
  if(mode>5)mode=0;
  EEPROM.put(0, mode);
  set_seconds();
  show_mode();
}

void clr() //clear leds
{
  digitalWrite(LED1,LOW);
  digitalWrite(LED2,LOW);
  digitalWrite(LED3,LOW);
  digitalWrite(LED4,LOW);
}

void show_mode() //show mode on leds
{
  clr();
  if(mode==0){  digitalWrite(LED1,LOW);digitalWrite(LED2,LOW);digitalWrite(LED3,LOW);digitalWrite(LED4,LOW);  }
  if(mode==1)digitalWrite(LED1,HIGH);
  if(mode==2)digitalWrite(LED2,HIGH);
  if(mode==3)digitalWrite(LED3,HIGH);
  if(mode==4)digitalWrite(LED4,HIGH);
  if(mode==5){  digitalWrite(LED1,HIGH);digitalWrite(LED2,HIGH);digitalWrite(LED3,HIGH);digitalWrite(LED4,HIGH);  }
}


ISR(TIMER1_COMPA_vect)
{     
    Serial.println(seconds);
    if(mode==0)return;
    if(mode==5)return;
    seconds--;
    if(seconds < 1)
    {   
        timer_flag=true;

    }
     
}

void on_off() //change power on/off
{
  power_on=!power_on;
  correcting();
  delay(50);
}

void set_seconds()
{
  if(power_on==true)seconds=on_seconds[mode];
  if(power_on==false)seconds=off_seconds[mode];
}

bool led_is_on()
{
  if(analogRead(LED_ext)<800)return true; else return false; //LED is on  //ext led measured: 1023>off>900 700>on>600  
}

void correcting() //correcting power on off correcponding led
{ 
  if((power_on==false)and(led_is_on()==true))
  {  
    
    //digitalWrite(LED3,HIGH);delay(200);digitalWrite(LED3,LOW);delay(50);//debug  
    digitalWrite(BTN_ext,HIGH);delay(200);digitalWrite(BTN_ext,LOW);delay(50);
    Serial.println("set off");//debug
    return;
  } //set power off (if external led is on)
  
  if((power_on==true)and(led_is_on()==false))
  {  
    //digitalWrite(LED1,HIGH);delay(200);digitalWrite(LED1,LOW);delay(50);//debug  
    digitalWrite(BTN_ext,HIGH);delay(200);digitalWrite(BTN_ext,LOW);delay(50);  
    Serial.println("set on");//debug
    return;
  } //set power on (if external led is off) 

}












//end

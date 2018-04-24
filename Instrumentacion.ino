#include "Thread.h"
#include "ThreadController.h"
#include <Button.h>
#include <Keypad.h>
#include <TM1637Display.h> 

#define Clima A0
#define Evaporador A1
#define OnOff 13
#define Compresor 12
#define Ventilador A3
#define button 11
#define CLK 10
#define DIO 9

const uint8_t SEGA[] = {  
  0x0,  
  0x0,  
  SEG_A | SEG_F | SEG_G,  
  SEG_A | SEG_B | SEG_F | SEG_G | SEG_E | SEG_C  };   
const uint8_t SEGP[] = {  
  0x0,  
  0x0,  
  SEG_A | SEG_F | SEG_G,  
  SEG_A | SEG_B | SEG_F | SEG_G | SEG_E  };
const uint8_t SEGS[] = {  
  0x0,  
  0x0,  
  SEG_A | SEG_F | SEG_G,  
  SEG_A | SEG_F | SEG_G | SEG_C | SEG_D  };
//limpia display
const uint8_t data[] = {0x0, 0x0, 0x0, 0x0};
const byte Filas = 2;     //Cuatro filas 
const byte Cols = 2;    //Cuatro columnas
byte Pins_Filas[] = {5,4};   //Pines Arduino a los que contamos las filas. 
byte Pins_Cols[] = {3,2};  // Pines Arduino a los que contamos las columnas. 
char Teclas [ Filas ][ Cols ] = 
{  
  {'1','2'},  
  {'3','4'} };
Keypad teclado = Keypad(makeKeymap(Teclas), Pins_Filas, Pins_Cols, Filas, Cols);

int clima=0;
int evaporador=0;
int TempAmbiente=0;
int TempEvaporador=0;
int tempReq=25; //25°C es la temp deseada
int tempCrComp=10; //10° C
//String incomingByte; //letura del Serial

TM1637Display display(CLK, DIO); 

Button myBtn(button, true, true, 20);    //Declare the button
boolean state;

ThreadController controll = ThreadController();
Thread* analogReads = new Thread();
Thread* main = new Thread();
Thread* displayThread =new  Thread(); 

//thread function
void analogReadsCallback(){
  clima=analogRead(Clima);
  TempAmbiente=temperatura(clima);
  delay(50);
  evaporador=analogRead(Evaporador);
  TempEvaporador=temperatura(evaporador);
//  Serial.println(TempAmbiente);
//  Serial.println(TempEvaporador);
}

void mainCallback(){
  if(state==true){  
    if(TempAmbiente>tempReq+3){
      if(TempEvaporador<tempCrComp){
        StandBy();
      }else{
        Enciende();
      }
    }else if(TempAmbiente + 3>tempReq){
      if(TempEvaporador<tempCrComp){
        StandBy();
      }else{
        Enciende();
      }
    }else if(TempAmbiente + 3<tempReq){
      StandBy();
    }
  }else{
    Apaga();
  }
}
void displayThreadCallback(){
  display.showNumberDec (TempAmbiente,false,2,0);  
  display.setSegments(SEGA);
}

//funciones
int temperatura(int sensor){
  float voltaje=sensor*(5.0/1023);
  float temperatura=((voltaje*-21.133)+78.018);
  int temp =temperatura;
  return temp;
}
/*Modo reposo, compresor apagado y ventilador encendido*/
void StandBy(){
  digitalWrite(Compresor,LOW);
  //digitalWrite(Ventilador,HIGH);
}
/*Modo Encendido, compresor y ventilador encendido*/
void Enciende(){
  digitalWrite(Compresor,HIGH);
  //digitalWrite(Ventilador,HIGH);
}
/*Modo Apagado*/
void Apaga(){
  digitalWrite(Compresor,LOW);
  //digitalWrite(Ventilador,LOW);
}
void setup() {
  // put your setup code here, to run once:
  pinMode(Compresor,OUTPUT);
  pinMode(OnOff, OUTPUT);
  Serial.begin(9600);
  teclado.addEventListener(keypadEvent);
  display.setBrightness (0x0f);
  
  analogReads->onRun(analogReadsCallback);
  analogReads->setInterval(100);

  main->onRun(mainCallback);
  main->setInterval(500);

  displayThread->onRun(displayThreadCallback);
  displayThread->setInterval(500);
  
  controll.add(analogReads);
  controll.add(main);
}

void loop() {
  // put your main code here, to run repeatedly:
  controll.run();
  teclado.getKey();
  myBtn.read();                    //Read the button
  if (myBtn.wasReleased()) {       //If the button was released, change the LED state
      state = !state;
      digitalWrite(OnOff, state);
    }
}

void keypadEvent (KeypadEvent eKey)
{
  switch (teclado.getState())
  {
    case PRESSED:
    Serial.print(eKey);
    switch(eKey)
    {
      case '1':
      display.setSegments(data);    
      tempReq++;
       if (tempReq > 29)    
       {      
        tempReq = 29;   
        }    
      display.setSegments(SEGS);        
      display.showNumberDec(tempReq,false,2,0);            
      delay(500); 
      display.setSegments(data);;    
      break;
    case '2':
    display.setSegments(data);    
     if (tempReq < 17)     
     {
      tempReq =  17;      
      }
    tempReq--;    
    display.setSegments(SEGS); 
    display.showNumberDec (tempReq,false,2,0);               
    delay(500);  
    display.setSegments(data);;   
    break;
    
    case '3':  
    display.setSegments(data);  
     if (tempCrComp > 9)     
     {    
      tempCrComp = 9;     
      } 
    tempCrComp++;    
    display.setSegments(SEGP);
    display.showNumberDec (tempCrComp,false,2,0);                    
    delay(500);
    display.setSegments(data);;     
    break;
    
    case '4':
    display.setSegments(data);
     if (tempCrComp < 6)    
     {    
      tempCrComp = 6;    
      }     
    tempCrComp--;    
    display.setSegments(SEGP);
    display.showNumberDec (tempCrComp,false,2,0);              
    delay(500);
    display.setSegments(SEGA);    
    break;   
      }
  }
  }

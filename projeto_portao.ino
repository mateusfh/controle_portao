#include <Ultrasonic.h>
 
#define pin_trigger 9
#define pin_echo 10

#define pin_button 11

int enB = 3;
int in3 = 5;
int in4 = 4;

float distance_close= 5;
float distance_1    = 15;
float distance_2    = 25;
float distance_open = 35;

enum states{
  OPEN, 
  CLOSE,
  OPENING_1,
  OPENING_2,
  OPENING_FINISH,
  CLOSING_1,
  CLOSING_2,
  CLOSING_FINISH,
  OPEN_1,
  OPEN_2,
  CLOSE_1,
  CLOSE_2
};                           

int current_state = OPEN;

Ultrasonic ultrasonic(pin_trigger, pin_echo);
 
void setup(){
  Serial.begin(9600);

  pinMode(pin_button, INPUT);
  pinMode(enB, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}


//Forward:  opening
//Backward: closing
void moving_gate(){
  switch(current_state){
      case OPENING_1:
        moveToForward(100);
        if(getGateDistance() >= distance_1){
          current_state = OPENING_2;
        }
        break;
      case OPENING_2:
        moveToForward(70);
        if(getGateDistance() >= distance_2){
          current_state = OPENING_FINISH;
        }
        break;
      case OPENING_FINISH:
        moveToForward(30);
        if(getGateDistance() >= distance_open){
          current_state = OPEN;
        }
        break;
      case CLOSING_1:
        moveToBackward(100);
        if(getGateDistance() <= distance_2){
          current_state = CLOSING_2;
        }
        break;
      case CLOSING_2:
        moveToBackward(70);
        if(getGateDistance() <= distance_1){
          current_state = CLOSING_FINISH;
        }
        break;
      case CLOSING_FINISH: 
        moveToBackward(30);
        if(getGateDistance() <= distance_close){
          current_state = CLOSE;
        }
        break;
  }
}
 
void loop(){
  /*
  Espera o botão ser pressionado
  */
  if(digitalRead(pin_button)){
    /*
    Espera o botão parar de ser pressionado
    */
    while(digitalRead(pin_button)){}

    /*
    Verifica o estado atual do portão e atualiza
    */
    switch(current_state){
      case OPEN:
        current_state = CLOSING_1;
        break;
      case CLOSE:
        current_state = OPENING_1;
        break;
      case OPENING_1:
        current_state = OPEN_1;
        break;
      case OPENING_2:
        current_state = OPEN_2;
        break;
      case CLOSING_1:
        current_state = CLOSE_1;
        break;
      case CLOSING_2:
        current_state = CLOSE_2;
        break;
      case OPEN_1:
        current_state = CLOSE;
        break;
      case OPEN_2:
        current_state = CLOSING_2;
        break;
      case CLOSE_1:
        current_state = OPEN;
        break;
      case CLOSE_2:
        current_state = OPENING_1;
        break;
      default:
        break;
    }
  }
  moving_gate();
  delay(1000);
}


void moveToForward(int speed_motor){
  analogWrite(enB, speed_motor);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  delay(500);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}
void moveToBackward(int speed_motor){
  analogWrite(enB, speed_motor);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  delay(500);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}


float getGateDistance(){
  float cmMsec;
  long microsec = ultrasonic.timing();
  cmMsec = ultrasonic.convert(microsec, Ultrasonic::CM);

  return cmMsec;
}

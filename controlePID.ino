#include <PID_v1.h>
#include <Ultrasonic.h>


/****PINS VARIABLES AND CONSTANTS****/
#define pin_trigger 9
#define pin_echo 10

#define pin_button 11

int enB = 3;
int in3 = 5;
int in4 = 4;
/*----------------------------------*/



/***********PID VARIABLES***********/
double outputSpeed   = 0;
double inputDistance = 0;
double Setpoint;

//double Kp=179.7666, Ki=22.904, Kd=266.1546; //Tune realizado a partir do LR
//double Kp=38.04, Ki=120.7789671874, Kd=2.995226805; // Ganho do RF
double Kp=24.867237, Ki=0.283305, Kd=46.081385; // Ganhos do ZN
/*----------------------------------*/



/*********REFERENCE DISTANCES*********/
float distance_close= 40;   //5
float distance_1    = 27;  //15
float distance_2    = 18;  //25
float distance_open = 8;  //35
/*----------------------------------*/



enum states{
  OPEN,             //0 -> Está aberto
  CLOSE,            //1 -> Está fechado
  
  OPENING_1,        //2 -> Começou a abrir 		/ Primeira fase de abertura / Distancia está saindo de 40 e indo para 27
  OPENING_2,        //3 -> Continua abrindo 	/ Segunda fase de abertura 	/ Distancia está saindo de 27 e indo para 18
  OPENING_FINISH,   //4 -> Terminando de abrir 	/ Última fase de abertura 	/ Distância está saindo de 18 e indo para 8
  
  CLOSING_1,        //5 -> Começou a fechar 	/ Primeira fase de fechamento 	/ Distância está saindo de 8 e indo para 18 
  CLOSING_2,        //6 -> Continua fechando	/ Segunda fase de fechamento	/ Distância está saindo de 18 e indo para 27
  CLOSING_FINISH,   //7 -> Terminando de fechar	/ Última fase de fechamento 	/ Distância está saindo de 27 e indo para 40
  
  TO_OPEN_1,        //8 -> Saindo de 40 e indo parar em 27
  OPEN_1,           //9 -> Parado em 27
  
  TO_OPEN_2,        //10 -> Saindo de 27 e indo parar em 18
  OPEN_2,           //11 -> Parado em 18
  
  TO_CLOSE_1,       //12 -> Saindo de 8 e indo parar em 18
  CLOSE_1,          //13 -> Parado em 18
  
  TO_CLOSE_2,       //14 -> Saindo de 18 e indo parar em 27
  CLOSE_2           //15 -> Parado em 27
};


long startTime;
int current_state = OPEN;

Ultrasonic ultrasonic(pin_trigger, pin_echo);
PID myPID(&inputDistance, &outputSpeed, &Setpoint, Kp, Ki, Kd, DIRECT);

void setup(){
	Serial.begin(9600);
	inputDistance = getGateDistance();
	
	//Start goal is close gate
	Setpoint = distance_close; 
	//myPID.SetOutputLimits(50, 90);
	myPID.SetMode(AUTOMATIC);
	
	pinMode(pin_button, INPUT);
	pinMode(enB, OUTPUT);
	pinMode(in3, OUTPUT);
	pinMode(in4, OUTPUT);
	
	digitalWrite(in3, LOW);
	digitalWrite(in4, LOW);
	
	startTime = millis();
}

void loop(){
  Serial.println(millis() - startTime);
  delay(20);
  Serial.print(getGateDistance()*10);
  Serial.print(" ");
	if(digitalRead(pin_button)){
		
		//Espera o botão parar de ser pressionado
		while(digitalRead(pin_button)){}
		
		//lastClickDistance = getGateDistance();
		
		//Verifica o estado atual do portão e atualiza
		switch(current_state){
			case OPEN:
				current_state = CLOSING_1;
				break;
			case CLOSE:
				current_state = OPENING_1;
				break;
			case OPENING_1:
				current_state = TO_OPEN_1;
				break;
			case OPENING_2:
				current_state = TO_OPEN_2;
				break;
			case CLOSING_1:
				current_state = TO_CLOSE_1;
				break;
			case CLOSING_2:
				current_state = TO_CLOSE_2;
				break;
			case OPEN_1:
				current_state = CLOSING_FINISH;
				break;
			case OPEN_2:
				current_state = CLOSING_2;
				break;
			case CLOSE_1:
				current_state = OPENING_FINISH;
				break;
			case CLOSE_2:
				current_state = OPENING_2;
				break;
			default:
				break;
		}
	}
	moving_gate();
}

void moving_gate(){
	inputOutputPID();
	myPID.Compute();
	outputSpeed = map(outputSpeed,0,1700,60,90);
	
	
	//Serial.print("Distance: ");
	//Serial.println(getGateDistance());
	//Serial.print("Speed: ");
	//Serial.println(outputSpeed);
	//Serial.print("Setpoint: ");
	//Serial.println(Setpoint);
	//Serial.print("Input: ");
	//Serial.println(inputDistance);
	//Serial.println("");
	
    switch(current_state){
      case OPENING_1:
		moveToForward(outputSpeed);
        if(getGateDistance() <= distance_1){
          current_state = OPENING_2;
        }
        break;
      case OPENING_2:
		moveToForward(outputSpeed);
        if(getGateDistance() <= distance_2){
          current_state = OPENING_FINISH;
        }
        break;
      case OPENING_FINISH:
		moveToForward(outputSpeed);
        if(getGateDistance() <= distance_open){
          current_state = OPEN;
        }
        break;
      case CLOSING_1:
		moveToBackward(outputSpeed);
        if(getGateDistance() >= distance_2){
          current_state = CLOSING_2;
        }
        break;
      case CLOSING_2:
		moveToBackward(outputSpeed);
        if(getGateDistance() >= distance_1){
          current_state = CLOSING_FINISH;
        }
        break;
       case CLOSING_FINISH: 
		moveToBackward(outputSpeed);
        if(getGateDistance() >= distance_close){
          current_state = CLOSE;
        }
        break;
      case TO_OPEN_1:
		moveToForward(outputSpeed);
        if(getGateDistance() <= distance_1){
          current_state = OPEN_1;
        }
        break;
      case TO_OPEN_2:
		moveToForward(outputSpeed);
        if(getGateDistance() <= distance_2){
          current_state = OPEN_2;
        }
        break;
      case TO_CLOSE_1:
		moveToBackward(outputSpeed);
        if(getGateDistance() >= distance_2){
          current_state = CLOSE_1;
        }
        break;
      case TO_CLOSE_2:
		moveToBackward(outputSpeed);
        if(getGateDistance() >= distance_1){
          current_state = CLOSE_2;
        }
        break;
	}
}

void inputOutputPID(){
	if((current_state == CLOSING_1) || (current_state == CLOSING_2) || (current_state == CLOSING_FINISH)){
		inputDistance = getGateDistance();
		Setpoint = distance_close;
	}else if(current_state == TO_CLOSE_1){
		inputDistance = getGateDistance();
		Setpoint = distance_2;
	}else if(current_state == TO_CLOSE_2){
		inputDistance = getGateDistance();
		Setpoint = distance_1;
	}
	
	else if((current_state == OPENING_1) || (current_state == OPENING_2) || (current_state == OPENING_FINISH)){
		inputDistance = distance_open;
		Setpoint = getGateDistance();
	}else if(current_state == TO_OPEN_1){
		inputDistance = distance_1;
		Setpoint = getGateDistance();
	}else if(current_state == TO_OPEN_2){
		inputDistance = distance_2;
		Setpoint = getGateDistance();
	}
}

float getGateDistance(){
  float cmMsec;
  long microsec = ultrasonic.timing();
  cmMsec = ultrasonic.convert(microsec, Ultrasonic::CM);
  return cmMsec;
}

void moveToBackward(int speed_motor){ //recebe como parametro o Output
  if (speed_motor > 0){
    analogWrite(enB, speed_motor);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
    delay(100);
    digitalWrite(in4, LOW);
    digitalWrite(in3, LOW);
  }
}

void moveToForward(int speed_motor){ //recebe como parametro o Output
  if (speed_motor > 0){
    analogWrite(enB, speed_motor);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
    delay(100);
    digitalWrite(in4, LOW);
    digitalWrite(in3, LOW);
  }
}

#include <LiquidCrystal_I2C.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(0x27,16,2);
double maxV = 24; //Volts
double maxC = 1500.0; //Cycle Length
double pulseW = 31.0; //ms

void setup()
{
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.begin(16,2); //dimension of LCD
  lcd.print("Vol Cyc1 Cyc2 Pw");

  pinMode(4,OUTPUT);
  pinMode(5,OUTPUT);
}

void loop(){
  double output;
  int power = 0; //turns device on or off
  int tim = 0; //timer
  
  int sensor0 = analogRead(A0);
  float volt = sensor0 * (maxV/1023.0); // - 1.4; //1.4 from transistors
  lcd.setCursor(0,1);
  lcd.print(String(round(volt)));
  volt = 1;

  if(volt < 10){
    lcd.setCursor(1,1);
    lcd.print(" ");
  }

  int sensor1 = analogRead(A1);
  float cyc1 = round((sensor1 * maxC/1023.0)/100)*100;
  lcd.setCursor(4,1);
  lcd.print(String(round(cyc1)));

  if(cyc1 < 1000){
    lcd.setCursor(7,1);
    lcd.print(" ");
  }

  int sensor2 = analogRead(A2);
  float cyc2 = round((sensor2 * maxC/1023.0)/100)*100;
  lcd.setCursor(9,1);
  lcd.print(String(round(cyc2)));

  if(cyc2 < 1000){
    lcd.setCursor(12,1);
    lcd.print(" ");
  }

  int sensor3 = analogRead(A3);
  float pwid = round(sensor3 * pulseW/1023.0);
  lcd.setCursor(14,1);
  if(pwid == 31.00){
    pwid = 5000;
    lcd.print("5k");
  } else{
    lcd.print(String(round(pwid)));
  }

  //Turns system on
  if(digitalRead(2) > 0){
    power =1;
  }

  
  while (power == 1) {
    //Creates square waveform
    //Positive
    output = volt;  
    configuration(1);
    changeFunction(pwid, output);

    //Negative
    output = -volt;
    configuration(2);
    changeFunction(pwid, output);

    //Rest depending on mode: PWID == 5k is used for testing while PWID < 100 is used for physiological setting
    if (pwid < 100){
      //Rest
      output = 0;
      configuration(0);
      changeFunction(100-2*pwid,output); //rest for remainder of one unit cycle length.
      tim = tim + 100;


      //Rest until new stim.
      while (1){ 
        //Breaks if needs to be stimulated; cyc1 is on by default, while cyc2 can be activated by a switch
        if((tim % (int)cyc1) == 0){
          break;
        }
        if((tim % (int)cyc2) == 0 && digitalRead(3) > 0){
          break;
        }


        //Otherwise, system rests until it can be stimulated again
        changeFunction(100,output);
        tim = tim + 100;
      }
      
      
      //Restarts tim to prevent infinity
      if((tim % (int)cyc1) == 0 && (tim % (int)cyc2) == 0){
        tim = 0;
      }
    }
    
    if(digitalRead(2) == 0){
      power = 0;
    }
  }
  output = 0;
}

void changeFunction(int interval, double output){
  int lock = 1;
  long lastChange = millis();
  while(lock == 1){
    Serial.println(output);
    if ((millis() - lastChange) >= interval){
      lock = 0;
    }
    if (digitalRead(2) == 0){
      break;
    }
  }
}

void configuration(int mode){
  if (mode == 1){
    digitalWrite(4,HIGH);
    digitalWrite(5,LOW);
  }
  else if (mode == 2){
    digitalWrite(4,LOW);
    digitalWrite(5,HIGH);
  }
  else{
    digitalWrite(4,LOW);
    digitalWrite(5,LOW);
  }
}

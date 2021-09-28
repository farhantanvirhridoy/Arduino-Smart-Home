#include <IRremote.h>
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include <Servo.h>




#define power 0x3D00FF
#define volu 0x3D12ED
#define vold 0x3D926D
#define chu 0x3D52AD
#define chd 0x3DD22D
#define echoPin 4
#define trigPin 5
#define DHTPIN 3
#define DHTTYPE DHT11

long duration;
int distance;
int fanpin1 = 37;
int fanpin2 = 39;
int fanpmw = 12;
int pumppmw = 13;
int pumppin1 = 41;
int pumppin2 = 43;
int ledPin = 7;
int buzzerPin = 6;
int servopin = 22;
int ir1vcc = 52;
int ir1gnd = 50;
int ir1 = 48;
int ir2vcc = 28;
int ir2gnd = 30;
int ir2 = 32;
int light = 24;
String str = "";
String str2 = "";
String str3 = "";
String lightstate = "";
long previous = 0;
long current = 0;
int person = 0;
int pumpstate = 0;
int haveto_on = 0;
int pumpspd = 80;
int fanspd = 50;
char code;
int issnd = 1;
int isauto = 1;
String sec ;
String data;
int _timeout;
String _buffer;
String number = "+8801984811676";
int k = 1;
char string[14] = "";

IRsend irsend;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);
Servo myservo;


void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(fanpin1, OUTPUT);
  pinMode(fanpin2, OUTPUT);
  pinMode(fanpmw, OUTPUT);
  pinMode(pumppin1, OUTPUT);
  pinMode(pumppin2, OUTPUT);
  pinMode(pumppmw, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(ir1vcc, OUTPUT);
  pinMode(ir1gnd, OUTPUT);
  pinMode(ir2vcc, OUTPUT);
  pinMode(ir2gnd, OUTPUT);
  pinMode(ir1, INPUT);
  pinMode(ir2, INPUT);
  pinMode(light, OUTPUT);
  pinMode(10, OUTPUT);

  digitalWrite(ir1vcc, 1);
  digitalWrite(ir2vcc, 1);
  digitalWrite(ir1gnd, 0);
  digitalWrite(ir2gnd, 0);
  digitalWrite(light, 1);
  digitalWrite(10, 1);

  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600);
  mlx.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(3, 0);
  lcd.print("SMART HOME");
  delay(3000);
  dht.begin();
  myservo.attach(servopin);
  myservo.write(180);
  code = 'a';


}

void loop() {
  sec = "M";

  if (Serial1.available())
  {


    code = Serial1.read();
    if (code == 'o') issnd = 0;
    if (code == 'z')
    {
      issnd = 1;

    }
    if (code == 'a') isauto = 1;
    else isauto = 0;
    Serial.println(code);
    Serial.println(fanspd);
    manual(code);

  }

  if (isauto) manual('a');



  if (issnd == 1)
  {
    data = "";

    if (sec == "F" || sec == "G" || sec == "T")
    {
      for (int i = 1; i <= 300  ; i++)
      {


        data.concat(sec);
        if (i % 5 == 0) data.concat(",");
        else data.concat("/");
      }

      for (int i = 1; i <= 60; i++)
      {
        Serial2.print(sec);
      }
      Serial2.print(",");



    }


    else
    {
      data.concat(String(person));
      data.concat("/");
      data.concat(String(get_temp(), 0));
      data.concat("/");
      data.concat(String(get_hum(), 0));
      data.concat("/");
      data.concat(String(waterlevel(), 0));
      data.concat("/");

      data.concat(sec);
      data.concat(",");
    }
    Serial1.print(data);



    String message = "Person: ";
    message.concat(String(person));
    message.concat("\nTemperature: ");
    message.concat(String(get_temp(), 0));
    
    message.concat(" C\nHumidity: ");
    message.concat(String(get_hum(), 0));
    message.concat("%\nWater Level: ");
    message.concat(String(waterlevel(), 0));
    message.concat("%,");
    Serial2.print(message);
    Serial.println(message);

  }






  delay(10);

}
void automatic()
{
  str = "";
  str2 = "";
  str3 = "";

  if ((get_temp() > 27.0 || get_hum() > 60) && person > 0)
  {
    fan(1, 50);
    pumpspd = 200;
    str = "Fan On ";
  }
  else
  {
    fan(0, 0);
    pumpspd = 80;
    str = "Fan Off";
  }

  if (get_distance() < 4)
  {
    if (pumpstate == 1)
    {
      pump(0, 0);
      haveto_on = 1;
    }
    lcd.setCursor(0, 0);
    lcd.print("Scanning Temp...");
    //delay(1000);
    lcd.setCursor(0, 1);
    lcd.print("Temperature:");
    lcd.print(int(get_body_temp()));
    lcd.print("  ");
    delay(1000);
    if (get_body_temp() < 37.5)
    {
      door(1);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Door Opening....");
      previous = millis();
      delay(1);
      current = millis();
      while ( current - previous < 5000)
      {
        //delay(100);
        int state = digitalRead(ir1);
        if (state == 0)
        {
          person = person + 1;
          delay(3000);
          door(0);
          lcd.setCursor(0, 0);
          lcd.print("Door Closing....");
          break;
        }
        current = millis();
      }
      door(0);
      //lcd.setCursor(0, 0);
      //lcd.print("Door Closing....");
    }
    else
    {
      lcd.setCursor(0, 0);
      alarm();
      lcd.print("High Temperature!");
      delay(1000);
      sec = "T";
    }
    if (haveto_on == 1)
    {
      pump(1, pumpspd);
      haveto_on = 0;
    }
  }
  if (digitalRead(ir2) == LOW)
  {
    if (pumpstate == 1)
    {
      pump(0, 0);
      haveto_on = 1;
    }
    person--;
    door(1);

    lcd.setCursor(0, 0);
    lcd.print("Door Opening....");
    delay(3000);
    door(0);

    lcd.setCursor(0, 0);
    lcd.print("Door Closing....");
    if (haveto_on == 1)
    {
      pump(1, pumpspd);
      haveto_on = 0;
    }

  }
  if (person > 0)
  {
    digitalWrite(light, 0);
    lightstate = "Bulb On ";
  }
  else
  {
    digitalWrite(light, 1);
    lightstate = "Bulb Off";
  }

  str2 = "Pump off";

  if (waterlevel() > 90.0)
  {
    pump(0, 0);
    pumpstate = 0;

    str2 = "Pump off";

  }
  else if (waterlevel() < 40.0)
  {
    pump(1, pumpspd);
    pumpstate = 1;
    str2 = "Pump On ";

  }

  if (millis() % 10000 <= 5000)
  {
    str3 = "Tem:";
    str3.concat(String(get_temp(), 1));
    str3.concat(",Hum:");
    str3.concat(String(get_hum(), 0));
    str3.concat("%");
    str2 = lightstate;
  }
  else
  {
    str3 = "Level:";
    str3.concat(String(waterlevel(), 0));
    str3.concat("%,Per:");
    str3.concat(person);
    str3.concat(" ");
  }

  if (fire())
  {
    alarm();
    str3 = "     Fire!!     ";
    sec = "F";

  }
  if (exploit())
  {
    alarm();
    str3 = "  Gas Leakage!  ";
    sec = "G";
  }

  lcd.setCursor(0, 0);
  lcd.print(str);
  lcd.setCursor(7, 0);
  lcd.print(",");
  lcd.print(str2);
  lcd.setCursor(0, 1);
  lcd.print(str3);

}

int get_distance()
{

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);

  distance = duration * 0.034 / 2;

  return distance;
}

float get_body_temp()
{
  return mlx.readObjectTempC();
}

void sendIR(char c)
{
  switch (c)
  {
    case 'p':

      {

        irsend.sendNEC(power, 32);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("IR send:POWER   ");

        break;
      }
    case 'b':
      {

        irsend.sendNEC(chu, 32);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("IR send:CH+     ");

        break;
      }
    case 'c':
      {

        irsend.sendNEC(chd, 32);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("IR send:CH-     ");

        break;
      }
    case 'd':
      {

        irsend.sendNEC(volu, 32);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("IR send:VOL+    ");

        break;
      }
    case 'e':
      {

        irsend.sendNEC(vold, 32);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("IR send:VOL-    ");

        break;
      }
  }
}

void pump(int state, int spd)
{
  if (state) {
    analogWrite(pumppmw, spd);
    digitalWrite(pumppin1, 1);
    digitalWrite(pumppin2, 0);
  }
  else
  {
    digitalWrite(pumppmw, 0);
    digitalWrite(pumppin1, 0);
    digitalWrite(pumppin2, 0);
  }
}

void fan(int state, int spd)
{
  if (state)
  {
    analogWrite(fanpmw, spd);
    digitalWrite(fanpin1, 1);
    digitalWrite(fanpin2, 0);
  }
  else
  {
    digitalWrite(fanpmw, 0);
    digitalWrite(fanpin1, 0);
    digitalWrite(fanpin2, 0);
  }
}
float get_temp()
{
  return dht.readTemperature();

}
float get_hum()
{
  return dht.readHumidity();
}
bool fire()
{
  return analogRead(A0) < 900;
}

bool exploit()
{
  return analogRead(A1) > 600;
}

void alarm()
{


  tone(buzzerPin, 500);
  digitalWrite(ledPin, 1);
  delay(300);
  noTone(buzzerPin);
  digitalWrite(ledPin, 0);
  delay(300);




}
float waterlevel()
{
  int sensor = analogRead(A3);
  float level = float(sensor) * 80.0 / 650.0 + 20.0;
  return level;
}

void door(int state)
{

  if (state)
  {
    for (int pos = 180; pos >= 0; pos -= 1) {
      myservo.write(pos);
      delay(3);
    }
  }
  else
  {
    for (int pos = 0; pos <= 180; pos += 1)

      myservo.write(pos);
    delay(3);
  }
}






void manual(char cod)
{
  switch (cod)
  {
    case 'a':
      {
        automatic();
        break;
      }
    case 'x':
      {
        fanspd = fanspd + 5;
        fan(1, fanspd);
        break;
      }
    case 'y':
      {
        fanspd = fanspd - 5;
        fan(1, fanspd);
        break;
      }
    case 'f':
      {
        digitalWrite(light, 0);
        lcd.setCursor(0, 0);
        lcd.print("    Light On    ");
        break;
      }
    case 'g':
      {
        digitalWrite(light, 1);
        lcd.setCursor(0, 0);
        lcd.print("    Light Off   ");
        break;
      }
    case 'l':
      {
        fan(1, fanspd);
        lcd.setCursor(0, 0);
        lcd.print("     Fan On     ");
        pumpspd = 200;
        break;
      }
    case 'n':
      {
        fan(0, 0);
        lcd.setCursor(0, 0);
        lcd.print("     Fan Off    ");
        pumpspd = 80;
        break;
      }
    case 's':
      {
        pump(1, pumpspd);
        lcd.setCursor(0, 0);
        lcd.print("    Pump On     ");
        break;
      }
    case 't':
      {
        pump(0, 0);
        lcd.setCursor(0, 0);
        lcd.print("    Pump Off    ");
        break;
      }
    case 'q':
      {
        myservo.write(0);;
        lcd.setCursor(0, 0);
        lcd.print("    Door On     ");
        break;
      }
    case 'r':
      {
        myservo.write(180);
        lcd.setCursor(0, 0);
        lcd.print("    Door Off     ");
        break;
      }
    case 'p':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
      {
        sendIR(cod);

        break;
      }
    default:
      {
        lcd.setCursor(0, 0);
        lcd.print("                ");
        lcd.setCursor(0, 1);
        lcd.print("                ");
      }
  }
}

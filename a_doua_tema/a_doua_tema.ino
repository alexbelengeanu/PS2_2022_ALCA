#include <EEPROM.h>

#define onoffPin 8 // pinul pentru mesajul de on/off
#define greenPin 9 // pin culoare verde RGB
#define redPin 10 // pin culoare rosie RGB
#define bluePin 11 // pin culoare albastra RGB
#define interval 1000 // intervalul la care se iau valorile de la senzori
#define adresaNrMsjScr 1022
#define adresaUltMsjScr 1023

String message; // variabila pentru mesajele de pe comunicatia seriala

char *newColor = new char[3]; // variabila auxiliara pentru stocarea culorilor RGB
char *redValue = new char[3]; // valoare canal rosu RGB
char *greenValue = new char[3]; // valoare canal verde RGB
char *blueValue = new char[3]; // valoare canal albastru RGB

int percent;

long previousMillis = 0; // variabila pentru stocarea timpului scurs de la ultima citire a valorilor senzorilor

byte numar_mesaje_scrise = EEPROM.read(adresaNrMsjScr);
byte ultimul_mesaj_scris = EEPROM.read(adresaUltMsjScr);
byte message_counter;

void afisare_mesaje_eeprom(){
  Serial.println("=================== AFISARE MESAGERIE ===================");
  if(numar_mesaje_scrise != 0){
    if(numar_mesaje_scrise == ultimul_mesaj_scris){
      message_counter = 1;
      for(int i = numar_mesaje_scrise - 1; i >= 0; i--){
        message = readStringFromEEPROM(40 * i);
        Serial.print(message_counter++);
        Serial.print(". ");
        Serial.println(message);
      }
    }
    else{
      message_counter = 1;
      for(int i = ultimul_mesaj_scris - 1; i >= 0; i--){
        message = readStringFromEEPROM(40 * i);
        Serial.print(message_counter++);
        Serial.print(". ");
        Serial.println(message);
      }
      for(int i = numar_mesaje_scrise - 1; i >= ultimul_mesaj_scris; i--){
        message = readStringFromEEPROM(40 * i);
        Serial.print(message_counter++);
        Serial.print(". ");
        Serial.println(message);
      }
    }
  }
  else
    Serial.println("Momentan mesageria este goala.");
  Serial.println("=========================================================");
  if(numar_mesaje_scrise == 10)
            Serial.println("WARNING : Mesageria este plina, mesajele transmise in continuare le vor suprascrie pe cele vechi.");
}

void setup() { // functie de setup la pornirea placii Arduino
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(onoffPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  afisare_mesaje_eeprom();
}

void setColor(int red, int green, int blue) // seteaza culoarea pentru fiecare canal RGB
{
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);  
}

char *getColor(char firstChar, char secondChar){ // preia culoarea pt fiecare canal RGB din mesajul de pe comunicatia seriala
  newColor[0]=firstChar;
  newColor[1]=secondChar;
  newColor[2]='\0';
  return newColor;
}

int StrToHex(char str[]) // converteste valoarea culorilor pt ledul RGB din string in hexadecimal
{
  return (int) strtol(str, 0, 16);
}

void printTemperature(){ // afiseaza temperatura pe comunicatia seriala
  int adc = analogRead(0);
  float voltage = adc * (5000 / 1024.0);
  float temperature = voltage / 10;
  Serial.print("3 ");
  Serial.print(temperature);
  Serial.print("\xC2\xB0"); // shows degree symbol
  Serial.println("C");
}

void printFloodState(){ // afiseaza starea senzorului de inundatie pe comunicatia seriala
  int adc = analogRead(1);
  percent = map(adc, 1023, 465, 0, 100);
  Serial.print("4 ");
  Serial.print(percent);
  Serial.println("% flooding");
}

void writeStringToEEPROM(int addrOffset, const String &strToWrite) // scrie un mesaj in eeprom
{
  byte len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++)
  {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
}

String readStringFromEEPROM(int addrOffset) // citeste mesaj din eeprom
{
  byte newStrLen = EEPROM.read(addrOffset);
  char data[newStrLen + 1];
  for (int i = 0; i < newStrLen; i++)
  {
    data[i] = EEPROM.read(addrOffset + 1 + i);
  }
  data[newStrLen] = '\0'; 
  return String(data);
}

void clearEEPROM() // seteaza toti bitii din eeprom pe 0
{
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    if(EEPROM.read(i) != 0)                     //skip already "empty" addresses
    {
      EEPROM.write(i, 0);                       //write 0 to address i
    }
  }
  Serial.println("EEPROM erased.");
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()){
    message = String(Serial.readString());
    message.trim();
    if(message[0] == '1'){
      if(message.substring(0) == "1 A")
        analogWrite(onoffPin, 255);
      else if(message.substring(0) == "1 S")
        analogWrite(onoffPin, 0);
    }
    else if(message[0] == '2'){
      strcpy(redValue,getColor(message[2], message[3]));
      strcpy(greenValue,getColor(message[4], message[5]));
      strcpy(blueValue,getColor(message[6], message[7]));
      setColor(StrToHex(redValue), StrToHex(greenValue), StrToHex(blueValue));
    }
    else if(message[0] == '6'){
      message = message.substring(2);
      if(message.length() > 39)
        Serial.println("Mesajul transmis este prea lung. Va rugam incercati din nou.");
      else{
        if(numar_mesaje_scrise < 10){
          Serial.print("Mesaj nou de la utilizator: ");
          Serial.println(message);
          writeStringToEEPROM(40 * numar_mesaje_scrise, message);
          numar_mesaje_scrise += 1;
          ultimul_mesaj_scris = numar_mesaje_scrise;
          EEPROM.write(adresaNrMsjScr, numar_mesaje_scrise); 
          EEPROM.write(adresaUltMsjScr, ultimul_mesaj_scris); 
          if(numar_mesaje_scrise == 10)
            Serial.println("WARNING : Mesageria este plina, mesajele transmise in continuare le vor suprascrie pe cele vechi.");
        }
        else{
          Serial.print("Mesaj nou de la utilizator: ");
          Serial.println(message);
          if(ultimul_mesaj_scris == 10){
            ultimul_mesaj_scris = 1;
            EEPROM.write(adresaUltMsjScr, ultimul_mesaj_scris);
            writeStringToEEPROM(0, message);
          }
          else{
            writeStringToEEPROM(40 * ultimul_mesaj_scris, message);
            ultimul_mesaj_scris += 1;
            EEPROM.write(adresaUltMsjScr, ultimul_mesaj_scris);
          }
        }
      }
    }
  }
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;
    printTemperature();
    printFloodState();
  }
}

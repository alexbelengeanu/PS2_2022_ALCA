String message;

char *newColor = new char[3];
char *redValue = new char[3];
char *greenValue = new char[3];
char *blueValue = new char[3];

int on_offPin = 8;
int greenPin = 9;
int redPin = 10;
int bluePin = 11;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(on_offPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
}

void setColor(int red, int green, int blue)
{
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);  
}

char *getColor(char firstChar, char secondChar){
  newColor[0]=firstChar;
  newColor[1]=secondChar;
  newColor[2]='\0';
  return newColor;
}

int StrToHex(char str[])
{
  return (int) strtol(str, 0, 16);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()){
    message = String(Serial.readString());
    message.trim();
    if(message[0] == '1'){
      if(message.substring(0) == "1 A")
        analogWrite(on_offPin, 255);
      else if(message.substring(0) == "1 S")
        analogWrite(on_offPin, 0);
    }
    else if(message[0] == '2'){
      strcpy(redValue,getColor(message[2], message[3]));
      strcpy(greenValue,getColor(message[4], message[5]));
      strcpy(blueValue,getColor(message[6], message[7]));
      
      Serial.println(StrToHex(redValue));
      Serial.println(StrToHex(greenValue));
      Serial.println(StrToHex(blueValue));

      setColor(StrToHex(redValue), StrToHex(greenValue), StrToHex(blueValue));
    }
  }
}

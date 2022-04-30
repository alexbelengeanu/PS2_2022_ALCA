#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>

#define adresaNrMsjScr 1022
#define adresaUltMsjScr 1023
#define bluePin 11 // pin culoare albastra RGB
#define redPin 10 // pin culoare rosie RGB
#define greenPin 9 // pin culoare verde RGB
#define onoffPin 8 // pinul pentru mesajul de on/off
#define BUTTON_BACK 7
#define BUTTON_UP 6
#define BUTTON_DOWN 5
#define BUTTON_OK 4
#define interval 1000 // intervalul la care se iau valorile de la senzori

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

int percent;
volatile int adresaFlagCitit;
float temperature = 0;
long previousMillis = 0; // variabila pentru stocarea timpului scurs de la ultima citire a valorilor senzorilor
String message; // variabila pentru mesajele de pe comunicatia seriala

byte numar_mesaje_scrise = EEPROM.read(adresaNrMsjScr);
byte ultimul_mesaj_scris = EEPROM.read(adresaUltMsjScr);
byte message_counter;
byte degree[8] = {
  B00111,
  B00101,
  B00111,
  B00000,
  B00000,
  B00000,
  B00000,
};

char *newColor = new char[3]; // variabila auxiliara pentru stocarea culorilor RGB
char *blueValue = new char[3]; // valoare canal albastru RGB
char *redValue = new char[3]; // valoare canal rosu RGB
char *greenValue = new char[3]; // valoare canal verde RGB

enum Buttons { // enum cu toate butoanele
  EV_BACK,
  EV_UP,
  EV_DOWN,
  EV_OK,
  EV_NONE,
  EV_MAX_NUM
};

enum MainMenu { // enum cu toate meniurile posibile
  MAIN_MENU = 0,
  MENU_MESAJE,
  MENU_CONTROL,
  MENU_TEMPERATURA,
  MENU_INUNDATII,
  MENU_MAX_NUM
};

enum MsgMenu {
  MSG_MAIN = 0,
  MSG_NECITITE,
  MSG_CITITE,
  MSG_STERGERE,
  MSG_MAX_NUM
};

enum Control { // enum cu toate starile de control
  MANUAL = 0,
  AUTOMAT,
  CONTROL_MAX_NUM
};

MainMenu scroll_main_menu = MAIN_MENU; // setez si scroll menu si current menu pe MAIN_MENU pt inceput
MainMenu current_main_menu =  MAIN_MENU;

MsgMenu scroll_message_menu = MSG_MAIN;
MsgMenu current_message_menu = MSG_MAIN;

Control control_state = MANUAL;

void state_machine_main(enum MainMenu menu, enum Buttons button);
Buttons GetButtons(void);

typedef void (state_machine_handler_t)(void);

void afisare_mesaje_eeprom(){
  Serial.println("=================== AFISARE MESAGERIE ===================");
  if(numar_mesaje_scrise != 0){
    if(numar_mesaje_scrise == ultimul_mesaj_scris){
      message_counter = 1;
      for(int i = numar_mesaje_scrise - 1; i >= 0; i--){
        Serial.print("Flag citire : ");           //TODO : poate sterg flag-ul asta mai tarziu
        Serial.print(EEPROM.read(400+i+1));
        Serial.print(" | ");
        Serial.print(401+i);
        Serial.print(" | ");
        message = readStringFromEEPROM(40 * i);
        Serial.print(message_counter++);
        Serial.print(". ");
        Serial.println(message);
      }
    }
    else{
      message_counter = 1;
      for(int i = ultimul_mesaj_scris - 1; i >= 0; i--){
        Serial.print("Flag citire : ");           //TODO : poate sterg flag-ul asta mai tarziu
        Serial.print(EEPROM.read(401+i));
        Serial.print(" | ");
        Serial.print(401+i);
        Serial.print(" | ");        
        message = readStringFromEEPROM(40 * i);
        Serial.print(message_counter++);
        Serial.print(". ");
        Serial.println(message);
      }
      for(int i = numar_mesaje_scrise - 1; i >= ultimul_mesaj_scris; i--){
        Serial.print("Flag citire : ");           //TODO : poate sterg flag-ul asta mai tarziu
        Serial.print(EEPROM.read(401+i));
        Serial.print(" | ");
        Serial.print(401+i);
        Serial.print(" | ");  
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

void print_menu(enum MainMenu menu){ // functie pt printarea meniului pe LCD
  lcd.clear();
  switch(menu)
  {
    case MENU_MESAJE:
      print_message_menu(current_message_menu);
      break;
    case MENU_CONTROL:
      print_control(control_state);
      break;
    case MENU_TEMPERATURA:
      lcd.print("Temperatura: ");
      lcd.setCursor(0, 1);
      lcd.print(temperature);
      lcd.write(byte(0));
      lcd.print("C");
      break;
    case MENU_INUNDATII:
      lcd.print("Mesaje inundatii"); // TODO: make scrollable through flood messages
      break;
    case MAIN_MENU:
      lcd.print("Meniu principal: ");
      lcd.setCursor(0, 1);
      lcd.print(getMainMenuName(scroll_main_menu));
      break;
    default:
      lcd.print("ERROR");
      break;
  }
}

void setup() { // functie de setup la pornirea placii Arduino
  // put your setup code here, to run once:
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, degree);
  Serial.begin(9600);
  pinMode(onoffPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(4, INPUT);
  digitalWrite(4, LOW); // pull-down
  pinMode(5, INPUT);
  digitalWrite(5, LOW); // pull-down
  pinMode(6, INPUT);
  digitalWrite(6, LOW); // pull-down
  pinMode(7, INPUT);
  digitalWrite(7, LOW); // pull-down
  afisare_mesaje_eeprom();
  print_menu(current_main_menu);
}

const char* getMainMenuName(enum MainMenu menu) // returneaza numele fiecarui meniu dupa enum
{
   switch (menu) 
   {
      case MAIN_MENU: return "Main";
      case MENU_MESAJE: return "Mesaje";
      case MENU_CONTROL: return "Control";
      case MENU_TEMPERATURA: return "Temperatura";
      case MENU_INUNDATII: return "Inundatii";
   }
}

const char* getMessageMenuName(enum MsgMenu menu) // returneaza numele fiecarui meniu dupa enum
{
   switch (menu) 
   {
      case MSG_MAIN: return "Main";
      case MSG_NECITITE: return "Mesaje necitite";
      case MSG_CITITE: return "Mesaje citite";
      case MSG_STERGERE: return "Sterge mesajele";
   }
}

//MainMenu - menu_main
void pressed_back(void) { // se apasa butonul cancel
  scroll_main_menu = MAIN_MENU;
  current_main_menu = scroll_main_menu;

  scroll_message_menu = MSG_MAIN;
  current_message_menu = scroll_message_menu;
}
void main_pressed_up(void) { // se apasa butonul next
  scroll_main_menu = (MainMenu) ((int)scroll_main_menu + 1);
  scroll_main_menu = (MainMenu) ((int)scroll_main_menu % MENU_MAX_NUM);
}
void main_pressed_down(void) { // se apasa butonul prev
  scroll_main_menu = (MainMenu) ((int)scroll_main_menu - 1);
  if(scroll_main_menu == -1){
    scroll_main_menu = (MainMenu) ((int)scroll_main_menu + MENU_MAX_NUM);
  }
}
void main_pressed_ok(void){ // se apasa butonul enter
  current_main_menu = scroll_main_menu;
}

//MainMenu - menu_mesaje
void msg_pressed_up(void) { // se apasa butonul next
  scroll_message_menu = (MsgMenu) ((int)scroll_message_menu + 1);
  scroll_message_menu = (MsgMenu) ((int)scroll_message_menu % MSG_MAX_NUM);
}
void msg_pressed_down(void) { // se apasa butonul prev
  scroll_message_menu = (MsgMenu) ((int)scroll_message_menu - 1);
  if(scroll_message_menu == -1){
    scroll_message_menu = (MsgMenu) ((int)scroll_message_menu + MSG_MAX_NUM);
  }
}
void msg_pressed_ok(void){ // se apasa butonul enter
  current_message_menu = scroll_message_menu;
}

//MainMenu - menu_control
void cnt_pressed_up(void) { // se apasa butonul next
  control_state = (Control) ((int)control_state + 1);
  control_state = (Control) ((int)control_state % CONTROL_MAX_NUM);
}
void cnt_pressed_down(void) { // se apasa butonul prev
  control_state = (Control) ((int)control_state - 1);
  if(control_state == -1){
    control_state = (Control) ((int)control_state + CONTROL_MAX_NUM);
  }
}

//MainMenu - menu_inundatii : pot folosi un for care sa itereze intre toate mesajele
void ind_pressed_up(void) { // se apasa butonul next
  //TODO
}
void ind_pressed_down(void) { // se apasa butonul prev
  //TODO
}


state_machine_handler_t* main_menu[MENU_MAX_NUM][EV_MAX_NUM] = 
{ //events: OK , CANCEL , NEXT, PREV
  {pressed_back, main_pressed_down, main_pressed_up, main_pressed_ok},  // MENU_MAIN
  {pressed_back, msg_pressed_down, msg_pressed_up, msg_pressed_ok},   // MENU_MESAJE
  {pressed_back, cnt_pressed_down, cnt_pressed_up, pressed_back},   // MENU_CONTROL
  {pressed_back, pressed_back, pressed_back, pressed_back},   // MENU_TEMPERATURA
  {pressed_back, ind_pressed_down, ind_pressed_up, pressed_back},   // MENU_INUNDATII
};

void state_machine(enum MainMenu menu, enum Buttons button) { main_menu[menu][button](); }

Buttons GetButtons(void) {
  enum Buttons ret_val = EV_NONE;
  if (digitalRead(BUTTON_OK)) { ret_val = EV_OK; }
  else if (digitalRead(BUTTON_DOWN)) { ret_val = EV_DOWN; }
  else if (digitalRead(BUTTON_UP)) { ret_val = EV_UP; }
  else if (digitalRead(BUTTON_BACK)) { ret_val = EV_BACK; }
  return ret_val;
}

void afisare_mesaje_necitite(){
  byte idxMesajeNecitite[11];
  byte counter = 0;
  for(int start = 401; start <= 400 + numar_mesaje_scrise; start++){
    if(EEPROM.read(start) == 0)
      idxMesajeNecitite[counter++] = start%100;
  }    
  if(counter==0){
    lcd.print("Nu exista mesaje");
    lcd.setCursor(0,1);
    lcd.print("necitite");
    delay(3000);
    pressed_back();
  }
  for(int i = 0; i < counter; i++){
    EEPROM.write((400+idxMesajeNecitite[i]), 1);
  }
  byte first_message;
  byte second_message;
  byte first_message_counter = 0;
  byte second_message_counter = 1;
  message_counter = 1;
  if(counter == 1){
    first_message = idxMesajeNecitite[0] - 1;
    message = readStringFromEEPROM(40 * first_message);
    lcd.print(message_counter++);
    lcd.print(". ");
    lcd.print(message);
    delay(4000);
    pressed_back();
  }
  else if(counter > 1){
    while(!digitalRead(BUTTON_BACK)){
      first_message = idxMesajeNecitite[first_message_counter] - 1;
      second_message = idxMesajeNecitite[second_message_counter] - 1;
      lcd.clear();
      message = readStringFromEEPROM(40 * first_message);
      lcd.print(first_message_counter + 1);
      first_message_counter++;
      lcd.print(". ");
      lcd.print(message);
      lcd.setCursor(0,1);
      message = readStringFromEEPROM(40 * second_message);
      lcd.print(second_message_counter + 1);
      second_message_counter++;
      lcd.print(". ");
      lcd.print(message);
      if(second_message_counter == counter){
        second_message_counter = 0;
        second_message = idxMesajeNecitite[second_message_counter] - 1;
      }
      if(first_message_counter == counter){
        first_message_counter = 0;
        first_message = idxMesajeNecitite[first_message_counter] - 1;
      }
      delay(4000);
    }
  }
}

void afisare_mesaje_citite(){
  byte idxMesajeCitite[11];
  byte counter = 0;
  for(int start = 401; start <= 400 + numar_mesaje_scrise; start++){
    if(EEPROM.read(start) == 1 && counter <= numar_mesaje_scrise - 1)
      idxMesajeCitite[counter++] = start%100;
  }
  if(counter==0){
    lcd.print("Nu exista mesaje");
    lcd.setCursor(0,1);
    lcd.print("citite");
    delay(3000);
    pressed_back();
  }
  byte first_message;
  byte second_message;
  byte first_message_counter = 0;
  byte second_message_counter = 1;
  message_counter = 1;
  if(counter == 1){
    first_message = idxMesajeCitite[0] - 1;
    message = readStringFromEEPROM(40 * first_message);
    lcd.print(message_counter++);
    lcd.print(". ");
    lcd.print(message);
    delay(4000);
    pressed_back();
  }
  else if(counter > 1){
    while(!digitalRead(BUTTON_BACK)){
      first_message = idxMesajeCitite[first_message_counter] - 1;
      second_message = idxMesajeCitite[second_message_counter] - 1;
      lcd.clear();
      message = readStringFromEEPROM(40 * first_message);
      lcd.print(first_message_counter + 1);
      first_message_counter++;
      lcd.print(". ");
      lcd.print(message);
      lcd.setCursor(0,1);
      message = readStringFromEEPROM(40 * second_message);
      lcd.print(second_message_counter + 1);
      second_message_counter++;
      lcd.print(". ");
      lcd.print(message);
      if(second_message_counter == counter){
        second_message_counter = 0;
        second_message = idxMesajeCitite[second_message_counter] - 1;
      }
      if(first_message_counter == counter){
        first_message_counter = 0;
        first_message = idxMesajeCitite[first_message_counter] - 1;
      }
      delay(4000);
    }
  }
}

void print_message_menu(enum MsgMenu menu){ // functie pt printarea meniului pe LCD
  lcd.clear();
  switch(menu)
  {
    case MSG_NECITITE:
      afisare_mesaje_necitite();
      break;
    case MSG_CITITE:
      afisare_mesaje_citite();
      break;
    case MSG_STERGERE:
      lcd.print("Sunteti sigur?");
      lcd.setCursor(0,1);
      lcd.print("(Y/n)");
      delay(2000);
      if (digitalRead(BUTTON_OK)){
        clearEEPROM();
        lcd.clear();
        lcd.print("EEPROM erased");
        delay(4000);
        pressed_back();
      }
      else if (digitalRead(BUTTON_BACK))
        pressed_back();
      break;
    case MSG_MAIN:
      lcd.print("Meniu mesaje: ");
      lcd.setCursor(0, 1);
      lcd.print(getMessageMenuName(scroll_message_menu));
      break;
    default:
      lcd.print("ERROR");
      break;
  }
}

void print_control(enum Control control_state){ // functie pt printarea meniului pe LCD
  lcd.clear();
  switch(control_state)
  {
    case MANUAL:
      lcd.print("Control: ");
      lcd.setCursor(0, 1);
      lcd.print("Manual");
      break;
    case AUTOMAT:
      lcd.print("Control: ");
      lcd.setCursor(0, 1);
      lcd.print("Automat"); // TODO: change here to add manual/automat
      break;
    default:
      lcd.print("ERROR");
      break;
  }
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
  temperature = voltage / 10;
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
        adresaFlagCitit = 400 + ultimul_mesaj_scris;
        EEPROM.write(adresaFlagCitit, 0);
      }
    }
  }
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > interval) {
    printTemperature();
    printFloodState();
    volatile Buttons event = GetButtons();
    if (event != EV_NONE){
          state_machine(current_main_menu, event);
    }
    print_menu(current_main_menu);
    previousMillis = currentMillis;
  }
}

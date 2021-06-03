#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

struct Aliment {
  const char* name;
  int calories;
  int fats;
  int carbohidrates;
  int proteins;
};

typedef struct Aliment Aliment;

Aliment* aliments;

LiquidCrystal_I2C lcd(0x27,20,4);

const int HX711_dout = 4;
const int HX711_sck = 5;

HX711_ADC LoadCell(HX711_dout, HX711_sck);

const int calVal_calVal_eepromAdress = 0;
unsigned long t = 0;

int changePressed = 0;
int weight = 125;

int alimentIndex = 0;
int numberOfAliments = 3;

void setup()
{
  setupCantar();
  
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  
  lcd.setCursor(0,0);
  custom_print("Weight:");
  
  lcd.setCursor(0,1);
  custom_print("Aliment:");

  lcd.setCursor(0,2);
  custom_print("Cal:");

  lcd.setCursor(10,2);
  custom_print("Fats:");

  lcd.setCursor(0,3);
  custom_print("Prot:");

  lcd.setCursor(10,3);
  custom_print("Carbs:");
  
  pinMode(13, INPUT);
  initAliments();
}

void initAliments() {
  aliments = (Aliment*)malloc(numberOfAliments * sizeof(struct Aliment));

  initAliment(0, "Avocado", 31, 16, 6, 2);

  initAliment(1, "Potatoes", 76, 0, 17 , 2);
  
  initAliment(2, "Bananas", 88, 0, 23 , 1);
  
}

void initAliment(int index, const char* name, int calories, int fats, int carbohidrates, int proteins) {
  aliments[index].name = name;
  aliments[index].calories = calories;
  aliments[index].fats = fats;
  aliments[index].carbohidrates = carbohidrates;
  aliments[index].proteins = proteins;
}
void loop()
{
  lcd.setCursor(7, 0);
  static boolean newDataReady = 0;
  const int serialPrintInterval = 500;
  
  if (LoadCell.update()) newDataReady = true;

  float i;
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      i = LoadCell.getData();
      if (i > 2)
        custom_print(String(round(i)) + "g  ");
      else 
        custom_print("0g ");
      newDataReady = 0;
      t = millis();
    }
  }

  int isPressed = !digitalRead(13);
  if(isPressed) {
    alimentIndex++;
    clearLCDLine(8, 1, 10); // clear name
    clearLCDLine(4, 2, 5);  // clean cals
    clearLCDLine(15, 2, 4); // clear fats
    clearLCDLine(5, 3, 4);  // clear prots
    clearLCDLine(16, 3, 3); // clear carbs
    if (alimentIndex == numberOfAliments)
      alimentIndex = 0;
      
    delay(250);
  }
    
  printNutritionalValues(i);
}

void printNutritionalValues(float weight) {
  lcd.setCursor(8,1);
  custom_print(getActualAlimentName());

  lcd.setCursor(4, 2);
  int roundedValue = round(weight/100);
  custom_print(String(roundedValue * aliments[alimentIndex].calories) +"g  ");
  
  lcd.setCursor(15, 2);
  custom_print(String(roundedValue * aliments[alimentIndex].fats )+"g  ");
  
  lcd.setCursor(5, 3);
  custom_print(String(roundedValue * aliments[alimentIndex].proteins ) +"g  ");
  
  lcd.setCursor(16, 3);
  
  String carbsValue = String(roundedValue * aliments[alimentIndex].carbohidrates ) +"g";
  
  custom_print(carbsValue);
  if (roundedValue < 10 ) 
    custom_print(" ");
}

String getActualAlimentName() {
  const char* name = aliments[alimentIndex].name;
  return String(name);
}

void clearLCDLine(int column, int line, int end)
{               
    lcd.setCursor(column,line);
    for(int n = 0; n < end; n++) {
            lcd.print(" ");
    }
}

//// due to some issues with this version of library and this i2c module
//// lcd.print() prints only the first character and this is the my workaround
void custom_print(String word) {
  for (int i = 0; i < word.length(); i++) {
    lcd.print(word[i]);
  }
}


void setupCantar() {

  float calibrationValue;
  calibrationValue = -205.49; 

  LoadCell.begin();
  unsigned long stabilizingtime = 2000;
  boolean _tare = true;
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {

  }
  else {
    LoadCell.setCalFactor(calibrationValue);
  }
  while (!LoadCell.update());
}

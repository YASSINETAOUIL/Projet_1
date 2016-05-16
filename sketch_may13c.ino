#include <EEPROM.h>
#include <OneWire.h>
#include <LiquidCrystal_I2C.h>//Inclusion de la librairie I2c
#define DS18B20 0x28     // Adresse 1-Wire du DS18B20
#define BROCHE_ONEWIRE 7 // Broche utilisée pour le bus 1-Wire

#define BROCHE_bouton0 2
#define BROCHE_bouton1 3//ze
#define BROCHE_bouton2 4

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define Huile_Minerale  0
#define Huile_PG  1
#define Huile_SHC 2

int address0 = 0; // adress stockage type huile
int address1 = 1; // adress stockage durée de vie

int Stats_bouton0 = 0; // button for huile minérale
int Stats_bouton1 = 0; // huile synthétique
int Stats_bouton2 = 0; //
int Stats_bouton3 = 0; // signale qui proviens ver le moteur
int Stats_bouton4 = 0; // for reset

double Duree_for_synthes = 20000; // 20 mille heur
double Duree_for_mineral = 15000; //15 mille heur

double Heur_to_second = 3600;
double Duree_vie_courant;

double TIME_Start, TIME_Stop;

int Type_huile = 2; // par defaut Huile_SHC=2

float temperature = 50, tmp_temperature;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  lcd.begin();
  lcd.clear();
  switch (EEPROM.read(address0)) {
    case 0: Type_huile = Huile_Minerale;
      break;
    case 1: Type_huile = Huile_PG;
      break;
    case 2: Type_huile = Huile_SHC;
      break;
  }
  Serial.print("OK i m sauvgarde in type huile:\t");
  Serial.println(Type_huile);

  if (EEPROM.read(address1) != 0) {
    EEPROM.get(address1, Duree_vie_courant);
  }
  Serial.print("OK i m sauvgarde in duré de vie:\t");
  Serial.println(Type_huile);
}

OneWire ds(BROCHE_ONEWIRE);

void loop() {
  // put your main code here, to run repeatedly:
  Get_stats_choix();
  Stats_bouton3 = 1; // tjr en marche
  if (Get_temp(&tmp_temperature)) {
    Test_on_bouton3();
  } else {
    Serial.println("Erreur temperature");
  }
}

void Alert_huile(void) {
  if (Type_huile == Huile_Minerale) {
    if (temperature > 85 || temperature < -10 || Duree_vie_courant <= 1) {
      Serial.println("Alerte Changer le Huile Minerale");
    }
  } else if (Type_huile == Huile_PG) {
    if (temperature > 105 || temperature < -25 || Duree_vie_courant <= 1) {
      Serial.println("Alerte Changer le Huile PG");
    }
  } else if (Type_huile == Huile_SHC) {
    if (temperature > 105 || temperature < -25 || Duree_vie_courant <= 1) {
      Serial.println("Alerte Changer le Huile SHC");
    }
  } else {
    Serial.println("EREUR Type huile not defined");
    delay(5000);
  }
}
void Test_on_bouton3(void) {
  if (Stats_bouton3 == 0) {
    Serial.println("OFF:: Signal d'arret from bouton 3 ");
  } else {
    TIME_Start = millis();
    Serial.println("ON:: Signal de marche from bouton 3");
    print_huile();
    for (int i = 0; i < 15; i++) {
      delay(1000);//1s
      Get_temp(&tmp_temperature);
      temperature += tmp_temperature;////////////////////
    }
    temperature = temperature / 15;
    Serial.print("Temperature=");
    Serial.println(temperature + 'C');
    TIME_Stop = millis();
    display_time_life();
    Alert_huile();
  }
}
void print_huile(void) {
  switch (Type_huile) {
    case 0: Serial.println("Type huile selectionne: Huile Minerale");
      break;
    case 1: Serial.println("Type huile selectionne: Huile PG");
      break;
    case 2: Serial.println("Type huile selectionne: Huile SHC");
      break;
    default:
      break;
  }
}

void display_time_life(void) {
  print_huile();
  if (Duree_vie_courant <= 0) {
    switch (Type_huile) {
      case 0: Duree_vie_courant = Duree_for_mineral;
        break;
      case 1: Duree_vie_courant = Duree_for_synthes;
        break;
      case 2: Duree_vie_courant = Duree_for_synthes;
        break;
      default:
        break;
    }
  }
  switch (Type_huile) {
    case 0:
      if (temperature > 20 && temperature < 80) {
        calcul_life(1);
      } else if (temperature > 80 && temperature < 85) {
        calcul_life(2);
      }
      break;
    case 1:
      if (temperature > 20 && temperature < 80) {
        calcul_life(1);
      } else if (temperature > 80 && temperature < 90) {
        calcul_life(1.92);
      } else if (temperature > 90 && temperature < 100) {
        calcul_life(2.17);
      }
      break;
    case 2:
      if (temperature > 20 && temperature < 80) {
        calcul_life(1);
      } else if (temperature > 80 && temperature < 90) {
        calcul_life(1.92);
      } else if (temperature > 90 && temperature < 100) {
        calcul_life(1.2);
      } else if (temperature > 100 && temperature < 105) {
        calcul_life(1.96);
      } else if (temperature > 100 && temperature < 105) {
        calcul_life(1.92);
      }
      break;
  }
}
void calcul_life(float coef) {
  Duree_vie_courant = Duree_vie_courant * Heur_to_second;
  Duree_vie_courant = Duree_vie_courant - coef * (TIME_Stop - TIME_Start) / 1000;
  Duree_vie_courant = Duree_vie_courant / Heur_to_second;
  Serial.print("Durée de vie == ");
  Serial.print(Duree_vie_courant);
  Serial.println(" H");
  EEPROM.put(address1, Duree_vie_courant);
}

boolean Get_temp(float *tempe) {
  byte data[9], addr[8];
  ds.reset_search();
  if (!ds.search(addr)) {
    lcd.print("Problème Capteur not found");
    Serial.println("Problème Capteur not found");
    ds.reset_search();
    return false;
  }
  if (OneWire::crc8(addr, 7) != addr[7])
    return false;
  if (addr[0] != DS18B20)
    return false;
  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);
  delay(800);
  ds.reset();
  ds.select(addr);
  ds.write(0xBE);
  for (byte i = 0; i < 9; i++)
    data[i] = ds.read();
  *tempe = ((data[1] << 8) | data[0]) * 0.0625;
  // Pas d'erreur
  return true;
}

void Get_stats_choix(void) {
  Stats_bouton0 = digitalRead(BROCHE_bouton0);
  Stats_bouton1 = digitalRead(BROCHE_bouton1);
  Stats_bouton2 = digitalRead(BROCHE_bouton2);
  // Stats_bouton3 = digitalRead(BROCHE_bouton3);
  // Stats_bouton4 = digitalRead(BROCHE_bouton4);
  if (Stats_bouton0 == 1) {
    if (Type_huile != Huile_Minerale) {
      Duree_vie_courant = Duree_for_mineral;
    }
    Type_huile = Huile_Minerale;
    LCD_print();
    lcd.print(temperature);
    lcd.print((char)223 + 'C');
    delay(5000);
    lcd.clear();
    EEPROM.update(address0, Huile_Minerale);
    EEPROM.put(address1, Duree_vie_courant);
    Serial.println("Type choisi :Huile Minerale");
  } else if (Stats_bouton1 == 1) {
    if (Type_huile != Huile_PG) {
      Duree_vie_courant = Duree_for_synthes;
    }
    Type_huile = Huile_PG;
    LCD_print();
    lcd.print(temperature);
    lcd.print((char)223 + 'C');
    delay(5000);
    lcd.clear();
    EEPROM.put(address1, Duree_vie_courant);
    EEPROM.update(address0, Huile_PG);
    Serial.println("Type choisi: Huile PG");
  } else if (Stats_bouton2 == 1) {
    if (Type_huile != Huile_SHC) {
      Duree_vie_courant = Duree_for_synthes;
    }
    Type_huile = Huile_SHC;
    LCD_print();
    lcd.print(temperature);
    lcd.print((char)223 + 'C');
    delay(5000);
    lcd.clear();
    EEPROM.update(address0, Huile_SHC);
    EEPROM.put(address1, Duree_vie_courant);

    Serial.println("Type choisi: Huile SHC");
  } else {
    Serial.println("Button is not clicked");
  }
}

void LCD_print(void) {
  lcd.clear();
  lcd.setCursor(0, 0);
  if (Type_huile == 0) {
    lcd.print("Type choisi :Huile Minerale");
  } else if (Type_huile == 1) {
    lcd.print("Type choisi :Huile PG");
  } else if (Type_huile == 2) {
    lcd.print("Type choisi :Huile SHC");
  } else {
    lcd.print("Erreur choix huile");
  }
  lcd.setCursor(0, 1);
  lcd.print("Temp:");
}

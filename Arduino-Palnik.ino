/*
 * Program vyžaduje knihovny LiquidCrystal_I2C a Streaming.
 * Pin A12 slouží jako indikace připravenosti na první odpal - log HIGH.
 * Pokud je odpalováno, pin A12 je v log LOW.
 * Tlačítko pro řízení je připojeno mezi piny A15 a GND. Aktivní je v LOW.
 * Smyčka kontroly stisku tlačítka se opakuje každých 100 ms.
 * Po dosažení čísla 57 je počítáno od 0.
 * Piny 20 a 21 slouží pro připojení LCD přes I2C a jsou přeskakovány. 50 - 53 vyhrazeny pro SPI.
 * Na LCD se zobrazují čísla fyzických digitálních pinů z pole outputs.
 * Přepínač MODE zapojen mezi A14 a GND: stisknuto - fire, jinak test.
 *   Při změně stavu je systém restartován do příslušného módu.
 * Pin A13 je určen pro připojení relé přepínající test/odpálení: HIGH - test.
 * Ve výpisu na LCD je uváděn formát pozice_v_poli[číslo_pinu]. Anologové piny A0 - A11: 54 - 65.
 * Na začátku testovacího módu dojde k otestování všech výstupů. Stav jse vypsán na sériový port.
 * Pin 13 je určen jako heartbeat.
 */

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Streaming.h>

//LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
LiquidCrystal_I2C lcd(0x27, 20, 4);

int outputs [56] = {A12,     2, 3, 4, 5, 6, 7, 8, 9,
                    10, 11, 12,     14, 15, 16, 17, 18, 19,
                            22, 23, 24, 25, 26, 27, 28, 29, // I2C: 20 = SDA, 21 = SCL
                    30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
                    40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
                    A0, A1, A2, A3, A4, A5, A6, A7, A8, A9,
                    };

#define PIN_MAN_PAL A15 // Tlačítko inkrementace výstupu
#define PIN_MAN_MODE A14 // Přepínač módu PAL/TEST
#define PIN_REL_CHK A13 // Výstup pro ovládání relé PAL/TEST
#define PIN_HB 13 // Výstup hearbeat
#define PIN_VOLT_U1 A10 // První voltmetr
#define PIN_VOLT_U2 A11 // Druhý voltmetr

int BTN_PAL_STAT_old = 0;
int BTN_MODE_old = 1;
int out;
int BTN_PAL_STAT = 0;
int iTmp = 0;

int HBledState = LOW;

void setup()
{
  Serial.begin(9600);
  Serial.println("Starting program...");
  Serial.println("");

  lcd.init();
  lcd.begin(16,2);
  lcd.backlight();

  lcd.home ();
  lcd.print("Init OUTPUTS");

  pinMode(PIN_MAN_PAL, INPUT);      // Nastavení tlačítka odpalu
  digitalWrite(PIN_MAN_PAL, HIGH);
  pinMode(PIN_MAN_MODE, INPUT);     // Nastavení tlačítka módu
  digitalWrite(PIN_MAN_MODE, HIGH);
  pinMode(PIN_HB, OUTPUT);     // Pin 13 - heartbeat
  pinMode(PIN_VOLT_U1, INPUT);
  //digitalWrite(PIN_VOLT_U1, LOW);
  pinMode(PIN_VOLT_U2, INPUT);
  //digitalWrite(PIN_VOLT_U2, LOW);

  for (out = 0 ; out < 56 ; ++out) {
    pinMode(outputs[out], OUTPUT);
    digitalWrite(outputs[out], LOW);
    Serial << "Setup PIN " << outputs[out] << " as OUTPUT/LOW." << endl;
  }

  pinMode(PIN_REL_CHK, OUTPUT);     // Nastavení výstupu pro přepnutí relé pro režim odpalování
  digitalWrite(PIN_REL_CHK, HIGH);

  digitalWrite(outputs[0], HIGH); // Nastavit pin 0 jako HIGH - indikace připravenosti (LED)
  lcd.print("..Ok");
  out = 0;
  delay(1000);
  lcd.home ();
//  lcd.print("Ready to operate.");
  float voltage1 = analogRead(PIN_VOLT_U1) * 5 / 1023;
  float voltage2 = analogRead(PIN_VOLT_U2) * 5 / 1023;
  lcd << "U1:" << voltage1 << "V,2:" << voltage2 << "V";

  BTN_MODE_old = digitalRead(PIN_MAN_MODE);
  if (BTN_MODE_old == 0) { // Inicializace režimu pro odpal
    digitalWrite(PIN_REL_CHK, HIGH); // Vypneme testovací režim - relé do HIGH
    lcd.setCursor(0, 1);
    lcd.print("... Fire mode!");
    Serial.println("... Fire mode!");
  } else { // Inicializace testovacího režimu
    lcd.setCursor(0, 1);
    lcd.print("... Check");
    Serial.println("... Checking...");
    digitalWrite(PIN_REL_CHK, LOW); // Zapneme testovací režim - relé do LOW
    delay(200); // Počkáme 200 ms na přepnutí relé a budeme testovat výstup palníku
    for (out = 0 ; out < 56 ; ++out) {
      pinMode(outputs[out], INPUT); // Nastavíme pin jako vstup
      digitalWrite(outputs[out], HIGH); // Připojíme pull-up
      if (digitalRead(outputs[out]) == LOW)
        iTmp = HIGH;
      else
        iTmp = LOW;
      Serial << "Stat: " << out << "[" << outputs[out] << "]:" << iTmp << endl; // Otestujeme výstup a vypíšeme stav
    }
    lcd.print(" ... OK");
    Serial.println("... checked");
    Serial.println("... Manual check mode>");
  }
}

void loop()
{
  if (digitalRead(PIN_MAN_MODE) != BTN_MODE_old) { // Při změně módu reset zařízení
    lcd.clear();
    Serial << "Mode changed: restart!" << endl;
    lcd << "Mode changed:";
    lcd.setCursor(0, 1);
    lcd << "... restart!";
    delay(1000);
    asm volatile( // Restart chipu
      "clr r30 \n\t"
      "clr r31 \n\t"
      "ijmp \n\t");
  }

  BTN_PAL_STAT = digitalRead(PIN_MAN_PAL);
  if (BTN_PAL_STAT != BTN_PAL_STAT_old) {
    BTN_PAL_STAT_old = BTN_PAL_STAT;
    if (BTN_PAL_STAT == 0) {
      if (digitalRead(PIN_MAN_MODE) == 0) { // Zpracování předchozího stisku - test, zda palník odpálil
        digitalWrite(outputs[out], LOW); // Vypneme výstup
        pinMode(outputs[out], INPUT); // Nastavíme pin jako vstup
        lcd.clear();
        lcd.setCursor(0, 1);
        digitalWrite(PIN_REL_CHK, LOW); // Zapneme testovací režim - relé do LOW
        delay(100); // Počkáme 100 ms na přepnutí relé a budeme testovat výstup palníku
        digitalWrite(outputs[out], HIGH); // Připojíme pull-up k testovanému pinu
        if (digitalRead(outputs[out]) == LOW)
          iTmp = HIGH;
        else
          iTmp = LOW;
        lcd << "Stat: " << out << "[" << outputs[out] << "]:" << iTmp; // Otestujeme výstup a vypíšeme stav
        Serial << "Stat: " << out << "[" << outputs[out] << "]:" << iTmp << endl; // Otestujeme výstup a vypíšeme stav
        pinMode(outputs[out], OUTPUT); // Opět přenastavíme pin jako výstup (ze vstupu pro režim testování)
        digitalWrite(outputs[out], LOW); // Nastavíme na pinu LOW
        digitalWrite(PIN_REL_CHK, HIGH); // Ukončíme test - relé do HIGH
      }
      ++out; // Inkrementace pozice výstupu - číslo výstupu je z pole outputs[out]
      if (out >= 56) { // Pokud jsme na konci pole, začneme znovu - je to samozřejmě zbytečné, ale hodí se pro ladění zařízení
        out = 0;
      }
      if (digitalRead(PIN_MAN_MODE) == 0) { // Odpálení - tlačítko MODE sepnuto
        pinMode(outputs[out], OUTPUT); // Nastavit jako výstup - mělo by být z předchozích kroků, ale pro jistotu
        digitalWrite(outputs[out], HIGH); // Pal!
        lcd.setCursor(0, 0);
        lcd << "Fire num " << out << "[" << outputs[out] << "]!         ";
        Serial << "Fire num " << out << "[" << outputs[out] << "]! " << endl;
        delay(100);
      } else { // Test - tlačítko MODE rozepnuto
        lcd.clear();
        lcd << "Chk num " << out << "[" << outputs[out] << "]>         ";
        Serial << "Chk num " << out << "[" << outputs[out] << "]> " << endl;
        lcd.setCursor(0, 1);
        if (digitalRead(outputs[out]) == LOW)
          iTmp = HIGH;
        else
          iTmp = LOW;
        lcd << "Stat: " << iTmp << " "; // Otestujeme výstup a vypíšeme stav
        Serial << "Stat: " << iTmp << endl; // Otestujeme výstup a vypíšeme stav
//        digitalWrite(PIN_REL_CHK, HIGH); // Ukončíme test - relé do HIGH
      }
    } else {
      delay(100);
    }
  } else {
    delay(100);
  }

  if (HBledState == LOW)
    HBledState = HIGH;
  else
    HBledState = LOW;
  digitalWrite(PIN_HB, HBledState);
}

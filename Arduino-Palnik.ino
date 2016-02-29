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

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Streaming.h> 

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

int outputs [58] = {A12,     2, 3, 4, 5, 6, 7, 8, 9,
                    10, 11, 12,     14, 15, 16, 17, 18, 19,
                            22, 23, 24, 25, 26, 27, 28, 29, // I2C: 20 = SDA, 21 = SCL
                    30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
                    40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
                    A0, A1, A2, A3, A4, A5, A6, A7, A8, A9,
                    A10, A11
                    };

//int outputs_last [58] = {};

#define PIN_MAN_PAL A15 // Tlačítko inkrementace výstupu
#define PIN_MAN_MODE A14 // Přepínač módu PAL/TEST
#define PIN_REL_CHK A13 // Výstup pro ovládání relé PAL/TEST
#define PIN_HB 13 // Výstup hearbeat

int BTN_PAL_STAT_old = 0;
int BTN_MODE_old = 1;
int out;
int BTN_PAL_STAT = 0;

int HBledState = LOW;

void setup()
{
  Serial.begin(9600);
  Serial.println("Starting program...");
  Serial.println(""); 
  
  lcd.begin(16,2);
  lcd.backlight();

  lcd.home ();
  lcd.print("Init OUTPUTS");

  pinMode(PIN_MAN_PAL, INPUT);      // Nastavení tlačítka odpalu
  digitalWrite(PIN_MAN_PAL, HIGH);
  pinMode(PIN_MAN_MODE, INPUT);     // Nastavení tlačítka módu
  digitalWrite(PIN_MAN_MODE, HIGH);
  pinMode(PIN_REL_CHK, OUTPUT);     // Nastavení výstupu pro přepnutí relé pro režim odpalování
  digitalWrite(PIN_REL_CHK, LOW);
  pinMode(PIN_HB, OUTPUT);     // Pin 13 - heartbeat

  for (out = 0 ; out < 58 ; ++out) {
    pinMode(outputs[out], OUTPUT);
    digitalWrite(outputs[out], LOW);
    Serial << "Setup PIN " << outputs[out] << " as OUTPUT/LOW." << endl;
  }
  digitalWrite(outputs[0], HIGH); // Nastavit pin 0 jako HIGH - indikace připravenosti (LED)
  lcd.print("..Ok");
  out = 0;
  delay(2000);
  lcd.home ();
  lcd.print("Ready to operate.");
  BTN_MODE_old = digitalRead(PIN_MAN_MODE);
  if (BTN_MODE_old == 0) { // Inicializace režimu pro odpal
    digitalWrite(PIN_REL_CHK, LOW); // Vypneme testovací režim - relé do LOW
    lcd.setCursor(0, 1);
    lcd.print("... Fire mode!");
    Serial.println("... Fire mode!");
  } else { // Inicializace testovacího režimu
    lcd.setCursor(0, 1);
    lcd.print("... Check");
    Serial.println("... Checking...");
    digitalWrite(PIN_REL_CHK, HIGH); // Zapneme testovací režim - relé do HIGH
    delay(100); // Počkáme 100 ms na přepnutí relé a budeme testovat výstup palníku
    for (out = 0 ; out < 58 ; ++out) {
      digitalWrite(outputs[out], LOW); // Vypneme výstup
      pinMode(outputs[out], INPUT); // Nastavíme výstup jako vstup
      Serial << "Stat: " << out << "[" << outputs[out] << "]:" << digitalRead(outputs[out]) << endl; // Otestujeme výstup a vypíšeme stav
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
    lcd << "Mode changed: restart!";
    Serial << "Mode changed: restart!" << endl;
    delay(200);
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
        pinMode(outputs[out], INPUT); // Nastavíme výstup jako vstup
        lcd.clear();
        lcd.setCursor(0, 1);
        digitalWrite(PIN_REL_CHK, HIGH); // Zapneme testovací režim - relé do HIGH
        delay(100); // Počkáme 100 ms na přepnutí relé a budeme testovat výstup palníku
        lcd << "Stat: " << out << "[" << outputs[out] << "]:" << digitalRead(outputs[out]); // Otestujeme výstup a vypíšeme stav
        Serial << "Stat: " << out << "[" << outputs[out] << "]:" << digitalRead(outputs[out]) << endl; // Otestujeme výstup a vypíšeme stav
        pinMode(outputs[out], OUTPUT); // Opět přenastavíme výstup jako výstup (ze vstupu pro režim testování)
        digitalWrite(outputs[out], LOW); // Pro jistotu nastavíme ještě jednou LOW
        digitalWrite(PIN_REL_CHK, LOW); // Ukončíme test - relé do LOW
      }
      ++out; // Inkrementace pozice výstupu - číslo výstupu je z pole outputs[out]
      if (out >= 58) { // Pokud jsme na konci pole, začneme znovu - je to samozřejmě zbytečné, ale hodí se pro ladění zařízení
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
//        digitalWrite(PIN_REL_CHK, HIGH); // Zapneme testovací režim - relé do HIGH
//        delay(100); // Počkáme 100 ms na přepnutí relé a budeme testovat výstup palníku
//        pinMode(outputs[out], INPUT);
        lcd.setCursor(0, 1);
        lcd << "Stat: " << digitalRead(outputs[out]) << " "; // Otestujeme výstup a vypíšeme stav
        Serial << "Stat: " << digitalRead(outputs[out]) << endl; // Otestujeme výstup a vypíšeme stav
//        digitalWrite(PIN_REL_CHK, LOW); // Ukončíme test - relé do LOW
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
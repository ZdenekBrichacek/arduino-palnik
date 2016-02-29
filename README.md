Ovládací program pro odpalování ohňostrojů
Pro MCU ATMEGA2560 - Arduino Mega 2560

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
 

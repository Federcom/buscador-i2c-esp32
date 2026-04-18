#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// ============================================================
// PINES
// ============================================================

#define OLED_SCK    18
#define OLED_MOSI   23
#define OLED_RES    16
#define OLED_DC     17
#define OLED_CS      5

#define PIN_BOTON    4

#define I2C_SDA     21
#define I2C_SCL     22

// ============================================================
// PANTALLA
// ============================================================

Adafruit_SH1106G display(128, 64,
                         OLED_MOSI, OLED_SCK, OLED_DC, OLED_RES, OLED_CS);

// ============================================================
// TIEMPOS
// ============================================================

const unsigned long INTERVALO_ESCANEO_MS = 1500;
const unsigned long INTERVALO_ANIM_MS    = 300;
const unsigned long INTERVALO_SCROLL_MS  = 2000;
const unsigned long INTERVALO_CAMBIO_MS  = 1200;

// ============================================================
// CONTROL
// ============================================================

bool escaneando = false;
bool pantallaSucia = true;

unsigned long ultimoEscaneo = 0;
unsigned long ultimoAnim = 0;
unsigned long ultimoScroll = 0;

byte estadoAnim = 0;
byte scrollIndex = 0;

// ============================================================
// RESULTADOS
// ============================================================

byte direcciones[32];
byte total = 0;

byte prevDirecciones[32];
byte prevTotal = 0;

// Cambios
byte nuevas[8];
byte totalNuevas = 0;

byte eliminadas[8];
byte totalEliminadas = 0;

unsigned long mostrarCambiosHasta = 0;

// ============================================================
// BOTÓN
// ============================================================

bool lastRead = HIGH;
bool stableState = HIGH;
unsigned long lastChange = 0;
unsigned long pressTime = 0;
bool longPress = false;

// ============================================================
// SETUP
// ============================================================

void setup() {
  pinMode(PIN_BOTON, INPUT_PULLUP);

  Wire.begin(I2C_SDA, I2C_SCL);

  Serial.begin(115200);
  Serial.println("I2C Scanner iniciado");

  if (!display.begin(0, true)) {
    while (true);
  }

  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);

  pantallaReposo();
}

// ============================================================
// LOOP
// ============================================================

void loop() {
  boton();

  if (escaneando) {

    if (millis() - ultimoEscaneo > INTERVALO_ESCANEO_MS) {
      ultimoEscaneo = millis();

      if (scanI2C()) {
        pantallaSucia = true;
      }
    }

    if (millis() - ultimoAnim > INTERVALO_ANIM_MS) {
      ultimoAnim = millis();
      estadoAnim = (estadoAnim + 1) % 4;
      dibujarPie();
    }

    if (millis() - ultimoScroll > INTERVALO_SCROLL_MS) {
      ultimoScroll = millis();
      scrollIndex++;
      pantallaSucia = true;
    }
  }

  if (pantallaSucia) {
    pantallaResultados();
    pantallaSucia = false;
  }
}

// ============================================================
// BOTÓN
// ============================================================

void boton() {
  bool lectura = digitalRead(PIN_BOTON);

  if (lectura != lastRead) {
    lastChange = millis();
    lastRead = lectura;
  }

  if (millis() - lastChange > 40) {
    if (lectura != stableState) {
      stableState = lectura;

      if (stableState == LOW) {
        pressTime = millis();
        longPress = false;
      } else {
        if (!longPress) {
          escaneando = !escaneando;
          pantallaSucia = true;
        }
      }
    }
  }

  if (stableState == LOW && !longPress) {
    if (millis() - pressTime > 800) {
      longPress = true;
      borrar();
    }
  }
}

// ============================================================
// ESCANEO
// ============================================================

bool scanI2C() {

  byte temp[32];
  byte tempTotal = 0;

  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      temp[tempTotal++] = addr;
    }
  }

  // Detectar cambios
  totalNuevas = 0;
  totalEliminadas = 0;

  for (byte i = 0; i < tempTotal; i++) {
    bool found = false;
    for (byte j = 0; j < prevTotal; j++) {
      if (temp[i] == prevDirecciones[j]) found = true;
    }
    if (!found && totalNuevas < 8) {
      nuevas[totalNuevas++] = temp[i];
      Serial.print("+ 0x"); Serial.println(temp[i], HEX);
    }
  }

  for (byte i = 0; i < prevTotal; i++) {
    bool found = false;
    for (byte j = 0; j < tempTotal; j++) {
      if (prevDirecciones[i] == temp[j]) found = true;
    }
    if (!found && totalEliminadas < 8) {
      eliminadas[totalEliminadas++] = prevDirecciones[i];
      Serial.print("- 0x"); Serial.println(prevDirecciones[i], HEX);
    }
  }

  bool cambiado = (tempTotal != total);

  for (byte i = 0; i < tempTotal && !cambiado; i++) {
    if (temp[i] != direcciones[i]) cambiado = true;
  }

  if (cambiado) {
    total = tempTotal;

    for (byte i = 0; i < total; i++) {
      direcciones[i] = temp[i];
      prevDirecciones[i] = temp[i];
    }
    prevTotal = total;

    mostrarCambiosHasta = millis() + INTERVALO_CAMBIO_MS;
  }

  return cambiado;
}

// ============================================================
// PANTALLA
// ============================================================

void pantallaReposo() {
  display.clearDisplay();
  display.setCursor(10, 25);
  display.println("Pulsa para escanear");
  display.setCursor(10, 40);
  display.println("Larga para borrar");
  display.display();
}

void pantallaResultados() {

  display.clearDisplay();

  display.drawRect(0, 0, 128, 64, SH110X_WHITE);
  display.drawFastHLine(0, 12, 128, SH110X_WHITE);
  display.drawFastHLine(0, 52, 128, SH110X_WHITE);

  centrar("BUSCADOR I2C", 2);

  int start = scrollIndex % (total == 0 ? 1 : total);

  for (byte i = 0; i < 4; i++) {
    byte idx = start + i;
    if (idx >= total) break;

    display.setCursor(8, 16 + i * 9);

    // Mostrar cambios
    if (millis() < mostrarCambiosHasta) {
      for (byte n = 0; n < totalNuevas; n++) {
        if (direcciones[idx] == nuevas[n]) display.print("+");
      }
    }

    display.print("0x");
    hex2(direcciones[idx]);
  }

  dibujarPie();

  display.display();
}

// ============================================================
// PIE
// ============================================================

void dibujarPie() {
  display.fillRect(1, 53, 126, 10, SH110X_BLACK);
  display.setCursor(4, 55);

  if (escaneando) {
    display.print("SCAN");
    for (byte i = 0; i < estadoAnim; i++) display.print(".");
  } else {
    display.print("STOP OK:");
    display.print(total);
  }

  display.display();
}

// ============================================================
// BORRAR
// ============================================================

void borrar() {
  total = 0;
  prevTotal = 0;
  pantallaReposo();
}

// ============================================================
// UTIL
// ============================================================

void hex2(byte v) {
  if (v < 16) display.print('0');
  display.print(v, HEX);
}

void centrar(const char* txt, int y) {
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(txt, 0, y, &x1, &y1, &w, &h);
  display.setCursor((128 - w) / 2, y);
  display.print(txt);
}

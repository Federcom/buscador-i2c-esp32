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

const unsigned long INTERVALO_ESCANEO_MS = 1800;
const unsigned long DEBOUNCE_MS = 40;
const unsigned long PULSACION_LARGA_MS = 900;
const unsigned long INTERVALO_ANIM_MS = 300;

// ============================================================
// CONTROL
// ============================================================

bool escaneando = false;
bool pantallaSucia = true;

unsigned long ultimoEscaneo = 0;
unsigned long ultimoAnim = 0;
byte estadoAnim = 0;

// Botón
bool ultimoEstadoLeidoBoton = HIGH;
bool estadoEstableBoton = HIGH;
unsigned long ultimoCambioBoton = 0;
unsigned long tiempoPulsado = 0;
bool pulsacionLargaDetectada = false;

// ============================================================
// MODOS DE PANTALLA
// ============================================================

enum ModoPantalla {
  PANTALLA_REPOSO,
  PANTALLA_RESULTADOS,
  PANTALLA_MENSAJE
};

ModoPantalla modoPantalla = PANTALLA_REPOSO;

// ============================================================
// RESULTADOS
// ============================================================

byte direccionesEncontradas[32];
byte totalEncontradas = 0;

byte tempDireccionesEncontradas[32];
byte tempTotalEncontradas = 0;

// Mensaje temporal
char mensajeLinea1[24] = "";
char mensajeLinea2[24] = "";
unsigned long mensajeHasta = 0;

// ============================================================
// SETUP
// ============================================================

void setup() {
  pinMode(PIN_BOTON, INPUT_PULLUP);
  Wire.begin(I2C_SDA, I2C_SCL);

  if (!display.begin(0, true)) {
    while (true) {
    }
  }

  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);

  modoPantalla = PANTALLA_REPOSO;
  pantallaSucia = true;
}

// ============================================================
// LOOP
// ============================================================

void loop() {
  gestionarBoton();

  if (escaneando) {
    if (millis() - ultimoEscaneo >= INTERVALO_ESCANEO_MS) {
      ultimoEscaneo = millis();

      if (escanearI2C()) {
        pantallaSucia = true;
      }
    }

    if (millis() - ultimoAnim >= INTERVALO_ANIM_MS) {
      ultimoAnim = millis();
      estadoAnim = (estadoAnim + 1) % 4;

      if (modoPantalla == PANTALLA_RESULTADOS) {
        dibujarPie();
      }
    }
  }

  if (modoPantalla == PANTALLA_MENSAJE && millis() >= mensajeHasta) {
    modoPantalla = PANTALLA_REPOSO;
    pantallaSucia = true;
  }

  if (pantallaSucia) {
    dibujarPantallaSegunModo();
    pantallaSucia = false;
  }
}

// ============================================================
// BOTÓN
// ============================================================

void gestionarBoton() {
  bool lectura = digitalRead(PIN_BOTON);

  if (lectura != ultimoEstadoLeidoBoton) {
    ultimoCambioBoton = millis();
    ultimoEstadoLeidoBoton = lectura;
  }

  if ((millis() - ultimoCambioBoton) > DEBOUNCE_MS) {
    if (lectura != estadoEstableBoton) {
      estadoEstableBoton = lectura;

      if (estadoEstableBoton == LOW) {
        tiempoPulsado = millis();
        pulsacionLargaDetectada = false;
      } else {
        if (!pulsacionLargaDetectada) {
          accionPulsacionCorta();
        }
      }
    }
  }

  if (estadoEstableBoton == LOW && !pulsacionLargaDetectada) {
    if (millis() - tiempoPulsado >= PULSACION_LARGA_MS) {
      pulsacionLargaDetectada = true;
      accionPulsacionLarga();
    }
  }
}

void accionPulsacionCorta() {
  escaneando = !escaneando;

  if (escaneando) {
    ultimoEscaneo = 0;
    modoPantalla = PANTALLA_RESULTADOS;
  } else {
    modoPantalla = PANTALLA_RESULTADOS;
  }

  pantallaSucia = true;
}

void accionPulsacionLarga() {
  escaneando = false;
  totalEncontradas = 0;
  estadoAnim = 0;

  strncpy(mensajeLinea1, "Resultados", sizeof(mensajeLinea1) - 1);
  strncpy(mensajeLinea2, "borrados", sizeof(mensajeLinea2) - 1);
  mensajeLinea1[sizeof(mensajeLinea1) - 1] = '\0';
  mensajeLinea2[sizeof(mensajeLinea2) - 1] = '\0';

  modoPantalla = PANTALLA_MENSAJE;
  mensajeHasta = millis() + 800;
  pantallaSucia = true;
}

// ============================================================
// ESCANEO I2C
// ============================================================

bool escanearI2C() {
  tempTotalEncontradas = 0;

  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      tempDireccionesEncontradas[tempTotalEncontradas++] = addr;
    }
  }

  if (tempTotalEncontradas != totalEncontradas) {
    copiarResultados();
    return true;
  }

  for (byte i = 0; i < tempTotalEncontradas; i++) {
    if (tempDireccionesEncontradas[i] != direccionesEncontradas[i]) {
      copiarResultados();
      return true;
    }
  }

  return false;
}

void copiarResultados() {
  totalEncontradas = tempTotalEncontradas;
  for (byte i = 0; i < totalEncontradas; i++) {
    direccionesEncontradas[i] = tempDireccionesEncontradas[i];
  }
}

// ============================================================
// DIBUJADO GENERAL
// ============================================================

void dibujarPantallaSegunModo() {
  display.clearDisplay();

  // Marco general siempre
  display.drawRect(0, 0, 128, 64, SH110X_WHITE);
  display.drawFastHLine(0, 12, 128, SH110X_WHITE);
  display.drawFastHLine(0, 52, 128, SH110X_WHITE);

  centrar("BUSCADOR I2C", 2);

  switch (modoPantalla) {
    case PANTALLA_REPOSO:
      dibujarPantallaReposo();
      break;

    case PANTALLA_RESULTADOS:
      dibujarPantallaResultados();
      break;

    case PANTALLA_MENSAJE:
      dibujarPantallaMensaje();
      break;
  }

  display.display();
}

void dibujarPantallaReposo() {
  display.setCursor(10, 22);
  display.println("Pulsa para escanear");
  display.setCursor(10, 34);
  display.println("Larga para borrar");
  dibujarPie();
}

void dibujarPantallaResultados() {
  dibujarLista();
  dibujarPie();
}

void dibujarPantallaMensaje() {
  centrar(mensajeLinea1, 24);
  centrar(mensajeLinea2, 36);
  dibujarPie();
}

void dibujarLista() {
  int y = 16;

  if (totalEncontradas == 0) {
    display.setCursor(8, 26);
    display.println("Sin dispositivos");
    return;
  }

  for (byte i = 0; i < totalEncontradas && i < 4; i++) {
    display.setCursor(8, y + i * 9);
    display.print("0x");
    hex2(direccionesEncontradas[i]);
  }
}

void dibujarPie() {
  display.fillRect(1, 53, 126, 10, SH110X_BLACK);
  display.setCursor(4, 55);

  if (modoPantalla == PANTALLA_REPOSO) {
    display.print("LISTO");
  } else if (modoPantalla == PANTALLA_MENSAJE) {
    display.print("INFO");
  } else if (escaneando) {
    display.print("SCAN");
    for (byte i = 0; i < estadoAnim; i++) {
      display.print(".");
    }
  } else {
    display.print("STOP OK:");
    display.print(totalEncontradas);
  }

  display.display();
}

// ============================================================
// UTILIDADES
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
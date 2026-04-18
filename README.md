# 🔍 Buscador I2C con ESP32 + OLED SPI

Herramienta sencilla para detectar direcciones I2C usando un ESP32 y mostrarlas en una pantalla OLED SH1106 por SPI.

---

## 📸 Vista del proyecto

![Montaje](fotos/montaje.jpg)

---

## ⚡ Características

- 🔄 Escaneo automático en segundo plano
- 🖥️ Pantalla OLED por SPI (sin interferencias con I2C)
- 🔍 Detección de dispositivos I2C
- 📋 Hasta 4 direcciones en pantalla
- 🔽 Scroll automático si hay más dispositivos
- ➕ Detección de cambios en vivo (+ aparece / - desaparece)
- 🧪 Salida por Serial para debug
- 🎯 Interfaz simple con 1 botón
- ✨ Animación discreta `SCAN...`
- 🚫 Sin parpadeos ni pantallas molestas
---

## 🎮 Control

| Acción | Función |
|------|--------|
| Pulsación corta | Iniciar / parar escaneo |
| Pulsación larga | Borrar resultados |

---

## 🖥️ Interfaz

### Reposo

Pulsa para escanear  
Larga para borrar  

### Escaneando

0x3C  
0x68  

SCAN...  

### Cambios detectados

+0x68  → dispositivo conectado  
-0x68  → dispositivo desconectado  

### Parado

STOP OK:n  

### Reposo

### Escaneando

### Parado

---

## 🔌 Conexiones

### OLED (SPI)

| OLED | ESP32 |
|------|------|
| GND | GND |
| VCC | 3.3V |
| SCK | GPIO 18 |
| SDA (MOSI) | GPIO 23 |
| RES | GPIO 16 |
| DC | GPIO 17 |
| CS | GPIO 5 |

---

### Botón

| Pin | Conexión |
|-----|--------|
| GPIO 4 | Botón → GND |

---

### Bus I2C de prueba

| Señal | ESP32 |
|------|------|
| SDA | GPIO 21 |
| SCL | GPIO 22 |

---
## 🧪 Debug por Serial

El sistema muestra cambios en el monitor serie:

+ 0x3C  
- 0x68  

Permite detectar conexiones y desconexiones en tiempo real.



## ⚠️ Notas

- El sistema funciona a **3.3V**
- No conectar módulos I2C de 5V directamente
- La pantalla usa SPI → no ocupa el bus I2C

---

## 🧰 Librerías

- Adafruit GFX
- Adafruit SH110X

---

## 🚀 Uso

1. Conecta un módulo I2C
2. Pulsa el botón
3. Lee la dirección en pantalla
4. Mantén pulsado para limpiar

---

## 🔧 Futuras mejoras

- Soporte 5V con conversor
- Selector 3.3V / 5V
- Caja impresa en 3D


---

## 📜 Licencia

MIT License



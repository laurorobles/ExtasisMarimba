# 📋 FICHA TÉCNICA — EXTASIS MARIMBA (v1.0)

---

### 1. INFORMACIÓN GENERAL

| Campo | Especificación |
| :--- | :--- |
| **Nombre del Producto** | Extasis Marimba |
| **Versión** | 1.0.0 (Release) |
| **Desarrollador** | Extasis Records / Lauro Robles |
| **Tipo de Instrumento** | Sintetizador de Modelado Físico / Resonador Modal |
| **Inspiración / Motor** | Arturia MicroFreak Modal & Physical Modeling Engine (`MARIMBITA`) |
| **Lenguaje de Programación** | C++17 / C++20 |
| **Framework de Audio** | JUCE Framework 7.x |

---

### 2. ESPECIFICACIONES DE AUDIO & DSP

| Parámetro | Valor |
| :--- | :--- |
| **Polifonía** | 16 voces con algoritmo inteligente de robo de voz |
| **Frecuencias de Muestreo Soportadas** | 44.1 kHz, 48 kHz, 88.2 kHz, 96 kHz, 176.4 kHz, 192 kHz |
| **Procesamiento Interno** | Punto flotante de 32 bits / 64 bits con prevención de subnormales (*ScopedNoDenormals*) |
| **Latencia Interna** | 0 muestras (Procesamiento directo en tiempo real sin latencia introducida) |
| **Bancos de Modos Resonantes** | 5 resonadores IIR por voz ($f_0$, $4.0 f_0$, $9.2 f_0$, $16.0 f_0$, $f_0 \text{ Pipe}$) |
| **Topología del Filtro** | Filtro de estado variable (*Trapezoidal SVF*) de 12 dB/octava con saturación no lineal |
| **Respuesta Dinámica** | Escalamiento continuo de velocidad MIDI a macillo, ataque, brillo y amplitud |

---

### 3. COMPATIBILIDAD DE PLATAFORMAS & FORMATOS

#### macOS:
* **Versiones de SO:** macOS 10.13 High Sierra hasta macOS 14 Sonoma y posteriores.
* **Arquitectura:** Binario Universal (Nativo en **Apple Silicon M1/M2/M3/M4** e **Intel Core x64**).
* **Formatos:**
  * **VST3** (64-bit)
  * **Audio Unit (AU / Component)** (64-bit)
  * **Aplicación Standalone** (Ejecutable autónomo sin DAW)

#### Windows:
* **Versiones de SO:** Windows 10 / Windows 11 (64-bit).
* **Formatos:** VST3 y Standalone (x64).

#### DAWs Verificados:
* Ableton Live 10 / 11 / 12
* FL Studio 20 / 21 / 24
* Apple Logic Pro X
* Cockos Reaper 6 / 7
* Avid Pro Tools (vía puente AU/VST3)
* Steinberg Cubase / Nuendo
* Bitwig Studio
* PreSonus Studio One 5 / 6

---

### 4. REQUERIMIENTOS DEL SISTEMA

* **Procesador:** Intel Core i5 / AMD Ryzen 5 / Apple Silicon M1 o superior.
* **Memoria RAM:** Mínimo 2 GB (Recomendado 4 GB o más).
* **Espacio en Disco:** Menos de 50 MB de almacenamiento para los plugins instalados.
* **Controlador MIDI:** Compatible con cualquier teclado o pad MIDI estándar con sensibilidad a la velocidad.

# 🪵 MANUAL DE USUARIO OFICIAL — EXTASIS MARIMBA (v1.0)
### *Sintetizador de Modelado Físico y Resonador Modal*

---

## 📑 ÍNDICE GENERAL

1. [INTRODUCCIÓN Y FILOSOFÍA](#1-introducción-y-filosofía)
2. [ARQUITECTURA DE SÍNTESIS Y MODELADO MODAL](#2-arquitectura-de-síntesis-y-modelado-modal)
3. [DESCRIPCIÓN DETALLADA DE CONTROLES](#3-descripción-detallada-de-controles)
   - [3.1 Sección 1: Mallet & Strike (Excitador Percusivo)](#31-sección-1-mallet--strike)
   - [3.2 Sección 2: Modal Resonator (Resonador de Barra y Tubo)](#32-sección-2-modal-resonator)
   - [3.3 Sección 3: SEM Filter (Filtro Analógico de Estado Variable)](#33-sección-3-sem-filter)
   - [3.4 Sección 4: Master & FX (Envolventes, Saturación y Espacio)](#34-sección-4-master--fx)
4. [PANTALLA INTERACTIVA LCD / OLED](#4-pantalla-interactiva-lcd--oled)
5. [GUÍA DE PRESETS DE FÁBRICA](#5-guía-de-presets-de-fábrica)
6. [MAPEO MIDI Y AUTOMATIZACIÓN EN DAWS](#6-mapeo-midi-y-automatización-en-daws)
7. [SISTEMA DE LICENCIAMIENTO OFFLINE](#7-sistema-de-licenciamiento-offline)
8. [SOLUCIÓN DE PROBLEMAS Y FAQ](#8-solución-de-problemas-y-faq)

---

## 1. INTRODUCCIÓN Y FILOSOFÍA

**Extasis Marimba** es un sintetizador virtual de modelado físico inspirado directamente en el legendario preset `MARIMBITA` y el motor de síntesis modal del **Arturia MicroFreak**.

A diferencia de los samplers tradicionales basados en grabaciones estáticas, Extasis Marimba genera cada sonido mediante **ecuaciones diferenciales de impacto y resonancia en tiempo real**. Esto permite una expresividad orgánica inigualable: cada nota responde de forma continua a la fuerza del golpe, variando la dureza del macillo, el brillo transitorio y la dispersión armónica a lo largo del registro.

---

## 2. ARQUITECTURA DE SÍNTESIS Y MODELADO MODAL

El instrumento está estructurado en 5 etapas secuenciales de procesamiento:

```
[ MIDI Velocity / Note ]
          │
          ▼
┌────────────────────────────────────────┐
│  1. MALLET EXCITER (Excitador)        │
│  - Impulso de impacto (Raised Cosine)  │
│  - Generador de chasquido transitorio  │
└──────────────────┬─────────────────────┘
                   │  (Impulso de audio)
                   ▼
┌────────────────────────────────────────┐
│  2. MODAL RESONATOR BANK               │
│  - Modo 0: Fundamental f0              │
│  - Modo 1: Doble Octava (4.0 x f0)     │
│  - Modo 2: Modo Transverso (9.2 x f0)  │
│  - Modo 3: Modo Superior (16.0 x f0)   │
│  - Tubo Resonador Acústico (f0 cavity) │
└──────────────────┬─────────────────────┘
                   │
                   ▼
┌────────────────────────────────────────┐
│  3. SEM 12dB STATE-VARIABLE FILTER     │
│  - Saturación no lineal analógica      │
│  - Dynamic Cutoff & Velocity Tracker   │
└──────────────────┬─────────────────────┘
                   │
                   ▼
┌────────────────────────────────────────┐
│  4. PERCUSSIVE ENVELOPE (EG2 / Snap)   │
│  - Curva de decaimiento exponencial    │
└──────────────────┬─────────────────────┘
                   │
                   ▼
┌────────────────────────────────────────┐
│  5. STEREO BODY PROCESSOR & MASTER     │
│  - Paneo acústico por nota (Spread)    │
│  - Warm Drive & Body Ambience          │
└────────────────────────────────────────┘
```

### Ecuación de los Modos Resonantes:
Cada modo resonador utiliza un filtro paso-banda IIR de segundo orden normalizado:
$$y[n] = 2 r \cos(\omega_k) y[n-1] - r^2 y[n-2] + g x[n]$$
Donde:
* $\omega_k = \frac{2\pi f_k}{f_s}$ es la frecuencia angular del modo $k$.
* $r = e^{-\frac{\pi B_k}{f_s}}$ es el radio de decaimiento modal según el tiempo $T_{60}$.
* La amortiguación $B_k$ de la madera se incrementa con la frecuencia para recrear el apagado natural de los armónicos agudos.

---

## 3. DESCRIPCIÓN DETALLADA DE CONTROLES

### 3.1 Sección 1: Mallet & Strike

* **HARDNESS (0% – 100%):** Controla la dureza del macillo. Valores bajos emulan macillos de fieltro suave (sonido redondo y oscuro); valores altos emulan macillos de goma dura o madera (ataque brillante y percusivo).
* **CLICK (0% – 100%):** Añade un transitorio de alta frecuencia generado en los primeros 2 milisegundos del golpe para cortar en mezclas densas.
* **ATTACK (0.1ms – 100ms):** Modifica la rampa inicial del impacto. Mantener por debajo de 2ms para instrumentos percusivos o elevar para sonidos tipo "pad frotado" (*bowed bar*).

---

### 3.2 Sección 2: Modal Resonator

* **BAR DECAY (0% – 100%):** Tiempo de resonancia de las láminas de madera (desde 100 milisegundos hasta más de 4 segundos).
* **MATERIAL (0% – 100%):**
  * `0% – 30%:` Madera de palisandro de Honduras (Honduras Rosewood).
  * `30% – 70%:` Madera africana de Balafón / Kalimba.
  * `70% – 100%:` Láminas de cristal / Vibráfono / Metal.
* **OVERTONES (0% – 100%):** Balance de mezcla entre la fundamental ($f_0$) y los sobretonos afinados ($4 f_0, 9.2 f_0, 16 f_0$).
* **PIPE BODY (0% – 100%):** Nivel de resonancia del tubo acústico inferior, aportando cuerpo, graves y presencia natural.

---

### 3.3 Sección 3: SEM Filter

* **CUTOFF (20 Hz – 20,000 Hz):** Frecuencia de corte del filtro analógico modelado de 12dB/octava.
* **RESO (0% – 95%):** Resonancia del filtro con saturación suave para evitar auto-oscilaciones estridentes.
* **ENV AMT (0% – 100%):** Cantidad de apertura de filtro controlada dinámicamente por la envolvente del golpe de macillo.

---

### 3.4 Sección 4: Master & FX

* **ENV DECAY (5ms – 3000ms):** Decaimiento de la envolvente de amplitud principal.
* **RELEASE (5ms – 2000ms):** Tiempo de liberación tras soltar la tecla.
* **SPREAD (0% – 100%):** Espacialización estéreo acústica inteligente: las notas graves se sitúan a la izquierda y las agudas a la derecha emulando la colocación física de una marimba real.
* **DRIVE (0% – 100%):** Saturación cálida de transformador/cinta analógica.
* **AMBIENCE (0% – 100%):** Emulación de reflexiones tempranas y aire acústico de la caja de resonancia.
* **VOLUME (0.0 – 1.5):** Control de ganancia maestra del plugin.

---

## 4. PANTALLA INTERACTIVA LCD / OLED

La pantalla superior ofrece retroalimentación visual en tiempo real:
1. **Espectro de Modos Armónicos:** Muestra en barras animadas el nivel de energía de $f_0$, $4 f_0$, $9 f_0$, $16 f_0$ y el tubo `PIPE`.
2. **Osciloscopio Transitorio:** Dibuja la curva de excitación y el impacto del macillo en milisegundos.
3. **Lectura de Parámetros:** Indica el nombre exacto y valor numérico de cualquier perilla en movimiento.
4. **Navegación de Presets:** Botones `<` y `>` y menú desplegable para cambio instantáneo de programas.

---

## 5. GUÍA DE PRESETS DE FÁBRICA

| Preset | Descripción y Uso Recomendado |
| :--- | :--- |
| **01: MicroFreak Marimbita** | Recreación idéntica del preset original. Seco, definido y percusivo con toque analógico. |
| **02: Concert Wooden Marimba** | Sonido profundo y orquestal con macillo suave y cuerpo de tubo resonador al máximo. |
| **03: African Balafon / Kalimba** | Tono rústico y crujiente con armónicos de madera tradicionales y transitorio marcado. |
| **04: Glass & Metal Bar** | Láminas brillantes con sobretonos inarmónicos tipo vibráfono y glockenspiel. |
| **05: Plucked Mallet Synth** | Envolvente sintética con resonancia filtrada y drive para líneas de bajo y leads melódicos. |

---

## 6. MAPEO MIDI Y AUTOMATIZACIÓN EN DAWS

* **Recepción MIDI:** Canales 1 al 16 con polifonía de hasta 16 voces dinámicas.
* **Respuesta a Velocidad:** Modula simultáneamente la ganancia, la dureza del macillo y el brillo del filtro.
* **Compatibilidad VST3 / AU:** Todos los parámetros disponen de nombres normalizados para automatización instantánea en Ableton Live, FL Studio, Logic Pro X, Cubase, Studio One, Reaper y Bitwig.

---

## 7. SISTEMA DE LICENCIAMIENTO OFFLINE

Extasis Marimba no requiere conexión a Internet ni software de terceros (iLok). Utiliza un sistema criptográfico de claves seriales con formato:

$$\text{EXTM-XXXX-XXXX-XXXX-XXXX}$$

Las licencias se almacenan automáticamente en:
* **macOS:** `~/Library/Application Support/ExtasisMarimba/license.key`
* **Windows:** `%APPDATA%\ExtasisMarimba\license.key`

---

## 8. SOLUCIÓN DE PROBLEMAS Y FAQ

* **¿El plugin no aparece en mi DAW?**
  * Realiza un "Rescan Plugins" en las preferencias de tu DAW.
  * Verifica que `ExtasisMarimba.vst3` esté en `~/Library/Audio/Plug-Ins/VST3/` o `ExtasisMarimba.component` en `~/Library/Audio/Plug-Ins/Components/`.
* **¿Cómo obtengo un sonido de marimba más cálido y oscuro?**
  * Reduce `HARDNESS` a 30%, baja `OVERTONES` a 40% y sube `PIPE BODY` a 80%.
* **¿Cómo consigo que la marimba corte mejor en una mezcla de reggaetón / techno?**
  * Sube `CLICK` a 60%, `DRIVE` a 25% y ajusta `CUTOFF` alrededor de 12 kHz.

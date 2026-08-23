# 🪵 EXTASIS MARIMBA — Official User Manual
**Mexican Physical-FM Synthesizer (VST3 / AU / Standalone)**  
*Developed & Designed by Lauro Robles / Extasis Records*

---

## 1. Introduction & Acoustic Philosophy

Traditional marimbas—especially the iconic instruments of Chiapas, Oaxaca, and Guatemala—possess a sonic identity that cannot be captured by static sampling. When a rubber mallet strikes a tuned wooden bar of *Hormiguillo*, a complex chain of acoustic events unfolds in milliseconds:

$$\text{Impact} \longrightarrow \text{Inharmonic Bar Resonance} \longrightarrow \text{Acoustic Air Tube} \longrightarrow \text{Vibrating Buzz Membrane (La Cachimba)} \longrightarrow \text{Air}$$

**Extasis Marimba** synthesizes this entire physical phenomenon in real time using a 24-bit/96kHz hybrid **Modal Physical Modeling & FM Synthesis Engine**.

---

## 2. Interface Overview & Signal Flow

```
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │ TOP HEADER: EXTASIS MARIMBA // coded by @laurorobles // [ LICENSED ] / [DEMO]│
 ├────────────────────────────────────────┬────────────────────────────────────┤
 │                                        │  [ PRESET SELECTOR v ] [<] [>] [SAVE]│
 │  OLED MULTI-MODE REALTIME DISPLAY      ├────────────────────────────────────┤
 │  • Real-time Audio Oscilloscope        │                                    │
 │  • 5 Tuned Modal Resonator Bars        │   [ EXTASIS TRIGGER PAD ]          │
 │  • Parameter & Status Readouts         │   • Click to Strike (C4)           │
 │                                        │   • Drag Vertically: +/-24 Semitones│
 ├───────────────────┬────────────────────┼───────────────────┬────────────────┤
 │ 1. MALLET & ATTACK│ 2. WOOD & MODAL FM │ 3. RESONATOR/BUZZ │ 4. MASTER & FX │
 │ • HARDNESS        │ • BAR DECAY        │ • TUBE CAVITY     │ • SPREAD       │
 │ • RUBBER NOISE    │ • MATERIAL         │ • CACHIMBA BUZZ   │ • WARM DRIVE   │
 │ • CLICK / SNAP    │ • OVERTONES        │ • BUZZ DYNAMICS   │ • AMBIENCE     │
 │ • ATTACK          │ • PUEBLO DRIFT     │ • SEM FILTER      │ • MASTER VOL   │
 └───────────────────┴────────────────────┴───────────────────┴────────────────┘
```

---

## 3. Parameter Reference

### Section 1: Mallet & Attack
* **`HARDNESS` (0–100%):** Mallet composition. Low values simulate soft rubber/felt mallets with warm fundamentals; high values simulate hard rubber/wood mallets with strong upper transients.
* **`RUBBER NOISE` (0–100%):** Transitory noise pulse filtered through a dedicated 2.5 kHz – 5.5 kHz bandpass filter (8–24 ms decay). Emulates the tactile "thud/tak" of raw Mexican rubber striking wood.
* **`CLICK / SNAP` (0–100%):** High-frequency mechanical transient snap providing crisp stick definition.
* **`ATTACK` (0.1–50 ms):** Enunciation time of the acoustic strike.

### Section 2: Wood & Modal FM
* **`BAR DECAY` (0–100%):** Natural exponential ringing time of the modal resonator (0.15s to 4.5s).
* **`MATERIAL` (0–100%):** Seamless continuous physical bar material interpolation:
  * `0–25%`: Traditional Mexican *Hormiguillo* / Rosewood Bar (Modal ratios: $1.0, 3.15, 9.2, 16.0$)
  * `26–70%`: African *Balafon* / Kalimba
  * `71–100%`: Crystal Glass / Vibraphone Bar (Modal ratios: $1.0, 2.76, 5.4, 8.93$)
* **`OVERTONES` (0–100%):** Amplitude balance of the higher inharmonic partials relative to the fundamental.
* **`PUEBLO DRIFT` (0–100%):** The "Marimba de Pueblo" engine. Imparts deterministic, artisanal micro-imperfections across every key:
  * Micro-pitch detuning ($\pm 6\text{ cents}$)
  * Bar decay variation ($\pm 18\%$)
  * Membrane tension drift ($\pm 25\%$)

### Section 3: Resonator & Buzz (La Cachimba)
* **`TUBE CAVITY` (0–100%):** The acoustic resonator pipe under the bar. Features micro-detuned air cavity beating ($\pm 3.5\text{--}6\text{ Hz}$) that produces the organic breathing "DOOO~WONNNG" resonance.
* **`CACHIMBA BUZZ` (0–100%):** Non-linear physical model of the pig intestine membrane (*cachimba*) mounted with bee's wax at the bottom of the resonator tube ($2f_0$ carrier modulated by $7.2f_0$).
* **`BUZZ DYNAMICS` (0–100%):** Velocity scaling of the buzz. When turned up, soft hits sound like pure warm wood, while hard hits explode with the signature *«TAK–ÑAAANG–BRR»* rattle.
* **`SEM FILTER` (100 Hz – 20 kHz):** 12dB/octave State-Variable Low-Pass filter with soft analog saturation and velocity tracking.

### Section 4: Master & FX
* **`SPREAD` (0–100%):** Acoustic keyboard spatialization (low keys panned left, high keys panned right, plus mid/side widening).
* **`WARM DRIVE` (0–100%):** Analog tube/tape saturation for body harmonics and punch.
* **`AMBIENCE` (0–100%):** Early reflection resonator chamber imitating wooden rehearsal rooms and village courtyards.
* **`MASTER VOL` (0–150%):** Clean master output ceiling gain.

---

## 4. Factory Preset Guide (30 Presets)

### Traditional Mexican & Chiapaneca (01 – 08)
1. **`01. Chiapas Cachimba Marimba`** — The authentic sound of Chiapas: balanced buzz membrane, rubber mallet, and deep resonator tubes.
2. **`02. Hormiguillo Fiesta Marimba`** — Bright, percussive, festive zapateado tuning with snappy transient bite.
3. **`03. Marimba de Pueblo (Vintage)`** — Maximum artisanal imperfection, aged wood bars, detuned tubes.
4. **`04. Zapateado Veracruzano`** — Fast attack, tight decay, perfect for rapid staccato leads.
5. **`05. Son Jarocho Wooden Bar`** — Warm, woody, earthy organic tone.
6. **`06. Tuxtla Gutierrez Twilight`** — Mellow rubber mallets with deep cavity air.
7. **`07. Oaxacan Village Mallet`** — Rich overtones and expressive buzz dynamics.
8. **`08. Chiapaneca Raw Membrane`** — Aggressive, buzzing membrane rattle for maximum acoustic presence.

### Latin Club, Cumbia & Bass (09 – 16)
9. **`09. Deep Latin Club Pluck`** — Subby fundamental with punchy rubber transient for modern club tracks.
10. **`10. Cumbia Rebajada Low End`** — Slow, heavy, slowed-down cumbia bassline punch.
11. **`11. Hypnotic Cumbia Mallet`** — Rhythmic hook synthesizer with warm tape saturation.
12. **`12. Latin House Strike`** — Piercing percussion mallet for 4/4 tech-house grooves.
13. **`13. Tribal Guarachero Mallet`** — High-energy, snappy wooden lead.
14. **`14. Afro-Cuban Bembé Bar`** — Complex rhythmic resonance tuned for Afro-Latin polyrhythms.
15. **`15. Reggaeton Mallet Lead`** — Melodic Latin pop pluck with wide stereo field.
16. **`16. Baile Funk Punch Bar`** — Heavy transient click and aggressive midrange drive.

### Concert & World Mallets (17 – 22)
17. **`17. Clean Concert Rosewood`** — Pristine orchestra marimba, soft felt mallets, zero buzz.
18. **`18. Soft Felt Warmth`** — Gentle, dark, intimate acoustic warmth.
19. **`19. Orchestra Symphony Marimba`** — Full dynamic range for classical and cinematic arrangements.
20. **`20. Wooden Xylophone Snap`** — Ultra-short, dry, woody percussive snap.
21. **`21. African Balafon / Kalimba`** — High inharmonicity with buzzy gourd resonance.
22. **`22. Indonesian Gamelan Gourd`** — Metallic wood hybrid with microtonal shimmer.

### Experimental & Hybrid FM (23 – 30)
23. **`23. Glass & Vibra Bars`** — Pure crystalline high modes and long glassy sustain.
24. **`24. Ambient Dream Chimes`** — Atmospheric, cavernous space with shimmering harmonics.
25. **`25. Cyberpunk Neon Mallet`** — Futuristic distorted physical pluck.
26. **`26. 12-Bit Lo-Fi Marimbita`** — Vintage sampler grit and warm downsampled nostalgia.
27. **`27. Sub-Bass Thumper Marimba`** — Fundamental sub resonator for 808-style low-end weight.
28. **`28. Micro-Tuned Village Market`** — Authentic non-western micro-intonation.
29. **`29. Space Echo Chamber Bar`** — Long acoustic early reflections and stereo diffusion.
30. **`30. Extasis Anthem Marimba`** — The flagship sound: huge, dynamic, buzzy, and cutting.

---

## 5. Preset Management & User Patches
* **Saving:** Click the `SAVE` button in the top-right preset panel, enter a custom name, and press Return.
* **Storage Location:** User presets are saved as XML `.empreset` files in:
  * **macOS:** `~/Documents/ExtasisRecords/ExtasisMarimba/Presets/`
  * **Windows:** `%USERPROFILE%\Documents\ExtasisRecords\ExtasisMarimba\Presets\`
* **Loading:** All user presets are automatically enumerated in the dropdown under `[User] <Preset Name>`.

---

## 6. Licensing & Registration
Extasis Marimba includes a **10-minute full-featured evaluation period**. To unlock permanently:
1. Click the **`[ DEMO ]`** badge in the upper right header.
2. Enter your serial key (format: `EXTM-XXXX-XXXX-XXXX-XXXX`).
3. Click **ACTIVATE LICENSE**. The badge will change to **`[ LICENSED ]`** and the key is securely saved offline to your local application support directory.

---
*© Extasis Records — Built with JUCE & Advanced Physical-FM DSP.*

# 📋 REGLAS DE LANZAMIENTO (RELEASE RULES) — EXTASIS RECORDS

Este documento define las reglas estrictas de control de versiones, documentación y compilación que se deben cumplir antes de realizar cualquier lanzamiento oficial.

## 1. Versiones y Metadatos
* **Estructura de Versión:** Se debe utilizar SemVer estricto (`MAJOR.MINOR.PATCH`).
* **CMakeLists.txt:** La versión declarada en `project(... VERSION X.Y.Z)` debe coincidir exactamente con la del tag de Git.
* **Documentación:** La ficha técnica (`TECHNICAL.md`) y el manual (`MANUAL.md`) deben actualizarse mencionando la versión actual.

## 2. Documentos Requeridos
Cada instrumento debe incluir obligatoriamente los siguientes archivos Markdown actualizados en su raíz:
1. `README.md`: Resumen ejecutivo y enlaces de descarga.
2. `MANUAL.md`: Guía de uso completa y mapa MIDI CC.
3. `PRESENTATION.md`: Descripción comercial y enlace de compra a Gumroad.
4. `TECHNICAL.md`: Especificaciones técnicas del motor DSP y requisitos del sistema.
5. `ARCHITECTURE.md`: Detalle y diagrama del flujo de señal (Mermaid).

## 3. Flujo de Git y GitHub Actions
* **Creación de Release:** Todo release formal debe iniciarse subiendo un tag con el formato `vX.Y.Z` (ej. `v3.0.0`).
* **Compilación Automática:** Las GitHub Actions detectarán el tag, compilarán en Windows, macOS y Linux de forma simultánea, empaquetarán los instaladores y la documentación, y crearán un borrador de Release en GitHub con los archivos adjuntos listos para subir a Gumroad.

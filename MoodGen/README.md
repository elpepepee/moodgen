# MoodGen (VST3)

Plugin que genera MIDI a partir de un género y un mood. Pensado para FL Studio.

## Qué hace

- Género: R&B, Reguetón, House, Pop, Jazz soul.
- Mood simple: un pad X/Y (oscuro a brillante, calmado a intenso).
- Mood avanzado: escribes con tus palabras ("sensual, nocturno, muy lento, jazzy, 72 bpm") y el plugin lo traduce a modo, densidad, swing, riqueza de acordes y BPM. Entiende español e inglés. Enter genera.
- Controles: tono, modo (Auto o fijo), BPM, compases (2, 4, 8), densidad, variación.
- Cuatro pistas MIDI: acordes, bajo, melodía y batería. Cada una se puede bloquear con el candado para regenerar solo el resto.
- Escuchar: preview con sintetizador interno.
- Arrastra cada pista (o "Arrastrar todo") directo al Piano Roll o a la playlist de FL Studio.

## Compilar

Necesitas CMake 3.22+ y un compilador de C++17 (Visual Studio 2022 en Windows, Xcode en macOS). JUCE se descarga solo.

```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target MoodGen_VST3
```

El resultado queda en `build/MoodGen_artefacts/Release/VST3/MoodGen.vst3`.

- Windows: copia `MoodGen.vst3` a `C:\Program Files\Common Files\VST3\`.
- macOS: cópialo a `~/Library/Audio/Plug-Ins/VST3/`.

Si no quieres instalar nada, sube esta carpeta a un repositorio de GitHub: el workflow de `.github/workflows/build.yml` compila Windows y macOS y deja el `.vst3` como artefacto descargable.

## Instalador de Windows (.exe)

El mismo workflow genera `MoodGen-Setup.exe` (Inno Setup, script en `installer/MoodGen.iss`).

1. Sube la carpeta a GitHub (repositorio nuevo).
2. Entra a la pestaña Actions, abre la ejecucion mas reciente y espera a que termine (unos 10 a 15 minutos).
3. En Artifacts descarga `MoodGen-Setup-Windows` (es un zip con el `.exe` adentro).
4. Ejecuta `MoodGen-Setup.exe` como administrador. Instala en `C:\Program Files\Common Files\VST3\MoodGen.vst3`, la carpeta estandar que FL Studio ya escanea. Se desinstala desde Configuracion de Windows > Aplicaciones.

El ejecutable no esta firmado, asi que Windows SmartScreen puede avisar. Elige "Mas informacion" y "Ejecutar de todas formas". El plugin usa el runtime de C++ incluido, no hace falta instalar nada mas.

## Uso en FL Studio

1. Options > Manage plugins > Find installed plugins.
2. Agrega MoodGen a un canal.
3. Genera, dale a Escuchar si quieres oírlo, y arrastra las pistas al Piano Roll de otro instrumento.

También puede mandar MIDI en vivo: en el wrapper del plugin elige un puerto de salida MIDI y enlaza otro instrumento a ese puerto.

## Estructura

- `Source/MoodEngine.*`, `MoodMelodyDrums.cpp`: generación de acordes, bajo, melodía y batería.
- `Source/MoodParser.cpp`: interpreta el texto del mood.
- `Source/PluginProcessor.*`: parámetros, salida MIDI sincronizada con el host, exportación de .mid, sintetizador de preview.
- `Source/PluginEditor.*`: interfaz.

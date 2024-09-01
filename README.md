# Tabla de Contenidos

- [Tabla de Contenidos](#tabla-de-contenidos)
- [📢 Descripción del proyecto](#-descripción-del-proyecto)
  - [⛓️ Dependencias](#️-dependencias)
    - [Bibliotecas](#bibliotecas)
    - [Entorno anfitrión](#entorno-anfitrión)
    - [Herramientas de construcción](#herramientas-de-construcción)
  - [🛠️ Construcción](#️-construcción)
    - [Recolección de fuentes](#recolección-de-fuentes)
    - [Configuración](#configuración)
    - [Compilación y enlace](#compilación-y-enlace)
  - [⏯️ Ejecución](#️-ejecución)
    - [Dependencias](#dependencias)
    - [Distribución](#distribución)
- [📜 Créditos](#-créditos)

# 📢 Descripción del proyecto

Este proyecto, motivado por el curso de Creación de Videojuegos (CI-0162) impartido durante el II-2024 en la Universidad de Costa Rica, consiste en la elaboración de un videojuego de computador 2D. 

¡Se encuentra en contínuo desarrollo! 🚧

## ⛓️ Dependencias

### Bibliotecas

El proyecto referencia varias bibliotecas en las que depende mediante submódulos de `Git`. Los repositorios donde éstas residen se encuentran en `GitHub` (con su versión respectiva). 

Para construir este proyecto, debe obtener una copia aquellos repositorios asociados a las bibliotecas. `Git` suele manejar esto al clonar el repositorio. Adicionalmente, debe ser capaz de construirlas desde el código fuente de éstos.

De momento, las bibliotecas externas utilizadas son:
- `SDL2` (versión `2.0`): API de multimedia
- `GLM` (versión `1.0.2`): Aritmética matricial y vectorial

### Entorno anfitrión

Este proyecto se ha logrado construir en el siguiente entorno para ser ejecutado en él mismo:

- **Sistema operativo**: `Debian GNU/Linux` versión 12 (**bookworm**)
- **Kernel**: `5.15.153.1-microsoft-standard-WSL2` (Debian bajo WSL2)

Es posible construir el proyecto desde otros sistemas anfitriones e incluso para plataformas distintas, pero no se garantiza. `CMake` es configurable para utilizar otras herramientas y dependencias de éstas según la plataforma.

### Herramientas de construcción

Este proyecto utiliza el sistema de construcción de `CMake`. Es posible que versiones anteriores funcionen, pero no se garantiza compatibilidad.

Las siguientes son algunas de las herramientas utilizadas para construir el ejecutable del proyecto:
- **Sistema de construcción**: `CMake` versión `3.25.1`
- **Compiladores**: `GCC/G++` versión `12.2.0`, `nasm` versión `2.16.01`
- **Generador usado por CMake**: `GNU Make` versión `4.3`, `Ninja` versión `1.11.1`

Adicionalmente, este proyecto utiliza submódulos de `git` (versión `2.39.2`) para recolectar fuentes mediante versión de control. 

Se omiten las herramientas necesarias para construir las bibliotecas incluidas como dependencias.

## 🛠️ Construcción

Las siguientes instrucciones detallan cómo construir el proyecto desde la plataforma anfitrión para ella misma. Es posible construir para plataformas distintas a la anfitrión, pero no se garantiza.

### Recolección de fuentes

Asegúrese de clonar el repositorio de donde obtuvo este fuente y sus submódulos recursivamente. Se recomiendan los siguientes comandos:

1. Si recién ha clonado el repositorio. La `<direccion_de_clonado>` es cualquier dirección que acepte git clone para clonar el repositorio remoto asociado (e.g: una URL):
```
git clone --recursive <direccion_de_clonado>
```

2. Si ya lo clonó, pero desea clonar sus submódulos recursivamente, navegue al directorio raíz del repositorio y ejecute:
```
git submodule update --init --recursive
```

### Configuración

Asuma que las siguientes instrucciones consideran una terminal de su elección, tomando el directorio raíz del proyecto como el directorio de trabajo. Se asume la notación de `shell` de sistemas Linux. 

Se sugiere el siguiente comando para configurar la construcción del ejecutable del proyecto, y sus dependencias, mediante `CMake`:
```
cmake [-D<opcion>:<tipo>=<valor> \]... -S./ -B./build
```

Las opciones (a especificar mediante la bandera -D) que este proyecto aporta mediante `CMake` son:
|Opción|Tipo|Valor(es)|Significado|
|---|---|---|---|
|`SDL2_LOCAL`|`BOOL`|`TRUE`/`FALSE`|Construir `SDL2` en vez de utilizar la instalación del sistema|
|`SDL2_STATIC`|`BOOL`|`TRUE`/`FALSE`|Utilizar la versión estática de `SDL2` en vez de la dinámica|
|`GLM_LOCAL`|`BOOL`|`TRUE`/`FALSE`|Construir `GLM` en vez de utilizar la instalación del sistema|
|`GLM_HEADER_ONLY`|`BOOL`|`TRUE`/`FALSE`|Utilizar la versión de encabezados de `GLM` en vez de la compilada|
|`GLM_SHARED`|`BOOL`|`TRUE`/`FALSE`|Utilizar la versión dinámica de `GLM` en vez de la dinámica|
|`SDL2_IMAGE_LOCAL`|`BOOL`|`TRUE`/`FALSE`|Construir `SDL_Image` en vez de utilizar la instalación del sistema|
|`SDL2_IMAGE_SHARED`|`BOOL`|`TRUE`/`FALSE`|Utilizar la versión dinámica de `SDL_Image` en vez de la dinámica|
|`SDL2_TTF_LOCAL`|`BOOL`|`TRUE`/`FALSE`|Construir `SDL_TTF` en vez de utilizar la instalación del sistema|
|`SDL2_TTF_SHARED`|`BOOL`|`TRUE`/`FALSE`|Utilizar la versión dinámica de `SDL_TTF` en vez de la dinámica|
|`CMAKE_EXPORT_COMPILE_COMMANDS`|`BOOL`|`TRUE`/`FALSE`|Generar un archivo `json` con los comandos utilizados por el generador|
|`CMAKE_BUILD_TYPE`|`STRING`|`Debug`/`Release`|Construir una versión para depuración (`Debug`) u optimizada (`Release`)|

Varias opciones ya están configuradas por defecto, pero pueden ser sobre-escritas por valores proporcionados mediante el comando anterior. Adicionalmente, puede revisar todas las opciones del proyecto y dependencias mediante varias herramientas, incluyendo `CMake GUI`.

Por ejemplo, para configurar la construcción del ejecutable del proyecto tal que:
- Se construyan las bibliotecas SDL2 y GLM
- Se utilizen las versiones estáticas de éstas
- Se utilize la versión compilada de GLM
- Se construya una versión de depuración del proyecto
- Se construya un archivo json con los comandos utilizados

Se podría utilizar el siguiente comando invocado en la terminal:
```
cmake \
    -DSDL2_VENDORED:BOOL=TRUE \
    -DGLM_VENDORED:BOOL=TRUE \
    -DSDL2_STATIC:BOOL=TRUE \
    -DGLM_SHARED:BOOL=FALSE \
    -DGLM_HEADER_ONLY:BOOL=FALSE \
    -DCMAKE_BUILD_TYPE:STRING=Debug \
    -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE \
    --no-warn-unused-cli \
    -B ./build \
    -S ./
```

### Compilación y enlace

Tras haber configurado la construcción del proyecto, la compilación y enlace puede llevarse a cabo mediante las herramientas y generador invocados por `CMake`.

Para compilar y enlazar el ejecutable del proyecto, o limpiar su construccion se recomienda el siguiente comando
```
cmake --build ./build --config <configuracion> --target <objetivo>
```

Las configuraciones a proveer se mencionan a continuación:
|Configuración|Significado|
|---|---|
|`Debug`|Optar por facilidades y símbolos de depuración|
|`Release`|Optar por velocidad y desempeño de aplicación|

Los objetivos de construccion también se detallan a continuación:
|Objetivo|Significado|
|---|---|
|`game`|Construir el ejecutable del proyecto|
|`clean`|Limpiar la construcción del ejecutable|
|`clean-first`|Limpiar la construcción del ejecutable y reconstruirlo|

Por ejemplo, para limpiar el proyecto se admite la invocación:
```
cmake --build ./build --config Debug --target clean
```

En otro ejemplo, para construir un ejecutable con configuración de depuración se admite la invocación:
```
cmake --build ./build --config Debug --target game
```

## ⏯️ Ejecución

### Dependencias

El ejecutable construido debería estar ubicado bajo la carpeta `game/`, hija de la carpeta raíz del proyecto, con la extensión `.exe`.

De haber configurado el ejecutable con alguna librería con enlazamiento dinámico, su correspondiente implementación debe ser accesible en el sistema que corre el ejecutable además de ser compatible.
- En plataformas `Windows`, es común que se encuentren distribuidas éstas implementaciones como archivos de extensión `.DLL`
- En plataformas `Linux` ocurre algo parecido, pero con la extensión `.so`

De incluir archivos de biblioteca, éstos deben ubicarse en la misma carpeta que el ejecutable.

Adicionalmente, el ejecutable depende de archivos multimedias ubicados bajo la misma carpeta `game/`. Éstos deben ser accesibles y mantenerse en la misma ruta relativa a esta carpeta, al igual que el ejecutable.

La invocación del juego se debe de realizar mediante una terminal, ubicada con la carpeta del juego como directorio de trabajo, ejecutando el siguiente comando:
```
./Game.exe <archivo_configuracion>
```

Donde `<archivo_configuracion>` corresponde a la dirección relativa del archivo de texto de configuración utilizado para esta entrega, que tiene el siguiente formato en encodificación ASCII:
```
window <wW> <wh> <wr> <wg> <wb><LF>
font <fp> <r> <g> <b> <s><LF>
[entity <l> <ep> <ew> <eh> <px> <py> <vx> <vy> <a><LF>]...
```

La terminología es la siguiente:

|Símbolo|Significado|Formato|
|---|---|---|
|`ww`|Anchura de la ventana|Entero positivo en notación base 10|
|`wh`|Altura de la ventana|Entero positivo en notación base 10|
|`wr`|Tonalidad rojo del fondo de la ventana|Entero positivo en notación base 10 entre 0 y 255|
|`wg`|Tonalidad verde del fondo de la ventana|Entero positivo en notación base 10 entre 0 y 255|
|`wb`|Tonalidad azul del fondo de la ventana|Entero positivo en notación base 10 entre 0 y 255|
|`fp`|Archivo de fuente a utilizar|Dirección sin espacios al archivo de extensión `.ttf` correspondiente|
|`fr`|Tonalidad rojo del fondo del texto|Entero positivo en notación base 10 entre 0 y 255|
|`fg`|Tonalidad verde del fondo del texto|Entero positivo en notación base 10 entre 0 y 255|
|`fb`|Tonalidad azul del fondo del texto|Entero positivo en notación base 10 entre 0 y 255|
|`s`|Tamaño de la fuente de texto a usar|Entero positivo en notación base 10|
|`l`|Palabra para identificar a la entidad correspondiente|Texto ASCII legible sin espacios|
|`ep`|Archivo de imagen para identificar la entidad|Dirección sin espacios al archivo de imagen correspondiente|
|`ew`|Anchura de la entidad|Entero positivo en notación base 10|
|`eh`|Altura de la entidad|Entero positivo en notación base 10|
|`px`|Posición horizontal inicial de la entidad en la ventana|Entero positivo en notación base 10|
|`py`|Posición vertical inicial de la entidad en la ventana|Entero positivo en notación base 10|
|`vx`|Velocidad horizontal de la entidad en la ventana|Entero positivo en notación base 10|
|`vy`|Velocidad vertical de la entidad en la ventana|Entero positivo en notación base 10|
|`a`|Grados de orientación de la entidad|Decimal de doble precisión|

### Distribución

El ejecutable puede distribuirse enviando una copia de la carpeta `/game` a plataformas compatibles, acompañándole en ella los archivos multimedia, implementaciones de bibliotecas y demás archivos misceláneos requeridos.

# 📜 Créditos

- Código fuente original (omitiendo dependencias): [Javier Solano Saltachín](javier.solanosaltachin@ucr.ac.cr)

Los derechos de autoría y distribución de las dependencias y arte son de sus respectivos dueños. Aplican sus licencias respectivas.

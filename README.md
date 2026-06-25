# trabajo-practico-lp1
# Quoridor Pac-Man - Arcade Edition

Una adaptación estratégica en C que fusiona la recolección clásica de Pac-Man con el bloqueo táctico mediante muros de Quoridor. Desarrollado utilizando la librería gráfica **Raylib**.

<img width="796" height="624" alt="Captura de pantalla 2026-06-23 a la(s) 16 30 26" src="https://github.com/user-attachments/assets/635256b6-ab36-4d40-b2db-10fd29cec193" />


---

##  Cómo compilarlo

Para compilar el juego a través de la terminal, asegúrate de tener instalado un compilador de C (`gcc` o `clang`) y la librería Raylib.

### Instrucciones de Compilacion:
Abre tu terminal en la carpeta raíz del proyecto y ejecuta el comando según tu sistema operativo:
### En windows: 
Si tienes raylib configurado en las rutas por defecto de MinGW, ejecuta:

gcc main.c -o quoridor_pacman.exe -lraylib -lopengl32 -lgdi32 -lwinmm
### En Linux:
gcc main.c -o quoridor_pacman -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
## Cómo jugar
El objetivo principal es devorar las 4 pac-bolas repartidas por el mapa sin ser atrapado por los fantasmas. Al comer una pac-bola, los fantasmas se vuelven vulnerables temporalmente.

El juego funciona por un sistema de turnos estructurado.


<img width="796" height="625" alt="Captura de pantalla 2026-06-23 a la(s) 16 30 43" src="https://github.com/user-attachments/assets/f630f50b-7985-4151-b3f7-4704ae2d87a8" />


## Controles de Pac-Man
### Moverse: Usa las flechas direccionales (Arriba, Abajo, Izquierda, Derecha).

### Cambiar de modo (Movimiento / Muros): Presiona la tecla [TAB].

### Colocar un muro temporal: En el modo de muros, haz Click Izquierdo entre dos casillas. Consumirá tus "muros en mano" y te costará una acción.

## Controles de Fantasmas (Modo PvP)
Si juegas en modo Player vs Player, el Jugador 2 controla los turnos de los fantasmas activos:

### Mover el fantasma actual: Usa las teclas [I], [K], [J], [L] (Arriba, Abajo, Izquierda, Derecha).

### Colocar muro de fantasmas: Haz Click Izquierdo entre dos casillas (consume "muros fantasma").

<img width="789" height="622" alt="Captura de pantalla 2026-06-23 a la(s) 16 31 24" src="https://github.com/user-attachments/assets/43b0fd3e-ecfe-43f6-b9c3-c999a8c702bd" />


## Dificultad de los Fantasmas
En el menú de configuración, puedes asignar un nivel de inteligencia individual a cada uno de los 4 fantasmas (Blinky, Inky, Pinky, Clyde). Esto define cómo deciden sus movimientos durante el turno de la IA:

### 0 - Deshabilitado: El fantasma no aparece en el mapa y su turno es omitido.

### 1 - Fácil: Movimiento 100% aleatorio. El fantasma elegirá cualquier casilla adyacente válida sin importar dónde esté Pac-Man.

### 2 - Medio: Comportamiento híbrido. Tiene un 50% de probabilidad de moverse aleatoriamente y un 50% de usar la ruta más corta hacia Pac-Man.

### 3 - Difícil (IA Completa): El fantasma calculará matemáticamente (mediante el algoritmo BFS) la ruta exacta y más corta hacia Pac-Man en cada turno. Si Pac-Man come una pac-bola y la caza se invierte, la IA calculará la ruta más eficiente para alejarse y sobrevivir.


<img width="799" height="624" alt="Captura de pantalla 2026-06-23 a la(s) 16 30 55" src="https://github.com/user-attachments/assets/8091bf4e-5b5a-410c-b03a-e2d5d8004cec" />

## Cómo usar el Editor de Mapas
El juego incluye un editor integrado para diseñar tus propios tableros. Para acceder, selecciona "ABRIR EDITOR DE MAPAS" en el menú principal.

<img width="787" height="622" alt="Captura de pantalla 2026-06-23 a la(s) 16 34 10" src="https://github.com/user-attachments/assets/cf53ab6b-7771-4a71-8ef4-1350076942d3" />


## Herramientas del Editor
Presiona los números en tu teclado para cambiar de herramienta y usa el Click Izquierdo en la cuadrícula para aplicar los cambios:

### [1] Pac-Man: Define la posición inicial del jugador.

### [2] Fantasmas: Coloca el punto de aparición de los fantasmas (cada click alterna entre los 4 fantasmas disponibles).

### [3] Pac-bolas: Agrega o elimina las pac-bolas. Importante: Debes colocar exactamente 4 pac-bolas en el mapa para poder guardarlo.

### [4] Muros fijos: Haz click entre dos casillas para colocar un muro permanente. Vuelve a hacer click sobre él para eliminarlo.


## Controles adicionales del Editor
### [Flecha Arriba / Abajo] : Aumenta o disminuye el tamaño general del mapa (desde 5x5 hasta 15x15).

### [ENTER] : Guarda el mapa actual (se generará el archivo mapa_user.dat) y regresa al menú.

### [M] : Vuelve al menú principal sin guardar los cambios.

Para jugar en tu mapa creado, simplemente elige la opción "Usuario (mapa_user.dat)" en la configuración de mapa del menú principal.






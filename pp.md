# Presentacion De Bruno Verdun y Agustin Benavente
## El Bucle del programa 
Para manejar el apartado grafico utilizamos la libreria indicada, raylib, que nos permite dibujar formas, mostrar imagenes y leer el teclado, funciona imprimiendo en la pantalla una serie de pixeles determinados por el codigo que terminan formando una imagen estatica llamada fotograma, por cada pequeña varaciacion borra el fotograma previo y genera otro, la rapida sucesion de estos fotgramas en un tiempo determinado permite crear la ilusion de movimiento.

Lo primero que debemos hacer es establecer cuantos fotogramas queremos que pasen en un segundo (framerate), para este juego usamos un framerate comun en los programas modernos de 60 fps, luego creamos el bucle principal del programa while (!WindowShouldClose())) hace que el bucle que crea cada fotograma se repita hasta que la ventana del programa se cierre, cada vuelta del bucle consta de tres fases:
```c
SetTargetFPS(60);

while (!WindowShouldClose()) {
```
1. Input: Revisa si presionaste alguna tecla o moviste el mouse.
2. Actualizacion: Hace los calculos correspondientes, modifica variables suma numeros etc.
3. Dibujado: Ejecuta todas las funciones visuales para plasmar los nuevos datos.

## Mecanicas del juego

### 1. Sistema de Turnos y Puntos de Acción

El flujo de la partida no es en tiempo real,sino que está impulsado por un estricto motor de turnos asíncronos controlado en el main.c y apoyado por AvanzarTurnoGlobal.

### 1.1. La Raíz: La Estructura de Datos
Agregamos la variable `acciones` directamente a la estructura del jugador.

```c
typedef struct {
    Posicion pos;
    Posicion pos_inicial;
    int vidas;
    int acciones; 
} PacMan;
```

### 1.2. El Control del Bucle Principal


```c
if (p.pacman.acciones > 0) {
    // Turno del Jugador 1
    if (IsKeyPressed(KEY_UP)) MoverPacMan(&p, 0);
} 
else {
    // Control de enemigos
    if (!p.es_pvp) {
        if (IsKeyPressed(KEY_SPACE)) MoverFantasmas(&p); 
    }
}
```


### 1.3. La Economía Dinámica

```c
if (EsMovimientoValido(p, p->pacman.pos, destino)) {
    p->pacman.pos = destino;
    p->pacman.acciones--; // Costo base por moverse
    
    // Lógica de Power-Up (Comer Pac-bola)
    if (p->grilla[destino.f][destino.c] == 2) {
        p->grilla[destino.f][destino.c] = 0;
        p->bolitas_restantes--;
        p->cazando = true; 
        
        p->pacman.acciones++; // 
    }
}
```


### 1.4. El "Reloj" Global

```c
void AvanzarTurnoGlobal(Partida *p) {
    // codigo...

    // Fin del turno global
    p->cazando = false; 
    p->pacman.acciones = 2; 
}
```

### 2-Motor de Colisiones y Validación Espacial

A diferencia de los motores físicos tradicionales que usan áreas superpuestas (hitboxes), la física de este proyecto valida el espacio de forma puramente matemática y matricial, garantizando movimientos exactos en la cuadrícula.

### 2.1. El "Guardia de Seguridad" (Validación de Muros)
Antes de alterar cualquier posición en la memoria, el juego intercepta la intención de movimiento con la función `EsMovimientoValido`. El algoritmo calcula si el vector de movimiento entre la celda origen y destino cruza las coordenadas exactas de un muro activo.

```c
bool EsMovimientoValido(Partida *p, Posicion origen, Posicion destino) {
    // 1. Verificación de límites del tablero (Out of bounds)
    if (destino.f < 0 || destino.f >= p->filas || destino.c < 0 || destino.c >= p->cols) return false;
    
    // 2. Verificación vectorial contra el arreglo dinámico de muros
    for (int i = 0; i < p->cant_muros; i++) {
        bool cruza_de_c1_a_c2 = (origen.f == p->muros[i].c1.f && origen.c == p->muros[i].c1.c) &&
                               (destino.f == p->muros[i].c2.f && destino.c == p->muros[i].c2.c);
        
        bool cruza_de_c2_a_c1 = (origen.f == p->muros[i].c2.f && origen.c == p->muros[i].c2.c) &&
                               (destino.f == p->muros[i].c1.f && destino.c == p->muros[i].c1.c);
        
        if (cruza_de_c1_a_c2 || cruza_de_c2_a_c1) return false; // Bloquea el paso
    }
    return true;
}
```
### 2.2. Interacción Celular (Consumibles)
Una vez que el movimiento es validado, Pac-Man entra a la celda. Aquí interactúa directamente con la matriz bidimensional p->grilla (O(1) en eficiencia) para verificar el consumo de pac-bolas y aplicar el power-up global.

```c
// Actualizamos la posición exacta en la estructura
p->pacman.pos = destino;

// Comprobamos la matriz en la coordenada
if (p->grilla[destino.f][destino.c] == 2) {
    p->grilla[destino.f][destino.c] = 0; // Eliminación en memoria
    p->bolitas_restantes--;
    
    // Modificador global de estado
    p->cazando = true; 
    
    if (p->bolitas_restantes <= 0) {
        p->estado_juego = 2; // Condición de victoria
        return true;
    }
}
```

### 3. Resolución de Choques (Intersección de Entidades)
El último paso de la validación física es comprobar las colisiones entre entidades vivas. Iteramos sobre los fantasmas activos evaluando la superposición exacta de coordenadas. La resolución final depende de una única variable de estado: cazando.

```c
for(int i = 0; i < 4; i++) {
    // Verificamos coincidencia de coordenadas
    if (p->fantasmas[i].activo && p->fantasmas[i].pos.f == destino.f && p->fantasmas[i].pos.c == destino.c) {
        
        if (p->cazando) {
            // Resolución A: Pac-Man devora al fantasma
            p->fantasmas[i].activo = false; 
        } else {
            // Resolución B: Colisión letal para Pac-Man
            p->pacman.vidas--;
            if (p->pacman.vidas <= 0) {
                p->estado_juego = 3; // Game Over
            } else {
                // Soft-reset de posiciones
                p->pacman.pos = p->pacman.pos_inicial;
                // (Reubicación de fantasmas...)
            }
        }
    }
}
```

## 3-Inteligencia Artificial y Búsqueda de Caminos (BFS)

Nuestros fantasmas no se mueven mediante rutas predefinidas. Utilizan teoría de grafos y probabilidad para tomar decisiones dinámicas en un mapa que cambia constantemente debido a la colocación y destrucción de muros.

### 3.1. Sistema de Dificultad Escalable
El comportamiento de la IA se define por su nivel de dificultad. En lugar de cambiar la velocidad de movimiento, modificamos su capacidad de tomar decisiones tácticas perfectas mediante el uso de probabilidad.

```c
// Dificultad 3: IA perfecta permanente. 
// Dificultad 2: 50% de probabilidad de usar IA perfecta o moverse al azar.
int dif_actual = p->fantasmas[i].dificultad;
if (dif_actual == 3) usar_ia = true;
else if (dif_actual == 2) usar_ia = (GetRandomValue(0, 1) == 0);
```

### 3.2. Inicialización del BFS y Memoria Dinámica
Cuando un fantasma decide usar la IA, calculamos el camino más corto hacia Pac-Man usando el algoritmo de Búsqueda en Anchura (BFS). Primero, inicializamos un mapa de distancias y construimos una Cola (Queue) utilizando asignación de memoria dinámica.

```c
// Inicialización del mapa con -1 (nodos no visitados)
for(int i = 0; i < p->filas; i++) {
    for(int j = 0; j < p->cols; j++) dist[i][j] = -1;
}

// Creación de la Cola FIFO dinámica en el Heap
Posicion *queue = malloc(p->filas * p->cols * sizeof(Posicion));
int head = 0, tail = 0;

// Pac-Man es el epicentro de la búsqueda (distancia 0)
Posicion inicio = p->pacman.pos;
dist[inicio.f][inicio.c] = 0;
queue[tail++] = inicio;
```

## 3.3. La Propagación de la Onda Matemática
El algoritmo extrae posiciones de la cabeza de la cola y expande la búsqueda en sus 4 celdas vecinas. Si un vecino es válido (no cruza un muro bloqueado) y no ha sido visitado, se registra su distancia.

 ```c
int df[] = {-1, 1, 0, 0};
int dc[] = {0, 0, -1, 1};

while(head < tail) {
    Posicion curr = queue[head++];
    for(int i = 0; i < 4; i++) {
        Posicion sig = {curr.f + df[i], curr.c + dc[i]};
        
        // Verificamos límites del mapa
        if (sig.f >= 0 && sig.f < p->filas && sig.c >= 0 && sig.c < p->cols) {
            
            // Evaluamos intersección con muros dinámicos
            if (dist[sig.f][sig.c] == -1 && EsMovimientoValido(p, curr, sig)) {
                
                dist[sig.f][sig.c] = dist[curr.f][curr.c] + 1; // Registra la distancia matemática
                queue[tail++] = sig;                           // Encola para continuar expandiendo
            }
        }
    }
}
free(queue); // Prevención estricta de fugas de memoria (Memory Leaks)
```
### 3.4. Ejecución Táctica (Caza vs Huida)
Con el mapa de calor de distancias generado, el fantasma evalúa sus propios vecinos inmediatos. La decisión final depende de la bandera global cazando, la cual invierte por completo el objetivo de la búsqueda.

```c
if (p->cazando) {
    // Modo Huida: El fantasma busca la casilla adyacente con la distancia MÁXIMA a Pac-Man
    int max_dist = -1;
    for (int d = 0; d < 4; d++) {
        // ... (Validación de casilla) ...
        if (d_pac != -1 && d_pac > max_dist) {
            max_dist = d_pac;
            mejor_destino = vec;
        }
    }
} else {
    // Modo Cacería: El fantasma busca la casilla adyacente con la distancia MÍNIMA a Pac-Man
    int min_dist = 9999;
    for (int d = 0; d < 4; d++) {
        // ... (Validación de casilla) ...
        if (d_pac != -1 && d_pac < min_dist) {
            min_dist = d_pac;
            mejor_destino = vec;
        }
    }
}
```
## Dibujado 

### 1. Preparacion del Fotograma 
```c
BeginDrawing();
    ClearBackground(GetColor(0x050510FF));
```
Comenzamos el ciclo abriendo el buffer de memoria grafica y borrando todo el fotograma anterior con un fondo de color azul oscuro 

### 2. Dibujar el menu principal 
```c
int btnY = 90 + i * 40;
 if (p.estado_juego == 0) {
    DrawText("CONFIGURACIÓN DE PARTIDA", 150, 40, 32, YELLOW);
    DrawText("[Izquierda/Derecha] Cambiar valor  -  [ENTER] Aceptar", 180, 560, 15, LIGHTGRAY);

    for (int i = 0; i < 11; i++) {
    .....
```
Cuando el estado_juego sea igual a 0 signfica que estamos en el menu principal, con drawtext dibujamos en la pantalla los textos fijos del menu y el bucle nos permite crear cada una de las opciones de manera mas eficiente, dicho bucle recorre un arreglo de opciones y, en cada fotograma, consulta el valor de las variables de configuración y la posición del cursor (menu_sel), en caso de ser la opcion elegida se le dibuja una caja oscura y se cambia el color del texto a uno especifico, btnY permite espaciar las opciones de una manera sencilla y simetrica. 
```c
if (menu_sel == i) DrawRectangle(120, btnY, 560, 35, GetColor(0x11112CFF));

    if (i == 0) DrawText(TextFormat("Modo de Juego: %s", cfg_pvp ? "Player vs Player" : "IA vs Player"), 150, btnY + 10, 20, (menu_sel == i) ? WHITE : GRAY);
```

### 3. El Motor del tablero 

Utilizamos el Algoritmo del Pintor, que consiste en dibujar la escena en capas, desde el fondo hacia adelante, para que los elementos se superpongan correctamente.

### 3.1 Grilla y Pac-bolas
```c
 for (int i = 0; i < p.filas; i++) {

                    for (int j = 0; j < p.cols; j++) {
                        if (p.grilla[i][j] == 2) DrawCircle(offsetX + j * cellSize + cellSize/2, offsetY + i * cellSize + cellSize/2, cellSize*0.12f, GOLD);
                    }
                }

                for (int i = 0; i < p.filas; i++) {
                    for (int j = 0; j < p.cols; j++) DrawRectangleLines(offsetX + j * cellSize, offsetY + i * cellSize, cellSize, cellSize, DARKGRAY);
                }
```
Con el primer bucle recorremos la matriz p.grilla. En este bucle, si detectamos un valor 2, dibujamos una pac-bola. El secreto aquí es la matemática de posicionamiento: calculamos el centro de cada celda sumando el offset (margen) más la mitad del tamaño de la celda (cellSize/2). Esto garantiza que los objetos siempre queden perfectamente centrado, mientras que con el segundo creamos el de dibujo de cuadricula del mapa creando el contorno Lines de cada celda.

### 3.2 Logica de Muros 
```c
for (int i = 0; i < p.cant_muros; i++) {
    float x1 = offsetX + p.muros[i].c1.c * cellSize, y1 = offsetY + p.muros[i].c1.f * cellSize;
    float x2 = offsetX + p.muros[i].c2.c * cellSize, y2 = offsetY + p.muros[i].c2.f * cellSize;
    Color colorMuro = (p.muros[i].propietario == -1) ? BLUE : ORANGE; 

    if (p.muros[i].c1.f != p.muros[i].c2.f) {
        float yLinea = (y1 > y2) ? y1 : y2;
            DrawLineEx((Vector2){x1, yLinea}, (Vector2){x1 + cellSize, yLinea}, 8, colorMuro);
            } else {
                float xLinea = (x1 > x2) ? x1 : x2;
                DrawLineEx((Vector2){xLinea, y1}, (Vector2){xLinea, y1 + cellSize}, 8, colorMuro);
            }
}
```
El sistema de muros utiliza una lógica de orientación absoluta. Se almacena los muros mediante dos puntos extremos. En el renderizado, analizamos la diferencia entre las coordenadas (f, c) de ambos puntos para determinar su orientación. Por lo cual la orientacion se deriva directamente de la geometría del muro en la matriz.

### 3.3 Entidades y animacion 
```c
DrawCircle(offsetX + p.pacman.pos.c * cellSize + cellSize/2, offsetY + p.pacman.pos.f * cellSize + cellSize/2, cellSize*0.3f, YELLOW);

for (int i = 0; i < 4; i++) {
                    if (p.fantasmas[i].activo && p.fantasmas[i].habilitado) {
                        float fx = offsetX + p.fantasmas[i].pos.c * cellSize + cellSize/2;
                        float fy = offsetY + p.fantasmas[i].pos.f * cellSize + cellSize/2;
                        ........
```
Dibujamos a pac-man y a cada uno de los fantasmas que esten habilitados y que todavia no hayan sido comidos por pac-man y los dinujan en el centro de sus respectivas casillas.
```c
Color colorActual;
    if (p.estado_juego == 1 && p.cazando) {
        colorActual = (((int)(GetTime() * 4) % 2) == 0) ? BLUE : SKYBLUE;
        } else {
        colorActual = p.fantasmas[i].color;
        }
                        
    if (p.estado_juego == 1 && p.es_pvp && p.pacman.acciones == 0 && p.fantasma_actual_idx == i) {
        if (((int)(GetTime() * 5) % 2) == 0) DrawCircle(fx, fy - cellSize*0.06f, cellSize*0.3f, WHITE);
    }

    DrawCircle(fx, fy - cellSize*0.06f, cellSize*0.23f, colorActual);
    DrawRectangle(fx - cellSize*0.23f, fy - cellSize*0.06f, cellSize*0.46f, cellSize*0.3f, colorActual);
                        
    if (p.estado_juego == 1 && p.cazando) {
            DrawCircle(fx - cellSize*0.1f, fy - cellSize*0.03f, cellSize*0.05f, WHITE);
            DrawCircle(fx + cellSize*0.1f, fy - cellSize*0.03f, cellSize*0.05f, WHITE);
    } else {
        DrawCircle(fx - cellSize*0.1f, fy - cellSize*0.06f, cellSize*0.06f, WHITE);
        DrawCircle(fx + cellSize*0.1f, fy - cellSize*0.06f, cellSize*0.06f, WHITE);
        DrawCircle(fx - cellSize*0.1f, fy - cellSize*0.06f, cellSize*0.03f, BLACK);
        DrawCircle(fx + cellSize*0.1f, fy - cellSize*0.06f, cellSize*0.03f, BLACK);
    }
```
El renderizado de los fantasmas funciona mediante un sistema de composición geométrica. En lugar de cargar archivos de imagen, construimos cada fantasma como un objeto compuesto por círculos y rectángulos. Se utiliza la sincronizacion de estados mediante GetTime() para causar cambios visuales en tiempo real, como el parpadeo en el modo 'cazando' o el indicador de seleccion en PvP. Esto permite al usuario identificar el estado lógico del juego (quién es vulnerable, de quién es el turno) simplemente mirando la pantalla, sin necesidad de consultar el HUD lateral.

## 4. La Interfaz Lateral 
```c
 DrawLineEx((Vector2){545, 0}, (Vector2){545, 600}, 3, BLUE);
DrawRectangle(548, 0, 252, 600, BLACK);

if (p.estado_juego == 1) {
    DrawText("QUORIDOR", 565, 20, 24, BLUE);
    DrawText("PAC-MAN", 565, 45, 24, YELLOW);
```
Primero creamos los graficos estaticos de la interfaz, la linea divisora azul y el rectangulo negro del fondo ademas de los textos QUORIDOR y PAC-MAN.

### 4.1 Monitoreo de Datos en Tiempo Real
```c
if (p.estado_juego == 1) {
                    DrawText("QUORIDOR", 565, 20, 24, BLUE);
                    DrawText("PAC-MAN", 565, 45, 24, YELLOW);
                    
                    DrawText(p.es_pvp ? "MODO: PVP LOCAL" : "MODO: VS COMPUTADORA", 565, 75, 13, SKYBLUE);
                    DrawLine(565, 95, 765, 95, DARKGRAY);

                    DrawText(TextFormat("ACCIONES: %d", p.pacman.acciones), 565, 115, 20, WHITE);
                    DrawText(TextFormat("VIDAS: %d", p.pacman.vidas), 565, 145, 20, p.pacman.vidas > 1 ? GREEN : RED);
                    DrawText(TextFormat("BOLITAS: %d / 4", p.bolitas_restantes), 565, 175, 20, GOLD);
                    DrawText(TextFormat("Muros P-M: %d", p.muros_mano_pacman), 565, 205, 16, ORANGE);
                    DrawText(TextFormat("Muros Fant: %d", p.muros_mano_fantasmas), 565, 225, 16, RED);

                    if (p.cazando) {
                        DrawText("¡CAZA ACTIVA!", 565, 250, 18, SKYBLUE);
                    }
```
Transforma las variables logicas internas del juego en texto dinamico para el jugador utilizando TextFormat(TextFormat es una funcion de raylibb que permite combinar texto estatico con variables dinamicas ya que las funciones de dibujado de raylib no permiten combinar enteros con caracteres), cambia dinamicamente el color de Vidas a rojo cuando solo queda una y avisa al jugador de la inversion de roles e inicio del modo caza cuando Pac-man come una pac-bola.

### 4.2 Maquina de Estados de Turnos 
```c
DrawLine(565, 275, 765, 275, DARKGRAY);
DrawText("TURNO ACTUAL:", 565, 290, 15, LIGHTGRAY);
                    
if (p.pacman.acciones > 0) {
    if (p.modo_juego == 0) DrawText("MOVER PACMAN", 565, 310, 20, YELLOW);
        else DrawText("COLOCAR MURO", 565, 310, 20, ORANGE);
        DrawText("[TAB] Cambiar Modo", 565, 335, 14, GRAY);
        } else {
            if (!p.es_pvp) {
                DrawRectangle(558, 305, 230, 70, MAROON);
                DrawRectangleLines(558, 305, 230, 70, RED);
                DrawText("TURNO IA", 573, 315, 16, WHITE);
                DrawText("[ESPACIO] Avanzar", 573, 342, 14, YELLOW);
                } else {
                    DrawRectangle(558, 305, 230, 110, GetColor(0x152535FF));
                    DrawRectangleLines(558, 305, 230, 110, SKYBLUE);      
                    int actIdx = p.fantasma_actual_idx;
                    Color cF = p.fantasmas[actIdx].activo ? p.fantasmas[actIdx].color : GRAY;  
                    DrawText("JUGADOR 2 (FANTASMAS)", 568, 315, 13, SKYBLUE);
                    DrawText(TextFormat("Moviendo índice: %d", actIdx), 568, 335, 13, cF);
                    DrawText("Teclas: [ I, J, K, L ] o Muro", 562, 365, 11, LIGHTGRAY);
                    }
                }
```
Este bloque implementa la lógica de renderizado condicional para la gestión de turnos. Funciona como una estructura anidada que actúa según el estado de los datos. El primer nivel evalúa las acciones restantes de Pac-Man para separar el turno del atacante del turno de los defensores.

En el segundo nivel, el HUD altera por completo su layout gráfico: si la partida es contra la Inteligencia Artificial, dibuja un contenedor de alerta para sincronizar el procesamiento del algoritmo de búsqueda; si la partida es PvP, renderiza un contenedor informativo para el Jugador 2 que mapea dinámicamente el color del fantasma seleccionado mediante operaciones condicionales y le explicita su esquema de entrada por teclado

## 5. Editor de Mapas 
```c
else if (p.estado_juego == 4) {
                    DrawText("MODO EDITOR", 565, 30, 24, PURPLE);
                    DrawLine(565, 75, 765, 75, DARKGRAY);
                    DrawText("HERRAMIENTAS:", 565, 95, 16, LIGHTGRAY);
                    DrawText("[1] PAC-MAN", 565, 130, 16, (herramienta == 0) ? YELLOW : GRAY);
                    DrawText("[2] FANTASMAS", 565, 170, 16, (herramienta == 1) ? RED : GRAY);
                    DrawText("[3] PAC-BOLAS", 565, 210, 16, (herramienta == 2) ? GOLD : GRAY);
                    DrawText("[4] MUROS FIJOS", 565, 250, 16, (herramienta == 3) ? RED : GRAY);
                    DrawLine(565, 300, 765, 300, DARKGRAY);
                    DrawText("CONTROLES:", 565, 320, 16, LIGHTGRAY);
                    DrawText("Click Izq: Editar", 565, 350, 15, WHITE);
                    DrawText("[FLECHAS] Tamaño Mapa", 565, 380, 15, SKYBLUE);
                    DrawText("[ENTER] GUARDAR", 575, 450, 16, GREEN);
                    DrawText("[M] VOLVER AL MENÚ", 575, 480, 15, GRAY);
                }
```
Dibuja todas las opciones e informacion del editor de mapas ademas de una ayuda visual para que el jugador sepa que opcion tiene seleccionada.
## 5. Pantallas de Fin de Partida 
```c
if (p.estado_juego == 2 || p.estado_juego == 3) { 
                    DrawRectangle(0, 0, 800, 600, Fade(BLACK, 0.88f));
                    
                    int comidas = 4 - p.bolitas_restantes;
                    const char* nivelTexto = "Pac-Man Principiante";
                    Color nivelColor = GRAY;

                    if (comidas == 1) { nivelTexto = "Pac-Man novato"; nivelColor = GREEN; }
                    else if (comidas == 2) { nivelTexto = "Pac-Man prometedor"; nivelColor = LIME; }
                    else if (comidas == 3) { nivelTexto = "Pac-Man de categoría"; nivelColor = ORANGE; }
                    else if (comidas == 4) { nivelTexto = "Pac-Man de élite"; nivelColor = GOLD; }

                    if (p.estado_juego == 2) { 
                        DrawText("¡VICTORIA!", 280, 160, 42, GREEN);
                        DrawText("¡Conseguiste el objetivo perfecto!", 220, 220, 20, WHITE);
                    } else { 
                        DrawText("GAME OVER", 285, 160, 42, RED);
                        DrawText("Los fantasmas te acorralaron...", 240, 220, 20, WHITE);
                    }

                    DrawText(TextFormat("Pac-bolas devoradas: %d / 4", comidas), 260, 280, 20, GOLD);
                    DrawText("Nivel Alcanzado:", 300, 330, 20, LIGHTGRAY);
                    DrawText(nivelTexto, 400 - (MeasureText(nivelTexto, 26) / 2), 370, 26, nivelColor);
                    DrawText("Presiona 'R' para regresar al Menú Principal", 185, 470, 18, GRAY);
                }
            }

        EndDrawing();
```
Se utiliza la funcion Fade para superponer un rectangulo negro con una opacidad del 88% para dar un efecto de oscurecimiento al activarse la pantalla de fin de partida, luego a nivel logico se procesan los resultados obtenidos en la partida guardados en las variables correspondientes para determinar un nivel de exito o fracaso, los textos y colores dibujados estan basados en ese nivel de exito, finalmente se implemente un algoritmo de centrado utilizando MeasureText que calcula dinamicamente el ancho del rango de pixeles obtenidos para ubicarse en ele centro.


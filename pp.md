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


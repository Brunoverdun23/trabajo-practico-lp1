
#include "raylib.h"
#include "logica.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

int main() {
    InitWindow(800, 600, "Quoridor Pac-Man - Arcade Edition");
    
    Partida p;
    p.grilla = NULL;
    p.muros = NULL;
    p.estado_juego = 0; 

    int herramienta = 0;      
    int ghost_editar_idx = 0; 

    // Configuracion
    bool cfg_pvp = false;
    int cfg_f_dif[4] = {2, 2, 2, 2}; 
    int cfg_mpac = 3;
    int cfg_mfant = 1;
    int cfg_dur = 4;
    int cfg_mapa = 0; 
    int menu_sel = 0;

    const char* str_dif[4] = {"Deshabilitado", "Fácil", "Medio", "Difícil"};
    const char* str_mapa[4] = {"Predeterminado 1", "Predeterminado 2", "Predeterminado 3", "Usuario (mapa_user.dat)"};

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        
        // Calculo de escala dinamica
        float cellSize = 60.0f;
        float offsetX = 0.0f;
        float offsetY = 0.0f;
        if (p.estado_juego == 1 || p.estado_juego == 4 || p.estado_juego == 2 || p.estado_juego == 3) {
            float cw = 540.0f / p.cols;
            float ch = 600.0f / p.filas;
            cellSize = (cw < ch) ? cw : ch;
            offsetX = (540.0f - (cellSize * p.cols)) / 2.0f;
            offsetY = (600.0f - (cellSize * p.filas)) / 2.0f;
        }

        // ==========================================
        // ---       1. LÓGICA DE CONTROL         ---
        // ==========================================
        
        if (p.estado_juego == 0) {
            if (IsKeyPressed(KEY_DOWN)) menu_sel = (menu_sel + 1) % 11;
            if (IsKeyPressed(KEY_UP)) menu_sel = (menu_sel - 1 + 11) % 11;
            
            if (IsKeyPressed(KEY_RIGHT)) {
                if (menu_sel == 0) cfg_pvp = !cfg_pvp;
                else if (menu_sel >= 1 && menu_sel <= 4) { if(cfg_f_dif[menu_sel-1] < 3) cfg_f_dif[menu_sel-1]++; }
                else if (menu_sel == 5) { if(cfg_mpac < 10) cfg_mpac++; }
                else if (menu_sel == 6) { if(cfg_mfant < 10) cfg_mfant++; }
                else if (menu_sel == 7) { if(cfg_dur < 10) cfg_dur++; }
                else if (menu_sel == 8) { if(cfg_mapa < 3) cfg_mapa++; }
            }
            if (IsKeyPressed(KEY_LEFT)) {
                if (menu_sel == 0) cfg_pvp = !cfg_pvp;
                else if (menu_sel >= 1 && menu_sel <= 4) { if(cfg_f_dif[menu_sel-1] > 0) cfg_f_dif[menu_sel-1]--; }
                else if (menu_sel == 5) { if(cfg_mpac > 0) cfg_mpac--; }
                else if (menu_sel == 6) { if(cfg_mfant > 0) cfg_mfant--; }
                else if (menu_sel == 7) { if(cfg_dur > 1) cfg_dur--; }
                else if (menu_sel == 8) { if(cfg_mapa > 0) cfg_mapa--; }
            }

            if (IsKeyPressed(KEY_ENTER)) {
                if (menu_sel == 9) {
                    IniciarJuego(&p, cfg_pvp, cfg_f_dif, cfg_mpac, cfg_mfant, cfg_dur, cfg_mapa);
                }
                else if (menu_sel == 10) {
                    p.filas = 9; p.cols = 9;
                    p.estado_juego = 4; p.cant_muros = 0; p.muros = NULL; p.bolitas_restantes = 0;
                    p.es_pvp = false; 
                    p.grilla = malloc(9 * sizeof(int *));
                    for(int i = 0; i < 9; i++) {
                        p.grilla[i] = malloc(9 * sizeof(int));
                        for(int j = 0; j < 9; j++) p.grilla[i][j] = 0;
                    }
                    p.pacman.pos = (Posicion){4, 4}; 
                    Color colores[4] = {RED, PINK, ORANGE, BLUE};
                    for(int i = 0; i < 4; i++) {
                        p.fantasmas[i].pos = (Posicion){0, 0}; 
                        p.fantasmas[i].color = colores[i];
                    }
                    herramienta = 0; ghost_editar_idx = 0;
                }
            }
        } 
        else if (p.estado_juego == 2 || p.estado_juego == 3) {
            if (IsKeyPressed(KEY_R)) {
                LiberarPartida(&p);
                p.estado_juego = 0; 
            }
        } 
        else if (p.estado_juego == 1) {
            if (p.pacman.acciones > 0 && IsKeyPressed(KEY_TAB)) {
                p.modo_juego = !p.modo_juego; 
            }

            if (p.pacman.acciones > 0) {
                if (p.modo_juego == 0) {
                    if (IsKeyPressed(KEY_UP))    MoverPacMan(&p, 0);
                    if (IsKeyPressed(KEY_DOWN))  MoverPacMan(&p, 1);
                    if (IsKeyPressed(KEY_LEFT))  MoverPacMan(&p, 2);
                    if (IsKeyPressed(KEY_RIGHT)) MoverPacMan(&p, 3);
                } else {
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && p.muros_mano_pacman > 0) {
                        Vector2 mousePos = GetMousePosition();
                        float cx = mousePos.x - offsetX;
                        float cy = mousePos.y - offsetY;
                        
                        if (cx >= 0 && cy >= 0 && cx <= p.cols * cellSize && cy <= p.filas * cellSize) {
                            int col = (int)(cx / cellSize);
                            int fila = (int)(cy / cellSize);
                            int localX = (int)cx % (int)cellSize;
                            int localY = (int)cy % (int)cellSize;

                            Posicion pos1 = {fila, col};
                            Posicion pos2;
                            bool valido = true;
                            
                            int margen = (int)(cellSize * 0.25f);
                            int margen_sup = (int)(cellSize * 0.75f);

                            if (localX < margen) pos2 = (Posicion){fila, col - 1};      
                            else if (localX > margen_sup) pos2 = (Posicion){fila, col + 1}; 
                            else if (localY < margen) pos2 = (Posicion){fila - 1, col}; 
                            else if (localY > margen_sup) pos2 = (Posicion){fila + 1, col}; 
                            else valido = false;

                            if (valido && pos2.f >= 0 && pos2.f < p.filas && pos2.c >= 0 && pos2.c < p.cols) {
                                AgregarMuro(&p, pos1, pos2, p.duracion_muros_global, 0); 
                                p.muros_mano_pacman--;
                                p.pacman.acciones--;
                            }
                        }
                    }
                }
            } 
            else {
                if (!p.es_pvp) {
                    if (IsKeyPressed(KEY_SPACE)) MoverFantasmas(&p); 
                } else {
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && p.muros_mano_fantasmas > 0) {
                        Vector2 mousePos = GetMousePosition();
                        float cx = mousePos.x - offsetX;
                        float cy = mousePos.y - offsetY;

                        if (cx >= 0 && cy >= 0 && cx <= p.cols * cellSize && cy <= p.filas * cellSize) {
                            int col = (int)(cx / cellSize);
                            int fila = (int)(cy / cellSize);
                            int localX = (int)cx % (int)cellSize;
                            int localY = (int)cy % (int)cellSize;

                            Posicion m1 = {fila, col};
                            Posicion m2;
                            bool m_valido = true;
                            
                            int margen = (int)(cellSize * 0.25f);
                            int margen_sup = (int)(cellSize * 0.75f);

                            if (localX < margen) m2 = (Posicion){fila, col - 1};
                            else if (localX > margen_sup) m2 = (Posicion){fila, col + 1};
                            else if (localY < margen) m2 = (Posicion){fila - 1, col};
                            else if (localY > margen_sup) m2 = (Posicion){fila + 1, col};
                            else m_valido = false;

                            if (m_valido && m2.f >= 0 && m2.f < p.filas && m2.c >= 0 && m2.c < p.cols) {
                                bool exito = ControlarFantasmaPvP(&p, -1, true, m1, m2);
                                if (exito && p.fantasma_actual_idx == 0) {
                                    AvanzarTurnoGlobal(&p); 
                                }
                            }
                        }
                    }

                    int dir_ghost = -1;
                    if (IsKeyPressed(KEY_I)) dir_ghost = 0; 
                    if (IsKeyPressed(KEY_K)) dir_ghost = 1; 
                    if (IsKeyPressed(KEY_J)) dir_ghost = 2; 
                    if (IsKeyPressed(KEY_L)) dir_ghost = 3; 

                    if (dir_ghost != -1) {
                        bool exito = ControlarFantasmaPvP(&p, dir_ghost, false, (Posicion){0,0}, (Posicion){0,0});
                        if (exito && p.fantasma_actual_idx == 0) {
                            AvanzarTurnoGlobal(&p);
                        }
                    }
                }
            }
        }
        else if (p.estado_juego == 4) {
            if (IsKeyPressed(KEY_ONE))   herramienta = 0;
            if (IsKeyPressed(KEY_TWO))   herramienta = 1;
            if (IsKeyPressed(KEY_THREE)) herramienta = 2;
            if (IsKeyPressed(KEY_FOUR))  herramienta = 3;

            if (IsKeyPressed(KEY_UP) && p.filas > 5) {
                int n_f = p.filas - 1; int n_c = p.cols - 1;
                int **n_g = malloc(n_f * sizeof(int *));
                for(int i = 0; i < n_f; i++) {
                    n_g[i] = malloc(n_c * sizeof(int));
                    for(int j = 0; j < n_c; j++) n_g[i][j] = p.grilla[i][j];
                }
                for(int i = 0; i < p.filas; i++) free(p.grilla[i]);
                free(p.grilla);
                p.grilla = n_g; p.filas = n_f; p.cols = n_c;
            }
            if (IsKeyPressed(KEY_DOWN) && p.filas < 15) {
                int n_f = p.filas + 1; int n_c = p.cols + 1;
                int **n_g = malloc(n_f * sizeof(int *));
                for(int i = 0; i < n_f; i++) {
                    n_g[i] = malloc(n_c * sizeof(int));
                    for(int j = 0; j < n_c; j++) {
                        if (i < p.filas && j < p.cols) n_g[i][j] = p.grilla[i][j];
                        else n_g[i][j] = 0;
                    }
                }
                for(int i = 0; i < p.filas; i++) free(p.grilla[i]);
                free(p.grilla);
                p.grilla = n_g; p.filas = n_f; p.cols = n_c;
            }

            if (IsKeyPressed(KEY_M)) {
                LiberarPartida(&p);
                p.estado_juego = 0; 
            }
            
            if (IsKeyPressed(KEY_ENTER)) {
                p.bolitas_restantes = 0;
                for(int i = 0; i < p.filas; i++) {
                    for(int j = 0; j < p.cols; j++) {
                        if (p.grilla[i][j] == 2) p.bolitas_restantes++;
                    }
                }
                if (p.bolitas_restantes == 4) {
                    MapaGuardado n_map;
                    memset(&n_map, 0, sizeof(MapaGuardado)); // LIMPIEZA VITAL DE MEMORIA
                    n_map.filas = p.filas;
                    n_map.cols = p.cols;
                    n_map.pos_pacman = p.pacman.pos;
                    for(int g=0; g<4; g++) n_map.pos_fantasmas[g] = p.fantasmas[g].pos;
                    for(int i=0; i<p.filas; i++) {
                        for(int j=0; j<p.cols; j++) n_map.grilla[i][j] = p.grilla[i][j];
                    }
                    n_map.cant_muros = (p.cant_muros <= MAX_MUROS_MAPA) ? p.cant_muros : MAX_MUROS_MAPA;
                    for(int i=0; i<n_map.cant_muros; i++) n_map.muros[i] = p.muros[i];

                    GuardarMapaPersonalizado("mapa_user.dat", &n_map);
                    LiberarPartida(&p);
                    p.estado_juego = 0; 
                }
            }

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mousePos = GetMousePosition();
                float cx = mousePos.x - offsetX;
                float cy = mousePos.y - offsetY;

                if (cx >= 0 && cy >= 0 && cx <= p.cols * cellSize && cy <= p.filas * cellSize) {
                    int col = (int)(cx / cellSize);
                    int fila = (int)(cy / cellSize);

                    if (herramienta == 0) p.pacman.pos = (Posicion){fila, col}; 
                    else if (herramienta == 1) {
                        p.fantasmas[ghost_editar_idx].pos = (Posicion){fila, col}; 
                        ghost_editar_idx = (ghost_editar_idx + 1) % 4; 
                    } 
                    else if (herramienta == 2) p.grilla[fila][col] = (p.grilla[fila][col] == 2) ? 0 : 2;
                    else if (herramienta == 3) {
                        int localX = (int)cx % (int)cellSize;
                        int localY = (int)cy % (int)cellSize;
                        Posicion pos1 = {fila, col};
                        Posicion pos2;
                        bool valido = true;
                        
                        int margen = (int)(cellSize * 0.25f);
                        int margen_sup = (int)(cellSize * 0.75f);

                        if (localX < margen) pos2 = (Posicion){fila, col - 1};      
                        else if (localX > margen_sup) pos2 = (Posicion){fila, col + 1}; 
                        else if (localY < margen) pos2 = (Posicion){fila - 1, col}; 
                        else if (localY > margen_sup) pos2 = (Posicion){fila + 1, col}; 
                        else valido = false;

                        if (valido && pos2.f >= 0 && pos2.f < p.filas && pos2.c >= 0 && pos2.c < p.cols) {
                            int indice_encontrado = -1;
                            for (int i = 0; i < p.cant_muros; i++) {
                                bool c1 = (p.muros[i].c1.f == pos1.f && p.muros[i].c1.c == pos1.c && p.muros[i].c2.f == pos2.f && p.muros[i].c2.c == pos2.c);
                                bool c2 = (p.muros[i].c1.f == pos2.f && p.muros[i].c1.c == pos2.c && p.muros[i].c2.f == pos1.f && p.muros[i].c2.c == pos1.c);
                                if (c1 || c2) { indice_encontrado = i; break; }
                            }

                            if (indice_encontrado != -1) {
                                for (int i = indice_encontrado; i < p.cant_muros - 1; i++) p.muros[i] = p.muros[i + 1];
                                p.cant_muros--;
                                if (p.cant_muros > 0) {
                                    Muro *temp = (Muro *)realloc(p.muros, p.cant_muros * sizeof(Muro));
                                    if (temp != NULL) p.muros = temp;
                                } else { free(p.muros); p.muros = NULL; }
                            } else {
                                AgregarMuro(&p, pos1, pos2, -1, -1);
                            }
                        }
                    }
                }
            }
        }

        // ==========================================
        // ---              2. RENDERS            ---
        // ==========================================
        BeginDrawing();
            ClearBackground(GetColor(0x050510FF)); 

            if (p.estado_juego == 0) {
                DrawText("CONFIGURACIÓN DE PARTIDA", 150, 40, 32, YELLOW);
                DrawText("[Izquierda/Derecha] Cambiar valor  -  [ENTER] Aceptar", 180, 560, 15, LIGHTGRAY);

                for (int i = 0; i < 11; i++) {
                    int btnY = 90 + i * 40;
                    if (menu_sel == i) DrawRectangle(120, btnY, 560, 35, GetColor(0x11112CFF));

                    if (i == 0) DrawText(TextFormat("Modo de Juego: %s", cfg_pvp ? "Player vs Player" : "IA vs Player"), 150, btnY + 10, 20, (menu_sel == i) ? WHITE : GRAY);
                    else if (i == 1) DrawText(TextFormat("Blinky (Rojo): %s", str_dif[cfg_f_dif[0]]), 150, btnY + 10, 20, (menu_sel == i) ? WHITE : GRAY);
                    else if (i == 2) DrawText(TextFormat("Inky (Cian): %s", str_dif[cfg_f_dif[1]]), 150, btnY + 10, 20, (menu_sel == i) ? WHITE : GRAY);
                    else if (i == 3) DrawText(TextFormat("Pinky (Rosa): %s", str_dif[cfg_f_dif[2]]), 150, btnY + 10, 20, (menu_sel == i) ? WHITE : GRAY);
                    else if (i == 4) DrawText(TextFormat("Clyde (Naranja): %s", str_dif[cfg_f_dif[3]]), 150, btnY + 10, 20, (menu_sel == i) ? WHITE : GRAY);
                    else if (i == 5) DrawText(TextFormat("Muros Iniciales Pac-Man: %d", cfg_mpac), 150, btnY + 10, 20, (menu_sel == i) ? WHITE : GRAY);
                    else if (i == 6) DrawText(TextFormat("Muros Iniciales Fantasmas: %d", cfg_mfant), 150, btnY + 10, 20, (menu_sel == i) ? WHITE : GRAY);
                    else if (i == 7) DrawText(TextFormat("Tiempo de Vida de Muros: %d", cfg_dur), 150, btnY + 10, 20, (menu_sel == i) ? WHITE : GRAY);
                    else if (i == 8) DrawText(TextFormat("Mapa a Jugar: %s", str_mapa[cfg_mapa]), 150, btnY + 10, 20, (menu_sel == i) ? WHITE : GRAY);
                    else if (i == 9) DrawText("-> ¡INICIAR JUEGO!", 150, btnY + 10, 20, (menu_sel == i) ? GREEN : LIME);
                    else if (i == 10) DrawText("-> ABRIR EDITOR DE MAPAS", 150, btnY + 10, 20, (menu_sel == i) ? PURPLE : MAGENTA);
                }
            } 
            else {
                for (int i = 0; i < p.filas; i++) {
                    for (int j = 0; j < p.cols; j++) {
                        if (p.grilla[i][j] == 2) DrawCircle(offsetX + j * cellSize + cellSize/2, offsetY + i * cellSize + cellSize/2, cellSize*0.12f, GOLD);
                    }
                }

                for (int i = 0; i < p.filas; i++) {
                    for (int j = 0; j < p.cols; j++) DrawRectangleLines(offsetX + j * cellSize, offsetY + i * cellSize, cellSize, cellSize, DARKGRAY);
                }

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

                DrawCircle(offsetX + p.pacman.pos.c * cellSize + cellSize/2, offsetY + p.pacman.pos.f * cellSize + cellSize/2, cellSize*0.3f, YELLOW);

                for (int i = 0; i < 4; i++) {
                    if (p.fantasmas[i].activo && p.fantasmas[i].habilitado) {
                        float fx = offsetX + p.fantasmas[i].pos.c * cellSize + cellSize/2;
                        float fy = offsetY + p.fantasmas[i].pos.f * cellSize + cellSize/2;
                        
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
                    }
                }

                // INTERFAZ LATERAL DERECHA
                DrawLineEx((Vector2){545, 0}, (Vector2){545, 600}, 3, BLUE);
                DrawRectangle(548, 0, 252, 600, BLACK);

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
                } 
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
    }

    if (p.estado_juego != 0) LiberarPartida(&p);
    CloseWindow();
    return 0;
}
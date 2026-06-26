#include "logica.h"
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// ==========================================
// --- FUNCIONES AUXILIARES (ESTÁTICAS)   ---
// ==========================================

static bool CasillaOcupadaPorFantasma(Partida *p, Posicion destino) {
    for (int i = 0; i < 4; i++) {
        if (p->fantasmas[i].activo && p->fantasmas[i].habilitado && p->fantasmas[i].pos.f == destino.f && p->fantasmas[i].pos.c == destino.c) {
            return true; 
        }
    }
    return false;
}

static bool TieneLineaDeVision(Partida *p, Posicion fantasma) {
    Posicion pacman = p->pacman.pos;
    if (fantasma.f != pacman.f && fantasma.c != pacman.c) return false;

    if (fantasma.f == pacman.f) {
        int inicio = (fantasma.c < pacman.c) ? fantasma.c : pacman.c;
        int fin = (fantasma.c < pacman.c) ? pacman.c : fantasma.c;
        for (int col = inicio; col < fin; col++) {
            Posicion actual = {fantasma.f, col};
            Posicion siguiente = {fantasma.f, col + 1};
            if (!EsMovimientoValido(p, actual, siguiente)) return false;
        }
    } else {
        int inicio = (fantasma.f < pacman.f) ? fantasma.f : pacman.f;
        int fin = (fantasma.f < pacman.f) ? pacman.f : fantasma.f;
        for (int fila = inicio; fila < fin; fila++) {
            Posicion actual = {fila, fantasma.c};
            Posicion siguiente = {fila + 1, fantasma.c};
            if (!EsMovimientoValido(p, actual, siguiente)) return false;
        }
    }
    return true;
}

// ==========================================
// ---        FUNCIONES DE LA PARTIDA     ---
// ==========================================

void CargarMapaPredeterminado(Partida *p, int id) {
    if (p->grilla != NULL) {
        for(int i = 0; i < p->filas; i++) free(p->grilla[i]);
        free(p->grilla);
    }
    free(p->muros);
    p->muros = NULL;
    p->cant_muros = 0;

    int f = 9, c = 9;
    if (id == 1) { f = 11; c = 11; }
    if (id == 2) { f = 13; c = 13; }

    p->filas = f;
    p->cols = c;
    p->grilla = malloc(f * sizeof(int *));
    for(int i = 0; i < f; i++) {
        p->grilla[i] = malloc(c * sizeof(int));
        for(int j = 0; j < c; j++) p->grilla[i][j] = 0;
    }

    if (id == 0) {
        p->pacman.pos_inicial = (Posicion){4, 4};
        p->fantasmas[0].pos_inicial = (Posicion){0, 0};
        p->fantasmas[1].pos_inicial = (Posicion){0, 8};
        p->fantasmas[2].pos_inicial = (Posicion){8, 0};
        p->fantasmas[3].pos_inicial = (Posicion){8, 8};
        p->grilla[2][2] = 2; p->grilla[2][6] = 2;
        p->grilla[6][2] = 2; p->grilla[6][6] = 2;
        AgregarMuro(p, (Posicion){3,3}, (Posicion){3,4}, -1, -1);
        AgregarMuro(p, (Posicion){3,5}, (Posicion){3,6}, -1, -1);
        AgregarMuro(p, (Posicion){5,3}, (Posicion){5,4}, -1, -1);
        AgregarMuro(p, (Posicion){5,5}, (Posicion){5,6}, -1, -1);
    } else if (id == 1) {
        p->pacman.pos_inicial = (Posicion){5, 5};
        p->fantasmas[0].pos_inicial = (Posicion){0, 0};
        p->fantasmas[1].pos_inicial = (Posicion){0, 10};
        p->fantasmas[2].pos_inicial = (Posicion){10, 0};
        p->fantasmas[3].pos_inicial = (Posicion){10, 10};
        p->grilla[2][2] = 2; p->grilla[2][8] = 2;
        p->grilla[8][2] = 2; p->grilla[8][8] = 2;
        AgregarMuro(p, (Posicion){4,4}, (Posicion){4,5}, -1, -1);
        AgregarMuro(p, (Posicion){4,6}, (Posicion){4,7}, -1, -1);
        AgregarMuro(p, (Posicion){6,4}, (Posicion){6,5}, -1, -1);
        AgregarMuro(p, (Posicion){6,6}, (Posicion){6,7}, -1, -1);
    } else if (id == 2) {
        p->pacman.pos_inicial = (Posicion){6, 6};
        p->fantasmas[0].pos_inicial = (Posicion){0, 0};
        p->fantasmas[1].pos_inicial = (Posicion){0, 12};
        p->fantasmas[2].pos_inicial = (Posicion){12, 0};
        p->fantasmas[3].pos_inicial = (Posicion){12, 12};
        p->grilla[3][3] = 2; p->grilla[3][9] = 2;
        p->grilla[9][3] = 2; p->grilla[9][9] = 2;
        int j;
        for(j=4; j<=8; j++) {
            if (j != 6) {
                AgregarMuro(p, (Posicion){j, 4}, (Posicion){j, 5}, -1, -1);
                AgregarMuro(p, (Posicion){j, 7}, (Posicion){j, 8}, -1, -1);
            }
        }
    }

    p->pacman.pos = p->pacman.pos_inicial;
    for(int i=0; i<4; i++) p->fantasmas[i].pos = p->fantasmas[i].pos_inicial;
    p->bolitas_restantes = 4;
}

void IniciarJuego(Partida *p, bool is_pvp, int dif[4], int m_pac, int m_fant, int dur_muros, int map_id) {
    srand(time(NULL));
    p->modo_juego = 0;
    p->estado_juego = 1;
    p->cazando = false; 
    p->es_pvp = is_pvp;
    p->fantasma_actual_idx = 0;
    p->muros_mano_pacman = m_pac;
    p->muros_mano_fantasmas = m_fant; 
    p->duracion_muros_global = dur_muros;
    p->pacman.vidas = 3;
    p->pacman.acciones = 2;

    if (map_id == 3) {
        if (!CargarMapaParaJugar(p, "mapa_user.dat")) {
            map_id = 0; 
        }
    }
    if (map_id < 3) {
        CargarMapaPredeterminado(p, map_id);
    }

    Color colores[4] = {RED, BLUE, PINK , ORANGE};
    for(int i = 0; i < 4; i++) {
        p->fantasmas[i].color = colores[i];
        if (dif[i] == 0) {
            p->fantasmas[i].habilitado = false;
            p->fantasmas[i].activo = false;
        } else {
            p->fantasmas[i].habilitado = true;
            p->fantasmas[i].activo = true;
            p->fantasmas[i].dificultad = dif[i];
        }
    }
}

bool EsMovimientoValido(Partida *p, Posicion origen, Posicion destino) {
    if (destino.f < 0 || destino.f >= p->filas || destino.c < 0 || destino.c >= p->cols) return false;
    for (int i = 0; i < p->cant_muros; i++) {
        bool cruza_de_c1_a_c2 = (origen.f == p->muros[i].c1.f && origen.c == p->muros[i].c1.c) &&
                               (destino.f == p->muros[i].c2.f && destino.c == p->muros[i].c2.c);
        
        bool cruza_de_c2_a_c1 = (origen.f == p->muros[i].c2.f && origen.c == p->muros[i].c2.c) &&
                               (destino.f == p->muros[i].c1.f && destino.c == p->muros[i].c1.c);
        
        if (cruza_de_c1_a_c2 || cruza_de_c2_a_c1) return false;
    }
    return true;
}

bool MoverPacMan(Partida *p, int dir) {
    if (p->pacman.acciones <= 0 || p->estado_juego != 1) return false;

    Posicion destino = p->pacman.pos;
    if (dir == 0) destino.f--;
    else if (dir == 1) destino.f++;
    else if (dir == 2) destino.c--;
    else if (dir == 3) destino.c++;

    if (EsMovimientoValido(p, p->pacman.pos, destino)) {
        p->pacman.pos = destino;
        p->pacman.acciones--; 
        
        if (p->grilla[destino.f][destino.c] == 2) {
            p->grilla[destino.f][destino.c] = 0;
            p->bolitas_restantes--;
            p->cazando = true; 
            p->pacman.acciones++; 
            if (p->bolitas_restantes <= 0) {
                p->estado_juego = 2;
                return true;
            }
        }
        
        for(int i = 0; i < 4; i++) {
            if (p->fantasmas[i].activo && p->fantasmas[i].habilitado && p->fantasmas[i].pos.f == destino.f && p->fantasmas[i].pos.c == destino.c) {
                if (p->cazando) {
                    p->fantasmas[i].activo = false;
                } else {
                    p->pacman.vidas--;
                    if (p->pacman.vidas <= 0) {
                        p->estado_juego = 3; 
                    } else {
                        p->pacman.pos = p->pacman.pos_inicial;
                        for(int j = 0; j < 4; j++) {
                            if (p->fantasmas[j].habilitado) {
                                p->fantasmas[j].pos = p->fantasmas[j].pos_inicial;
                                p->fantasmas[j].activo = true; 
                            }
                        }
                        return true;
                    }
                }
            }
        }
        return true;
    }
    return false;
}

void CalcularDistancias(Partida *p, int dist[15][15]) {
    for(int i = 0; i < p->filas; i++) {
        for(int j = 0; j < p->cols; j++) dist[i][j] = -1;
    }
    
    Posicion *queue = malloc(p->filas * p->cols * sizeof(Posicion));
    int head = 0, tail = 0;
    Posicion inicio = p->pacman.pos;
    dist[inicio.f][inicio.c] = 0;
    queue[tail++] = inicio;
    int df[] = {-1, 1, 0, 0};
    int dc[] = {0, 0, -1, 1};
    
    while(head < tail) {
        Posicion curr = queue[head++];
        for(int i = 0; i < 4; i++) {
            Posicion sig = {curr.f + df[i], curr.c + dc[i]};
            if (sig.f >= 0 && sig.f < p->filas && sig.c >= 0 && sig.c < p->cols) {
                if (dist[sig.f][sig.c] == -1 && EsMovimientoValido(p, curr, sig)) {
                    dist[sig.f][sig.c] = dist[curr.f][curr.c] + 1;
                    queue[tail++] = sig;
                }
            }
        }
    }
    free(queue);
}

void MoverFantasmas(Partida *p) {
    if (p->estado_juego != 1) return;

    int dist[15][15];
    CalcularDistancias(p, dist);

    int df[] = {-1, 1, 0, 0};
    int dc[] = {0, 0, -1, 1};

    for (int i = 0; i < 4; i++) {
        if (!p->fantasmas[i].activo || !p->fantasmas[i].habilitado) continue;

        int pasos_totales = 1;
        if (!p->cazando && TieneLineaDeVision(p, p->fantasmas[i].pos)) {
            pasos_totales = 2;
        }

        for (int paso = 0; paso < pasos_totales; paso++) {
            if (!p->fantasmas[i].activo || p->estado_juego != 1) break;

            Posicion origen = p->fantasmas[i].pos;
            bool usar_ia = false;
            
            int dif_actual = p->fantasmas[i].dificultad;
            if (dif_actual == 3) usar_ia = true;
            else if (dif_actual == 2) usar_ia = (GetRandomValue(0, 1) == 0);

            Posicion mejor_destino = origen;
            bool movido = false;

            if (usar_ia) {
                if (p->cazando) {
                    int max_dist = -1;
                    for (int d = 0; d < 4; d++) {
                        Posicion vec = {origen.f + df[d], origen.c + dc[d]};
                        if (EsMovimientoValido(p, origen, vec) && !CasillaOcupadaPorFantasma(p, vec)) {
                            int d_pac = dist[vec.f][vec.c];
                            if (d_pac != -1 && d_pac > max_dist) {
                                max_dist = d_pac;
                                mejor_destino = vec;
                                movido = true;
                            }
                        }
                    }
                } else {
                    int min_dist = 9999;
                    for (int d = 0; d < 4; d++) {
                        Posicion vec = {origen.f + df[d], origen.c + dc[d]};
                        if (EsMovimientoValido(p, origen, vec) && !CasillaOcupadaPorFantasma(p, vec)) {
                            int d_pac = dist[vec.f][vec.c];
                            if (d_pac != -1 && d_pac < min_dist) {
                                min_dist = d_pac;
                                mejor_destino = vec;
                                movido = true;
                            }
                        }
                    }
                }
            }

            if (!movido) {
                Posicion validos[4];
                int cant_validos = 0;
                for (int d = 0; d < 4; d++) {
                    Posicion vec = {origen.f + df[d], origen.c + dc[d]};
                    if (EsMovimientoValido(p, origen, vec) && !CasillaOcupadaPorFantasma(p, vec)) {
                        validos[cant_validos++] = vec;
                    }
                }
                if (cant_validos > 0) {
                    mejor_destino = validos[GetRandomValue(0, cant_validos - 1)];
                }
            }

            p->fantasmas[i].pos = mejor_destino;

            if (p->fantasmas[i].pos.f == p->pacman.pos.f && p->fantasmas[i].pos.c == p->pacman.pos.c) {
                if (p->cazando) {
                    p->fantasmas[i].activo = false;
                    break;
                } else {
                    p->pacman.vidas--;
                    if (p->pacman.vidas <= 0) {
                        p->estado_juego = 3; 
                        break;
                    } else {
                        p->pacman.pos = p->pacman.pos_inicial;
                        for(int j = 0; j < 4; j++) {
                            if (p->fantasmas[j].habilitado) {
                                p->fantasmas[j].pos = p->fantasmas[j].pos_inicial;
                                p->fantasmas[j].activo = true; 
                            }
                        }
                        break; 
                    }
                }
            }
        } 
    } 
    AvanzarTurnoGlobal(p);
}

void AgregarMuro(Partida *p, Posicion c1, Posicion c2, int turnos, int propietario) {
    Muro *temp = (Muro *)realloc(p->muros, (p->cant_muros + 1) * sizeof(Muro));
    if (temp != NULL) {
        p->muros = temp;
        p->muros[p->cant_muros].c1 = c1;
        p->muros[p->cant_muros].c2 = c2;
        p->muros[p->cant_muros].turnos_restantes = turnos;
        p->muros[p->cant_muros].propietario = propietario;
        p->cant_muros++;
    }
}

void LiberarPartida(Partida *p) {
    if (p->grilla != NULL) {
        for(int i = 0; i < p->filas; i++) free(p->grilla[i]);
        free(p->grilla);
        p->grilla = NULL;
    }
    if (p->muros != NULL) {
        free(p->muros);
        p->muros = NULL;
    }
}

void AvanzarTurnoGlobal(Partida *p) {
    int nuevos_muros = 0;
    for (int i = 0; i < p->cant_muros; i++) {
        if (p->muros[i].turnos_restantes > 0) {
            p->muros[i].turnos_restantes--;
        }
        
        if (p->muros[i].turnos_restantes != 0) {
            p->muros[nuevos_muros] = p->muros[i];
            nuevos_muros++;
        } else {
            if (p->muros[i].propietario == 0) p->muros_mano_pacman++;
            else if (p->muros[i].propietario == 1) p->muros_mano_fantasmas++;
        }
    }
    p->cant_muros = nuevos_muros;

    if (p->cant_muros > 0) {
        Muro *temp = (Muro *)realloc(p->muros, p->cant_muros * sizeof(Muro));
        if (temp != NULL) p->muros = temp;
    } else {
        free(p->muros);
        p->muros = NULL;
    }

    p->cazando = false; 
    p->pacman.acciones = 2; 
}

bool ControlarFantasmaPvP(Partida *p, int dir, bool quiere_poner_muro, Posicion m1, Posicion m2) {
    if (p->estado_juego != 1) return false;

    int idx = p->fantasma_actual_idx;
    
    int intentos = 0;
    while ((!p->fantasmas[idx].activo || !p->fantasmas[idx].habilitado) && intentos < 4) {
        p->fantasma_actual_idx = (p->fantasma_actual_idx + 1) % 4;
        idx = p->fantasma_actual_idx;
        intentos++;
    }
    if (!p->fantasmas[idx].activo || !p->fantasmas[idx].habilitado) return false;

    if (quiere_poner_muro) {
        if (p->muros_mano_fantasmas > 0) {
            AgregarMuro(p, m1, m2, p->duracion_muros_global, 1);
            p->muros_mano_fantasmas--; 
            p->fantasma_actual_idx = (p->fantasma_actual_idx + 1) % 4;
            while (p->fantasma_actual_idx != 0 && (!p->fantasmas[p->fantasma_actual_idx].activo || !p->fantasmas[p->fantasma_actual_idx].habilitado)) {
                p->fantasma_actual_idx = (p->fantasma_actual_idx + 1) % 4;
            }
            return true;
        }
        return false; 
    }

    Posicion origen = p->fantasmas[idx].pos;
    Posicion destino = origen;

    int pasos_perm = 1;
    if (!p->cazando && TieneLineaDeVision(p, origen)) {
        pasos_perm = 2;
    }

    int df = 0, dc = 0;
    if (dir == 0) df = -1;
    else if (dir == 1) df = 1;
    else if (dir == 2) dc = -1;
    else if (dir == 3) dc = 1;

    for (int paso = 1; paso <= pasos_perm; paso++) {
        Posicion paso_intermedio = {origen.f + (df * paso), origen.c + (dc * paso)};
        
        if (paso_intermedio.f < 0 || paso_intermedio.f >= p->filas || 
            paso_intermedio.c < 0 || paso_intermedio.c >= p->cols) return false;
            
        if (!EsMovimientoValido(p, destino, paso_intermedio)) return false;
        
        destino = paso_intermedio;
    }

    if (CasillaOcupadaPorFantasma(p, destino)) return false;

    p->fantasmas[idx].pos = destino;

    if (destino.f == p->pacman.pos.f && destino.c == p->pacman.pos.c) {
        if (p->cazando) {
            p->fantasmas[idx].activo = false;
        } else {
            p->pacman.vidas--;
            if (p->pacman.vidas <= 0) {
                p->estado_juego = 3; 
            } else {
                p->pacman.pos = p->pacman.pos_inicial;
                for(int j = 0; j < 4; j++) {
                    if (p->fantasmas[j].habilitado) {
                        p->fantasmas[j].pos = p->fantasmas[j].pos_inicial;
                        p->fantasmas[j].activo = true;
                    }
                }
            }
        }
    }

    p->fantasma_actual_idx = (p->fantasma_actual_idx + 1) % 4;
    while (p->fantasma_actual_idx != 0 && (!p->fantasmas[p->fantasma_actual_idx].activo || !p->fantasmas[p->fantasma_actual_idx].habilitado)) {
        p->fantasma_actual_idx = (p->fantasma_actual_idx + 1) % 4;
    }
    return true;
}

bool GuardarMapaPersonalizado(const char *nombreArchivo, MapaGuardado *mapa) {
    FILE *archivo = fopen(nombreArchivo, "wb"); 
    if (archivo == NULL) return false;

    fwrite(mapa, sizeof(MapaGuardado), 1, archivo);
    fclose(archivo);
    return true;
}

bool CargarMapaParaJugar(Partida *p, const char *nombreArchivo) {
    FILE *archivo = fopen(nombreArchivo, "rb"); 
    if (archivo == NULL) return false;

    MapaGuardado mapa;
    fread(&mapa, sizeof(MapaGuardado), 1, archivo);
    fclose(archivo);

    int filasViejas = p->filas;
    
    if (p->grilla != NULL) {
        for(int i = 0; i < filasViejas; i++) free(p->grilla[i]);
        free(p->grilla);
    }

    p->filas = mapa.filas;
    p->cols = mapa.cols;

    p->grilla = malloc(p->filas * sizeof(int *));
    for(int i = 0; i < p->filas; i++) {
        p->grilla[i] = malloc(p->cols * sizeof(int));
        for(int j = 0; j < p->cols; j++) {
            p->grilla[i][j] = mapa.grilla[i][j];
        }
    }

    free(p->muros);
    p->cant_muros = mapa.cant_muros;
    if (p->cant_muros > 0) {
        p->muros = malloc(p->cant_muros * sizeof(Muro));
        for(int i = 0; i < p->cant_muros; i++) {
            p->muros[i] = mapa.muros[i];
        }
    } else p->muros = NULL;

    p->bolitas_restantes = 0;
    for(int i = 0; i < p->filas; i++) {
        for(int j = 0; j < p->cols; j++) {
            if (p->grilla[i][j] == 2) p->bolitas_restantes++;
        }
    }

    p->pacman.pos_inicial = mapa.pos_pacman;
    p->pacman.pos = mapa.pos_pacman;
    for(int i=0; i<4; i++) {
        p->fantasmas[i].pos_inicial = mapa.pos_fantasmas[i];
        p->fantasmas[i].pos = mapa.pos_fantasmas[i];
    }

    return true;
}
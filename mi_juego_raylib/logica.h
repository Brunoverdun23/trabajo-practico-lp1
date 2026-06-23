#ifndef LOGICA_H
#define LOGICA_H

#include "raylib.h"
#include <stdbool.h>
#include <stdlib.h>

typedef struct { int f, c; } Posicion;

typedef struct {
    Posicion pos;
    Posicion pos_inicial;
    int vidas, acciones;
} PacMan;

typedef struct {
    Posicion pos;
    Posicion pos_inicial;
    Color color;
    bool activo;
    bool habilitado;
    int dificultad;
} Fantasma;

typedef struct {
    Posicion c1, c2;
    int turnos_restantes; 
    int propietario; // 0: Pac-Man, 1: Fantasmas, -1: Fijo
} Muro;

typedef struct {
    PacMan pacman;
    Fantasma fantasmas[4];
    Muro *muros;            
    int cant_muros;         
    int **grilla;          
    int filas, cols;
    int modo_juego;        
    int bolitas_restantes; 
    int estado_juego;      
    bool cazando;   
    int fantasma_actual_idx;   
    int muros_mano_pacman;
    int muros_mano_fantasmas;  
    int duracion_muros_global;
    bool es_pvp;
} Partida;

#define MAX_MUROS_MAPA 50

typedef struct {
    int filas;
    int cols;
    int grilla[15][15]; 
    int cant_muros;
    Muro muros[MAX_MUROS_MAPA];
    Posicion pos_pacman;
    Posicion pos_fantasmas[4];
    char nombre[32];
} MapaGuardado;

// --- PROTOTIPOS ---
void IniciarJuego(Partida *p, bool is_pvp, int dif[4], int m_pac, int m_fant, int dur_muros, int map_id);
void CargarMapaPredeterminado(Partida *p, int id);
bool CargarMapaParaJugar(Partida *p, const char *nombreArchivo);
bool GuardarMapaPersonalizado(const char *nombreArchivo, MapaGuardado *mapa);

void LiberarPartida(Partida *p);
bool MoverPacMan(Partida *p, int dir);
void MoverFantasmas(Partida *p); 
bool ControlarFantasmaPvP(Partida *p, int dir, bool quiere_poner_muro, Posicion m1, Posicion m2);
void AvanzarTurnoGlobal(Partida *p);
bool EsMovimientoValido(Partida *p, Posicion origen, Posicion destino);
void AgregarMuro(Partida *p, Posicion c1, Posicion c2, int turnos, int propietario);

#endif
#ifndef OBSTACULOS_H
#define OBSTACULOS_H

#include <allegro5/allegro5.h>
#include "personagem.h"

typedef struct {
    float x;
    float y;
    float w;
    float h;
} Espinho;

typedef struct {
    float x;
    float y;
    float w;
    float h;
} Montanha;

typedef struct {
    float x;
    float y;
    float w;
    float h;
} HitboxMontanha;



void desenha_espinhos(ALLEGRO_BITMAP *sprite, Espinho *e);
int colisao_espinho(Espinho *e, personagem *j);  // retorna 1 se colidiu

void desenha_montanha(ALLEGRO_BITMAP *sprite, Montanha *m);
void resolve_colisao_montanhas(personagem *j, HitboxMontanha hitboxes[], int n);

#endif

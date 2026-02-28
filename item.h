#ifndef ITEM_H
#define ITEM_H

#include <allegro5/allegro5.h>
#include "personagem.h"

typedef struct
{
    float x;
    float y;
    float w;
    float h;
    int ativo;
} ItemVida;

typedef struct {
    float x;
    float y;
    float w;
    float h;
    int ativo;
} ItemVitoria;


void desenha_item_vida(ALLEGRO_BITMAP *sprite, ItemVida *it);
int colisao_item_vida(ItemVida *it, personagem *j);
void reset_itens_vida(ItemVida itens[], int n);
int colisao_item_vitoria(ItemVitoria *it, personagem *j);

#endif

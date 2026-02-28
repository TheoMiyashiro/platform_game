#include "item.h"

void desenha_item_vida(ALLEGRO_BITMAP *sprite, ItemVida *it)
{
    if (!it->ativo)
        return;

    al_draw_bitmap(sprite, it->x, it->y, 0);
}

int colisao_item_vida(ItemVida *it, personagem *j)
{
    if (!it->ativo)
        return 0;

    float px1 = j->x;
    float py1 = j->y;
    float px2 = j->x + j->side;
    float py2 = j->y + j->side;

    float ix1 = it->x;
    float iy1 = it->y;
    float ix2 = it->x + it->w;
    float iy2 = it->y + it->h;

    if (px1 < ix2 && px2 > ix1 && py1 < iy2 && py2 > iy1)
        return 1;

    return 0;
}

void reset_itens_vida(ItemVida itens[], int n)
{
    for (int i = 0; i < n; i++)
        itens[i].ativo = 1;
}

int colisao_item_vitoria(ItemVitoria *it, personagem *j)
{
    if (!it->ativo)
        return 0;

    float px1 = j->x;
    float py1 = j->y;
    float px2 = j->x + j->side;
    float py2 = j->y + j->side;

    float ix1 = it->x;
    float iy1 = it->y;
    float ix2 = it->x + it->w;
    float iy2 = it->y + it->h;

    if (px1 < ix2 && px2 > ix1 && py1 < iy2 && py2 > iy1)
        return 1;

    return 0;
}
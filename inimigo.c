#include "inimigo.h"

#define INIMIGO_SPRITE_W 47
#define INIMIGO_SPRITE_H 47

Inimigo cria_inimigo(float x, float y, float limite_esq, float limite_dir, float velocidade)
{
    Inimigo i;

    i.x = x;
    i.y = y;
    i.w = INIMIGO_SPRITE_W;
    i.h = INIMIGO_SPRITE_H;

    i.limite_esq = limite_esq;
    i.limite_dir = limite_dir;
    i.velocidade = velocidade;

    i.direcao = 0; /* começa indo para a direita */
    i.frame = 0;
    i.anim_counter = 0;

    return i;
}

void atualiza_inimigo(Inimigo *i)
{
    /* movimento de patrulha esquerda <-> direita */
    if (i->direcao == 0) /* direita */
    {
        i->x += i->velocidade;
        if (i->x + i->w >= i->limite_dir)
        {
            i->x = i->limite_dir - i->w;
            i->direcao = 1; /* vira para esquerda */
        }
    }
    else /* esquerda */
    {
        i->x -= i->velocidade;
        if (i->x <= i->limite_esq)
        {
            i->x = i->limite_esq;
            i->direcao = 0; /* vira para direita */
        }
    }

    /* animação com 3 quadros */
    i->anim_counter++;
    if (i->anim_counter >= 7)  /* controla velocidade da animação */
    {
        i->anim_counter = 0;
        i->frame++;
        if (i->frame >= 3)
            i->frame = 0;
    }
}

void desenha_inimigo(ALLEGRO_BITMAP *sheet, Inimigo *i)
{
    int frame = i->frame;

    int direcao = (i->direcao == 0 ? 1 : 0);
    
    int sx = frame * INIMIGO_SPRITE_W;
    int sy = direcao * INIMIGO_SPRITE_H;

    al_draw_bitmap_region(sheet, sx, sy,  INIMIGO_SPRITE_W, INIMIGO_SPRITE_H,  i->x, i->y,  0);
}

int colisao_inimigo(Inimigo *i, personagem *j)
{
    float px1 = j->x;
    float py1 = j->y;
    float px2 = j->x + j->side;
    float py2 = j->y + j->side;

    float ex1 = i->x;
    float ey1 = i->y;
    float ex2 = i->x + i->w;
    float ey2 = i->y + i->h;

    if (px1 < ex2 && px2 > ex1 && py1 < ey2 && py2 > ey1)
        return 1;

    return 0;
}

#include "obstaculos.h"
#include <allegro5/allegro_primitives.h> 

void desenha_espinhos(ALLEGRO_BITMAP *sprite, Espinho *e)
{
    const int TILE = 32;  // tamanho do sprite daquele espinho
    int tiles = e->w / TILE;

    for (int i = 0; i < tiles; i++)
    {
        al_draw_bitmap(sprite, e->x + i * TILE, e->y, 0);
    }
}

int colisao_espinho(Espinho *e, personagem *j)
{
    float px1 = j->x;
    float py1 = j->y;
    float px2 = j->x + j->side;
    float py2 = j->y + j->side;

    float ex1 = e->x;
    float ey1 = e->y;
    float ex2 = e->x + e->w;
    float ey2 = e->y + e->h;

    if (px1 < ex2 && px2 > ex1 && py1 < ey2 && py2 > ey1)
        return 1;

    return 0;
}

void desenha_montanha(ALLEGRO_BITMAP *sprite, Montanha *m)
{
    float sw = (float) al_get_bitmap_width(sprite);
    float sh = (float) al_get_bitmap_height(sprite);

    al_draw_scaled_bitmap(sprite, 0, 0, sw, sh, m->x, m->y, m->w, m->h, 0);
}



void resolve_colisao_montanhas(personagem *j,  HitboxMontanha hitboxes[], int n)
{
    float w = j->side;
    float h = j->side;

    for (int k = 0; k < n; k++)
    {
        HitboxMontanha *m = &hitboxes[k];

        float px1 = j->x;
        float py1 = j->y;
        float px2 = j->x + w;
        float py2 = j->y + h;

        float mx1 = m->x;
        float my1 = m->y;
        float mx2 = m->x + m->w;
        float my2 = m->y + m->h;

        // se nao colidiu, passa para o proximo
        if (!(px1 < mx2 && px2 > mx1 && py1 < my2 && py2 > my1))
            continue;

        // quanto o jogador invadiu cada lado
        float overlap_left   = px2 - mx1;
        float overlap_right  = mx2 - px1;
        float overlap_top    = py2 - my1;
        float overlap_bottom = my2 - py1;

        float menor = overlap_left;
        int lado = 0;

        if (overlap_right < menor)   { menor = overlap_right;  lado = 1; }
        if (overlap_top   < menor)   { menor = overlap_top;    lado = 2; }
        if (overlap_bottom < menor)  { menor = overlap_bottom; lado = 3; }

        // esquerda
        if (lado == 0)
        {
            j->x = mx1 - w;
            if (j->vx > 0) j->vx = 0;
            j->escalando = true;
            j->wall_dir = -1;
        }

        // direita
        else if (lado == 1)
        {
            j->x = mx2;
            if (j->vx < 0) j->vx = 0;
            j->escalando = true;
            j->wall_dir = 1;
        }

        // topo (caiu em cima da montanha)
        else if (lado == 2)
        {
            j->y = my1 - h;
            j->vy = 0;
            j->no_chao = true;
            j->escalando = false;
            j->wall_dir = 0;
        }

        // base (bateu a cabeça)
        else if (lado == 3)
        {
            j->y = my2;
            if (j->vy < 0) j->vy = 0;  // impede de subir atraves da montanha
        }
    }
}


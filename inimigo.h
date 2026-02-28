#ifndef __INIMIGO__
#define __INIMIGO__

#include <allegro5/allegro5.h>
#include "personagem.h"

typedef struct
{
    float x;
    float y;
    float w;
    float h;

    int direcao;       
    float velocidade;

    float limite_esq;  // ate onde ele anda pra esq
    float limite_dir;  // ate onde ele anda pra dir
    int frame;         //quadro de animaçao
    int anim_counter;  // contador para trocar de quadro
} Inimigo;

// cria um inimigo que anda entre limite_esq e limite_dir, começando na posição x,y
Inimigo cria_inimigo(float x, float y, float limite_esq, float limite_dir, float velocidade);

// atualiza movimento + animação
void atualiza_inimigo(Inimigo *i);

// desenha o inimigo a partir do spritesheet
void desenha_inimigo(ALLEGRO_BITMAP *sheet, Inimigo *i);

// retorna 1 se houve colisão com o jogador, 0 caso contrário
int colisao_inimigo(Inimigo *i, personagem *j);

#endif

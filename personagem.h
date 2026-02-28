#ifndef __PERSONAGEM__
#define __PERSONAGEM__

#include "joystick.h"
#include <stdbool.h>

#define STEP 10

typedef struct
{
    unsigned char side;
    unsigned char direcao;
    float x;
    float y;
    joystick *control;

    float vy; //velocidade vertical
    bool no_chao; //esta encostado no chao?
    float vx; //velocidade horizontal
    bool dashing; //esta dashando?
    int dash_timer; //quantos frames ainda duram o dash
    int vida; //vida atual
    int vida_max; // vida maxima
    bool invencivel; // fica invencivel por um periodo apos tomar dano
    int inv_timer; //periodo de invencibilidade
    bool knockback;   // esta sofrendo kb?
    int knock_timer;   //quantos frames faltam do kb
    bool escalando;   // esta grudado na parede
    int wall_dir;     // -1 parede na esquerda, +1 parede na direita


} personagem;

personagem* cria_personagem(unsigned char side, unsigned char direcao, unsigned short x, unsigned short y, unsigned short max_x, unsigned short max_y);
void movimento_persongem(personagem *element, char steps, unsigned char trajectory, unsigned short max_x, unsigned short max_y);
void dano_personagem(personagem *j, int dano);
void destroi_personagem(personagem *element);		
void reset_jogador(personagem *j, float inicio_x, float inicio_y);


#endif
#include <stdlib.h>
#include "personagem.h"

personagem* cria_personagem(unsigned char side, unsigned char direcao, unsigned short x, unsigned short y, unsigned short max_x, unsigned short max_y)
{
	if ((x - side/2 < 0) || (x + side/2 > max_x) || (y - side/2 < 0) || (y + side/2 > max_y)) return NULL;	

	personagem *novo_personagem = (personagem*) malloc(sizeof(personagem));		
	novo_personagem->side = side;				
	novo_personagem->x = x;					
	novo_personagem->y = y;
    novo_personagem->direcao = direcao;
	novo_personagem->vy = 0.0f;
	novo_personagem->no_chao = false;
    novo_personagem->control = joystick_create();
	novo_personagem->dash_timer = 0;
	novo_personagem->vx = 0.0f;
	novo_personagem->dashing = false;
	novo_personagem->vida_max = 5;
	novo_personagem->vida = 5;
	novo_personagem->invencivel = false;
	novo_personagem->inv_timer = 0;
	novo_personagem->knockback = false;
	novo_personagem->knock_timer = 0;
	novo_personagem->escalando = false;
	novo_personagem->wall_dir = 0;

	
	return novo_personagem;						
}

void movimento_persongem(personagem *element, char steps, unsigned char trajectory, unsigned short max_x, unsigned short max_y)
{
    if (!trajectory){ if ((element->x - steps*STEP) - element->side/2 >= 0) element->x = element->x - steps*STEP;} 				
	else if (trajectory == 1){ if ((element->x + steps*STEP) + element->side/2 <= max_x) element->x = element->x + steps*STEP;}	
	else if (trajectory == 2){ if ((element->y - steps*STEP) - element->side/2 >= 0) element->y = element->y - steps*STEP;}		
	else if (trajectory == 3){ if ((element->y + steps*STEP) + element->side/2 <= max_y) element->y = element->y + steps*STEP;}	
}

void dano_personagem(personagem *j, int dano)
{
	if (j->invencivel)
		return;

	j->vida -= dano;

	if (j->vida < 0)
		j->vida = 0;
	
	j->invencivel = true;
	j->inv_timer = 30;

	int dir = (j->direcao == 0) ? -1 : 1;

    j->knockback = true;
    j->knock_timer = 10;      
    j->vx = 10.0f * dir; 
    j->vy = -8.0f; 
	
}

void destroi_personagem(personagem *element)
{
    joystick_destroy(element->control);
    free(element);
}



void reset_jogador(personagem *j, float inicio_x, float inicio_y)
{
    j->x = inicio_x;
    j->y = inicio_y;

    j->vx = 0.0f;
    j->vy = 0.0f;

    j->no_chao = false;
    j->dashing = false;
    j->dash_timer = 0;

    j->invencivel = false;
    j->inv_timer = 0;

    j->knockback = false;
    j->knock_timer = 0;

    j->vida = j->vida_max;

    j->control->left = false;
    j->control->right = false;
    j->control->down = false;
    j->control->up = false;

	j->escalando = false;
	j->wall_dir = 0;

}


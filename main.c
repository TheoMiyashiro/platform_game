#include <stdio.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include "personagem.h"
#include "joystick.h"
#include <stdbool.h>
#include "obstaculos.h"
#include "inimigo.h"
#include "item.h"
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>



#define X_SCREEN 1280
#define Y_SCREEN 720
#define CHAO_Y (Y_SCREEN - 80)
#define GRAVIDADE   1.0f
#define PULO        15.0f

#define ZOOM_NORMAL 1.5f
#define ZOOM_DEATH  4.0f  
#define DEATH_FRAMES 50   

#define DASH_SPEED  25.0f
#define DASH_FRAMES 5

#define PLATFORM_W 96
#define PLATFORM_H 24

typedef enum
{
    STATE_MENU,
    STATE_GAME,
    STATE_DYING,
    STATE_GAMEOVER,
    STATE_OPTIONS,
    STATE_WINNING,
    STATE_WINSCREEN 

} GameState;

typedef struct
{
    float x;
    float y;
    float w;
    float h;
} Plataforma;

//tamanho
int BG_W = X_SCREEN;
int BG_H = Y_SCREEN;

//musica
ALLEGRO_SAMPLE *mus_menu = NULL;
ALLEGRO_SAMPLE *mus_fase = NULL;

ALLEGRO_SAMPLE_INSTANCE *inst_menu = NULL;
ALLEGRO_SAMPLE_INSTANCE *inst_fase = NULL;

float volume_musica = 1.0f; 

void fastfall(personagem *j)
{
    j->vy += GRAVIDADE * 1.5f;
}

void atualiza_camera(personagem *jogador, float *cam_x, float *cam_y, float zoom)
{
    float alvo_x = jogador->x + jogador->side / 2.0f;
    float alvo_y = jogador->y + jogador->side / 2.0f;

    float visivel_w = X_SCREEN / zoom;
    float visivel_h = Y_SCREEN / zoom;

    *cam_x = alvo_x - visivel_w / 2.0f;
    *cam_y = alvo_y - visivel_h / 1.8f;

    if (*cam_x < 0) *cam_x = 0;
    if (*cam_y < 0) *cam_y = 0;
}

void desenha_sprite(ALLEGRO_BITMAP *sheet, int frame, int direcao, float x, float y, bool invencivel)
{
    const int SPRITE_W = 47;
    const int SPRITE_H = 47;

    int sx = frame * SPRITE_W;
    int sy = direcao * SPRITE_H;

    if (invencivel)
    {
        al_draw_tinted_bitmap_region(sheet, al_map_rgba(255, 80, 80, 255), sx, sy, SPRITE_W, SPRITE_H, x, y, 0);
    }
    else
    {
        al_draw_bitmap_region(sheet, sx, sy, SPRITE_W, SPRITE_H, x, y, 0);
    }
}

void update_position(personagem* jogador, HitboxMontanha hitboxes[], int num_hitboxes)
{
    // knockback
    if (jogador->knockback)
    {
        jogador->x += jogador->vx;

        if (jogador->x < 0)
            jogador->x = 0;

        if (jogador->x + jogador->side > BG_W)
            jogador->x = BG_W - jogador->side;

        jogador->knock_timer--;

        if (jogador->knock_timer <= 0)
        {
            jogador->knockback = false;
            jogador->vx = 0.0f;
        }

        return; // ignora controles durante kb
    }

    // dash
    if (jogador->dashing)
    {
        jogador->x += jogador->vx;

        if (jogador->x < 0)
            jogador->x = 0;

        if (jogador->x + jogador->side > BG_W)
            jogador->x = BG_W - jogador->side;

        jogador->dash_timer--;

        if (jogador->dash_timer <= 0)
        {
            jogador->dashing = false;
            jogador->vx = 0.0f;
        }
        return;
    }

    // escalar parede
    if (jogador->escalando && !jogador->no_chao)
    {
        // sobe
        if (jogador->control->up)
        {
            jogador->y -= 4.0f;
            jogador->vy = 0.0f;
        }

        // desce
        if (jogador->control->down)
        {
            jogador->y += 3.0f;
            jogador->vy = 0.0f;
        }
    }

    // movimento normal
    if (jogador->control->left)
    {
        jogador->direcao = 1;
        movimento_persongem(jogador, 1, 0, BG_W, Y_SCREEN);
    }
    if (jogador->control->right)
    {
        jogador->direcao = 0;
        movimento_persongem(jogador, 1, 1, BG_W, Y_SCREEN);
    }

    // limites da tela
    if (jogador->x < 0)
        jogador->x = 0;
    if (jogador->x + jogador->side > BG_W)
        jogador->x = BG_W - jogador->side;

    if (!jogador->knockback && !jogador->dashing)
    {
        bool encostado = false;

        for (int i = 0; i < num_hitboxes; i++)
        {
            HitboxMontanha *m = &hitboxes[i];

            if (jogador->x + jogador->side == m->x)
                encostado = true;

            if (jogador->x == m->x + m->w)
                encostado = true;
        }

        if (!encostado)
        {
            jogador->escalando = false;
            jogador->wall_dir = 0;
        }
    }
}


void aplica_gravidade_plataformas(personagem* j, Plataforma plataformas[], int n_plat)
{
    // gravidade reduzida ao escalar
    if (j->escalando && !j->no_chao)
        j->vy += GRAVIDADE * 0.2f;
    else
        j->vy += GRAVIDADE;

    float old_y = j->y;
    j->y += j->vy;

    j->no_chao = false;

    float sprite_h = 47.0f;
    float player_bottom_old = old_y + sprite_h;
    float player_bottom_new = j->y + sprite_h;
    float player_left       = j->x;
    float player_right      = j->x + 47.0f;

    if (j->vy > 0.0f)
    {
        for (int i = 0; i < n_plat; i++)
        {
            Plataforma *p = &plataformas[i];

            float plat_top    = p->y;
            float plat_left   = p->x;
            float plat_right  = p->x + p->w;

            bool cruzou_topo = (player_bottom_old <= plat_top && player_bottom_new >= plat_top);
            bool sobre_x = (player_right > plat_left && player_left  <  plat_right);

            if (cruzou_topo && sobre_x)
            {
                j->y  = plat_top - sprite_h;
                j->vy = 0.0f;
                j->no_chao = true;
                j->escalando = false;
                j->wall_dir = 0;
                return;
            }
        }
    }

    if (j->y + sprite_h >= CHAO_Y)
    {
        j->y  = CHAO_Y - sprite_h;
        j->vy = 0.0f;
        j->no_chao = true;
        j->escalando = false;
        j->wall_dir = 0;
    }
}

void desenha_vida(ALLEGRO_BITMAP *spr_vida, personagem *j)
{
    ALLEGRO_TRANSFORM old;
    al_copy_transform(&old, al_get_current_transform());

    ALLEGRO_TRANSFORM hud;
    al_identity_transform(&hud);
    al_use_transform(&hud);

    int w = al_get_bitmap_width(spr_vida);

    for (int i = 0; i < j->vida; i++)
    {
        al_draw_bitmap(spr_vida, 10 + i * (w + 4), 10, 0);
    }

    al_use_transform(&old);
}

void desenha_menu(ALLEGRO_BITMAP *menu, ALLEGRO_BITMAP *btn_start, ALLEGRO_BITMAP *btn_quit, int menu_opcao)
{
    ALLEGRO_TRANSFORM t;
    al_identity_transform(&t);
    al_use_transform(&t);

    if (menu)
        al_draw_bitmap(menu, 0, 0, 0);
    else
        al_clear_to_color(al_map_rgb(20, 20, 20));

    int w_start = al_get_bitmap_width(btn_start);
    int h_start = al_get_bitmap_height(btn_start);

    int w_quit  = al_get_bitmap_width(btn_quit);
    int h_quit  = al_get_bitmap_height(btn_quit);

    float cx = X_SCREEN / 2.0f;

    float y_start = Y_SCREEN / 2.0f - 40;
    float y_quit  = Y_SCREEN / 2.0f + 40;

    if (menu_opcao == 0) {
        float scale = 1.1f;
        float dw = w_start * scale;
        float dh = h_start * scale;
        al_draw_scaled_bitmap(btn_start, 0, 0, w_start, h_start, cx - dw / 2, y_start - (dh - h_start) / 2, dw, dh, 0);
    } else {
        al_draw_bitmap(btn_start,  cx - w_start / 2, y_start, 0);
    }

    if (menu_opcao == 1) {
        float scale = 1.1f;
        float dw = w_quit * scale;
        float dh = h_quit * scale;
        al_draw_scaled_bitmap(btn_quit, 0, 0, w_quit, h_quit, cx - dw / 2, y_quit - (dh - h_quit) / 2, dw, dh, 0);
    } else {
        al_draw_bitmap(btn_quit, cx - w_quit / 2, y_quit, 0);
    }

    ALLEGRO_FONT *font = al_create_builtin_font();
    al_draw_text(font, al_map_rgb(255, 255, 255), X_SCREEN / 2, Y_SCREEN - 80, ALLEGRO_ALIGN_CENTER, "Aperte O para configuracoes");
    al_destroy_font(font);
}

void desenha_gameover(ALLEGRO_BITMAP *gameover)
{
    ALLEGRO_TRANSFORM t;
    al_identity_transform(&t);
    al_use_transform(&t);

    if (gameover)
        al_draw_bitmap(gameover, 0, 0, 0);
    else
        al_clear_to_color(al_map_rgb(0, 0, 0));

    al_draw_textf(al_create_builtin_font(), al_map_rgb(255, 255, 255),  X_SCREEN/2, Y_SCREEN - 100, ALLEGRO_ALIGN_CENTER, "Pressione ENTER para voltar ao menu");
}

void atualiza_volume()
{
    if (inst_menu)
        al_set_sample_instance_gain(inst_menu, volume_musica);
    if (inst_fase)
        al_set_sample_instance_gain(inst_fase, volume_musica);
}

void toca_menu()
{
    if (inst_fase)
        al_stop_sample_instance(inst_fase);
    if (inst_menu)
    {
        al_set_sample_instance_playmode(inst_menu, ALLEGRO_PLAYMODE_LOOP);
        atualiza_volume();
        al_play_sample_instance(inst_menu);
    }
}

void toca_fase()
{
    if (inst_menu)
        al_stop_sample_instance(inst_menu);
    if (inst_fase)
    {
        al_set_sample_instance_playmode(inst_fase, ALLEGRO_PLAYMODE_LOOP);
        atualiza_volume();
        al_play_sample_instance(inst_fase);
    }
}

void desenha_options(ALLEGRO_BITMAP *img_options)
{
    ALLEGRO_TRANSFORM t;
    al_identity_transform(&t);
    al_use_transform(&t);

    // desenha o fundo da imagem de options
    if (img_options)
        al_draw_bitmap(img_options, 0, 0, 0);
    else
        al_clear_to_color(al_map_rgb(10, 10, 30));

    ALLEGRO_FONT *font = al_create_builtin_font();

    // --- barra de volume ---
    int bar_x = X_SCREEN/2 - 200;
    int bar_y = Y_SCREEN/2 + 80;    
    int bar_w = 400;
    int bar_h = 20;

    al_draw_rectangle(bar_x, bar_y, bar_x + bar_w, bar_y + bar_h,  al_map_rgb(200,200,200), 2);

    int fill_w = (int)(bar_w * volume_musica);
    al_draw_filled_rectangle(bar_x, bar_y, bar_x + fill_w, bar_y + bar_h,al_map_rgb(100, 200, 100));

    // texto de % do volume
    char txt[64];
    snprintf(txt, sizeof(txt), "Volume: %d%%", (int)(volume_musica * 100));

    al_draw_text(font, al_map_rgb(255,255,255), X_SCREEN/2, bar_y + 40, ALLEGRO_ALIGN_CENTER, txt);

    // instrução de controle
    al_draw_text(font, al_map_rgb(200,200,200), X_SCREEN/2, Y_SCREEN - 80, ALLEGRO_ALIGN_CENTER, "A/D ou LEFT/RIGHT ajustam   ESC volta");

    al_destroy_font(font);
}

int main()
{
    al_init();
    al_init_primitives_addon();
    al_init_image_addon();
    al_install_keyboard();

    // audio
    al_install_audio();
    al_init_acodec_addon();
    al_reserve_samples(16);

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 30.0);
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    ALLEGRO_FONT* font = al_create_builtin_font();
    ALLEGRO_DISPLAY* disp = al_create_display(X_SCREEN, Y_SCREEN);

    ALLEGRO_BITMAP *sheet = al_load_bitmap("sprites/spritepersonagem.png");
    if (!sheet)
    {
        printf("Erro ao carregar spritesheet!\n");
        return 1;
    }

    ALLEGRO_BITMAP *spr_plataforma = al_load_bitmap("sprites/plataforma.png");
    if (!spr_plataforma)
    {
        printf("Erro ao carregar sprite da plataforma!\n");
        return 1;
    }

    ALLEGRO_BITMAP *spr_vida = al_load_bitmap("sprites/vida.png");
    if (!spr_vida)
    {
        printf("Erro ao carregar vida.png!\n");
        return 1;
    }

    ALLEGRO_BITMAP *bg = al_load_bitmap("sprites/background.jpg");
    if (!bg)
    {
        printf("Erro ao carregar background.jpg!\n");
        return 1;
    }

    ALLEGRO_BITMAP *menu_bg = al_load_bitmap("sprites/menu.jpg");
    if (!menu_bg) {
        printf("Erro ao carregar menu.jpg!\n");
        return 1;
    }

    ALLEGRO_BITMAP *btn_start = al_load_bitmap("sprites/start.png");
    if (!btn_start)
    {
        printf("Erro ao carregar start.png!\n");
        return 1;
    }

    ALLEGRO_BITMAP *btn_quit = al_load_bitmap("sprites/quit.png");
    if (!btn_quit)
    {
        printf("Erro ao carregar quit.png!\n");
        return 1;
    }

    ALLEGRO_BITMAP *spr_espinho = al_load_bitmap("sprites/espinhos.png");
    if (!spr_espinho)
    {
        printf("Erro ao carregar espinhos.png!\n");
        return 1;
    }

    ALLEGRO_BITMAP *gameover_bg = al_load_bitmap("sprites/gameover.jpg");
    if (!gameover_bg) {
        printf("Erro ao carregar gameover.jpg!\n");
        return 1;
    }

    ALLEGRO_BITMAP *spr_inimigo = al_load_bitmap("sprites/inimigo.png");
    if (!spr_inimigo)
    {
        printf("Erro ao carregar inimigo.png!\n");
        return 1;
    }

    ALLEGRO_BITMAP *spr_montanha = al_load_bitmap("sprites/montanha.png");
    if (!spr_montanha)
    {
        printf("Erro ao carregar montanha.png!\n");
        return 1;
    }

    ALLEGRO_BITMAP *spr_inimigo2 = al_load_bitmap("sprites/inimigo2.png");
    if (!spr_inimigo2)
    {
        printf("Erro ao carregar inimigo2.png!\n");
        return 1;
    }

    ALLEGRO_BITMAP *spr_inimigo3 = al_load_bitmap("sprites/inimigo3.png");
    if (!spr_inimigo3)
    {
        printf("Erro ao carregar inimigo3.png!\n");
        return 1;
    }

    ALLEGRO_BITMAP *spr_inimigo4 = al_load_bitmap("sprites/inimigo4.png");
    if (!spr_inimigo4)
    {
        printf("Erro ao carregar inimigo4.png!\n");
        return 1;
    }

    ALLEGRO_BITMAP *spr_inimigo5 = al_load_bitmap("sprites/inimigo5.png");
    if (!spr_inimigo5)
    {
        printf("Erro ao carregar inimigo5.png!\n");
        return 1;
    }

    ALLEGRO_BITMAP *spr_item_vida = al_load_bitmap("sprites/itemvida.png");
    if (!spr_item_vida)
    {
        printf("Erro ao carregar itemvida.png!\n");
        return 1;
    }

    mus_menu = al_load_sample("audio/winner.ogg");
    if (!mus_menu)
    {
        printf("Erro ao carregar audio/menu.ogg\n");
        return 1;
    }

    mus_fase = al_load_sample("audio/winner.ogg");
    if (!mus_fase)
    {
        printf("Erro ao carregar audio/fase.ogg\n");
        return 1;
    }

    ALLEGRO_BITMAP *spr_vitoria = al_load_bitmap("sprites/cenoura.png");
    if (!spr_vitoria) {
        printf("Erro ao carregar cenoura.png!\n");
        return 1;
    }

    ALLEGRO_BITMAP *win_bg = al_load_bitmap("sprites/win.jpg");
    if (!win_bg) {
        printf("Erro ao carregar win.jpg!\n");
        return 1;
    }





    // cria instancias para poder controlar volume
    inst_menu = al_create_sample_instance(mus_menu);
    inst_fase = al_create_sample_instance(mus_fase);

    al_attach_sample_instance_to_mixer(inst_menu, al_get_default_mixer());
    al_attach_sample_instance_to_mixer(inst_fase, al_get_default_mixer());

    atualiza_volume();


    BG_W = al_get_bitmap_width(bg);
    BG_H = al_get_bitmap_height(bg);

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));

    personagem* jogador = cria_personagem(20, 0, 10, Y_SCREEN / 2, BG_W, Y_SCREEN);
    if (!jogador) return 1;

    Plataforma plataformas[] =
    {
        {400,  CHAO_Y - 100, PLATFORM_W, PLATFORM_H},
        {600,  CHAO_Y - 150, PLATFORM_W, PLATFORM_H},
        {1000,  CHAO_Y - 200, PLATFORM_W, PLATFORM_H},
        {1200, CHAO_Y - 200, PLATFORM_W, PLATFORM_H},
        {1400, CHAO_Y - 300, PLATFORM_W * 2, PLATFORM_H},
        {1600,  CHAO_Y - 350, PLATFORM_W, PLATFORM_H},
        {2400,  CHAO_Y - 350, PLATFORM_W, PLATFORM_H},
        {2800,  CHAO_Y - 350, PLATFORM_W, PLATFORM_H},
        {3000,  CHAO_Y - 350, PLATFORM_W, PLATFORM_H},
        {3200,  CHAO_Y - 350, PLATFORM_W, PLATFORM_H},
        {2800,  CHAO_Y - 100, PLATFORM_W, PLATFORM_H},
        {2500,  CHAO_Y - 100, PLATFORM_W, PLATFORM_H},
        {4100,  CHAO_Y - 150, PLATFORM_W, PLATFORM_H},
        {4400,  CHAO_Y - 150, PLATFORM_W, PLATFORM_H},
        {4700,  CHAO_Y - 150, PLATFORM_W, PLATFORM_H},
        {4900,  CHAO_Y - 150, PLATFORM_W, PLATFORM_H},
    };

    int num_plataformas = sizeof(plataformas) / sizeof(plataformas[0]);

    Espinho espinhos[] =
    {
        { 300, CHAO_Y - 32, 32 * 3, 32 }, 
        { 900, CHAO_Y - 32, 32 * 5, 32 },
        { 2500, CHAO_Y - 32, 32 * 15, 32 },
        { 3900, CHAO_Y - 32, 32 * 3, 32 },

    };
    int num_esp = sizeof(espinhos) / sizeof(espinhos[0]);

    Inimigo inimigos[] =
    {
        /* x inicial,     y,              limite_esq, limite_dir, velocidade */
        cria_inimigo(600,  CHAO_Y - 47,   550,       800,        2.0f),
        cria_inimigo(1500, CHAO_Y - 47,   1450,      1800,       2.5f),
        cria_inimigo(2400, CHAO_Y - 47,   2250,      2500,       2.5f),
        cria_inimigo(3200, CHAO_Y - 47,   3000,      3300,       2.5f),
    };
    int num_inimigos = sizeof(inimigos) / sizeof(inimigos[0]);


    Inimigo inimigos_ceu[] =
    {
        /* x inicial,     y (mais alto),        limite_esq, limite_dir, velocidade */
        cria_inimigo(400,  CHAO_Y - 350,        300,       700,        3.0f),
        cria_inimigo(2400, CHAO_Y - 475,       1800,       2150,       3.5f),
        cria_inimigo(2600, CHAO_Y - 470,       2200,       2750,       3.5f),
        cria_inimigo(2600, CHAO_Y - 472,       2300,       2700,       3.5f),
        cria_inimigo(3300, CHAO_Y - 300,       3300,       3500,       2.0f),
        cria_inimigo(3300, CHAO_Y - 200,       3300,       3500,       3.5f),
        cria_inimigo(3200, CHAO_Y - 400,       3000,       3400,       3.5f),
        cria_inimigo(3100, CHAO_Y - 400,       2900,       3400,       1.5f),
        cria_inimigo(1100, CHAO_Y - 250,       1000,       1500,       1.0f),
        cria_inimigo(1200, CHAO_Y - 400,       1000,       1500,       0.7f)
    };
    int num_inimigos_ceu = sizeof(inimigos_ceu) / sizeof(inimigos_ceu[0]);

    Inimigo inimigos_ceu2[] =
    {
        /* x inicial,     y (mais alto),        limite_esq, limite_dir, velocidade */
        cria_inimigo(2700,  CHAO_Y - 150,        2500,       2900,        10.0f),
    };
    int num_inimigos_ceu2 = sizeof(inimigos_ceu2) / sizeof(inimigos_ceu2[0]);

    Inimigo inimigo_laranja[] =
    {
        /* x inicial,     y (mais alto),        limite_esq, limite_dir, velocidade */
        cria_inimigo(4000,  CHAO_Y - 47,        4000,       5000,        35.0f),
    };
    int num_inimigo_laranja = sizeof(inimigo_laranja) / sizeof(inimigo_laranja[0]);

    Inimigo inimigo_balao[] =
    {
        /* x inicial,     y (mais alto),        limite_esq, limite_dir, velocidade */
        cria_inimigo(4300,  CHAO_Y - 220,        4200,       4400,        0.5),
        cria_inimigo(4500,  CHAO_Y - 220,        4350,       4650,        0.8),
        cria_inimigo(4800,  CHAO_Y - 220,        4700,       4900,        0.5),
    };
    int num_inimigo_balao = sizeof(inimigo_balao) / sizeof(inimigo_balao[0]);


    int item_w = al_get_bitmap_width(spr_item_vida);
    int item_h = al_get_bitmap_height(spr_item_vida);

    ItemVida itens_vida[] =
    {
        { 4500,  CHAO_Y - item_h - 10, item_w, item_h, 1 },
    };
    int num_itens_vida = sizeof(itens_vida) / sizeof(itens_vida[0]);


    Montanha montanhas[] = 
    {
        { 1800,  CHAO_Y - 422, 384, 422 },
        { 3500, CHAO_Y - 422, 384, 422 },
        { 5800, CHAO_Y - 422, 384, 422 },
    };
    int num_montanhas = sizeof(montanhas) / sizeof(montanhas[0]);

    // hitbox independente das montanhas
    HitboxMontanha hitboxes_montanha[] =
    {
        { 1800,  CHAO_Y - 448, 370, 448 },
        { 3500, CHAO_Y - 448, 370, 448 },
        { 5800, CHAO_Y - 448, 370, 422 },
    };
    int num_hitboxes_montanha = sizeof(hitboxes_montanha) / sizeof(hitboxes_montanha[0]);


    int vit_w = al_get_bitmap_width(spr_vitoria);
    int vit_h = al_get_bitmap_height(spr_vitoria);

    ItemVitoria item_vitoria = { 5600, CHAO_Y - vit_h - 10, vit_w, vit_h, 1 }; 



    GameState game_state = STATE_MENU;
    int menu_opcao = 0;

    toca_menu();

    ALLEGRO_EVENT event;
    al_start_timer(timer);

    int frame = 0;
    int anim_counter = 0;
    const int anim_speed = 5;

    float camera_x = 0.0f;
    float camera_y = 0.0f;
    float zoom = ZOOM_NORMAL;
    int death_frames = 0;

    while (1)
    {
        al_wait_for_event(queue, &event);

        if (event.type == ALLEGRO_EVENT_TIMER)
        {
            if (game_state == STATE_GAMEOVER)
            {
                desenha_gameover(gameover_bg);
                al_flip_display();
                continue;
            }
            else if (game_state == STATE_WINSCREEN)
            {
                ALLEGRO_TRANSFORM t;
                al_identity_transform(&t);
                al_use_transform(&t);

                if (win_bg)
                    al_draw_bitmap(win_bg, 0, 0, 0);
                else
                    al_clear_to_color(al_map_rgb(0, 80, 0));

                al_draw_textf(al_create_builtin_font(),
                    al_map_rgb(255, 255, 255),
                    X_SCREEN / 2, Y_SCREEN - 100,
                    ALLEGRO_ALIGN_CENTER,
                    "Parabens! Voce venceu! Aperte ENTER para voltar ao menu");

                al_flip_display();
                continue;
            }
            else if (game_state == STATE_OPTIONS)
            {
                desenha_options(menu_bg);
                al_flip_display();
                continue;
            }


            /* se morrer durante o jogo, vai para a animação de morte */
            if (game_state == STATE_GAME && jogador->vida <= 0)
            {
                game_state = STATE_DYING;
                death_frames = 0;

                jogador->vx = 0.0f;
                jogador->vy = 0.0f;
            }

            if (game_state == STATE_MENU)
            {
                desenha_menu(menu_bg, btn_start, btn_quit, menu_opcao);
                al_flip_display();
                continue;
            }

            /* jogo iniciado */
            if (game_state == STATE_GAME)
            {
                update_position(jogador, hitboxes_montanha, num_hitboxes_montanha);

                if (jogador->control->down && !jogador->no_chao)
                    fastfall(jogador);

                aplica_gravidade_plataformas(jogador, plataformas, num_plataformas);

                resolve_colisao_montanhas(jogador, hitboxes_montanha, num_hitboxes_montanha);


                if (jogador->invencivel)
                {
                    jogador->inv_timer--;
                    if (jogador->inv_timer <= 0)
                        jogador->invencivel = false;
                }


                for (int i = 0; i < num_itens_vida; i++)
                {
                    if (colisao_item_vida(&itens_vida[i], jogador))
                    {
                        itens_vida[i].ativo = 0;

                        if (jogador->vida < jogador->vida_max)
                            jogador->vida += 1;
                    }
                }

                for (int i = 0; i < num_inimigos; i++)
                    atualiza_inimigo(&inimigos[i]);
                
                for (int i = 0; i < num_inimigos_ceu; i++)
                    atualiza_inimigo(&inimigos_ceu[i]);

                for (int i = 0; i < num_inimigos_ceu2; i++)
                    atualiza_inimigo(&inimigos_ceu2[i]);

                for (int i = 0; i < num_inimigo_laranja; i++)
                    atualiza_inimigo(&inimigo_laranja[i]);

                for (int i = 0; i < num_inimigo_balao; i++)
                    atualiza_inimigo(&inimigo_balao[i]);

                if (zoom > ZOOM_NORMAL)
                    zoom = ZOOM_NORMAL;
            }
            else if (game_state == STATE_DYING)
            {
                // morte, zoom gradual no personagem
                if (zoom < ZOOM_DEATH)
                {
                    float step = (ZOOM_DEATH - ZOOM_NORMAL) / (float)DEATH_FRAMES;
                    zoom += step;
                    if (zoom > ZOOM_DEATH)
                        zoom = ZOOM_DEATH;
                }

                death_frames++;

                if (death_frames >= DEATH_FRAMES)
                {
                    game_state = STATE_GAMEOVER;
                    zoom = ZOOM_NORMAL;
                }
            }
            else if (game_state == STATE_WINNING)
            {
                if (zoom < ZOOM_DEATH)
                {
                    float step = (ZOOM_DEATH - ZOOM_NORMAL) / (float)DEATH_FRAMES;
                    zoom += step;
                    if (zoom > ZOOM_DEATH)
                        zoom = ZOOM_DEATH;
                }

                death_frames++;

                if (death_frames >= DEATH_FRAMES)
                {
                    game_state = STATE_WINSCREEN;
                    zoom = ZOOM_NORMAL;
                }
            }


            // atualiza câmera com zoom atual
            atualiza_camera(jogador, &camera_x, &camera_y, zoom);

            ALLEGRO_TRANSFORM camera;
            al_identity_transform(&camera);

            al_scale_transform(&camera, zoom, zoom);
            al_translate_transform(&camera, -camera_x * zoom, -camera_y * zoom);
            al_use_transform(&camera);

            // limpa a tela com preto, o que passar do fundo vai aparecer preto
            al_clear_to_color(al_map_rgb(0, 0, 0));

            float bg_y = CHAO_Y - BG_H;
            al_draw_bitmap(bg, 0, bg_y, 0);

            bool moving = jogador->control->left  || jogador->control->right ||  jogador->control->up    || jogador->control->down;

            if (moving && game_state == STATE_GAME)
            {
                anim_counter++;
                if (anim_counter >= anim_speed)
                {
                    anim_counter = 0;
                    frame++;
                    if (frame >= 3)
                        frame = 0;
                }
            }
            else
            {
                frame = 0;
                anim_counter = 0;
            }

            al_draw_filled_rectangle(0, CHAO_Y, BG_W, 900, al_map_rgb(100, 60, 20));

            for (int i = 0; i < num_plataformas; i++)
            {
                Plataforma *p = &plataformas[i];
                int tiles = p->w / PLATFORM_W;

                for (int t = 0; t < tiles; t++)
                {
                    al_draw_bitmap(spr_plataforma, p->x + t * PLATFORM_W, p->y, 0);
                }
            }

            //desenha espinhos
            for (int i = 0; i < num_esp; i++)
                desenha_espinhos(spr_espinho, &espinhos[i]);

            // desenha inimigos
            for (int i = 0; i < num_inimigos; i++)
                desenha_inimigo(spr_inimigo, &inimigos[i]);

            // desenha inimigos do ceu
            for (int i = 0; i < num_inimigos_ceu; i++)
                desenha_inimigo(spr_inimigo2, &inimigos_ceu[i]);

            //desenha segundo inimigo do ceu
            for (int i = 0; i < num_inimigos_ceu2; i++)
                desenha_inimigo(spr_inimigo3, &inimigos_ceu2[i]);
            
            //desenha inimigo laranja
            for (int i = 0; i < num_inimigo_laranja; i++)
                desenha_inimigo(spr_inimigo4, &inimigo_laranja[i]);

            //desenha inimigo balao
            for (int i = 0; i < num_inimigo_balao; i++)
                desenha_inimigo(spr_inimigo5, &inimigo_balao[i]);

            //desenha montanha
            for (int i = 0; i < num_montanhas; i++)
                desenha_montanha(spr_montanha, &montanhas[i]);

            //desenha item vida
            for (int i = 0; i < num_itens_vida; i++)
                desenha_item_vida(spr_item_vida, &itens_vida[i]);
            
            //desenha item vitoria
            if (item_vitoria.ativo)
                al_draw_bitmap(spr_vitoria, item_vitoria.x, item_vitoria.y, 0);




            if (game_state == STATE_GAME)
            {
                //vitoria
                if (colisao_item_vitoria(&item_vitoria, jogador))
                {
                    item_vitoria.ativo = 0;
                    game_state = STATE_WINNING;
                    death_frames = 0; // reaproveita contador de zoom da morte

                    jogador->vx = 0;
                    jogador->vy = 0;
                }


                //dano se esta somente se esta jogando
                for (int i = 0; i < num_esp; i++)
                {
                    if (colisao_espinho(&espinhos[i], jogador))
                    {
                        dano_personagem(jogador, 5);
                    }
                }

                for (int i = 0; i < num_inimigos; i++)
                {
                    if (colisao_inimigo(&inimigos[i], jogador))
                    {
                        dano_personagem(jogador, 1); 
                    }
                }

                for (int i = 0; i < num_inimigos_ceu; i++)
                {
                    if (colisao_inimigo(&inimigos_ceu[i], jogador))
                    {
                        dano_personagem(jogador, 1);
                    }
                }

                for (int i = 0; i < num_inimigos_ceu2; i++)
                {
                    if (colisao_inimigo(&inimigos_ceu2[i], jogador))
                    {
                        dano_personagem(jogador, 2);
                    }
                }

                for (int i = 0; i < num_inimigo_laranja; i++)
                {
                    if (colisao_inimigo(&inimigo_laranja[i], jogador))
                    {
                        dano_personagem(jogador, 5);
                    }
                }

                for (int i = 0; i < num_inimigo_balao; i++)
                {
                    if (colisao_inimigo(&inimigo_balao[i], jogador))
                    {
                        dano_personagem(jogador, 2);
                    }
                }

            }

            int coluna_sprite = frame;

            if (jogador->dashing)
                coluna_sprite = 6;
            else if(jogador->escalando)
                coluna_sprite = 7;
            else if (jogador->control->down)
                coluna_sprite = 3;
            else if (!jogador->no_chao)
            {
                if (jogador->vy < 0.0f)
                    coluna_sprite = 5;
                else
                    coluna_sprite = 4;
            }

            desenha_sprite(sheet, coluna_sprite, jogador->direcao, jogador->x, jogador->y, jogador->invencivel);

            desenha_vida(spr_vida, jogador);

            al_flip_display();
        }
        else if (event.type == ALLEGRO_EVENT_KEY_DOWN || event.type == ALLEGRO_EVENT_KEY_UP)
        {
            bool pressionado = (event.type == ALLEGRO_EVENT_KEY_DOWN);

            if (game_state == STATE_MENU)
            {
                if (!pressionado) continue;

                if (event.keyboard.keycode == ALLEGRO_KEY_W || event.keyboard.keycode == ALLEGRO_KEY_UP)
                {
                    menu_opcao--;
                    if (menu_opcao < 0) menu_opcao = 1;
                }
                else if (event.keyboard.keycode == ALLEGRO_KEY_S || event.keyboard.keycode == ALLEGRO_KEY_DOWN)
                {
                    menu_opcao++;
                    if (menu_opcao > 1) menu_opcao = 0;
                }
                else if (event.keyboard.keycode == ALLEGRO_KEY_O)
                {
                    if (pressionado)
                        game_state = STATE_OPTIONS;
                }
                else if (event.keyboard.keycode == ALLEGRO_KEY_ENTER)
                {
                    if (menu_opcao == 0)
                    {
                        game_state = STATE_GAME;
                        zoom = ZOOM_NORMAL;

                        toca_fase();
                    }
                    else
                        break;
                }
            }
            else if (game_state == STATE_GAME)
            {
                if (event.keyboard.keycode == ALLEGRO_KEY_A)
                {
                    if (pressionado)
                        joystick_left(jogador->control);
                    else
                        joystick_left_release(jogador->control);
                }
                else if (event.keyboard.keycode == ALLEGRO_KEY_D)
                {
                    if (pressionado)
                        joystick_right(jogador->control);
                    else
                        joystick_right_release(jogador->control);
                }
                    else if (event.keyboard.keycode == ALLEGRO_KEY_SPACE)
                    {
                        if (pressionado)
                        {
                            if (jogador->no_chao)
                            {
                                jogador->vy = -PULO;
                                jogador->no_chao = false;
                            }
                            else if (jogador->escalando)
                            {
                                jogador->vy = -PULO * 0.9f;
                                jogador->vx = 12.0f * -jogador->wall_dir; 
                                jogador->escalando = false;
                                jogador->wall_dir = 0;
                            }
                        }
                    }
                else if (event.keyboard.keycode == ALLEGRO_KEY_S)
                {
                    if (pressionado)
                        joystick_down(jogador->control);
                    else
                        joystick_down_release(jogador->control);
                }
                else if (event.keyboard.keycode == ALLEGRO_KEY_LSHIFT)
                {
                    if (pressionado && !jogador->dashing && jogador->no_chao)
                    {
                        int dir = 0;

                        if (jogador->control->right)
                            dir = 1;
                        else if (jogador->control->left)
                            dir = -1;
                        else
                            dir = (jogador->direcao == 0) ? 1 : -1;

                        jogador->dashing    = true;
                        jogador->dash_timer = DASH_FRAMES;
                        jogador->vx         = DASH_SPEED * dir;
                    }
                }
                else if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE && pressionado)
                {
                    game_state = STATE_MENU;
                }
            }
            else if (game_state == STATE_DYING)
            {
                // durante a animação de morte, ignora as entradas
            }
            else if (game_state == STATE_GAMEOVER)
            {
                if (!pressionado) continue;

                if (event.keyboard.keycode == ALLEGRO_KEY_ENTER)
                {
                    reset_jogador(jogador, 10, Y_SCREEN/2);
                    reset_itens_vida(itens_vida, num_itens_vida);
                    game_state = STATE_MENU;
                    zoom = ZOOM_NORMAL;
                }
            }
            else if (game_state == STATE_WINSCREEN)
            {
                if (!pressionado) continue;

                if (event.keyboard.keycode == ALLEGRO_KEY_ENTER)
                {
                    reset_jogador(jogador, 10, Y_SCREEN/2);
                    item_vitoria.ativo = 1;
                    game_state = STATE_MENU;
                    zoom = ZOOM_NORMAL;
                }
            }
            
            else if (game_state == STATE_OPTIONS)
            {
                if (!pressionado) continue;

                if (event.keyboard.keycode == ALLEGRO_KEY_LEFT || event.keyboard.keycode == ALLEGRO_KEY_A)
                {
                    volume_musica -= 0.05f;
                    if (volume_musica < 0.0f) volume_musica = 0.0f;
                    atualiza_volume();
                }
                else if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT || event.keyboard.keycode == ALLEGRO_KEY_D)
                {
                    volume_musica += 0.05f;
                    if (volume_musica > 1.0f) volume_musica = 1.0f;
                    atualiza_volume();
                }
                else if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
                {
                    game_state = STATE_MENU;
                }
            }
        }
        else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
        {
            break;
        }
    }

    destroi_personagem(jogador);

    al_destroy_bitmap(sheet);
    al_destroy_bitmap(spr_plataforma);
    al_destroy_bitmap(spr_vida);
    al_destroy_bitmap(bg);
    al_destroy_bitmap(menu_bg);
    al_destroy_bitmap(btn_start);
    al_destroy_bitmap(btn_quit);
    al_destroy_bitmap(spr_espinho);
    al_destroy_bitmap(gameover_bg);
    al_destroy_bitmap(spr_inimigo);
    al_destroy_bitmap(spr_inimigo2);
    al_destroy_bitmap(spr_inimigo3);
    al_destroy_bitmap(spr_inimigo4);
    al_destroy_bitmap(spr_inimigo5);
    al_destroy_bitmap(spr_montanha);
    al_destroy_bitmap(spr_item_vida);
    al_destroy_font(font);
    al_destroy_display(disp);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    if (inst_menu) al_destroy_sample_instance(inst_menu);
    if (inst_fase) al_destroy_sample_instance(inst_fase);
    if (mus_menu)  al_destroy_sample(mus_menu);
    if (mus_fase)  al_destroy_sample(mus_fase);

    return 0;
}

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 \
    $(shell pkg-config --cflags allegro-5 allegro_font-5 allegro_primitives-5 allegro_image-5 allegro_audio-5 allegro_acodec-5)

LDFLAGS = $(shell pkg-config --libs allegro-5 allegro_font-5 allegro_primitives-5 allegro_image-5 allegro_audio-5 allegro_acodec-5)

SRC = main.c personagem.c joystick.c obstaculos.c inimigo.c item.c

all: jogo

jogo:
	$(CC) $(SRC) -o jogo $(CFLAGS) $(LDFLAGS)

clean:
	rm -f jogo

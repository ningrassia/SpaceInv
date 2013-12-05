#ifndef __GAME_H__
#define __GAME_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct {
	uint8_t xPos;
	uint8_t yPos;
} PLAYER;

typedef struct {
	uint8_t xPos;
	uint8_t yPos;
	uint8_t Directon;
} BULLET;

#endif

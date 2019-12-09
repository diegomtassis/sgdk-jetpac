/*
 * game.h
 *
 *  Created on: Apr 20, 2019
 *      Author: diegomtassis
 */

#ifndef INC_GAME_H_
#define INC_GAME_H_

#include "elements.h"
#include "events.h"

GameResult runGame(Config config[static 1]);

void scoreByEvent(GameEvent event, u8 player);

#endif /* INC_GAME_H_ */

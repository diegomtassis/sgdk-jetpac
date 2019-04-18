/*
 * levels.h
 *
 *  Created on: Apr 7, 2019
 *      Author: diegomtassis
 */

#ifndef INC_LEVELS_H_
#define INC_LEVELS_H_

typedef struct {
	u8 xPos;
	u8 yPos;
	u8 length;
} Platform;

typedef struct {
	Platform* floor;
	Platform* platforms;
	u8 num_platforms;
} Level;

void runGame();

#endif /* INC_LEVELS_H_ */

/*
 * jetman.h
 *
 *  Created on: Apr 16, 2019
 *      Author: diegomtassis
 */

#ifndef INC_JETMAN_H_
#define INC_JETMAN_H_

#include "elements.h"

Jetman* startJetman(Player* player, Planet planet[static 1]);
void releaseJetmen(Planet planet[static 1]);
void killJetman(Jetman* jetman, Planet planet[static 1], bool exploding);
void resetJetman(Jetman* jetman, Planet planet[static 1]);

bool resurrectOrRelease(Jetman* jetman, Planet planet[static 1]);
bool isJetmanAlive(Jetman* jetman);

void jetmanActs(Jetman* jetman, Planet planet[static 1]);

#endif /* INC_JETMAN_H_ */

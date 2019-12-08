/*
 * game.c
 *
 *  Created on: Apr 20, 2019
 *      Author: diegomtassis
 */

#include "../inc/game.h"

#include <genesis.h>

#include "../inc/collectables.h"
#include "../inc/config.h"
#include "../inc/elements.h"
#include "../inc/enemies.h"
#include "../inc/spaceship.h"
#include "../inc/explosions.h"
#include "../inc/shooting.h"
#include "../inc/hud.h"
#include "../inc/events.h"
#include "../inc/jetman.h"
#include "../inc/planet.h"
#include "../inc/planets.h"
#include "../inc/fwk/physics.h"

#define DEFAULT_FLASH_WAIT	2000

/**
 * @brief run a planet.
 *
 * @param planet
 * @return goal accomplished
 */
static bool runPlanet(Planet planet[static 1]);

static bool isJetmanAlive(Planet planet[static 1]);
static bool isMissionAccomplished(Planet planet[static 1]);

static void waitForLanding(Planet planet[static 1]);
static void leavePlanet(Planet planet[static 1]);

static void handleCollisionsBetweenMovingObjects(Planet planet[static 1]);
static void handleElementsLeavingScreenUnder(Planet planet[static 1]);

static void scoreBonus(Planet planet[static 1]);
static void scorePoints(u16 points);

static void printMessage(const char* message);
static void flashMessage(const char *message, long wait);

static void joyEvent(u16 joy, u16 changed, u16 state);

volatile bool paused = FALSE;

static const V2u16 message_pos = { .x = 16, .y = 7 };

Game * current_game;

/**
 *
 * @param game
 */
void runGame(Game* game) {

	current_game = game;

	SPR_init();

	bool game_over = FALSE;

	/* when all the planets are passed start again */

	u8 planet_number = 0;

	displayAmmo(game->config.mode == MD);

	while (!game_over) {

		//	log_memory();

		Planet* current_planet = game->createPlanet[planet_number]();
		current_planet->game = game;

		startPlanet(current_planet);

		startSpaceship(current_planet);
		waitForLanding(current_planet);

		startJetman(current_planet, game->config.mode == MD);
		startEnemies(current_planet);

		startCollectables(current_planet);
		initShots(current_planet);
		initExplosions(current_planet);

		SPR_update();
		VDP_waitVSync();

		JOY_setEventHandler(joyEvent);

		game_over = !runPlanet(current_planet);

		if (game_over) {
			flashMessage("Game Over", DEFAULT_FLASH_WAIT);

		} else {
			SPR_setVisibility(current_planet->jetman->sprite, HIDDEN);
			leavePlanet(current_planet);
			scoreBonus(current_planet);
			updateHud(current_game, current_planet->jetman);

			if (++planet_number == game->num_planets) {
				planet_number = 0;
			}

			waitMs(500);
		}

		releaseExplosions(current_planet);
		releaseShots(current_planet);
		releaseCollectables(current_planet);
		releaseEnemies(current_planet);
		releaseJetman(current_planet);
		releaseSpaceship(current_planet);
		releasePlanet(current_planet);
		current_planet = 0;

		SPR_update();

		VDP_clearPlan(PLAN_B, TRUE);
	}

	SPR_end();

	current_game = 0;
	return;
}

void releaseGame(Game* game) {

	if (!game) {
		return;
	}

	// No need to release the createPlanet individual functions, only the array
	if (game->createPlanet) {
		MEM_free(game->createPlanet);
		game->createPlanet = 0;
	}

	MEM_free(game);
}

void scoreByEvent(GameEvent event) {

	if (!current_game) {
		return;
	}

	switch (event) {

	case KILLED_ENEMY:
		current_game->score += 25;
		break;

	case GRABBED_SPACESHIP_PART:
		current_game->score += 100;
		break;

	case GRABBED_FUEL:
		current_game->score += 100;
		break;

	case GRABBED_COLLECTABLE:
		current_game->score += 250;
		break;

	case LOST_FUEL:
		current_game->score -= 50;
		break;

	case LOST_COLLECTABLE:
		current_game->score -= 25;
		break;

	default:
		break;
	}
}

static bool runPlanet(Planet current_planet[static 1]) {

	bool jetman_alive = TRUE;
	bool game_over = FALSE;
	bool mission_accomplished = FALSE;

	while (!game_over && !mission_accomplished) {

		if (!paused) {

			if (jetman_alive) {

				jetmanActs(current_planet);
				enemiesAct(current_planet);
				handleCollisionsBetweenMovingObjects(current_planet);
				if (current_planet->def.mind_bottom) {
					handleElementsLeavingScreenUnder(current_planet);
				}

				handleSpaceship(current_planet);

				jetman_alive = isJetmanAlive(current_planet);
				if (!jetman_alive) {
					dropIfGrabbed(current_planet->spaceship);
					current_game->lives--;
				}

			} else {

				// Smart dying, wait for explosions to finish
				if (!current_planet->booms.count) {

					waitMs(100);

					releaseEnemies(current_planet);
					if (current_game->lives > 0) {
						resetJetman(current_planet);
						startEnemies(current_planet);
						jetman_alive = TRUE;
					}
				}
			}

			updateCollectables(current_planet);
			updateShots(current_planet);
			updateExplosions(current_planet);

			mission_accomplished = jetman_alive && isMissionAccomplished(current_planet);
			game_over = !current_game->lives && !current_planet->booms.count;
			SPR_update();
		}

		updateHud(current_game, current_planet->jetman);
		VDP_waitVSync();
	}

	return mission_accomplished;
}

static void handleCollisionsBetweenMovingObjects(Planet planet[static 1]) {

	for (u8 enemy_idx = 0; enemy_idx < planet->enemies.size; enemy_idx++) {

		Enemy* enemy = planet->enemies.e[enemy_idx];
		if (enemy && (ALIVE & enemy->health)) {

			// enemy & shot
			if (checkHit(enemy->object.box, planet)) {

				killEnemy(enemy, planet, TRUE);
				onEvent(KILLED_ENEMY);

				continue;
			}

			// enemy & jetman
			if (overlap(planet->jetman->object.box, enemy->object.box)) {
				killJetman(planet, TRUE);
				killEnemy(enemy, planet, TRUE);
				break;
			}
		}
	}
}

static void handleElementsLeavingScreenUnder(Planet planet[static 1]) {

	if ((ALIVE & planet->jetman->health) && planet->jetman->object.box.pos.y > BOTTOM_POS_V_PX_S16) {
		killJetman(planet, FALSE);
	}

	for (u8 enemy_idx = 0; enemy_idx < planet->enemies.size; enemy_idx++) {

		Enemy* enemy = planet->enemies.e[enemy_idx];
		if (enemy && (ALIVE & enemy->health) && enemy->object.box.pos.y > BOTTOM_POS_V_PX_S16) {
			killEnemy(enemy, planet, FALSE);
		}
	}
}

static bool isJetmanAlive(Planet planet[static 1]) {

	return ALIVE & planet->jetman->health;
}

static bool isMissionAccomplished(Planet planet[static 1]) {

	return (planet->spaceship->step == READY)
			&& shareBase(planet->jetman->object.box, planet->spaceship->base_object->box);
}

static void waitForLanding(Planet planet[static 1]) {

	while (planet->spaceship->step == LANDING) {

		handleSpaceship(planet);

		SPR_update();
		VDP_waitVSync();
	}
}

static void leavePlanet(Planet planet[static 1]) {

	launch(planet->spaceship);
	do {
		// keep life going while the orbiter lifts
		handleSpaceship(planet);
		enemiesAct(planet);
		updateExplosions(planet);
		updateShots(planet);
		updateCollectables(planet);

		SPR_update();
		VDP_waitVSync();

	} while (planet->spaceship->step == LIFTING);
}

void static scoreBonus(Planet planet[static 1]) {

	if (current_game->config.mode == MD) {

		u16 ammo_bonus = 0;
		char bonus_message[19];

		sprintf(bonus_message, "Bonus %03d", ammo_bonus);
		printMessage(bonus_message);
		waitMs(500);

		while (planet->jetman->ammo--) {

			ammo_bonus += 10;
			sprintf(bonus_message, "Bonus %03d", ammo_bonus);
			printMessage(bonus_message);
			waitMs(25);
			updateAmmo(planet->jetman);
			VDP_waitVSync();
		}

		flashMessage(bonus_message, 1000);
		planet->jetman->ammo = 0;
		scorePoints(ammo_bonus);
	}
}

static void scorePoints(u16 points) {

	if (!current_game) {
		return;
	}

	current_game->score += points;
}

static void printMessage(const char *message) {

	// center the message
	u8 x_pos = message_pos.x - strlen(message) / 2;
	VDP_drawText(message, x_pos, message_pos.y);
}

static void flashMessage(const char *message, long wait) {

	printMessage(message);
	waitMs(wait);
	VDP_clearTextLine(message_pos.y);
}

static void joyEvent(u16 joy, u16 changed, u16 state) {

	if (BUTTON_START & changed & ~state) {
		paused ^= 1;
	}
}

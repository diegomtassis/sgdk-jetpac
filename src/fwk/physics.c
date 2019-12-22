/*
 * physics.c
 *
 *  Created on: Apr 19, 2019
 *      Author: diegomtassis
 */

#include "../../inc/fwk/physics.h"

void updateBox(Object_f16* object) {

	object->box.pos.x = fix16ToInt(object->pos.x);
	object->box.pos.y = fix16ToInt(object->pos.y);
}

Box_s16 targetBox(Object_f16 object) {

	Box_s16 box = { .w = object.size.x, .h = object.size.y };
	box.pos.x = fix16ToInt(fix16Add(object.pos.x, object.mov.x));
	box.pos.y = fix16ToInt(fix16Add(object.pos.y, object.mov.y));

	return box;
}

Box_s16 targetHBox(Object_f16 object) {

	Box_s16 box = { .w = object.size.x, .h = object.size.y };
	box.pos.x = fix16ToInt(fix16Add(object.pos.x, object.mov.x));
	box.pos.y = fix16ToInt(object.pos.y);

	return box;
}

Box_s16 targetVBox(Object_f16 object) {

	Box_s16 box = { .w = object.size.x, .h = object.size.y };
	box.pos.x = fix16ToInt(object.pos.x);
	box.pos.y = fix16ToInt(fix16Add(object.pos.y, object.mov.y));

	return box;
}

bool contained(V2s16 subject_pos, Box_s16 object_box) {

	return (IN_BETWEEN & axisXPxRelativePos(subject_pos.x, object_box))
			&& (IN_BETWEEN & axisYPxRelativePos(subject_pos.y, object_box));
}

bool overlap(Box_s16 subject_box, Box_s16 object_box) {

	return (OVERLAPPED & axisXBoxRelativePos(subject_box, object_box))
			&& (OVERLAPPED & axisYBoxRelativePos(subject_box, object_box));
}

bool isAboveBaseUpwardProjection(Box_s16 subject_box, Box_s16 object_box) {

	if (subject_box.pos.y + subject_box.h > object_box.pos.y + object_box.h) {
		return FALSE;
	}

	return IN_BETWEEN & axisXPxRelativePos(subject_box.pos.x + subject_box.w / 2, object_box);
}

bool shareBase(Box_s16 subject_box, Box_s16 object_box) {

	if (subject_box.pos.y + subject_box.h != object_box.pos.y + object_box.h) {
		return FALSE;
	}

	return IN_BETWEEN & axisXPxRelativePos(subject_box.pos.x + subject_box.w / 2, object_box);
}

bool hitAbove(Box_s16 subject_box, Box_s16 object_box) {

	if (OVERLAPPED & axisXBoxRelativePos(subject_box, object_box)) {
		return IN_BETWEEN & axisYPxRelativePos(subject_box.pos.y + subject_box.h, object_box);
	}

	return FALSE;
}

bool hitUnder(Box_s16 subject_box, Box_s16 object_box) {

	if (OVERLAPPED & axisXBoxRelativePos(subject_box, object_box)) {
		return IN_BETWEEN & axisYPxRelativePos(subject_box.pos.y - 1, object_box);
	}

	return FALSE;
}

bool hitLeft(Box_s16 subject_box, Box_s16 object_box) {

	if (OVERLAPPED & axisYBoxRelativePos(subject_box, object_box)) {
		return IN_BETWEEN & axisXPxRelativePos(subject_box.pos.x + subject_box.w, object_box);
	}

	return FALSE;
}

bool hitRight(Box_s16 subject_box, Box_s16 object_box) {

	if (OVERLAPPED & axisYBoxRelativePos(subject_box, object_box)) {
		return IN_BETWEEN & axisXPxRelativePos(subject_box.pos.x - 1, object_box);
	}

	return FALSE;
}

s16 adjacentYAbove(Box_s16 subject_box, Box_s16 object_box) {
	return object_box.pos.y - subject_box.h;
}

s16 adjacentYUnder(Box_s16 subject_box, Box_s16 object_box) {
	return object_box.pos.y + object_box.h;
}

s16 adjacentXOnTheLeft(Box_s16 subject_box, Box_s16 object_box) {
	return object_box.pos.x - subject_box.w;
}

s16 adjacentXOnTheRight(Box_s16 subject_box, Box_s16 object_box) {
	return object_box.pos.x + object_box.w;
}

u8 axisXBoxRelativePos(Box_s16 subject_box, Box_s16 object_box) {

	if (TO_THE_RIGHT & axisXPxRelativePos(subject_box.pos.x, object_box)) {
		return TO_THE_RIGHT;
	}

	if (TO_THE_LEFT & axisXPxRelativePos(subject_box.pos.x + subject_box.w - 1, object_box)) {
		return TO_THE_LEFT;
	}

	return OVERLAPPED;
}

u8 axisYBoxRelativePos(Box_s16 subject_box, Box_s16 object_box) {

	if (UNDER & axisYPxRelativePos(subject_box.pos.y, object_box)) {
		return UNDER;
	}

	if (ABOVE & axisYPxRelativePos(subject_box.pos.y + subject_box.h - 1, object_box)) {
		return ABOVE;
	}

	return OVERLAPPED;
}

u8 axisXPxRelativePos(s16 x_px, Box_s16 object_box) {

	s16 x_s16 = object_box.pos.x;
	if (x_px < x_s16) {
		return TO_THE_LEFT;
	}

	if (x_px >= x_s16 + object_box.w) {
		return TO_THE_RIGHT;
	}

	return IN_BETWEEN;
}

u8 axisYPxRelativePos(s16 y_px, Box_s16 object_box) {

	s16 y_s16 = object_box.pos.y;
	if (y_px < y_s16) {
		return ABOVE;
	}

	if (y_px >= y_s16 + object_box.h) {
		return UNDER;
	}

	return IN_BETWEEN;
}

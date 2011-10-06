#include <iostream>
#include <math.h>
#include "dart.h"

const float radToDec = 180/3.141592653589793;


Dart::Dart() {
	this->x = 0;
	this->y = 0;
	this->z = 0;

	this->destx = 0;
	this->desty = 0;
	this->destz = 0;

	this->dx = 0;
	this->dy = 0;
	this->dz = 0;

	this->angleZ = 0.0;

	draw = false;
	flies = false;

	justHit = false;
}

Dart::Dart(position3D startPos, position3D destPos, float power) {
	setAttributes(startPos, destPos, power);
}

position3D Dart::move(double dt) {
	justHit = false;
	if (flies) {
		if (z+this->dz*dt*50 < this->destz) {
			this->z+=this->dz*dt*50;
			this->x+=this->dx*dt*50;
			this->y+=this->dy*dt*50;
		}
		else {
			flies = false;
			justHit = true;
			this->x = this->destx;
			this->y = this->desty;
			this->z = this->destz;
		}
	}


	angleZ+=2.0;

	return getPosition();
}

bool Dart::isFlying() {
	return flies;
}

float Dart::getAngleX() {
	return angleX;
}

float Dart::getAngleY() {
	return angleY;
}

float Dart::getAngleZ() {
	return angleZ;
}

/**
 * Vraci aktualni souradnici sipky
 */
position3D Dart::getPosition() {
	position3D position;
	position.x = this->x;
	position.y = this->y;
	position.z = this->z;

	return position;
}

/**
 *
 */
bool Dart::isVisible() {
	return draw;
}

void Dart::setVisible(bool v) {
	draw = v;
}

void Dart::setAttributes(position3D startPos, position3D destPos, float power) {
	this->x = startPos.x;
	this->y = startPos.y;
	this->z = startPos.z;

	this->destx = destPos.x;
	this->desty = destPos.y;
	this->destz = destPos.z;

	this->dx = (destx - x)/30;
	this->dy = (desty - y)/30;
	this->dz = (destz - z)/30;

	this->angleX = 0.0;
	this->angleY = 0.0;
	this->angleZ = 0.0;

	draw = false;
	flies = true;
	justHit = false;

	angleX = atanf(dx/dz)*radToDec;
	angleY = atanf(dy/dz)*radToDec;
}

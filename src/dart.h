#ifndef __DART_H__
#define __DART_H__

struct pointer {
	int position;
	int direction;
};

struct position2D {
	int x;
	int y;
};


struct position3D {
	float x;
	float y;
	float z;
};

struct sightDef {
	int x;
	int y;
	int dx;
	int dy;
};

class Dart {
	//Pozice sipky
	float x;
	float y;
	float z;

	//Pozice cile sipky
	float destx;
	float desty;
	float destz;

	//Prirustek
	float dx;
	float dy;
	float dz;

	//Ma se vykreslit
	bool draw;
	//Leti
	bool flies;

	float angleX;
	float angleY;
	float angleZ;

	

public:
	position3D getPosition();
	Dart();
	Dart(position3D startPos, position3D destPos, float power);
	position3D move(double dt);
	bool isVisible();
	void setVisible(bool v);
	void setAttributes(position3D startPos, position3D destPos, float power);
	float getAngleX();
	float getAngleY();
	float getAngleZ();
	bool isFlying();

	bool justHit;
};

#endif

#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdio.h>

#include "glew.h"
#include <GL/glaux.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glext.h>

#include "MilkshapeModel.h"
#include "dart.h"
#include "darts.h"
#include "logic/down.h"
#include "logic/rtc.h"
#include "logic/training.h"
#include "math.h"

#define TARGET_MODEL "models/Target.ms3d"
#define DART_MODEL "models/Dart.ms3d"
#define TITLE "Darts 3D"

#define HAHA "sounds/haha.wav"
#define BEBACK "sounds/illbebck.wav"
#define CHAMPION "sounds/champion.wav"
#define WOOHOO "sounds/woohoo.wav"
#define DARTHIT "sounds/bang.wav"
#define BEEP "sounds/beep.wav"

float mat_ambient[]={0.5, 0.5, 0.5, 1.0};       /* ambientni slozka materialu */
float mat_diffuse[]={0.5, 0.5, 0.5, 1.0};       /* difuzni slozka materialu */
float mat_specular[]={1.0, 1.0, 1.0, 1.0};		/* specular */
float lt_ambient[]= {0.9, 0.9, 0.9, 1.0};       /* ambientni slozka svetla */
float lt_diffuse[]= {0.9, 0.9, 0.9, 1.0};       /* difuzni slozka svetla */
float lt_specular[]={1.0, 1.0, 1.0, 1.0};       /* barva odlesku */
float lt_position[]={5.0, 5.0, 5.0, 0.0};       /* definice pozice svetla */
float shininess = 50;

float dartScale = 0.3f;

int   xnew=0, ynew=0, znew=0;                   /* soucasna pozice, ze ktere se pocitaji rotace a posuny */
int   xold=xnew, yold=ynew, zold=znew;          /* minula pozice, ze ktere se pocitaji rotace a posuny */
int   xx1=0, yy1=0, zz1=0;                      /* body, ve kterych se nachazi kurzor mysi */
int   stav=0;                                   /* stav tlacitek mysi */

int mode = MENU;
int modeStack = MENU;
int score = 0;

int winPosX = 10;
int winPosY = 10;

int resX = 800;
int resY = 600;

float dartRadiusP = resY;    // prumer terce v pixelech
float dartRadiusW = 10.0f;    // prumer terce v souradnicich sveta
float auxVal = -dartRadiusW/dartRadiusP;

const float radToDec = 180/3.141592653589793;
const float decToRad = 3.141592653589793/180;

bool fullscreen = false;
bool powerBarDraw = true;
bool motionBlurEnabled = true;
bool gameOver = true;
bool waitForClick = false;
bool soundOn = true;
bool cheatsOn = false;
int cheatState = 0;
int settingsPlayers = 2;
int settingsType = 2;
int settingsDifficulty = 1;

Dart **darts = NULL;
Game *gameLogic = NULL;

float fov=45.0;                                 /* zorny uhel - field of view */
float near_plane=1;                             /* blizsi orezavaci rovina */
float far_plane=1000.0;                         /* vzdalenejsi orezavaci rovina */
int   WindowWidth;                              /* sirka a vyska okna */
int   WindowHeight;
int   ObjectType=0;                             /* typ vykreslovaneho objektu */

Model *target = NULL;
Model **dart = NULL;

// pozice kamery
position3D camera;	
position3D cameraRandom;	
// vychyleni zamerovace
sightDef mySightDef;
// pozice zamerovace
position2D sight;
// ukazatel sili hodu
pointer myPointer;

static double lastTime = 0.0;
double deltaT;

GLuint TextureTiles;                            /* textura pozadi */
GLuint TextureMenu;                             /* textura menu */

/********************************************************************/
// nacteni obrazku
AUX_RGBImageRec *LoadBMP(const char *Filename)
{
	FILE *File=NULL;

	if (!Filename)
	{
		return NULL;
	}

	File=fopen(Filename,"r");

	if (File)
	{
		fclose(File);
		return auxDIBImageLoad(Filename);
	}

	return NULL;
}

/********************************************************************/
// nacteni bitmapy and prevedeni na texturu
GLuint LoadGLTexture(const char *filename)
{
	AUX_RGBImageRec *pImage;
	GLuint texture = 0;

	pImage = LoadBMP(filename);

	if (pImage != NULL && pImage->data != NULL)
	{
		glGenTextures(1, &texture);

		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, pImage->sizeX, pImage->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, pImage->data);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

		free(pImage->data);
		free(pImage);
	}

	return texture;
}

/********************************************************************/
// casovac
void Timer(int time)
{
	glutPostRedisplay();			/* prekresleni */
	glutTimerFunc(20, Timer, 0);	/* nastaveni casovace */
}

/********************************************************************/
// inicializace programu
void Init(void)
{
	glClearColor (0.0, 0.0, 0.0, 0.0);					/* barva pro mazani color-bufferu */
	glShadeModel(GL_SMOOTH);							/* nastaveni stinovaciho rezimu */
	glClearDepth(1.0f);									/* barva pro mazani z-bufferu */
	glEnable (GL_DEPTH_TEST);							/* nastaveni funkce pro testovani hodnot v z-bufferu */
	glDepthFunc (GL_LESS);								/* kterou funkci vybrat */
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  /* vylepseni zobrazovani */
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);			/* nastaveni vykresleni vyplnenych polygonu */

	glLineWidth(1.0);									/* sirka vykreslovanych car */
	glEnable(GL_LINE_SMOOTH);							/* povoleni antialiasingu car */

	glLightfv(GL_LIGHT0, GL_AMBIENT, lt_ambient);		/* nastaveni parametru svetla */
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lt_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lt_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, lt_position);
	
	// nacteni textur
	TextureTiles = LoadGLTexture("textures/bricks.bmp");
	TextureMenu = LoadGLTexture("textures/menu.bmp");

	myPointer.position = 0;
	myPointer.direction = 1;

	mySightDef.x = 0;
	mySightDef.y = 0;
	mySightDef.dx = 1;
	mySightDef.dy = 1;

	camera.x = 0.0f;
	camera.y = 0.0f;
	camera.z = -5.0f;

	cameraRandom.z = -5.0f;

	/* vypnuti kurzoru mysi */
	ShowCursor(FALSE);

	/* nacteni modelu */
	target = new MilkshapeModel();
	if ( target->loadModelData( TARGET_MODEL ) == false )
	{
		fprintf(stderr, "Error: Cannot load model!\n");
		return;
	}

	dart = new Model*[DARTS];

	/* nastaveni modelu */
	target->reloadTextures();

	/* pole sipek */
	darts = new Dart*[DARTS];
	for(int i=0; i<DARTS; i++) {
		darts[i] = NULL;
		dart[i] = new MilkshapeModel();
		if ( dart[i]->loadModelData( DART_MODEL ) == false )
		{
			fprintf(stderr, "Error: Cannot load model!\n");
			return;
		}
		dart[i]->reloadTextures();
	}
}

/********************************************************************/
// zjisteni uhlu
float getAngle(float x1, float y1, float x2, float y2)
{
    float vectX = x1 - x2;
    float vectY = y1 - y2;

    float vect2X = 1.0f;
    float vect2Y = 0.0f;

    float angle = acos((vectX*vect2X + vectY*vect2Y)/sqrt((vectX*vectX+vectY*vectY)*(vect2X*vect2X+vect2Y*vect2Y)))*radToDec;
    if (y2 < 0.0f) angle = 360.0f - angle;
    return angle;
}

/********************************************************************/
// zjisteni vzdalenosti
float getDistance(float x1, float y1, float x2, float y2)
{
    float vectX = x1 - x2;
    float vectY = y1 - y2;

    float distance = sqrt(vectX*vectX + vectY*vectY);
    return distance;
}

/********************************************************************/
// vypocet bodu
int getPoints(float x2, float y2)
{
	float x1 = 0.0, 
		  y1 = 0.0; 
    float angle = getAngle(x1, y1, x2, y2);
    float distance = getDistance(x1, y1, x2, y2);
    int mult;
    int value;

    // osetreni vzdalenosti
    if (distance < 0.2f) return 50.0f;
    else if (distance < 0.4f) return 25.0f;
    else if (distance > 2.13f && distance < 2.36f) mult = 3;
    else if (distance > 3.47f && distance < 3.7f) mult = 2;
    else if (distance >= 3.7f) mult = 0;
    else mult = 1;

    // osetreni uhlu
    if (angle > 9.0f && angle <= 27.0f) value = 14;
    else if (angle > 27.0f && angle <= 45.0f) value = 9;
    else if (angle > 45.0f && angle <= 63.0f) value = 12;
    else if (angle > 63.0f && angle <= 81.0f) value = 5;
    else if (angle > 81.0f && angle <= 99.0f) value = 20;
    else if (angle > 99.0f && angle <= 117.0f) value = 1;
    else if (angle > 117.0f && angle <= 135.0f) value = 18;
    else if (angle > 135.0f && angle <= 153.0f) value = 4;
    else if (angle > 153.0f && angle <= 171.0f) value = 13;
    else if (angle > 171.0f && angle <= 189.0f) value = 6;
    else if (angle > 189.0f && angle <= 207.0f) value = 10;
    else if (angle > 207.0f && angle <= 225.0f) value = 15;
    else if (angle > 225.0f && angle <= 243.0f) value = 2;
    else if (angle > 243.0f && angle <= 261.0f) value = 17;
    else if (angle > 261.0f && angle <= 279.0f) value = 3;
    else if (angle > 279.0f && angle <= 297.0f) value = 19;
    else if (angle > 297.0f && angle <= 315.0f) value = 7;
    else if (angle > 315.0f && angle <= 333.0f) value = 16;
    else if (angle > 333.0f && angle <= 351.0f) value = 8;
    else value = 11;

    return value * mult;
}

/********************************************************************/
// zmena velikosti
void onReshape(int w, int h)
{
	glViewport(0, 0, w, h);                       /* viditelna oblast */
	glMatrixMode(GL_PROJECTION);                  /* projekcni matice */
	glLoadIdentity();                             /* nahrat jednotkovou matici */
	gluPerspective(fov, (double)w/(double)h, near_plane, far_plane);
	glMatrixMode(GL_MODELVIEW);                   /* modelova matice */
	gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0f, 1.0f, 0.0f);	/* pozice kamery */
	WindowWidth = w;                              /* ulozeni rozmeru okna */
	WindowHeight = h;
	dartRadiusP = h;
	auxVal = -dartRadiusW/dartRadiusP;
}

/********************************************************************/
// vraci cas v sekundach
double getTime(void)
{
#if defined(__WIN32__) || defined(_WIN32) || defined(__CYGWIN__)
	static int initialized = 0;
	static LARGE_INTEGER frequency;
	LARGE_INTEGER value;
	BOOL r;

	if (!initialized) {
		initialized = 1;
		r = QueryPerformanceFrequency(&frequency);
		if (r == 0) {
			exit(-1);
		}
	}

	r = QueryPerformanceCounter(&value);
	return (double)value.QuadPart / (double)frequency.QuadPart;

#else
	struct timeval tv;
	if (gettimeofday(&tv, NULL) == -1) {
		exit(-1);
	}
	return (double)tv.tv_sec + (double)tv.tv_usec/1000000.;
#endif
}

/********************************************************************/
// aktualizace sceny (vypocita casovy rozdil mezi snimky)
void timeTick(void)
{
	if (lastTime == 0.0) {
		lastTime = getTime();
		deltaT = 0.0;
	}
	else {
		double currentTime = getTime();
		deltaT = currentTime - lastTime;
		lastTime = currentTime;
	}
}

/********************************************************************/
// prepnuti do pravouhle projekce
void ViewOrtho(int x, int y)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, x , y , 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

/********************************************************************/
// prepnuti do perspektivni projekce
void ViewPerspective(void)
{
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

/********************************************************************/
void drawDartsView() {
    position3D dartPos;

    for(int i=0; i<DARTS; i++) {
        if (darts[i] != NULL && darts[i]->isVisible()) {
            float angleX = darts[i]->getAngleX();
            float angleY = darts[i]->getAngleY();
            float angleZ = darts[i]->getAngleZ();
            dartPos = darts[i]->getPosition();
            glPushMatrix();
                    glTranslatef(dartPos.x, dartPos.y, dartPos.z);

                    glRotatef(-90.0, 0.0f, 1.0f, 0.0f);
                    glRotatef(angleY, 0.0f, 0.0f, 1.0f);
                    glRotatef(angleX, 0.0f, 1.0f, 0.0f);
                    glRotatef(angleZ, 1.0, 0.0, 0.0);
                    glScalef(dartScale, dartScale, dartScale);

                    dart[i]->draw();
            glPopMatrix();

        }
    }
}

/********************************************************************/
// vykresleni ukazatele sily hodu
void drawPowerBar(void)
{
    ViewOrtho(WindowWidth, WindowHeight);
	glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

	int ptrSize = WindowHeight/100;
	if (ptrSize <= 0) ptrSize = 1;

	int widthDiv = WindowWidth/20;
	int heightDiv = WindowHeight/15;

	int shift = myPointer.position*WindowHeight/21.9;

	glPushMatrix();
		glEnable(GL_BLEND);	// zapne blending
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);	// funkce blendingu
		glDepthMask(GL_FALSE);	// vypne depth buffer

		glBegin(GL_QUADS);
			// spodni obdelnik
			glColor4f(0.5, 0.0, 0.0, 0.2);
			glVertex2i(18.5*widthDiv, 14.5*heightDiv);	// levy dolni
			glVertex2i(19.5*widthDiv, 14.5*heightDiv);	// pravy dolni
			glColor4f(0.5, 0.0, 0.0, 0.8);
			glVertex2i(19.5*widthDiv, 7.5*heightDiv);	// pravy horni
			glVertex2i(18.5*widthDiv, 7.5*heightDiv);	// levy horni
			// horni obdelnik
			glColor4f(0.5, 0.0, 0.0, 0.8);
			glVertex2i(18.5*widthDiv, 7.5*heightDiv);	// levy dolni
			glVertex2i(19.5*widthDiv, 7.5*heightDiv);	// pravy dolni
			glColor4f(0.5, 0.0, 0.0, 0.2);
			glVertex2i(19.5*widthDiv, 0.5*heightDiv);	// pravy horni
			glVertex2i(18.5*widthDiv, 0.5*heightDiv);	// levy horni
			// ukazatel
			glColor4f(0.0, 0.0, 0.0, 0.8);
			glVertex2i(18.5*widthDiv, 7.5*heightDiv-ptrSize + shift);	// levy dolni
			glVertex2i(19.5*widthDiv, 7.5*heightDiv-ptrSize + shift);	// pravy dolni
			glVertex2i(19.5*widthDiv, 7.5*heightDiv+ptrSize + shift);	// pravy horni
			glVertex2i(18.5*widthDiv, 7.5*heightDiv+ptrSize + shift);	// levy horni
			glColor3f(1.0, 1.0, 1.0);
		glEnd();

		glDepthMask(GL_TRUE);	// zapne depth buffer
		glDisable(GL_BLEND);	// vypne blending
	glPopMatrix();

	// posun ukazatele
	if (mode == GAME) {
		if (cheatsOn) {
			myPointer.position = 0;
		}
		else {
			myPointer.position += myPointer.direction*deltaT*(48+3*settingsDifficulty);
			// zmena smeru pohybu
			if (myPointer.position > 10){
				myPointer.position = 10;
				myPointer.direction = -myPointer.direction;
			}
			if (myPointer.position < -10){
				myPointer.position = -10;
				myPointer.direction = -myPointer.direction;
			}
		}
	}

	ViewPerspective();
	glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
}

/********************************************************************/
// zmena vychyleni zamerovace
void changeSightDef()
{
	// zmena smeru
	if ((mySightDef.x > 50 && mySightDef.dx == 1) || (mySightDef.x < -50 && mySightDef.dx == -1)){
		mySightDef.dx = -mySightDef.dx;
	}
	if ((mySightDef.y > 50 && mySightDef.dy == 1) || (mySightDef.y < -50 && mySightDef.dy == -1)) {
		mySightDef.dy = -mySightDef.dy;
	}

	// zmena velikosti
	mySightDef.x += (rand()%10) * mySightDef.dx;
	mySightDef.y += (rand()%10) * mySightDef.dy;
}

/********************************************************************/
// vykresleni zamerovace
void drawSight(void)
{
	ViewOrtho(WindowWidth, WindowHeight);
	glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

	// ziskani pozice mysi
	POINT mousePos;
	GetCursorPos(&mousePos);

	changeSightDef();

	//sight.x = mousePos.x + mySightDef.x;
	//sight.y = mousePos.y + mySightDef.y;
	if (cheatsOn) {
		sight.x = mousePos.x;
		sight.y = mousePos.y;
	} else {
		sight.x = mousePos.x + mySightDef.x;
		sight.y = mousePos.y + mySightDef.y;
	}


	int sightHeight = WindowHeight/60;
	int sightWidth = WindowWidth/200;
	int dist = 5;

	glPushMatrix();
		glBegin(GL_TRIANGLES);
			glColor3i(80,35, 35);
			glVertex2i(sight.x+sightWidth, sight.y+sightHeight+dist);
			glVertex2i(sight.x-sightWidth, sight.y+sightHeight+dist);
			glColor3f(1.0, 0.0, 0.0);
			glVertex2i(sight.x, sight.y+dist);

			glColor3i(80,35, 35);
			glVertex2i(sight.x+sightWidth, sight.y-sightHeight-dist);
			glVertex2i(sight.x-sightWidth, sight.y-sightHeight-dist);
			glColor3f(1.0, 0.0, 0.0);
			glVertex2i(sight.x, sight.y-dist);

			
			glColor3f(1.0, 0.0, 0.0);
			glVertex2i(sight.x+dist, sight.y);
			glColor3i(80,35, 35);
			glVertex2i(sight.x+sightHeight+dist, sight.y+sightWidth);
			glVertex2i(sight.x+sightHeight+dist, sight.y-sightWidth);

			glColor3f(1.0, 0.0, 0.0);
			glVertex2i(sight.x-dist, sight.y);
			glColor3i(80,35, 35);
			glVertex2i(sight.x-sightHeight-dist, sight.y+sightWidth);
			glVertex2i(sight.x-sightHeight-dist, sight.y-sightWidth);
			glColor3f(1.0,1.0,1.0);
		glEnd();
	glPopMatrix();

	ViewPerspective();
	glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
}

/********************************************************************/
// nastaveni pozice kamery
void cameraPosition(float x, float y, float z)
{
	camera.x = x;
	camera.y = y;
	camera.z = z;
}

/********************************************************************/
void drawView() {
    //kontrola natoceni
    if (znew>5) {
        znew = 5;
    } else if (znew < -4) {
        znew = -4;
    }

    if (xnew < -30) {
        xnew = -30;
    } else if (xnew > 30) {
        xnew = 30;
    }

    if (ynew > 30) {
        ynew = 30;
    } else if (ynew < -30) {
        ynew = -30;
    }

    // kamera se diva na pozici urcenou polohou znew
    gluLookAt(camera.x, camera.y, camera.z, 0.0, 0.0, znew, 0.0, 1.0, 0.0);

    // vykresleni modelu (terce)
    glPushMatrix();
        /* nakonfigurujeme texturovani */
        // zaktivuje dlazdice
        glActiveTextureARB(GL_TEXTURE0_ARB);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, TextureTiles);   /* navazani textury */
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        glTranslatef(-tan(-xnew*decToRad)*(znew+5), 0.0f, 0.0f);
        glTranslatef(0.0f, tan(ynew*decToRad)*(znew+5), 0.0f);
        glTranslatef(0.0f, 0.0f, znew);
        
        glRotatef(ynew, 1.0f, 0.0f, 0.0f);
        glRotatef(-xnew, 0.0f, 1.0f, 0.0f);
        

        glPushMatrix();
            glTranslatef(0.0f, 0.0f, 7);
            glRotatef(90, 1.0f, 0.0f, 0.0f);
            glRotatef(180, 0.0f, 0.0f, 1.0f);
            glBegin(GL_QUADS);
                glNormal3f(0.0, 1.0, 0.0);
                glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0, 0.0);  glVertex3f(-50.0, -0.5, -50.0); 
                glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0, 8.5);  glVertex3f(-50.0, -0.5, 50.0); 
                glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 4.0, 8.5);  glVertex3f(50.0, -0.5, 50.0); 
                glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 4.0, 0.0);  glVertex3f(50.0, -0.5, -50.0); 
            glEnd();
            glDisable(GL_TEXTURE_2D);
            
			glEnable(GL_LIGHTING);
			glEnable(GL_LIGHT0);
			target->draw();
			glDisable(GL_LIGHTING);
			glDisable(GL_LIGHT0);
        glPopMatrix();
        
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		drawDartsView();
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);

    glPopMatrix();

	if (!gameOver) {
		drawSwitchPlayer();
	}
}

/********************************************************************/
void setDartPosition()
{
	position3D src;
	position3D dst;

    // pozice sipky od stredu terce
    dst.x = (sight.x - WindowWidth/2)*auxVal;
	dst.y = (sight.y - WindowHeight/2)*auxVal-myPointer.position/5;
	dst.z = 7;

	src.x = camera.x;
	src.y = camera.y;
	src.z = camera.z+1;

	//aktualni sipka
	if (darts[DARTS-gameLogic->getCurrentPlayerDarts()] == NULL) {
		darts[DARTS-gameLogic->getCurrentPlayerDarts()] = new Dart(src, dst, myPointer.position);
	}
	else {
		darts[DARTS-gameLogic->getCurrentPlayerDarts()]->setAttributes(src, dst, myPointer.position);
	}
	darts[DARTS-gameLogic->getCurrentPlayerDarts()]->setVisible(true);
}

/********************************************************************/
void drawDartsGame() {
	position3D dartPos;
	int status;
	
	for(int i=0; i<DARTS; i++) {
		if (darts[i] != NULL && darts[i]->isVisible()) {
			float angleX = darts[i]->getAngleX();
			float angleY = darts[i]->getAngleY();
			float angleZ = darts[i]->getAngleZ();
			if (darts[i]->isFlying()) {
				dartPos = darts[i]->move(deltaT);
			} else {
				dartPos = darts[i]->getPosition();
				if (darts[i]->justHit) {
					darts[i]->justHit = false;
					mode = WAITING;
					score = getPoints(-dartPos.x,dartPos.y);
					if (soundOn) {
						PlaySound(DARTHIT, NULL, SND_ASYNC);
					}
					switch(score) {
						case 0 : if (soundOn) {
									PlaySound(HAHA, NULL, SND_ASYNC);
								 }
							break;
						case 50 : if (soundOn) {
										PlaySound(WOOHOO, NULL, SND_ASYNC);
									}
							break;
						default: 
							break;
					}
						
					status = gameLogic->throwDart(score);
					switch (status) {
						case NEXT : 
							//mode = VIEW;
							waitForClick = true;
							break;
						case CONT :
							//mode = GAME;
							break;
						case END :
							if (soundOn) {
								PlaySound(CHAMPION, NULL, SND_ASYNC);
							}
							gameOver = true;
							mode = VIEW;
							break;
					}
				}
			}
							glPushMatrix();
					glTranslatef(dartPos.x, dartPos.y, dartPos.z);
    				glRotatef(-90.0, 0.0, 1.0, 0.0);
					glRotatef(angleY, 0.0, 0.0, 1.0);
					glRotatef(angleX, 0.0, 1.0, 0.0);
					glRotatef(angleZ, 1.0, 0.0, 0.0);
					glScalef(dartScale, dartScale, dartScale);
					dart[i]->draw();
				glPopMatrix();
		}
	}
}


/********************************************************************/
void drawGame() {
	// kamera se diva na pozici urcenou polohou znew
	if (mode == THROWING || mode == WAITING) {
		gluLookAt(cameraRandom.x, cameraRandom.y, camera.z, 0.0, 0.0, 7, 0.0, 1.0, 0.0);
	} else {
		gluLookAt(camera.x, camera.y, camera.z, 0.0, 0.0, 7, 0.0, 1.0, 0.0);
	}

	// vykresleni modelu (terce)
	glPushMatrix();
		glActiveTextureARB(GL_TEXTURE0_ARB);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, TextureTiles);   /* navazani textury */
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		
		glTranslatef(0.0f, 0.0f, 7.0f);
		glRotatef(90, 1.0f, 0.0f, 0.0f);
		glRotatef(180, 0.0f, 0.0f, 1.0f);
		glBegin(GL_QUADS);
			glNormal3f(0.0, 1.0, 0.0);
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0, 0.0);  glVertex3f(-50.0, -0.5, -50.0); 
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0, 8.5);  glVertex3f(-50.0, -0.5, 50.0); 
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 4.0, 8.5);  glVertex3f(50.0, -0.5, 50.0); 
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 4.0, 0.0);  glVertex3f(50.0, -0.5, -50.0); 
		glEnd();
		glDisable(GL_TEXTURE_2D);
		
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		target->draw();
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);
	glPopMatrix();
	
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	drawDartsGame();
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);

	if (motionBlurEnabled) {
		// motion blur
		float q = 0.5;
		glAccum(GL_MULT, q);
		glAccum(GL_ACCUM, 1-q);
		glAccum(GL_RETURN, 1.0);
	}

	timeTick();
	drawPowerBar();
	if (mode == GAME) {
		drawSight();
	}
}

/********************************************************************/
// Funkce pro tisk retezce zadanym rastrovym fontem
void printStringUsingRasterFont(char * string, void * font, int x, int y, float r, float g, float b)
{
	glColor3f(r,g,b);                             /* nastaveni barvy bitmapy */
	glRasterPos2i(x,y);                           /* nastaveni pozice pocatku bitmapy */
	while (*string)                               /* prochazeni celym retezcem */
	glutBitmapCharacter(font, *string++);       /* a tisk jednotlivych znaku */
}

/********************************************************************/
// vykresleni dialogu win
void drawWin(void)
{
    ViewOrtho(WindowWidth, WindowHeight);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    int widthDiv = WindowWidth/20;
    int heightDiv = WindowHeight/15;
    glPushMatrix();
        glEnable(GL_BLEND);    // zapne blending
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);    // funkce blendingu
        glDepthMask(GL_FALSE);    // vypne depth buffer
        glBegin(GL_QUADS);
            glColor4f(0.0, 0.0, 0.0, 0.8);
            glVertex2i(6*widthDiv, 8.5*heightDiv);    // levy dolni
            glVertex2i(14*widthDiv, 8.5*heightDiv);    // pravy dolni
            glVertex2i(14*widthDiv, 6.5*heightDiv);    // pravy horni
            glVertex2i(6*widthDiv, 6.5*heightDiv);    // levy horni
        glEnd();
        glDepthMask(GL_TRUE);    // zapne depth buffer
        glDisable(GL_BLEND);    // vypne blending
    glPopMatrix();

    char winStr[32];
    char winBuf[8];
    itoa(gameLogic->getWinner()+1, winBuf, 10);
    strcpy(winStr, "Player ");
    strcat(winStr, winBuf);
    strcat(winStr, " has won!");

    printStringUsingRasterFont(winStr, GLUT_BITMAP_TIMES_ROMAN_24, 7.7*widthDiv, 7.7*heightDiv, 0.8, 0.8, 0.8);

    ViewPerspective();
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glColor3f(1.0, 1.0, 1.0);
}

/********************************************************************/
// vykresleni ukazatele sily hodu
void drawHud(void)
{
    ViewOrtho(WindowWidth, WindowHeight);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    int widthDiv = WindowWidth/20;
    int heightDiv = WindowHeight/15;
    glPushMatrix();
        glEnable(GL_BLEND);    // zapne blending
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);    // funkce blendingu
        glDepthMask(GL_FALSE);    // vypne depth buffer
        // horni hud
        glBegin(GL_QUADS);
            glColor4f(0.0, 0.0, 0.0, 0.6);
            glVertex2i(0.5*widthDiv, (0.5*(gameLogic->getPlayers()+1)+0.5)*heightDiv);    // levy dolni
            glVertex2i(4.5*widthDiv, (0.5*(gameLogic->getPlayers()+1)+0.5)*heightDiv);    // pravy dolni
            glVertex2i(4.5*widthDiv, 0.5*heightDiv);    // pravy horni
            glVertex2i(0.5*widthDiv, 0.5*heightDiv);    // levy horni
        glEnd();
        // dolni hud
        glBegin(GL_QUADS);
            glColor4f(0.0, 0.0, 0.0, 0.6);
            glVertex2i(0.5*widthDiv, 14.5*heightDiv);    // levy dolni
            glVertex2i(4.5*widthDiv, 14.5*heightDiv);    // pravy dolni
            glVertex2i(4.5*widthDiv, 12.5*heightDiv);    // pravy horni
            glVertex2i(0.5*widthDiv, 12.5*heightDiv);    // levy horni
        glEnd();
        glDepthMask(GL_TRUE);    // zapne depth buffer
        glDisable(GL_BLEND);    // vypne blending
    glPopMatrix();

    // text horni hud
    char gameTypeStr[16];
    switch(settingsType)
    {
        case 0: strcpy(gameTypeStr, "Training"); break;
        case 1: strcpy(gameTypeStr, "Game 101"); break;
        case 2: strcpy(gameTypeStr, "Game 301"); break;
        case 3: strcpy(gameTypeStr, "Game 501"); break;
        case 4: strcpy(gameTypeStr, "Round the clock"); break;
        default: strcpy(gameTypeStr, "Players:"); break;
    }
    printStringUsingRasterFont(gameTypeStr, GLUT_BITMAP_9_BY_15, 0.5*widthDiv+4, 0.5*heightDiv+15, 0.8, 0.8, 0.8);
    
    char str1[32];
    char str2[32];
    char str3[32];
    char str4[32];
    char buffer1[8];
    char buffer2[8];
    char buffer3[8];
    char buffer4[8];
    for(int i=0;i<(gameLogic->getPlayers());i++)
    {
        itoa(gameLogic->getPlayerScore(i), buffer1, 10);
        itoa(i+1, buffer2, 10);

        strcpy (str1,"Player ");
        strcat (str1,buffer2);
        strcat (str1,": ");
        strcat (str1,buffer1);

        printStringUsingRasterFont(str1, GLUT_BITMAP_9_BY_15, 0.5*widthDiv+4, 0.5*(i+2)*heightDiv+15, 0.8, 0.8, 0.8);
    }

    // text dolni hud
    itoa(gameLogic->getCurrentPlayer()+1, buffer1, 10);
    itoa(gameLogic->getCurrentPlayerDarts(), buffer2, 10);
    itoa(score, buffer3, 10);
    itoa(gameLogic->getPlayerScore(gameLogic->getCurrentPlayer()), buffer4, 10);

	if (gameOver) {
		strcpy(str1, "Player "); strcat(str1, buffer1); strcat(str1, " has won!");
	} else {
		strcpy(str1, "Player "); strcat(str1, buffer1); strcat(str1, " GO!");
	}
    strcpy(str2, "Darts left: "); strcat(str2, buffer2);
    strcpy(str3, "Current score: "); strcat(str3, buffer3);
    strcpy(str4, "Total score: "); strcat(str4, buffer4);

    printStringUsingRasterFont(str1, GLUT_BITMAP_9_BY_15, 0.5*widthDiv+4, 12.5*heightDiv+15, 0.8, 0.8, 0.8);
    printStringUsingRasterFont(str2, GLUT_BITMAP_9_BY_15, 0.5*widthDiv+4, 13.0*heightDiv+15, 0.8, 0.8, 0.8);
    printStringUsingRasterFont(str3, GLUT_BITMAP_9_BY_15, 0.5*widthDiv+4, 13.5*heightDiv+15, 0.8, 0.8, 0.8);
    printStringUsingRasterFont(str4, GLUT_BITMAP_9_BY_15, 0.5*widthDiv+4, 14.0*heightDiv+15, 0.8, 0.8, 0.8);

    ViewPerspective();
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glColor3f(1.0, 1.0, 1.0);

    if (gameOver) {
        drawWin();
    }
}

/********************************************************************/
// vypis menu
void drawMenu() {
    ViewOrtho(WindowWidth, WindowHeight);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    int widthDiv = WindowWidth/20;
    int heightDiv = WindowHeight/15;
    glPushMatrix();
        /* nakonfigurujeme texturovani */
        // zaktivuje dlazdice
        glActiveTextureARB(GL_TEXTURE0_ARB);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, TextureMenu);   /* navazani textury */
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        glBegin(GL_QUADS);
            glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0, 0); glVertex2i(0, WindowHeight);    // levy dolni
            glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1, 0); glVertex2i(WindowWidth, WindowHeight);    // pravy dolni
            glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1, 1); glVertex2i(WindowWidth, 0);    // pravy horni
            glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0, 1); glVertex2i(0, 0);    // levy horni
        glEnd();

        glDisable(GL_TEXTURE_2D);

        // ramecek
        glEnable(GL_BLEND);    // zapne blending
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);    // funkce blendingu
        glDepthMask(GL_FALSE);    // vypne depth buffer
        glBegin(GL_QUADS);
            glColor4f(0.0, 0.0, 0.0, 0.6);
            glVertex2i(2*widthDiv, 13*heightDiv);    // levy dolni
            glVertex2i(13*widthDiv, 13*heightDiv);    // pravy dolni
            glVertex2i(13*widthDiv, 2*heightDiv);    // pravy horni
            glVertex2i(2*widthDiv, 2*heightDiv);    // levy horni
        glEnd();
        glDepthMask(GL_TRUE);    // zapne depth buffer
        glDisable(GL_BLEND);    // vypne blending
    glPopMatrix();

    char soundOnStr[32];
    if(soundOn) strcpy(soundOnStr, "[S] sound: on");
    else strcpy(soundOnStr, "[S] sound: off");

    char fullscreenStr[32];
    if(fullscreen) strcpy(fullscreenStr, "[F] fullscreen: on");
    else strcpy(fullscreenStr, "[F] fullscreen: off");

    char motionBlurStr[32];
    if(motionBlurEnabled) strcpy(motionBlurStr, "[B] motion blur: on");
    else strcpy(motionBlurStr, "[B] motion blur: off");

    char settingsPlayersStr[32];
    char settingsPlayersBuf[8];
    itoa(settingsPlayers, settingsPlayersBuf, 10);
    strcpy(settingsPlayersStr, "[P] number of players: ");
    strcat(settingsPlayersStr, settingsPlayersBuf);

    char settingsDifficultyStr[32];
    if(settingsDifficulty==0) strcpy(settingsDifficultyStr, "[D] difficulty: noob");
    if(settingsDifficulty==1) strcpy(settingsDifficultyStr, "[D] difficulty: regular darter");
    if(settingsDifficulty==2) strcpy(settingsDifficultyStr, "[D] difficulty: professional");
    if(settingsDifficulty==3) strcpy(settingsDifficultyStr, "[D] difficulty: killer");

    char settingsTypeStr[32];
    if(settingsType==0) strcpy(settingsTypeStr, "[T] game type: Training");
    if(settingsType==1) strcpy(settingsTypeStr, "[T] game type: 101");
    if(settingsType==2) strcpy(settingsTypeStr, "[T] game type: 301");
    if(settingsType==3) strcpy(settingsTypeStr, "[T] game type: 501");
    if(settingsType==4) strcpy(settingsTypeStr, "[T] game type: Round the Clock");

    printStringUsingRasterFont("DARTS 3D", GLUT_BITMAP_TIMES_ROMAN_24, 2.5*widthDiv+15, 3*heightDiv+10, 0.8, 0.8, 0.8);
    printStringUsingRasterFont("Settings:", GLUT_BITMAP_TIMES_ROMAN_24, 3*widthDiv+15, 4*heightDiv+10, 0.8, 0.8, 0.8);
    printStringUsingRasterFont(settingsPlayersStr, GLUT_BITMAP_TIMES_ROMAN_24, 3*widthDiv+15, 4.7*heightDiv+10, 0.8, 0.8, 0.8);
    printStringUsingRasterFont(settingsTypeStr, GLUT_BITMAP_TIMES_ROMAN_24, 3*widthDiv+15, 5.4*heightDiv+10, 0.8, 0.8, 0.8);
    printStringUsingRasterFont(settingsDifficultyStr, GLUT_BITMAP_TIMES_ROMAN_24, 3*widthDiv+15, 6.1*heightDiv+10, 0.8, 0.8, 0.8);
    printStringUsingRasterFont(soundOnStr, GLUT_BITMAP_TIMES_ROMAN_24, 3*widthDiv+15, 6.8*heightDiv+10, 0.8, 0.8, 0.8);    
    printStringUsingRasterFont(fullscreenStr, GLUT_BITMAP_TIMES_ROMAN_24, 3*widthDiv+15, 7.5*heightDiv+10, 0.8, 0.8, 0.8);
    printStringUsingRasterFont(motionBlurStr, GLUT_BITMAP_TIMES_ROMAN_24, 3*widthDiv+15, 8.2*heightDiv+10, 0.8, 0.8, 0.8);
    
    printStringUsingRasterFont("Controls:", GLUT_BITMAP_TIMES_ROMAN_24, 3*widthDiv+15, 9.2*heightDiv+10, 0.8, 0.8, 0.8);
    printStringUsingRasterFont("[N] new game", GLUT_BITMAP_TIMES_ROMAN_24, 3*widthDiv+15, 9.9*heightDiv+10, 0.8, 0.8, 0.8);
    printStringUsingRasterFont("[H] help", GLUT_BITMAP_TIMES_ROMAN_24, 3*widthDiv+15, 10.6*heightDiv+10, 0.8, 0.8, 0.8);
    printStringUsingRasterFont("[Q] quit", GLUT_BITMAP_TIMES_ROMAN_24, 3*widthDiv+15, 11.3*heightDiv+10, 0.8, 0.8, 0.8);
    if(gameLogic!=NULL) printStringUsingRasterFont("[C] continue", GLUT_BITMAP_TIMES_ROMAN_24, 3*widthDiv+15, 12*heightDiv+10, 0.8, 0.8, 0.8);    
    
    ViewPerspective();
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glColor3f(1.0, 1.0, 1.0);
}

/********************************************************************/
// vykresleni dialogu win
void drawSwitchPlayer(void)
{
    ViewOrtho(WindowWidth, WindowHeight);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    int widthDiv = WindowWidth/20;
    int heightDiv = WindowHeight/15;
    glPushMatrix();
        glEnable(GL_BLEND);    // zapne blending
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);    // funkce blendingu
        glDepthMask(GL_FALSE);    // vypne depth buffer
        glBegin(GL_QUADS);
            glColor4f(0.0, 0.0, 0.0, 0.6);
            glVertex2i(6*widthDiv, 14.5*heightDiv);    // levy dolni
            glVertex2i(14*widthDiv, 14.5*heightDiv);    // pravy dolni
            glVertex2i(14*widthDiv, 13.5*heightDiv);    // pravy horni
            glVertex2i(6*widthDiv, 13.5*heightDiv);    // levy horni
        glEnd();
        glDepthMask(GL_TRUE);    // zapne depth buffer
        glDisable(GL_BLEND);    // vypne blending
    glPopMatrix();

    printStringUsingRasterFont("Press spacebar to switch player!", GLUT_BITMAP_9_BY_15, 6.45*widthDiv, 14.1*heightDiv, 0.8, 0.8, 0.8);

    ViewPerspective();
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glColor3f(1.0, 1.0, 1.0);
}


/********************************************************************/
// vypis help
void drawHelp() {
	ViewOrtho(WindowWidth, WindowHeight);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	int widthDiv = WindowWidth/20;
	int heightDiv = WindowHeight/15;
	glPushMatrix();
		/* nakonfigurujeme texturovani */
		// zaktivuje dlazdice
		glActiveTextureARB(GL_TEXTURE0_ARB);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, TextureMenu);   /* navazani textury */
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		glBegin(GL_QUADS);
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0, 0); glVertex2i(0, WindowHeight);	// levy dolni
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1, 0); glVertex2i(WindowWidth, WindowHeight);	// pravy dolni
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1, 1); glVertex2i(WindowWidth, 0);	// pravy horni
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0, 1); glVertex2i(0, 0);	// levy horni
		glEnd();

		glDisable(GL_TEXTURE_2D);

		// ramecek
		glEnable(GL_BLEND);	// zapne blending
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);	// funkce blendingu
		glDepthMask(GL_FALSE);	// vypne depth buffer
		glBegin(GL_QUADS);
			glColor4f(0.0, 0.0, 0.0, 0.6);
			glVertex2i(1*widthDiv, 14*heightDiv);	// levy dolni
			glVertex2i(19*widthDiv, 14*heightDiv);	// pravy dolni
			glVertex2i(19*widthDiv, 1*heightDiv);	// pravy horni
			glVertex2i(1*widthDiv, 1*heightDiv);	// levy horni
		glEnd();
		glDepthMask(GL_TRUE);	// zapne depth buffer
		glDisable(GL_BLEND);	// vypne blending
	glPopMatrix();

	printStringUsingRasterFont("HELP", GLUT_BITMAP_TIMES_ROMAN_24, 1*widthDiv+15, 1.0*heightDiv+30, 0.8, 0.8, 0.8);
	printStringUsingRasterFont("", GLUT_BITMAP_TIMES_ROMAN_24, 1*widthDiv+15, 1.7*heightDiv+30, 0.8, 0.8, 0.8);
	printStringUsingRasterFont("The new awesome great darts offers you a brand new game types:", GLUT_BITMAP_TIMES_ROMAN_24, 1*widthDiv+15, 2.4*heightDiv+30, 0.8, 0.8, 0.8);
	printStringUsingRasterFont("      501, 301, 101 - player starts with specified score", GLUT_BITMAP_TIMES_ROMAN_24, 1*widthDiv+15, 3.1*heightDiv+30, 0.8, 0.8, 0.8);
	printStringUsingRasterFont("                                 and counts-down the hits till zero", GLUT_BITMAP_TIMES_ROMAN_24, 1*widthDiv+15, 3.8*heightDiv+30, 0.8, 0.8, 0.8);
	printStringUsingRasterFont("      Round the clock - player hits increase figures from 1 to 20, bull", GLUT_BITMAP_TIMES_ROMAN_24, 1*widthDiv+15, 4.5*heightDiv+30, 0.8, 0.8, 0.8);
	printStringUsingRasterFont("                                     and bullseye", GLUT_BITMAP_TIMES_ROMAN_24, 1*widthDiv+15, 5.2*heightDiv+30, 0.8, 0.8, 0.8);
	printStringUsingRasterFont("      Training - just throw darts and watch the hits", GLUT_BITMAP_TIMES_ROMAN_24, 1*widthDiv+15, 5.9*heightDiv+30, 0.8, 0.8, 0.8);
	printStringUsingRasterFont("", GLUT_BITMAP_TIMES_ROMAN_24, 1*widthDiv+15, 6.6*heightDiv+30, 0.8, 0.8, 0.8);
	printStringUsingRasterFont("Power-bar on the right side indicates the strength of throw,", GLUT_BITMAP_TIMES_ROMAN_24, 1*widthDiv+15, 7.3*heightDiv+30, 0.8, 0.8, 0.8);
	printStringUsingRasterFont("middle value is the ideal position for throw.", GLUT_BITMAP_TIMES_ROMAN_24, 1*widthDiv+15, 8.0*heightDiv+30, 0.8, 0.8, 0.8);
	printStringUsingRasterFont("", GLUT_BITMAP_TIMES_ROMAN_24, 1*widthDiv+15, 8.7*heightDiv+30, 0.8, 0.8, 0.8);
	printStringUsingRasterFont("", GLUT_BITMAP_TIMES_ROMAN_24, 1*widthDiv+15, 9.4*heightDiv+30, 0.8, 0.8, 0.8);
	printStringUsingRasterFont("Every player has three darts to throw. When target is hit, click", GLUT_BITMAP_TIMES_ROMAN_24, 1*widthDiv+15, 10.1*heightDiv+30, 0.8, 0.8, 0.8);
	printStringUsingRasterFont("      mouse button to continue throwing.", GLUT_BITMAP_TIMES_ROMAN_24, 1*widthDiv+15, 10.8*heightDiv+30, 0.8, 0.8, 0.8);
	printStringUsingRasterFont("When the player is out of darts, next player comes to turn", GLUT_BITMAP_TIMES_ROMAN_24, 1*widthDiv+15, 11.5*heightDiv+30, 0.8, 0.8, 0.8);
	printStringUsingRasterFont("      by pressing the spacebar.", GLUT_BITMAP_TIMES_ROMAN_24, 1*widthDiv+15, 12.2*heightDiv+30, 0.8, 0.8, 0.8);
	//printStringUsingRasterFont("FIT is SHIT!", GLUT_BITMAP_TIMES_ROMAN_24, 1*widthDiv+15, 12.9*heightDiv+30, 0.8, 0.8, 0.8);

	ViewPerspective();
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glColor3f(1.0, 1.0, 1.0);
}

/********************************************************************/
// vykresleni
void onDisplay()
{	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	/* zapneme osvetleni, nastavime svetlo */
	//glEnable(GL_LIGHTING);
	//glEnable(GL_LIGHT0);
	//glLightfv(GL_LIGHT0, GL_POSITION, lt_position);

	/* nastavime material - vsichni maji stejny pro jednoduchost */
	//glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
	//glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
	//glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	//glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 64);

	switch(mode) {
		case VIEW : drawView(); drawHud();
			break;
		case GAME : 
		case WAITING :
		case THROWING : drawGame(); drawHud();
			break;
		case MENU : drawMenu();
			break;
		case HELP : drawHelp();
			break;
	}

	glFlush();
	glutSwapBuffers();
}

/********************************************************************/
// klavesnice
//void onSpecKey(int key, int x, int y)
//{
//	switch(key)
//	{
//		case GLUT_KEY_F1:
//			mode = HELP;
//			break;
//		case GLUT_KEY_F2:
//			mode = MENU;
//			break;
//		case GLUT_KEY_F3:
//			mode = GAME;
//			break;
//		case GLUT_KEY_F4:
//			mode = VIEW;
//			break;
//		case GLUT_KEY_F5:
//		case GLUT_KEY_F6:
//		case GLUT_KEY_F7:
//		case GLUT_KEY_F8:
//		case GLUT_KEY_F9:
//		case GLUT_KEY_UP:
//		case GLUT_KEY_LEFT:
//		case GLUT_KEY_DOWN:
//		case GLUT_KEY_RIGHT:
//		default:
//			break;
//	}
//
//	glutPostRedisplay();
//}


/********************************************************************/
void onExit() {
	if (soundOn) {
		PlaySound(BEBACK, NULL, SND_SYNC);
	}
	if (gameLogic != NULL) {
		delete gameLogic;
	}
	if (darts != NULL) {
		for( int i=0; i<DARTS; i++ ) {
			if (darts[i] != NULL) {
				delete darts[i];
				darts[i] = NULL;
			}
		}
		delete *darts;
	}
	if (target != NULL) {
		delete target;
	}
	if (target != NULL) {
		delete dart;
	}
	exit(0);
}


/********************************************************************/
// klavesnice
void onKeyboard(unsigned char key, int x, int y)
{
	if (key>='A' && key<='Z') key += 'a'-'A';     /* uprava velkych pismen na mala */

	// MENU mode
	if(mode == MENU)
		switch (key)
		{
			// nova hra
			case 'n':
				if(gameLogic!=NULL) {
					delete gameLogic;
					gameLogic = NULL;
				}
				switch(settingsType) {
					case 1 : gameLogic = new Down(settingsPlayers, 101);
						break;
					case 2 : gameLogic = new Down(settingsPlayers, 301);
						break;
					case 3 : gameLogic = new Down(settingsPlayers, 501);
						break;
					case 4 : gameLogic = new Rtc(settingsPlayers);
						break;
					default : gameLogic = new Training();
						break;
				}
				mode = GAME;
				gameOver = false;
				score = 0;
				if (soundOn) {
					PlaySound(BEEP, NULL, SND_ASYNC);
				}
				for( int i = 0; i < DARTS; i++ ) {
					if (darts[i] != NULL) {
						darts[i]->setVisible(false);
					}
				}
				break;
			// continue
			case 'c':
				if(gameLogic!=NULL) mode = modeStack;
				if (soundOn) PlaySound(BEEP, NULL, SND_ASYNC);
				break;
			// napoveda
			case 'h':
				mode = HELP;
				if (soundOn) PlaySound(BEEP, NULL, SND_ASYNC);
				break;
			// pocet hracu
			case 'p':
				settingsPlayers = (settingsPlayers%8) + 1;
				if (soundOn) PlaySound(BEEP, NULL, SND_ASYNC);
				break;
			// typ hry
			case 't':
				settingsType = (settingsType+1)%5;
				if (soundOn) PlaySound(BEEP, NULL, SND_ASYNC);
				break;
			// obtiznost hry
			case 'd':
				settingsDifficulty = (settingsDifficulty+1)%4;
				if (soundOn) PlaySound(BEEP, NULL, SND_ASYNC);
				break;
			case 'f':
				if (fullscreen) {
					glutReshapeWindow(resX, resY);              /* zmena velikosti okna */
					glutPositionWindow(winPosX, winPosY);       /* posun okna */
					fullscreen = false;
				}
				else {
					glutFullScreen();
					winPosX = glutGet((GLenum)GLUT_WINDOW_X);
					winPosY = glutGet((GLenum)GLUT_WINDOW_Y);
					fullscreen = true;
				}
				if (soundOn) PlaySound(BEEP, NULL, SND_ASYNC);
				break;
			case 'b': 
				motionBlurEnabled = !motionBlurEnabled;
				if (soundOn) PlaySound(BEEP, NULL, SND_ASYNC);
				break;
			case 's': 
                soundOn = !soundOn;
				if (soundOn) PlaySound(BEEP, NULL, SND_ASYNC);
                break;
			// konec
			case 27: //escape
			case 'q':
				if (soundOn) PlaySound(BEEP, NULL, SND_ASYNC);
				onExit();
				break;
			default:
				break;
		}

	// GAME mode
	else {
		switch (key)
		{	
			case 27: //escape
				if(mode!=HELP) modeStack = mode;
				mode = MENU;
				if (soundOn) PlaySound(BEEP, NULL, SND_ASYNC);
				break;
			case 'h' : cheatState = 1;
				break;
			case 'o' : if (cheatState == 1) {
						   cheatState = 2;
					   }
					   else if (cheatState == 4) {
						   cheatsOn = !cheatsOn;
						   cheatState = 0;
					   }
					   else {
						   cheatState = 0;
					   }
				break;
			case 'v' : if (cheatState == 2) {
						   cheatState = 3;
					   }
					   else {
						   cheatState = 0;
					   }
				break;
			case 'n' : if (cheatState == 3) {
						   cheatState = 4;
						   }
					   else {
						   cheatState = 0;
					   }
				break;
			case 32 : 
					if (mode == VIEW && !gameOver) {
						score = 0;
						gameLogic->nextTurn();
						for(int i=0; i<DARTS; i++) {
							darts[i]->setVisible(false);
						}
						mode = GAME;
						if (soundOn) PlaySound(BEEP, NULL, SND_ASYNC);
					}
			default:
				break;
		}
	}
}

/********************************************************************/
// funkce volana pri stisku tlacitek mysi
void onMouseClick(int button, int state, int x, int y)
{
	if (mode == VIEW) {
		if (button == GLUT_LEFT_BUTTON) {             /* leve tlacitko aktivuje rotaci */
			if (state == GLUT_DOWN) {                   /* pri stlaceni */
				stav = 1;                                 /* nastaveni pro funkci motion */
				xx1 = x;                                  /* zapamatovat pozici kurzoru mysi */
				yy1 = y;
			} else {                                    /* GLUT_UP */
				stav = 0;                                 /* normalni stav */
				xold = xnew;                              /* zapamatovat novy pocatek */
				yold = ynew;
			}
		}
		if (button == GLUT_RIGHT_BUTTON) {            /* prave tlacitko aktivuje posun */
			if (state == GLUT_DOWN) {                   /* pri stlaceni */
				stav = 2;                                 /* nastaveni pro funkci motion */
				zz1 = y;                                  /* zapamatovat pozici kurzoru mysi */
			}
			else {
				stav = 0;
				zold = znew;                              /* zapamatovat novy pocatek */
			}
		}
	}
	else if (mode == GAME) {
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
			setDartPosition();
			cameraRandom.x = rand()%10-5;
			cameraRandom.y = rand()%10-5;
			mode = THROWING;
		}
	}
	else if (mode == WAITING && state == GLUT_DOWN) {
		if (waitForClick) {
			mode = VIEW;
			waitForClick = false;
		} else {
			mode = GAME;
		}
	} 
	glutPostRedisplay();                          /* prekresleni sceny */
}

/*************************************************************************/
// funkce volana jestlize se mys pohybuje a je stlaceno nektere tlacitko
void onMouseMotion(int x, int y)
{
	if (mode == VIEW) {
		if (stav == 1) {			/* stav presunu */
			xnew = xold+x-xx1;		/* vypocitat novou pozici */
			ynew = yold+y-yy1;
			glutPostRedisplay();	/* a prekreslit scenu */
		}
		if (stav == 2) {			/* stav presunu */
			znew = zold+y-zz1;		/* vypocitat novou pozici */
			glutPostRedisplay();	/* a prekreslit scenu */
		}
	}
}

/********************************************************************/
// hlavni funkce main
int main(int argc, char* argv[])
{
	glutInit(&argc, argv);				/* inicializace knihovny GLUT */
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);	/* graficky mod okna */
	glutInitWindowSize(resX, resY);		/* pocatecni velikost okna */
	glutInitWindowPosition(winPosY, winPosY);		/* pocatecni pozice okna */
	glutCreateWindow(TITLE);			/* vytvoreni okna pro kresleni */
	glutDisplayFunc(onDisplay);			/* registrace funkce volane pri prekreslovani */
	glutReshapeFunc(onReshape);			/* registrace funkce volane pri zmene velikosti */
	//glutSpecialFunc(onSpecKey);			/* registrace funkce volane pri stisku klavesy */
	glutKeyboardFunc(onKeyboard);		/* registrace funkce volane pri stisku klavesy */
	glutMouseFunc(onMouseClick);		/* registrace funkce volane pri stisku tlacitek mysi */
	glutMotionFunc(onMouseMotion);		/* registrace funkce volane pri pohybu mysi */

	dartRadiusP = resY;

	// inicializace knihovny GLEW pro praci s shadery - OpenGL Extension Wrangler Library 
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}

	Init();								/* provest inicializaci OpenGL */
	Timer(0);							/* casovac */
	glutMainLoop();						/* nekonecna smycka, kde se volaji zaregistrovane funkce */
	return 0;							/* ANSI C potrebuje ukoncit fci main prikazem return */
}

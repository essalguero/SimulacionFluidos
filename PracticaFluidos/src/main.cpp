#include <stdlib.h>
#include <stdio.h>
#include <glut.h>
#include <math.h> 
#include "solver.h"

#include <string>
#include <time.h>

/* global variables */
static int N; // Numero de celdas en la pantalla
static float dt, diff, visc;
static float force, source;
static bool dvel;

static int win_id;
static int win_x, win_y;	//Window Size.
static int mouse_down[3];	//Mouse button states.
static int omx, omy, mx, my;

static Solver solver;

using namespace std;

char* sJacobi = "Using Jacobi method";

char* sGauss = "Using Gauss-Seidel method";

char* sMethod = sGauss;


//time_t lastRenderTime{0};
//double lastRenderTimeSeconds{0};
int framesPerSecond{ 0 };


void renderBitmapString(float x, float y, void *font, const char *string) {
	const char *c;
	glRasterPos2f(x, y);

	glColor3f(0.5f, 0.5f, 1.0f);
	for (c = string; *c != '\0'; c++) {
		glutBitmapCharacter(font, *c);
	}
}

/*void drawStrokeText(char* string,int x,int y,int z)
{
char *c;
char empty = '\0';
glPushMatrix();
glTranslatef(x, y+8,z);
glScalef(0.09f,-0.08f,z);

for (c=string; *c != empty; c++)
{
glutStrokeCharacter(GLUT_STROKE_ROMAN , *c);
}
glPopMatrix();
}*/

/*
----------------------------------------------------------------------
OpenGL specific drawing routines
----------------------------------------------------------------------
*/


void setOrthographicProjection() {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, win_x, 0, win_y - 60);
	glScalef(1, -1, 1);
	glTranslatef(0, -win_y, 0);
	glMatrixMode(GL_MODELVIEW);
}


void resetPerspectiveProjection() {
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}


static void displayText() {
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	string stringFPS = to_string(framesPerSecond) + " FPS";

	const char * charFPS = stringFPS.c_str();

	//string stringIteraciones = "Numero de Iteraciones: " + to_string(solver.getNumeroIteraciones());
	//const char * charIteraciones = stringIteraciones.c_str();

	

	glColor3d(0.5, 0.5, 1.0);
	setOrthographicProjection();
	glPushMatrix();
	glLoadIdentity();
	renderBitmapString(20, win_y - 40, (void *)GLUT_BITMAP_9_BY_15, sMethod);
	//renderBitmapString(20, win_y - 40, (void *)GLUT_BITMAP_9_BY_15, charIteraciones);
	renderBitmapString(20, win_y - 20, (void *)GLUT_BITMAP_9_BY_15, charFPS);
	glPopMatrix();
	resetPerspectiveProjection();
	//glutSwapBuffers();
}

static void PreDisplay(void)
{
	/*glViewport(0, 0, win_x, win_y);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, 1.0, 0.0, 1.0);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);*/



	glViewport(0, 0, win_x, win_y);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, 1.0, 0.0, 1.0);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);


}


static void PostDisplay(void)
{
	displayText();

	glutSwapBuffers();
}

static void DrawVelocity(void)
{
	/*De la misma manera, rellenaremos la función de pintado del campo vectorial:
	static void DrawVelocity(void)
	Esta vez con las funciones glLineWidth(1.0f), glBegin(GL_LINES), glVertex2f(x, y) y glEnd().
	En la medida de lo posible, debería pintarse el campo vectorial de tal manera que se pueda
	diferenciar el sentido en el que fluyen los vectores, esto se consigue añadiendo un color
	dependiente del sentido del vector.
	Para comprobar que todo funciona, probaremos la aplicación y veremos que podemos pintar en ella
	si todo está correctamente hecho. Con “v” podremos ver el campo vectorial.
	*/

	int i, j;

	int altoCelda = win_y / N;
	int anchoCelda = win_x / N;

	float x, y;

	float initX;
	float initY;

	float endX;
	float endY;

	int arrayPosition;

	float h = 1 / (double)(N + 2);

	// Calcular las casillas hechas por Jesus

	/*for (i = 0; i <= N; i++)
	{
	x = (i - 0.5f) * h;
	for (j = 0; j <= N; j++) {
	y = (j - 0.5) * h;
	d00 = solver.dens[XY_TO_ARRAY(i, j)];
	d01 = solver.dens[XY_TO_ARRAY(i, j + 1)];
	...
	}
	}*/

	/*for (i = 0; i <= N; i++)
	{
	x = (i - 0.5f) * altoCelda;
	for (j = 0; j <= N; j++) {
	y = (j - 0.5) * altoCelda;
	}
	}*/

	glLineWidth(1.0f);


	float xPosOrig;
	float yPosOrig;
	float xPosEnd;
	float yPosEnd;

	glBegin(GL_LINES);


	for (int i = 1; i < N + 1; i++)
	{
		xPosOrig = (i - 0.5f) * h;

		for (int j = 1; j < N + 1; ++j)
		{

			yPosOrig = (j - 0.5f) * h;

			xPosEnd = xPosOrig + solver.u[XY_TO_ARRAY(i, j)];
			yPosEnd = yPosOrig + solver.v[XY_TO_ARRAY(i, j)];

			glColor3f(abs(255.0f * (xPosEnd - xPosOrig)), abs(255.0f * (yPosEnd - yPosOrig)), 0.0f);
			//glColor3f(255.0f * abs(static_cast<int>(xPosEnd - xPosOrig)), 255.0f * abs(static_cast<int>(yPosEnd - yPosOrig)), 0.0f);

			glVertex2f(xPosOrig, yPosOrig);

			glVertex2f(xPosEnd, yPosEnd);


		}
	}

	glEnd();

}

/*static void DrawDensity(void)
{
float h = 1 / (double)(N + 2);
float x, y;
float initX;
float initY;
float endX;
float endY;
int arrayPosition;
glBegin(GL_QUADS);
for (int i = 0; i <= N; ++i)
{
for (int j = 0; j <= N; ++j)
{
arrayPosition = XY_TO_ARRAY(i, j);
glColor3f(solver.dens[arrayPosition], solver.dens[arrayPosition], solver.dens[arrayPosition]);
glVertex2f((i - 0.5f) * h, (j - 0.5f) * h);
glVertex2f((i - 0.5f) * h, (j + 0.5f) * h);
glVertex2f((i + 0.5f) * h, (j + 0.5f) * h);
glVertex2f((i + 0.5f) * h, (j - 0.5f) * h);
}
}
glEnd();
}*/

static void DrawDensity(void)
{
	//int i, j;


	float h{ 1 / (float)(N + 2) };


	//int altoCelda = win_y / N;
	//int anchoCelda = win_x / N;

	float x, y;

	float initX;
	float initY;

	float endX;
	float endY;

	//int arrayPosition;

	// Calcular las casillas hechas por Jesus

	//for (i = 0; i <= N; i++)
	//{
	//x = (i - 0.5f) * h;
	//
	//for (j = 0; j <= N; j++) {
	//y = (j - 0.5) * h;
	//
	//d00 = solver.dens[XY_TO_ARRAY(i, j)];
	//d01 = solver.dens[XY_TO_ARRAY(i, j + 1)];
	//...
	//}
	//}

	int i, j;

	glBegin(GL_QUADS);

	//for (int i = 0; i <= N; ++i)
	//{
	//for (int j = 0; j <= N; ++j)
	//{
	FOR_EACH_CELL
	{
		//arrayPosition = XY_TO_ARRAY(i, j);
		float arribaIzquierda{ solver.dens[XY_TO_ARRAY(i - 1, j - 1)] };

		float izquierda{ solver.dens[XY_TO_ARRAY(i, j - 1)] };

		float derecha{ solver.dens[XY_TO_ARRAY(i, j + 1)] };

		float arribaDerecha{ solver.dens[XY_TO_ARRAY(i - 1, j + 1)] };

		float arriba{ solver.dens[XY_TO_ARRAY(i - 1, j)] };
		float abajo{ solver.dens[XY_TO_ARRAY(i + 1, j)] };

		float actual{ solver.dens[XY_TO_ARRAY(i, j)] };

		float abajoIzquierda{ solver.dens[XY_TO_ARRAY(i + 1, j - 1)] };
		float abajoDerecha{ solver.dens[XY_TO_ARRAY(i + 1, j + 1)] };

		float valor{ (actual + arriba + izquierda + arribaIzquierda) / 4 };


		glColor3f(valor, valor, valor);
		glVertex2f((i - 0.5f) * h, (j - 0.5f) * h);


		valor = (actual + arriba + derecha + arribaDerecha) / 4;
		glColor3f(valor, valor, valor);
		glVertex2f((i - 0.5f) * h, (j + 0.5f) * h);


		valor = (actual + abajo + derecha + abajoDerecha) / 4;
		glColor3f(valor, valor, valor);
		glVertex2f((i + 0.5f) * h, (j + 0.5f) * h);


		valor = (actual + abajo + izquierda + abajoIzquierda) / 4;
		glColor3f(valor, valor, valor);
		glVertex2f((i + 0.5f) * h, (j - 0.5f) * h);

	}
	END_FOR
	glEnd();

}

/*
----------------------------------------------------------------------
relates mouse movements to forces and sources
----------------------------------------------------------------------
*/
static void AddInteractionFromUI()
{
	int i, j;

	if (!mouse_down[0] && !mouse_down[2]) return;

	i = (int)((mx / (float)win_x)*N + 1);
	j = (int)(((win_y - my) / (float)win_y)*N + 1);

	if (i<1 || i>N || j<1 || j>N) return;

	if (mouse_down[GLUT_LEFT_BUTTON]) {
		solver.AddVelocity(i, j, force * (mx - omx), force * (omy - my));
	}

	if (mouse_down[GLUT_RIGHT_BUTTON]) {
		solver.AddDensity(i, j, source);
	}

	omx = mx;
	omy = my;

	return;
}

/*
----------------------------------------------------------------------
GLUT callback routines
----------------------------------------------------------------------
*/

static void KeyFunc(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'c':
	case 'C':
		solver.ClearData();
		break;

	case 'q':
	case 'Q':
		solver.FreeData();
		exit(0);
		break;

	case 'v':
	case 'V':
		dvel = !dvel;
		break;

	case 'j':
	case'J':
		solver.setIterativeMethod(1);
		sMethod = sJacobi;
		break;
	case 'g':
	case 'G':
		solver.setIterativeMethod(0);
		sMethod = sGauss;
		break;
	}
}

static void MouseFunc(int button, int state, int x, int y)
{
	omx = mx = x;
	omx = my = y;

	mouse_down[button] = state == GLUT_DOWN;
}

static void MotionFunc(int x, int y)
{
	mx = x;
	my = y;
}

static void ReshapeFunc(int width, int height)
{
	glutSetWindow(win_id);
	glutReshapeWindow(width, height);

	win_x = width;
	win_y = height;
}

static void IdleFunc(void)
{
	solver.ClearPrevData(); //Clean last step forces
	AddInteractionFromUI();	//Add Forces and Densities

	solver.Solve();			//Calculate the next step

	glutSetWindow(win_id);
	glutPostRedisplay();
}


void CalculateFrameRate()
{
	static int localFramesPerSecond = 0.0f;       // This will store our fps
	static float lastTime = 0.0f;       // This will hold the time from the last frame
	int currentTime = clock() / CLOCKS_PER_SEC;
	++localFramesPerSecond;
	if (currentTime - lastTime > 1.0f)
	{
		lastTime = currentTime;
		//if(SHOW_FPS == 1) fprintf(stderr, "\nCurrent Frames Per Second: %d\n\n", (int)framesPerSecond);
		framesPerSecond = localFramesPerSecond;
		localFramesPerSecond = 0;
	}
}


static void DisplayFunc(void)
{

	CalculateFrameRate();

	PreDisplay();

	if (dvel) DrawVelocity();
	else		DrawDensity();

	PostDisplay();
}


/*
----------------------------------------------------------------------
open_glut_window --- open a glut compatible window and set callbacks
----------------------------------------------------------------------
*/

static void OpenGlutWindow(void)
{
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);

	glutInitWindowPosition(0, 0);
	glutInitWindowSize(win_x, win_y);
	win_id = glutCreateWindow("Alias | wavefront");

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glutSwapBuffers();

	PreDisplay();

	glutKeyboardFunc(KeyFunc);
	glutMouseFunc(MouseFunc);
	glutMotionFunc(MotionFunc);
	glutReshapeFunc(ReshapeFunc);
	glutIdleFunc(IdleFunc);
	glutDisplayFunc(DisplayFunc);
}


/*
----------------------------------------------------------------------
main --- main routine
----------------------------------------------------------------------
*/

int main(int argc, char ** argv)
{
	glutInit(&argc, argv);

	if (argc != 1 && argc != 6) {
		fprintf(stderr, "usage : %s N dt diff visc force source\n", argv[0]);
		fprintf(stderr, "where:\n"); \
			fprintf(stderr, "\t N      : grid resolution\n");
		fprintf(stderr, "\t dt     : time step\n");
		fprintf(stderr, "\t diff   : diffusion rate of the density\n");
		fprintf(stderr, "\t visc   : viscosity of the fluid\n");
		fprintf(stderr, "\t force  : scales the mouse movement that generate a force\n");
		fprintf(stderr, "\t source : amount of density that will be deposited\n");
		exit(1);
	}

	if (argc == 1) {
		N = 64;
		//N = 2;
		dt = 0.1f;
		diff = 0.0001f;
		visc = 0.0f;
		force = 5.0f;
		source = 100.0f;
		fprintf(stderr, "Using defaults : N=%d dt=%g diff=%g visc=%g force = %g source=%g\n",
			N, dt, diff, visc, force, source);
	}
	else {
		N = atoi(argv[1]);
		dt = atof(argv[2]);
		diff = atof(argv[3]);
		visc = atof(argv[4]);
		force = atof(argv[5]);
		source = atof(argv[6]);
	}

	printf("\n\nHow to use this demo:\n\n");
	printf("\t Add densities with the right mouse button\n");
	printf("\t Add velocities with the left mouse button and dragging the mouse\n");
	printf("\n");
	printf("\t Toggle density/velocity display with the 'v' key\n");
	printf("\t Clear the simulation by pressing the 'c' key\n");
	printf("\n");
	printf("\t Set the iterative method to Gauss-Seidel by pressing 'g' key\n");
	printf("\t Set the iterative method to Jacovi by pressing 'j' key\n");
	printf("\t Quit by pressing the 'q' key\n");

	dvel = false;

	solver.Init(N, dt, diff, visc);

	if (!solver.AllocateData()) exit(1);

	win_x = 512;
	win_y = 512;
	OpenGlutWindow();

	glutMainLoop();

	exit(0);
}

#include <stdlib.h>
#include <stdio.h>
#include <glut.h>
#include <math.h> 
#include "solver.h"

#include <string>
#include <time.h>

#include <iostream>

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


// Set of variables to show as 'information' in the screen
char* sJacobi = "Using Jacobi method";
char* sGauss = "Using Gauss-Seidel method";
char* sOverRelaxed = "Using Gauss-Seidel Over-Relaxed method";
char* sMethod = sGauss;

char* sVelocity = "Velocity View";
char* sDensity = "Density View";
char* sView = sDensity;

int framesPerSecond{ 0 };

bool displayInfo{ true };

bool drawCube{ false };

using namespace std;

// This function writes a string using glut functions
// It has been obtained from Internet
void renderBitmapString(float x, float y, void *font, const char *string) {
	const char *c;
	glRasterPos2f(x, y);

	glColor3f(0.5f, 0.5f, 1.0f);
	for (c = string; *c != '\0'; c++) {
		glutBitmapCharacter(font, *c);
	}
}


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


// This function is in charge of displaying execution information in the OpenGL window
static void displayText() {
	
	string stringFPS = to_string(framesPerSecond) + " FPS";
	const char * charFPS = stringFPS.c_str();

	string stringIteraciones = "Numero de Iteraciones: " + to_string(solver.getNumeroIteraciones());
	const char * charIteraciones = stringIteraciones.c_str();

	glColor3d(0.5, 0.5, 1.0);
	setOrthographicProjection();
	glPushMatrix();
	glLoadIdentity();
	renderBitmapString(20, win_y - 60, (void *)GLUT_BITMAP_9_BY_15, sMethod);
	renderBitmapString(20, win_y - 40, (void *)GLUT_BITMAP_9_BY_15, charIteraciones);
	renderBitmapString(win_x - 140, win_y - 40, (void *)GLUT_BITMAP_9_BY_15, sView);
	renderBitmapString(20, win_y - 20, (void *)GLUT_BITMAP_9_BY_15, charFPS);
	glPopMatrix();
	resetPerspectiveProjection();
}


static void PreDisplay(void)
{
	
	glViewport(0, 0, win_x, win_y);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, 1.0, 0.0, 1.0);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);


}


static void PostDisplay(void)
{
	// If the option is active, show 'execution' info in the screen
	if (displayInfo)
		displayText();

	// Send the command to display the image
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


	float h = 1 / (double)(N + 2);

	
	glLineWidth(1.0f);

	float xPosOrig;
	float yPosOrig;
	float xPosEnd;
	float yPosEnd;

	glBegin(GL_LINES);


	FOR_EACH_CELL
	{
		xPosOrig = (i - 0.5f) * h;
		yPosOrig = (j - 0.5f) * h;

		int arrayPosition = XY_TO_ARRAY(i, j);

		xPosEnd = xPosOrig + solver.u[XY_TO_ARRAY(i, j)];
		yPosEnd = yPosOrig + solver.v[XY_TO_ARRAY(i, j)];

		glColor3f(abs(20.0f * ((xPosEnd - xPosOrig))), abs(20.0f * ((yPosEnd - yPosOrig))), 0.0f);

		glVertex2f(xPosOrig, yPosOrig);


		glVertex2f(xPosEnd, yPosEnd);

	}
	END_FOR


	glEnd();

}


static void DrawFixedSquare(void)
{
	float h{ 1 / (float)(N + 2) };


	int i = FIXED_OBJECT_POSITION;
	int j = FIXED_OBJECT_POSITION;

	glBegin(GL_QUADS);

	float valor = 255.0f;

	glColor3f(0.0f, 0.0f, valor);



	glVertex2f((i - 0.5f) * h, (j - 0.5f) * h);
	glVertex2f((i + FIXED_OBJECT_WIDTH - 0.5f) * h, (j - 0.5f) * h);
	glVertex2f((i + FIXED_OBJECT_WIDTH - 0.5f) * h, (j + FIXED_OBJECT_WIDTH - 0.5f) * h);
	glVertex2f((i - 0.5f) * h, (j + FIXED_OBJECT_WIDTH - 0.5f) * h);

	glEnd();
}

static void DrawDensity(void)
{
	//int i, j;


	float h{ 1 / (float)(N + 2) };


	

	int i, j;

	glBegin(GL_QUADS);

	
	FOR_EACH_CELL
	{
		// Intermediate variables to make the code easier to read
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


		// To avoid pixelation, make an interpolation using 4 cells
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

		if (dvel)
			sView = sVelocity;
		else
			sView = sDensity;

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
	case 'o':
	case 'O':
		solver.setIterativeMethod(2);
		sMethod = sOverRelaxed;
		break;
	case 'd':
	case 'D':
			drawCube = !drawCube;
			break;
	case 'i':
	case 'I':
		displayInfo = !displayInfo;
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


// This function calculates a frame rate
void CalculateFrameRate()
{
	//Variables that need to be kept among calls to this function
	static int localFramesPerSecond = 0.0f;
	static float lastTime = 0.0f;

	// Get the current time
	int currentTime = clock() / CLOCKS_PER_SEC;
	++localFramesPerSecond;

	//Once it has been more than a second since the last update, it is time to get the frame rate
	if (currentTime - lastTime > 1.0f)
	{
		// update the variable lastTime to use it in the next calculation
		lastTime = currentTime;
		
		// Update the global variable used to print the information in the screen
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

	if (drawCube)
	{
		DrawFixedSquare();
		solver.ActivateFixedObject(FIXED_OBJECT_POSITION, FIXED_OBJECT_WIDTH);
	}
	else
	{
		solver.DeactivateFixedObject();
	}

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
	printf("'\n");
	printf("\t Activate the fixed cube by pressing 'd' key\n");
	printf("\n");
	printf("\t Set the iterative method to Gauss-Seidel by pressing 'g' key\n");
	printf("\t Set the iterative method to Gauss-Seidel Over-Relaxation by pressing 'o' key\n");
	printf("\t Set the iterative method to Jacobi by pressing 'j' key\n");
	printf("\n");
	printf("\t Toggle density/velocity display with the 'v' key\n");
	printf("\t Toggle info display with the 'i' key\n");
	printf("\n");
	printf("\t Clear the simulation by pressing the 'c' key\n");
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

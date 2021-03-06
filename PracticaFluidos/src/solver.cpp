#include "solver.h"
#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <math.h>
#include <string>
using namespace std;

void Solver::Init(unsigned N, float dt, float diff, float visc)
{
	this->dt = dt;
	this->diff = diff;
	this->visc = visc;
	this->N = N;
	this->iterativeMethod = 0;
}

/*
----------------------------------------------------------------------
free/clear/allocate simulation data
----------------------------------------------------------------------
*/
void Solver::FreeData(void)
{
	//TODO: Libera los buffers de memoria.
	if (u_prev != 0)
	{
		free(u_prev);
	}

	if (v_prev != 0)
	{
		free(v_prev);
	}

	if (dens_prev != 0)
	{
		free(dens_prev);
	}


	if (u != 0)
	{
		free(u);
	}

	if (v != 0)
	{
		free(v);
	}

	if (dens != 0)
	{
		free(dens);
	}
}

void Solver::ClearData(void)
{

	for (int i = 0; i < (N + 2) * (N + 2); ++i)
	{
		u_prev[i] = 0;
		v_prev[i] = 0;
		dens_prev[i] = 0;

		u[i] = 0;
		v[i] = 0;
		dens[i] = 0;
	}
}

bool Solver::AllocateData(void)
{
	//TODO:
	//Reservamos memoria, en caso de fallo devlvemos false.
	//Antes de devolver true, hay que limpiar la memoria reservada con un ClearData().

	if (!(u_prev = (float *)malloc((N + 2) * (N + 2) * sizeof(float))))
		return false;

	if (!(v_prev = (float *)malloc((N + 2) * (N + 2) * sizeof(float))))
		return false;

	if (!(dens_prev = (float *)malloc((N + 2) * (N + 2) * sizeof(float))))
		return false;

	if (!(u = (float *)malloc((N + 2) * (N + 2) * sizeof(float))))
		return false;

	if (!(v = (float *)malloc((N + 2) * (N + 2) * sizeof(float))))
		return false;

	if (!(dens = (float *)malloc((N + 2) * (N + 2) * sizeof(float))))
		return false;

	ClearData();

	return true;

}

void Solver::ClearPrevData()
{
	//TODO: Borra el contenido de los buffers _prev

	for (int i = 0; i < (N + 2) * (N + 2); ++i)
	{
		u_prev[i] = 0;
		v_prev[i] = 0;
		dens_prev[i] = 0;
	}
}

void Solver::AddDensity(unsigned x, unsigned y, float source)
{
	//TODO: Añade el valor de source al array de densidades. SerÃ­a interesante usar la macro: XY_TO_ARRAY

	int position = XY_TO_ARRAY(x, y);

	dens_prev[position] += source;
}

void Solver::AddVelocity(unsigned x, unsigned y, float forceX, float forceY)
{
	//TODO: Añade el valor de fuerza a sus respectivos arrays. SerÃ­a interesante usar la macro: XY_TO_ARRAY
	int position = XY_TO_ARRAY(x, y);

	u_prev[position] += forceX;
	v_prev[position] += forceY;
}

void Solver::Solve()
{
	VelStep();
	DensStep();
}

void Solver::DensStep()
{
	AddSource(dens, dens_prev);			//Adding input density (dens_prev) to final density (dens).
	SWAP(dens_prev, dens)				//Swapping matrixes, because we want save the next result in dens, not in dens_prev.
	Diffuse(0, dens, dens_prev);		//Writing result in dens because we made the swap before. bi = dens_prev. The initial trash in dens matrix, doesnt matter, because it converges anyways.
	SWAP(dens_prev, dens)				//Swapping matrixes, because we want save the next result in dens, not in dens_prev.
	Advect(0, dens, dens_prev, u, v);	//Advect phase, result in dens.
}

void Solver::VelStep()
{
	AddSource(u, u_prev);
	AddSource(v, v_prev);
	SWAP(u_prev, u)
	SWAP(v_prev, v)
	Diffuse(1, u, u_prev);
	Diffuse(2, v, v_prev);
	Project(u, v, u_prev, v_prev);		//Mass conserving.
	SWAP(u_prev, u)
	SWAP(v_prev, v)
	Advect(1, u, u_prev, u_prev, v_prev);
	Advect(2, v, v_prev, u_prev, v_prev);
	Project(u, v, u_prev, v_prev);		//Mass conserving.
}

void Solver::AddSource(float * base, float * source)
{
	//TODO: Teniendo en cuenta dt (Delta Time), incrementar el array base con nuestro source. Esto sirve tanto para aÃ±adir las nuevas densidades como las nuevas fuerzas.
	
	int i, j;

	if (base != 0 && source != 0)
	{
		FOR_EACH_CELL
		{
			base[XY_TO_ARRAY(i, j)] += source[XY_TO_ARRAY(i, j)] * dt;
		}
		END_FOR
	}
}

void Solver::SetBounds(int b, float * x)
{
	/*TODO:
	Input b: 0, 1 or 2.
	0: borders = same value than the inner value.
	1: x axis borders inverted, y axis equal.
	2: y axis borders inverted, x axis equal.
	Corner values allways are mean value between associated edges.
	*/

	int initialPosition = ((N / 2) - 2) * N + (N / 2 - 2);
	int i, j;

	switch (b)
	{
	case 0:
		// inicializar horizontales
		for (i = 1; i < (N * N); i += (N + 2))
		{
			x[i - 1] = x[i];
		}

		for (i = N + 1; i < (N * N); i += (N + 2))
		{
			x[i] = x[i - 1];
		}

		// inicializar verticales
		for (i = 1; i <= N; ++i)
		{
			x[i] = x[i + N + 2];
			x[((N + 2) * (N + 1)) + i] = x[((N + 2) * (N)) + i];
		}

		break;
	case 1:
		// inicializar horizontales
		for (i = 1; i < (N * N); i += (N + 2))
		{
			x[i - 1] = -(x[i]);
		}

		for (i = N + 1; i < (N * N); i += (N + 2))
		{
			x[i] = -(x[i - 1]);
		}

		// inicializar verticales
		for (i = 1; i <= N; ++i)
		{
			x[i] = x[i + N + 2];
			x[((N + 2) * (N + 1)) + i] = x[((N + 2) * (N)) + i];
		}
		
		break;
	case 2:
		// inicializar horizontales
		for (i = 1; i < (N * N); i += (N + 2))
		{
			x[i - 1] = x[i];
		}

		for (i = N + 1; i < (N * N); i += (N + 2))
		{
			x[i] = x[i - 1];
		}

		// inicializar verticales
		for (i = 1; i <= N; ++i)
		{
			x[i] = -(x[i + N + 2]);
			x[((N + 2) * (N + 1)) + i] = -(x[((N + 2) * (N)) + i]);
		}

		break;
	}

	// Check if the option to display an object in the screen is active
	// In that case, set the boundary conditions for the fixed object
	if (fixedObjectActive)
	{
		for (i = objectPosition; i < (objectPosition + objectSize); ++i)
		{
			for (j = objectPosition; j < (objectPosition + objectSize); ++j)
			{
				x[XY_TO_ARRAY(i, j)] = 0;
			}
		}

		for (i = objectPosition; i <= objectPosition; ++i)
		{
			for (j = objectPosition; j < (objectPosition + objectSize); ++j)
			{
				x[XY_TO_ARRAY(i, j)] = -x[XY_TO_ARRAY(i - 1, j)];
				x[XY_TO_ARRAY(i + (objectSize - 1), j)] = -x[XY_TO_ARRAY(i + objectSize, j)];
			}
		}

		for (i = objectPosition; i < objectPosition + objectSize; ++i)
		{
			for (j = objectPosition; j <= objectPosition; ++j)
			{
				x[XY_TO_ARRAY(i, j)] = -x[XY_TO_ARRAY(i, j - 1)];
				x[XY_TO_ARRAY(i, j + (objectSize - 1))] = -x[XY_TO_ARRAY(i, j + objectSize)];
			}
		}
	}


}


// Jacobi es similar a Gauss-Seidel, excepto en que en Gauss-Seidel, en la misma iteración se
// empiezan a utilizar los valores ya calculados y en Jacobi no se usan hasta la siguiente iteración
void Solver::Jacobi(int b, float * x, float * x0, float aij, float aii)
{

	int i = 0;
	int j = 0;


	float *result = (float *)malloc(sizeof(float) * (N + 2) * (N + 2));

	float sumaTerminos;

	int k;
	for (k = 0; k < NUMERO_ITERACIONES; k++)
	{
		FOR_EACH_CELL
		{

			sumaTerminos = 0;

			sumaTerminos -= x0[XY_TO_ARRAY(i, j - 1)];

			sumaTerminos -= x0[XY_TO_ARRAY(i - 1, j)];

			sumaTerminos -= x0[XY_TO_ARRAY(i + 1, j)];

			sumaTerminos -= x0[XY_TO_ARRAY(i, j + 1)];


			x[XY_TO_ARRAY(i, j)] = ((-aij * sumaTerminos) + x0[XY_TO_ARRAY(i, j)]) / aii;
		}
		END_FOR


		numeroIteraciones = k + 1;

		SetBounds(b, x);

		memccpy(x0, x, (N + 2) * (N + 2), sizeof(float));

	}
	free(result);
}

/*
https://www.youtube.com/watch?v=62_RUX_hrT4
https://es.wikipedia.org/wiki/M%C3%A9todo_de_Gauss-Seidel <- SoluciÃ³n de valores independientes.
Despreciando posibles valores de x no contiguos, se simplifica mucho. Mirar diapositivas y la soluciÃ³n de Gauss Seidel de tÃ©rminos independientes.
Gauss Seidel -> Matrix x and x0
*/
void Solver::LinSolve(int b, float * x, float * x0, float aij, float aii)
{
	//TODO: Se recomienda usar FOR_EACH_CELL, END_FOR y XY_TO_ARRAY.

	int i = 0;
	int j = 0;

	float sumaTerminos;


	int k;
	for (k = 0; k < NUMERO_ITERACIONES; k++)
	{

		FOR_EACH_CELL
		{

			sumaTerminos = 0;

			sumaTerminos -= x[XY_TO_ARRAY(i, j - 1)];

			sumaTerminos -= x[XY_TO_ARRAY(i - 1, j)];

			sumaTerminos -= x[XY_TO_ARRAY(i + 1, j)];

			sumaTerminos -= x[XY_TO_ARRAY(i, j + 1)];


			x[XY_TO_ARRAY(i, j)] = ((-aij * sumaTerminos) + x0[XY_TO_ARRAY(i, j)]) / aii;

		}
		END_FOR

		SetBounds(b, x);

		numeroIteraciones = k + 1;

		memccpy(x0, x, (N + 2) * (N + 2), sizeof(float));
	}


}


void Solver::OverRelaxation(int b, float * x, float * x0, float aij, float aii)
{
	float w = 1.5f;

	int i = 0;
	int j = 0;
	//int arrayPosition;

	float sumaTerminos;

	int k;
	for (k = 0; k < NUMERO_ITERACIONES; k++)
	{
		FOR_EACH_CELL
		{

			sumaTerminos = 0;

			sumaTerminos -= x[XY_TO_ARRAY(i, j - 1)];

			sumaTerminos -= x[XY_TO_ARRAY(i - 1, j)];

			sumaTerminos -= x[XY_TO_ARRAY(i + 1, j)];

			sumaTerminos -= x[XY_TO_ARRAY(i, j + 1)];


			x[XY_TO_ARRAY(i, j)] = ((-aij * sumaTerminos) + x0[XY_TO_ARRAY(i, j)]);

			x[XY_TO_ARRAY(i, j)] *= w / aii;

			x[XY_TO_ARRAY(i, j)] += (1 - w) * x0[XY_TO_ARRAY(i, j)];

		}
		END_FOR

		SetBounds(b, x);

		numeroIteraciones = k + 1;

		memccpy(x0, x, (N + 2) * (N + 2), sizeof(float));
	}

}

/*
Nuestra funciÃ³n de difusiÃ³n solo debe resolver el sistema de ecuaciones simplificado a las celdas contiguas de la casilla que queremos resolver,
por lo que solo con la entrada de dos valores, debemos poder obtener el resultado.
*/
void Solver::Diffuse(int b, float * x, float * x0)
{
	//TODO: Solo necesitaremos pasar dos parÃ¡metros a nuestro resolutor de sistemas de ecuaciones de Gauss Seidel. Calculamos dichos valores y llamamos a la resoluciÃ³n del sistema.
	// Done.

	float aij = this->diff * this->dt * this->N * this->N;
	float aii = 1 + (4 * aij);


	switch (iterativeMethod)
	{
		case 0:
			LinSolve(b, x, x0, aij, aii);
			break;
		case 1:
			Jacobi(b, x, x0, aij, aii);
			break;
		case 2:
			OverRelaxation(b, x, x0, aij, aii);
			break;
	}
}

/*
d is overwrited with the initial d0 data and affected by the u & v vectorfield.
Hay que tener en cuenta que el centro de las casillas representa la posiciÃ³n entera dentro de la casilla, por lo que los bordes estan
en las posiciones x,5.
*/
void Solver::Advect(int b, float * d, float * d0, float * u, float * v)
{
	//TODO: Se aplica el campo vectorial realizando una interploaciÃ³n lineal entre las 4 casillas mÃ¡s cercanas donde caiga el nuevo valor.

	//Variables necesarias para realizar el bucle. Mirar definicion de macro FOR_EACH_CELL
	int i;
	int j;

	// Guardan el valor actual de la casilla donde se estan realizando los calculos
	float currentUValue;
	float currentVValue;

	//Guardan la posicion desde la que iniciar la interpolacion lineal
	//Desde esta posicion (multiplicando por la velocidad, llegan los nuevos valores
	float originUPosition;
	float originVPosition;

	//Guardan valores enteros donde acceder a la matriz (para realizar la interpolacion)
	int integerUPosition;
	int integerVPosition;

	//Guardan el resto de la posicion. Estos valores se utilizan para hacer la ponderacion de los valores
	//en la interpolacion del nuevo valor
	float remainderUPosition;
	float remainderVPosition;

	//Guardan valores intermedios de la interpolacion (los valores horizontales)
	float u0Interpolation;
	float u1Interpolation;

	// Se recorren todas las celdas del array
	FOR_EACH_CELL
	{
		//Obtener los valores actuales de velocidades
		currentUValue = u[XY_TO_ARRAY(i, j)];
		currentVValue = v[XY_TO_ARRAY(i, j)];

		//Utilizando la velocidad, el tamaÂ–o del grid y delta time (dt) calcular la posicion desde
		//la que hay que calcular el nuevo valor
		originUPosition = i - (currentUValue * dt * N);
		originVPosition = j - (currentVValue * dt * N);

		//Obtener valores enteros para poder acceder al array
		integerUPosition = static_cast<int>(trunc(originUPosition));
		integerVPosition = static_cast<int>(trunc(originVPosition));

		//Los valores fraccionarios se deben guardar para realizar la interpolacion lineal del nuevo valor
		//Usado para calcular la ponderacion de las celdas de la pantalla al calcular el nuevo valor
		remainderUPosition = abs(originUPosition - integerUPosition);
		remainderVPosition = abs(originVPosition - integerVPosition);

		//Comprobar si las posiciones a utilizar para los calculos estan dentro del array
		//Si estan dentro del array se hace una interpolacion para calcular el nuevo valor
		if (integerUPosition > 1 && integerUPosition < N + 1 && integerVPosition > 1 && integerVPosition < N + 1)
		{
			// Valor de interpolacion de 2 casillas en horizontal -> Las dos casillas superiores
			u0Interpolation = (d0[XY_TO_ARRAY(integerUPosition, integerVPosition)] * (1 - remainderUPosition)) +
								(d0[XY_TO_ARRAY(integerUPosition + 1, integerVPosition)] * remainderUPosition);

			// Valor de interpolacion de 2 casillas en horizontal -> Las dos casillas inferiores
			u1Interpolation = (d0[XY_TO_ARRAY(integerUPosition, integerVPosition + 1)] * (1 - remainderUPosition)) +
								(d0[XY_TO_ARRAY(integerUPosition + 1, integerVPosition + 1)] * remainderUPosition);

			// Valor de interpolacion de las casillas en vertical -> Calculado sobre las interpolaciones anteriores
			d[XY_TO_ARRAY(i, j)] = (u0Interpolation * (1 - remainderVPosition)) + (u1Interpolation * remainderVPosition);
		}
		else
		{
			//Si alguna de las posiciones calculadas estan fuera del grid, se pone el valor 0 como nuevo valor
			//No se puede realizar otro calculo al no tener conocimiento de valores externos al grid
			d[XY_TO_ARRAY(i, j)] = 0;
		}

	}
	END_FOR
}



/*
Se encarga de estabilizar el fluido y hacerlo conservativo de masa. Se usa solo en las matrices de velocidades.
No necesaria implementaciÃ³n por su complejidad.
*/
void Solver::Project(float * u, float * v, float * p, float * div)
{
	int i, j;

	FOR_EACH_CELL
		div[XY_TO_ARRAY(i, j)] = -0.5f*(u[XY_TO_ARRAY(i + 1, j)] - u[XY_TO_ARRAY(i - 1, j)] + v[XY_TO_ARRAY(i, j + 1)] - v[XY_TO_ARRAY(i, j - 1)]) / N;
	p[XY_TO_ARRAY(i, j)] = 0;
	END_FOR
		SetBounds(0, div);
	SetBounds(0, p);

	LinSolve(0, p, div, 1, 4);

	//Aproximamos: Laplaciano de q a su gradiente.
	FOR_EACH_CELL
		u[XY_TO_ARRAY(i, j)] -= 0.5f*N*(p[XY_TO_ARRAY(i + 1, j)] - p[XY_TO_ARRAY(i - 1, j)]);
	v[XY_TO_ARRAY(i, j)] -= 0.5f*N*(p[XY_TO_ARRAY(i, j + 1)] - p[XY_TO_ARRAY(i, j - 1)]);
	END_FOR
		SetBounds(1, u);
	SetBounds(2, v);
}

void Solver::setIterativeMethod(int method)
{
	iterativeMethod = method;
}

void Solver::ActivateFixedObject(int position, int size)
{
	fixedObjectActive = true;

	objectPosition = position;
	objectSize = size;

}
void Solver::DeactivateFixedObject()
{
	fixedObjectActive = false;
}
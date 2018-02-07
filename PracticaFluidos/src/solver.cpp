#include "solver.h"
#include <stdlib.h>
#include <stdio.h>

void Solver::Init(unsigned N, float dt, float diff, float visc)
{
	this->dt = dt;
	this->diff = diff;
	this->visc = visc;
	this->N = N;
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
	/*if (u_prev != 0 && v_prev != 0 && dens_prev != 0 && 
		u != 0 && v != 0 && dens != 0)*/
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

	if (! (u_prev = (float *)malloc((N + 2) * (N + 2) * sizeof(float))) )
		return false;

	if (! (v_prev = (float *)malloc((N + 2) * (N + 2) * sizeof(float))) )
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
//TODO: Añade el valor de source al array de densidades. Sería interesante usar la macro: XY_TO_ARRAY

	int position = XY_TO_ARRAY(x, y);

	dens_prev[position] += source;
}

void Solver::AddVelocity(unsigned x, unsigned y, float forceX, float forceY)
{
//TODO: Añade el valor de fuerza a sus respectivos arrays. Sería interesante usar la macro: XY_TO_ARRAY
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
	SWAP (u_prev,u)			
	SWAP (v_prev, v)
	Diffuse(1, u, u_prev);  
	Diffuse(2, v, v_prev); 
	//Project(u, v, u_prev, v_prev);		//Mass conserving.
	SWAP (u_prev,u)			
	SWAP (v_prev,v)
	Advect(1, u, u_prev, u_prev, v_prev);
	Advect(2, v, v_prev, u_prev, v_prev);
	//Project(u, v, u_prev, v_prev);		//Mass conserving.
}

void Solver::AddSource(float * base, float * source)
{
//TODO: Teniendo en cuenta dt (Delta Time), incrementar el array base con nuestro source. Esto sirve tanto para añadir las nuevas densidades como las nuevas fuerzas.
	if (base != 0 && source != 0)
	{
		for (int i = 0; i < (N + 2) * (N + 2); ++i)
		{

			/*if (source[i])
				printf("Punto definido para depuracion\n");*/

			base[i] += source[i] * dt;
		}
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
	switch (b)
	{
	case 0:
		// inicializar horizontales
		for (int i = 1; i < (N * N); i += (N + 2))
		{
			x[i - 1] = x[i];
		}

		for (int i = N + 1; i < (N * N); i += (N + 2))
		{
			x[i] = x[i + 1];
		}

		// inicializar verticales
		for (int i = 1; i <= N; ++i)
		{
			x[i] = x[i + N + 2];
			x[((N + 2) * (N + 1)) + i] = x[((N + 2) * (N)) + i];
		}

		break;
	case 1:
		// inicializar horizontales
		for (int i = 1; i < (N * N); i += (N + 2))
		{
			x[i - 1] = -(x[i]);
		}

		for (int i = N + 1; i < (N * N); i += (N + 2))
		{
			x[i] = -(x[i + 1]);
		}

		// inicializar verticales
		for (int i = 1; i <= N; ++i)
		{
			x[i] = x[i + N + 2];
			x[((N + 2) * (N + 1)) + i] = x[((N + 2) * (N)) + i];
		}

		break;
	case 2:
		// inicializar horizontales
		for (int i = 1; i < (N * N); i += (N + 2))
		{
			x[i - 1] = x[i];
		}

		for (int i = N + 1; i < (N * N); i += (N + 2))
		{
			x[i] = x[i + 1];
		}

		// inicializar verticales
		for (int i = 1; i <= N; ++i)
		{
			x[i] = -(x[i + N + 2]);
			x[((N + 2) * (N + 1)) + i] = -(x[((N + 2) * (N)) + i]);
		}
		break;
	}
}

/*
https://www.youtube.com/watch?v=62_RUX_hrT4
https://es.wikipedia.org/wiki/M%C3%A9todo_de_Gauss-Seidel <- Solución de valores independientes.
Despreciando posibles valores de x no contiguos, se simplifica mucho. Mirar diapositivas y la solución de Gauss Seidel de términos independientes.
Gauss Seidel -> Matrix x and x0
*/
void Solver::LinSolve(int b, float * x, float * x0, float aij, float aii)
{
//TODO: Se recomienda usar FOR_EACH_CELL, END_FOR y XY_TO_ARRAY.

	int i = 0;
	int j = 0;
	int arrayPosition;



	FOR_EACH_CELL
		arrayPosition = XY_TO_ARRAY(i, j);

	    x[XY_TO_ARRAY(i, j)] = (-aij * (x[XY_TO_ARRAY(i, j - 1)] - x[XY_TO_ARRAY(i - 1, j)] - x[XY_TO_ARRAY(i + 1, j)]) - x[XY_TO_ARRAY(i, j + 1)] + b) / aii;
	END_FOR

}

/*
Nuestra función de difusión solo debe resolver el sistema de ecuaciones simplificado a las celdas contiguas de la casilla que queremos resolver,
por lo que solo con la entrada de dos valores, debemos poder obtener el resultado.
*/
void Solver::Diffuse(int b, float * x, float * x0)
{
//TODO: Solo necesitaremos pasar dos parámetros a nuestro resolutor de sistemas de ecuaciones de Gauss Seidel. Calculamos dichos valores y llamamos a la resolución del sistema.

	float a = this->diff * this->dt * this->N * this->N;
	LinSolve(b, x, x0, a, 4 * a);

}

/*
d is overwrited with the initial d0 data and affected by the u & v vectorfield.
Hay que tener en cuenta que el centro de las casillas representa la posición entera dentro de la casilla, por lo que los bordes estan
en las posiciones x,5.
*/
void Solver::Advect(int b, float * d, float * d0, float * u, float * v)
{
//TODO: Se aplica el campo vectorial realizando una interploación lineal entre las 4 casillas más cercanas donde caiga el nuevo valor.
}

/*
Se encarga de estabilizar el fluido y hacerlo conservativo de masa. Se usa solo en las matrices de velocidades.
No necesaria implementación por su complejidad.
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
#ifndef _SOLVER_H_
#define _SOLVER_H_

#define XY_TO_ARRAY(i,j) ((i)+(N+2)*(j))
#define FOR_EACH_CELL for ( i=1 ; i<=N ; i++ ) { for ( j=1 ; j<=N ; j++ ) {
#define END_FOR }}
#define SWAP(x0,x) {float * tmp=x0;x0=x;x=tmp;}

#define FIXED_OBJECT_POSITION 30
#define FIXED_OBJECT_WIDTH 8 

// Define el numero de veces que se va a ejecutar el merodo iterativo de resolucion
#define NUMERO_ITERACIONES 30

class Solver
{
	float dt, diff, visc;
	unsigned N;
	float * u_prev, *v_prev, *dens_prev;

	// This variables sets the iterative method to be used by the solver
	// 0: Gauss-Seidel
	// 1: Jacobi
	int iterativeMethod;

	int numeroIteraciones;

public:
	float * u, *v, *dens;
	void Init(unsigned N, float dt, float diff, float visc);
	void FreeData(void);
	void ClearData(void);
	bool AllocateData(void);
	void ClearPrevData(void);
	void AddDensity(unsigned i, unsigned j, float source);
	void AddVelocity(unsigned i, unsigned j, float forceX, float forceY);
	void Solve(void);

	void setIterativeMethod(int method);

	inline int getNumeroIteraciones() { return numeroIteraciones; };

	void ActivateFixedObject(int position, int size);
	void DeactivateFixedObject();

private:
	void DensStep(void);
	void VelStep(void);

	void AddSource(float * x, float * s);
	void SetBounds(int b, float * x);
	void LinSolve(int b, float * x, float * x0, float a, float c);
	void Diffuse(int b, float * x, float * x0);
	void Advect(int b, float * d, float * d0, float * u, float * v);
	void Project(float * u, float * v, float * p, float * div);

	void Jacobi(int b, float * x, float * x0, float aij, float aii);
	void OverRelaxation(int b, float * x, float * x0, float aij, float aii);

	bool fixedObjectActive;
	int objectPosition;
	int objectSize;
};

#endif
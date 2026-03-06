#ifndef SIMULATION_H
#define SIMULATION_H

#define SWAP(x0,x) {float *tmp=x0;x0=x;x=tmp;}
#define IX(i, j) ((i) + (j) * (N + 2))

void diffuse(int N, int b, float *x, float *x0, float diff, float dt);
void advect(int N, int b, float *d, float *d0, float *u, float *v, float dt);
void project(int N, float *u, float *v, float *p, float *div);
void dens_step(int N, float *x, float *x0, float *u, float *v, float diff, float dt);
void vel_step(int N, float *u, float *v, float *u0, float *v0, float visc, float dt);
void set_bnd(int N, int b, float *x);

#endif // SIMULATION_H

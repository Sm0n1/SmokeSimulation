#ifndef SIMULATION_H
#define SIMULATION_H

#define SWAP(x0,x) {float *tmp=x0;x0=x;x=tmp;}
#define IX(i, j) ((i) + (j) * (N + 2))

void diffuse(int N, int b, float *x, float *x0, float diff, float dt, bool *bnd);
void advect(int N, int b, float *d, float *d0, float *u, float *v, float dt, bool *bnd);
void project(int N, float *u, float *v, float *p, float *div, bool *bnd);
void dens_step(int N, float *x, float *x0, float *u, float *v, float diff, float dt, bool *bnd);
void vel_step(int N, float *u, float *v, float *u0, float *v0, float visc, float dt, bool *bnd);
void set_bnd(int N, int b, float *x, bool *bnd);

#endif // SIMULATION_H

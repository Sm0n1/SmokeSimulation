#include <SDL3/SDL.h>
#include "simulation.h"

// Implemented as specified in "Real-Time Fluid Dynamics for Games" by Jos Stam

/**
 * Diffusion.
 * 
 * @param N Grid size.
 * @param b idk
 * @param x The field. The result is written to this array.
 * @param x0 The previous values of the field.
 * @param diff The diffusion rate.
 * @param dt Delta time.
 */
void diffuse(int N, int b, float *x, float *x0, float diff, float dt) {
    float a = diff * N * N * dt;

    for (int k = 0; k < 20; k++) {
        for (int i = 1; i <= N; i++) {
            for (int j = 1; j <= N; j++) {
                x[IX(i, j)] = (x0[IX(i, j)] + a * (x[IX(i-1, j)] + x[IX(i+1, j)] + x[IX(i, j-1)] + x[IX(i, j+1)])) / (1 + 4 * a);
            }
        }
        set_bnd(N, b, x);
    }
}

/**
 * Advection.
 * 
 * @param N Grid size.
 * @param b idk
 * @param d The field. The result is written to this array.
 * @param d0 The previous values of the field.
 * @param u The horizontal velocity field.
 * @param v The vertical velocity field.
 * @param dt Delta time.
 */
void advect(int N, int b, float *d, float *d0, float *u, float *v, float dt) {
    float dt0 = dt * N;
    for (int i = 1; i <= N; i++) {
        for (int j = 1; j <= N; j++) {
            float x = i - dt0 * u[IX(i, j)];
            float y = j - dt0 * v[IX(i, j)];

            if (x < 0.5) x = 0.5;
            if (x > N + 0.5f) x = N + 0.5f;
            int i0 = (int)x;
            int i1 = i0 + 1;

            if (y < 0.5f) y = 0.5f;
            if (y > N + 0.5f) y = N + 0.5f;
            int j0 = (int)y;
            int j1 = j0 + 1;

            float s1 = x - i0;
            float s0 = 1 - s1;
            float t1 = y - j0;
            float t0 = 1 - t1;

            d[IX(i, j)] =
                s0 * (t0 * d0[IX(i0, j0)] + t1 * d0[IX(i0, j1)]) +
                s1 * (t0 * d0[IX(i1, j0)] + t1 * d0[IX(i1, j1)]);
        }
    }
    set_bnd(N, b, d);
}

/**
 * Ensure that the velocity field is mass-conserving (divergence free). This is to
 * ensure fluids are incompressible and that no fluid can disappear or be created
 * out of thin air.
 * 
 * This is done by solving the Poisson equation
 *     ∇²p = ∇·u
 * In other words, this function searches for a pressure field p that ensures the
 * velocity field is divergence free.
 * 
 * @param N Grid size.
 * @param u The horizontal velocity field. The result is written to this array.
 * @param v The vertical velocity field. The result is written to this array.
 * @param p A temporary array where the pressure is stored.
 * @param div A temporary array where the divergence is stored.
 */
void project(int N, float *u, float *v, float *p, float *div) {
    float h = 1.0f / N;

    // Calculate an approximation of the divergence of the velocity field using a
    // finite difference. Also set the initial pressure to 0.
    for (int i = 1; i <= N; i++) {
        for (int j = 1; j <= N; j++) {
            // ∇·u = ∂u/∂x + ∂v/∂y
            // which can be approximated using a finite difference as
            // ∇·u ≈ Δu/Δx + Δv/Δy
            // In this case Δx = Δy = ??? TODO
            float delta_u = u[IX(i+1, j)] - u[IX(i-1, j)];
            float delta_v = v[IX(i, j+1)] - v[IX(i, j-1)];
            div[IX(i, j)] = -0.5 * h * (delta_u + delta_v);
            p[IX(i, j)] = 0;
        }
    }

    // Set boundary conditions for div and p. TODO what are they set to?
    set_bnd(N, 0, div);
    set_bnd(N, 0, p);

    for (int k = 0; k < 20; k++) {
        for (int i = 1; i <= N; i++) {
            for (int j = 1; j <= N; j++) {
                p[IX(i, j)] = (div[IX(i, j)] + p[IX(i-1, j)] + p[IX(i+1, j)] + p[IX(i, j-1)] + p[IX(i, j+1)]) / 4;
            }
        }
        set_bnd(N, 0, p);
    }

    for (int i = 1; i <= N; i++) {
        for (int j = 1; j <= N; j++) {
            u[IX(i, j)] -= 0.5f * (p[IX(i+1, j)] - p[IX(i-1, j)]) / h;
            v[IX(i, j)] -= 0.5f * (p[IX(i, j+1)] - p[IX(i, j-1)]) / h;
        }
    }
    set_bnd(N, 1, u);
    set_bnd(N, 2, v);
}

/**
 * Density step.
 * 
 * @param N Grid size.
 * @param x The density field. The result is written to this array.
 * @param x0 The previous values of the density field.
 * @param u The horizontal velocity field.
 * @param v The vertical velocity field.
 * @param diff The diffusion rate.
 * @param dt Delta time.
 */
void dens_step(int N, float *x, float *x0, float *u, float *v, float diff, float dt) {
    SWAP(x0, x);
    diffuse(N, 0, x, x0, diff, dt);
    SWAP(x0, x);
    advect(N, 0, x, x0, u, v, dt);
}

/**
 * Velocity step.
 * 
 * @param N Grid size.
 * @param u The horizontal velocity field. The result is written to this array.
 * @param v The vertical velocity field. The result is written to this array.
 * @param u0 The previous values of the horizontal velocity field.
 * @param v0 The previous values of the vertical velocity field.
 * @param visc The viscosity.
 * @param dt Delta time.
 */
void vel_step(int N, float *u, float *v, float *u0, float *v0, float visc, float dt) {
    SWAP(u0, u);
    diffuse(N, 1, u, u0, visc, dt);
    SWAP(v0, v);
    diffuse(N, 2, v, v0, visc, dt);

    project(N, u0, v0, u, v);

    SWAP(u0, u);
    SWAP(v0, v);
    advect(N, 1, u, u0, u0, v0, dt);
    advect(N, 2, v, v0, u0, v0, dt);

    project(N, u, v, u0, v0);
}

void set_bnd(int N, int b, float *x) {
    for (int i = 1; i <= N; i++) {
        x[IX(0, i)] = b == 1 ? -x[IX(1, i)] : x[IX(1, i)];
        x[IX(N+1, i)] = b == 1 ? -x[IX(N, i)] : x[IX(N, i)];
        x[IX(i, 0)] = b == 2 ? -x[IX(i, 1)] : x[IX(i, 1)];
        x[IX(i, N+1)] = b == 2 ? -x[IX(i, N)] : x[IX(i, N)];
    }
    x[IX(0, 0)] = 0.5f * (x[IX(1, 0)] + x[IX(0, 1)]);
    x[IX(0, N+1)] = 0.5f * (x[IX(1, N+1)] + x[IX(0, N)]);
    x[IX(N+1, 0)] = 0.5f * (x[IX(N, 0)] + x[IX(N+1, 1)]);
    x[IX(N+1, N+1)] = 0.5f * (x[IX(N, N+1)] + x[IX(N+1, N)]);
}

#include <SDL3/SDL.h>
#include "simulation.h"

// Implemented as specified in "Real-Time Fluid Dynamics for Games" by Jos Stam

/**
 * Diffusion.
 * 
 * @param N    Grid size.
 * @param b    idk
 * @param x    The field. The result is written to this array.
 * @param x0   The previous values of the field.
 * @param diff The diffusion rate.
 * @param dt   Delta time.
 * @param bnd  The internal boundary.
 */
void diffuse(int N, int b, float *x, float *x0, float diff, float dt, bool *bnd)
{
    float a = diff * N * N * dt;

    for (int k = 0; k < 20; k += 1)
    {
        for (int i = 1; i <= N; i += 1)
        {
            for (int j = 1; j <= N; j += 1)
            {
                x[IX(i, j)] = (x0[IX(i, j)] + a * (x[IX(i-1, j)] + x[IX(i+1, j)] + x[IX(i, j-1)] + x[IX(i, j+1)])) / (1 + 4 * a);
            }
        }
        set_bnd(N, b, x, bnd);
    }
}

/**
 * Advection.
 * 
 * @param N   Grid size.
 * @param b   idk
 * @param d   The field. The result is written to this array.
 * @param d0  The previous values of the field.
 * @param u   The horizontal velocity field.
 * @param v   The vertical velocity field.
 * @param dt  Delta time.
 * @param bnd The internal boundary.
 */
void advect(int N, int b, float *d, float *d0, float *u, float *v, float dt, bool *bnd)
{
    const float dt0 = dt * N;

    for (int i = 1; i <= N; i += 1)
    {
        for (int j = 1; j <= N; j += 1)
        {
            float x = i - dt0 * u[IX(i, j)];
            float y = j - dt0 * v[IX(i, j)];

            if (x < 0.5) x = 0.5;
            if (x > N + 0.5f) x = N + 0.5f;
            const int i0 = (int)x;
            const int i1 = i0 + 1;

            if (y < 0.5f) y = 0.5f;
            if (y > N + 0.5f) y = N + 0.5f;
            const int j0 = (int)y;
            const int j1 = j0 + 1;

            const float s1 = x - i0;
            const float s0 = 1 - s1;
            const float t1 = y - j0;
            const float t0 = 1 - t1;

            d[IX(i, j)] =
                s0 * (t0 * d0[IX(i0, j0)] + t1 * d0[IX(i0, j1)]) +
                s1 * (t0 * d0[IX(i1, j0)] + t1 * d0[IX(i1, j1)]);
        }
    }

    set_bnd(N, b, d, bnd);
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
 * @param N   Grid size.
 * @param u   The horizontal velocity field. The result is written to this array.
 * @param v   The vertical velocity field. The result is written to this array.
 * @param p   A temporary array where the pressure is stored.
 * @param div A temporary array where the divergence is stored.
 * @param bnd The internal boundary.
 */
void project(int N, float *u, float *v, float *p, float *div, bool *bnd)
{
    const float h = 1.0f / N;

    // Calculate an approximation of the divergence of the velocity field using a
    // finite difference. Also set the initial pressure to 0.
    for (int i = 1; i <= N; i += 1)
    {
        for (int j = 1; j <= N; j += 1)
        {
            // ∇·u = ∂u/∂x + ∂v/∂y
            // which can be approximated using a finite difference as
            // ∇·u ≈ Δu/Δx + Δv/Δy
            // In this case Δx = Δy = ??? TODO
            const float delta_u = u[IX(i+1, j)] - u[IX(i-1, j)];
            const float delta_v = v[IX(i, j+1)] - v[IX(i, j-1)];
            div[IX(i, j)] = -0.5f * h * (delta_u + delta_v);
            p[IX(i, j)] = 0;
        }
    }

    // Set boundary conditions for div and p. TODO what are they set to?
    set_bnd(N, 0, div, bnd);
    set_bnd(N, 0, p, bnd);

    for (int k = 0; k < 20; k += 1)
    {
        for (int i = 1; i <= N; i += 1)
        {
            for (int j = 1; j <= N; j += 1)
            {
                p[IX(i, j)] = (div[IX(i, j)] + p[IX(i-1, j)] + p[IX(i+1, j)] + p[IX(i, j-1)] + p[IX(i, j+1)]) / 4;
            }
        }

        set_bnd(N, 0, p, bnd);
    }

    for (int i = 1; i <= N; i += 1)
    {
        for (int j = 1; j <= N; j += 1)
        {
            u[IX(i, j)] -= 0.5f * (p[IX(i+1, j)] - p[IX(i-1, j)]) / h;
            v[IX(i, j)] -= 0.5f * (p[IX(i, j+1)] - p[IX(i, j-1)]) / h;
        }
    }

    set_bnd(N, 1, u, bnd);
    set_bnd(N, 2, v, bnd);
}

/**
 * Density step.
 * 
 * @param N    Grid size.
 * @param x    The density field. The result is written to this array.
 * @param x0   The previous values of the density field.
 * @param u    The horizontal velocity field.
 * @param v    The vertical velocity field.
 * @param diff The diffusion rate.
 * @param dt   Delta time.
 * @param bnd  The internal boundary.
 */
void dens_step(int N, float *x, float *x0, float *u, float *v, float diff, float dt, bool *bnd)
{
    SWAP(x0, x);
    diffuse(N, 0, x, x0, diff, dt, bnd);
    SWAP(x0, x);
    advect(N, 0, x, x0, u, v, dt, bnd);
}

/**
 * Velocity step.
 * 
 * @param N    Grid size.
 * @param u    The horizontal velocity field. The result is written to this array.
 * @param v    The vertical velocity field. The result is written to this array.
 * @param u0   The previous values of the horizontal velocity field.
 * @param v0   The previous values of the vertical velocity field.
 * @param visc The viscosity.
 * @param dt   Delta time.
 * @param bnd  The internal boundary.
 */
void vel_step(int N, float *u, float *v, float *u0, float *v0, float visc, float dt, bool *bnd)
{
    SWAP(u0, u);
    diffuse(N, 1, u, u0, visc, dt, bnd);
    SWAP(v0, v);
    diffuse(N, 2, v, v0, visc, dt, bnd);

    project(N, u, v, u0, v0, bnd);

    SWAP(u0, u);
    SWAP(v0, v);
    advect(N, 1, u, u0, u0, v0, dt, bnd);
    advect(N, 2, v, v0, u0, v0, dt, bnd);

    project(N, u, v, u0, v0, bnd);
}

/**
 * Fluid containment and interaction with objects 
 *
 * @param N    Grid size.
 * @param b    idk
 * @param x    The density field. The result is written to this array.
 * @param bnd  The internal boundary.
 */
void set_bnd(int N, int b, float *x, bool *bnd)
{
    for (int i = 1; i <= N; i += 1)
    {
        x[IX(0, i)] = b == 1 ? -x[IX(1, i)] : x[IX(1, i)];
        x[IX(N+1, i)] = b == 1 ? -x[IX(N, i)] : x[IX(N, i)];
        x[IX(i, 0)] = b == 2 ? -x[IX(i, 1)] : x[IX(i, 1)];
        x[IX(i, N+1)] = b == 2 ? -x[IX(i, N)] : x[IX(i, N)];
    }

    x[IX(0, 0)] = 0.5f * (x[IX(1, 0)] + x[IX(0, 1)]);
    x[IX(0, N+1)] = 0.5f * (x[IX(1, N+1)] + x[IX(0, N)]);
    x[IX(N+1, 0)] = 0.5f * (x[IX(N, 0)] + x[IX(N+1, 1)]);
    x[IX(N+1, N+1)] = 0.5f * (x[IX(N, N+1)] + x[IX(N+1, N)]);

    (void)bnd;
}

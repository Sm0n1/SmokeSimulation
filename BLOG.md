# Project Specification Musings

2026-03-05

This blog entry is written a tad late. Sorry about that.

Our initial idea was to create a Bernoulli effect simulation, suspending a ping pong ball in the air. We wanted to use an Eulerian approach, i.e. simulating
the air flow using a grid, but it was surprisingly difficult to find resources regarding two-way interaction, that is the ball should influence the air, and
the air should influence the ball. As such, we made the decision to focus on one-way interactions for this project, leaving two-way interactions as a point
of further study. This should keep it manageable within our given time frame. One of our main goals is however to further our understanding of grid-based
fluid simulations such that we in the future would be able to implement a two-way simulation.

The basic idea as it currently stands is then to simulate smoke envolping a ball rather than suspending it in the air. We found a resource that explains a
general algorithm for solving the Navier-Stokes equations when representing the fluid as a grid. It is written by Joe Stam and called Real-Time Fluid Dynamics
for Games. He does not cover the aspect of actually adding smoke or objects to the fluid, but mentions them as possible (and easy) extensions.
As an initial implementation, we will be following his methods closely. We are aiming for a real-time simulation, making his methods an obvious choice, being
related to games and all.

We did find a fun little website (https://rachelbhadra.github.io/smoke_simulator/index.html#contributions) that has a similar feel to what we wish to accomplish.

That is it for now.
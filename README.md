# N-Pendulum

A real-time, interactive physics simulator for pendulum systems, written in C++. It starts with a single pendulum and grows into a general **N-link pendulum**: N bobs joined end to end, a classic example of deterministic chaos.

> **Current status:** Stage 1 (single pendulum) works. You can drag the bob to set it up, change the parameters live, watch the equations update with real numbers, and see live graphs of angle, velocity, energy and phase space. The double pendulum and the general N-pendulum come next (see [Roadmap](#5-objectives-and-applications)).

---

## 1. What this folder is

This folder holds the full source of the simulator and the libraries it uses.

```
N-Pendulum/
├── README.md            ← you are here
├── func_Read_me.md      ← reference for every function in the codebase
└── GUI/
    ├── include/         ← headers
    │   ├── Pendulum.h   ← physics: state, equations of motion, RK4, history buffer
    │   ├── Canvas.h     ← window layout (4 sections) and split ratios
    │   ├── Drawing.h    ← animation drawing
    │   ├── Panels.h     ← ImGui panels (working, graphs, controls)
    │   ├── Mouse.h      ← drag-the-bob input
    │   ├── keyboard.h   ← keyboard shortcuts
    │   └── Colors.h     ← color palette
    ├── src/
    │   ├── Canvas.cpp   ← entry point: window, OpenGL context, main loop
    │   ├── Pendulum.cpp ← physics implementation
    │   ├── Drawing.cpp  ← pendulum, trail, angle arc, ceiling, scale bar
    │   ├── Panels.cpp   ← live equations, ImPlot graphs, sliders and presets
    │   ├── Mouse.cpp / keyboard.cpp
    │   └── Test.cpp     ← scratch file for learning GLFW (not part of the build)
    └── external/
        ├── imgui/       ← Dear ImGui sources
        ├── implot/      ← ImPlot sources
        └── lib/         ← prebuilt libimgui.a (ImGui + ImPlot), glad, glfw
```

### The window

The window is split into four live sections:

| Section | Position | What it shows |
|---|---|---|
| **Animation** | left, top 70% | The pendulum, with a fading red trail, angle arc, θ = 0 reference line and a 1 m scale bar. Drag the bob to set the start angle. |
| **Working** | left, bottom 30% | The equation of motion with the current numbers filled in, plus state values (θ, ω, α, v, x, y, tension) and energies (KE, PE, total, drift ΔE). |
| **Graphs** | right, top 50% | Live ImPlot charts: θ and ω vs time, energy vs time, and phase space (θ vs ω). |
| **Controls** | right, bottom 50% | Play/pause/reset, length L, mass m, start angle θ₀, start velocity ω₀, gravity g (presets for Earth, Moon, Mars and Jupiter), damping b, simulation speed, graph time window. |

**Shortcuts:** `Space` plays or pauses, `R` resets, and you can click and drag the bob to set the start angle.

### Build and run (from the `GUI` folder, MinGW g++ on Windows)

```bash
g++ src/Canvas.cpp src/Pendulum.cpp src/Drawing.cpp src/Panels.cpp src/Mouse.cpp src/keyboard.cpp \
    -I. -Iexternal/imgui -Iexternal/imgui/backends -Iexternal/implot \
    -Lexternal/lib -limgui -lglad -lglfw3 -lopengl32 -lgdi32 \
    -o src/Canvas.exe

./src/Canvas.exe
```

---

## 2. Why we are making this

The double pendulum is about the simplest system that is completely deterministic yet practically unpredictable. Change its starting angle by a billionth of a degree and within seconds it does something totally different. The N-pendulum takes this further, and the same mathematics describes robot arms, human limbs, hanging cables and crane loads.

Most pendulum simulators fall into one of two groups:

- **Visual toys** look nice but hide the physics. You can't see the equations, the numbers or how accurate the result is.
- **Research and engineering codes** (MATLAB scripts, multibody solvers) are accurate but not interactive or visual, and are hard to learn from.

**What this project adds:**

1. **Transparent physics.** The working panel shows the equation of motion with the live numbers filled in, so the simulation explains itself as it runs.
2. **Honest accuracy.** The energy drift ΔE is always on screen. With damping at 0 the total energy should stay constant, so any drift is numerical error you can see. The simulator reports how far it can be trusted.
3. **A path from simple to chaotic.** The same tool and interface go from a pendulum a school student understands (1 bob) to a system researchers still study (N bobs), so you can watch order turn into chaos as N grows.
4. **A fast native foundation.** It's written in C++ with OpenGL, so it can grow to many links, many pendulums at once (to visualise the butterfly effect) and long, accurate runs that a browser toy can't handle.

---

## 3. Technology

| Layer | Technology | Role |
|---|---|---|
| Language | **C++** (g++/MinGW on Windows) | Physics engine and application |
| Windowing and input | **GLFW 3** | Creates the window and OpenGL context, handles mouse and keyboard |
| Graphics API | **OpenGL 3.3 Core Profile** (loaded with **glad**) | GPU rendering, per-section viewports and scissoring |
| UI | **Dear ImGui v1.93.0** | Immediate-mode panels, sliders, buttons and the animation draw list |
| Plotting | **ImPlot v1.1** | Real-time graphs, reading directly from the ring-buffer history |
| Fonts | Segoe UI and Consolas, scaled for monitor DPI | Readable text and equations |

**Architecture notes**

- **Physics runs separately from rendering.** Physics always advances in fixed 1 ms steps (`PHYSICS_DT`) using a time accumulator, however fast or slow the frames are. Long frames are clamped to 50 ms so the simulation doesn't jump ahead after a stall.
- **Ring-buffer history.** 10,000 samples, about 2.5 minutes at 60 fps. Once full, the newest sample overwrites the oldest. The buffer feeds both the graphs and the bob trail.
- **Data and UI are split.** `Simulation` / `SinglePendulum` hold all the state, and the drawing and panel code only read and change it. This split is what lets the physics core be swapped for an N-pendulum later.

---

## 4. Mathematics and physics

### 4.1 Single pendulum (implemented)

A point mass $m$ hangs on a massless rigid rod of length $L$. The angle $\theta$ is measured from the downward vertical, and $b$ is a viscous damping coefficient.

**Equation of motion** (from Newton's second law in the tangential direction, or from the Lagrangian $\mathcal{L} = T - V$):

```math
\ddot{\theta} = -\frac{g}{L}\sin\theta \;-\; b\,\dot{\theta}
```

This equation is **nonlinear** (because of $\sin\theta$) and has no elementary closed-form solution, so it's integrated numerically.

**Derived quantities shown live:**

| Quantity | Formula |
|---|---|
| Bob position | $x = L\sin\theta,\quad y = -L\cos\theta$ |
| Speed | $v = L\,\omega$ |
| Kinetic energy | $KE = \tfrac12 m L^2 \omega^2$ |
| Potential energy (zero at the bottom) | $PE = m g L (1 - \cos\theta)$ |
| Rod tension | $T = m\,(g\cos\theta + L\omega^2)$ |
| Small-angle period | $T_0 = 2\pi\sqrt{L/g}$ (only valid when $\sin\theta \approx \theta$) |

The **energy drift** $\Delta E = E(t) - E(0)$ measures damping loss when $b > 0$, and numerical error when $b = 0$.

**Phase space.** The state is the pair $(\theta, \omega)$. Plotting $\omega$ against $\theta$ gives closed loops for swinging (libration), open curves for full rotations, and inward spirals when damping is on. The boundary between swinging and rotating is the **separatrix**.

### 4.2 Numerical method: Runge–Kutta 4 (implemented)

The second-order ODE is written as two first-order equations, $\dot\theta = \omega$ and $\dot\omega = \alpha(\theta, \omega)$, and advanced with classical **RK4**:

```math
y_{n+1} = y_n + \frac{\Delta t}{6}\,(k_1 + 2k_2 + 2k_3 + k_4)
```

RK4 has a local error of $O(\Delta t^5)$ and a global error of $O(\Delta t^4)$. With $\Delta t = 1$ ms the drift is tiny, but RK4 is **not symplectic**, so over very long runs the energy still drifts slowly (see [Challenges](#6-going-further-what-to-study-and-where-human-thinking-is-needed)).

### 4.3 Double and N-pendulum (planned)

Take $N$ point masses $m_1 \dots m_N$ on massless rods $l_1 \dots l_N$, where $\theta_i$ is the angle of rod $i$ from the vertical. The positions are:

```math
x_i = \sum_{j=1}^{i} l_j \sin\theta_j, \qquad y_i = -\sum_{j=1}^{i} l_j \cos\theta_j
```

Define $\mu_{jk} = \sum_{i \ge \max(j,k)} m_i$, the total mass hanging at or below the lower of links $j$ and $k$. The **Lagrangian method** (Euler–Lagrange equations in the generalised coordinates $\theta_i$) then gives, for each $j = 1 \dots N$:

```math
\sum_{k=1}^{N} \mu_{jk}\, l_j l_k \cos(\theta_j - \theta_k)\,\ddot{\theta}_k
\;+\; \sum_{k=1}^{N} \mu_{jk}\, l_j l_k \sin(\theta_j - \theta_k)\,\dot{\theta}_k^{\,2}
\;+\; g\,\mu_{jj}\, l_j \sin\theta_j \;=\; 0
```

In matrix form:

```math
M(\boldsymbol\theta)\,\ddot{\boldsymbol\theta} = \mathbf{f}(\boldsymbol\theta, \dot{\boldsymbol\theta})
```

where $M_{jk} = \mu_{jk} l_j l_k \cos(\theta_j - \theta_k)$ is the **mass matrix**, which is symmetric and positive-definite. Each time step solves this $N \times N$ linear system (for example with a Cholesky factorisation) to get the accelerations, then integrates. For $N = 1$ it reduces exactly to the single-pendulum equation above, which makes a good unit test.

**Topics this brings in:**

- **Lagrangian and Hamiltonian mechanics:** generalised coordinates, the Euler–Lagrange equations, conjugate momenta, and conservation of energy as a check on correctness.
- **Linear algebra:** solving $M\ddot\theta = f$, conditioning of $M$, and Cholesky and LU decompositions.
- **Linearisation and normal modes:** for small angles, $\det(K - \omega^2 M_0) = 0$ gives the $N$ natural frequencies and mode shapes.
- **Chaos theory:** sensitive dependence on initial conditions, **Lyapunov exponents** (how fast nearby trajectories separate), **Poincaré sections** (slices through the $2N$-dimensional phase space) and KAM theory (why some regions stay regular while others become chaotic).

---

## 5. Objectives and applications

### Objectives (roadmap)

1. ✅ **Single pendulum:** accurate RK4 physics, interactive animation, live working, energy and phase-space graphs, adjustable parameters and gravity presets.
2. ⬜ **Double pendulum:** derive and implement the coupled equations, animate both bobs, and show the chaotic phase space.
3. ⬜ **General N-pendulum:** solve the mass-matrix system for any N with a per-link length and mass editor, and keep it fast for large N.
4. ⬜ **Chaos tools:**
   - a "butterfly mode" that runs many pendulums with almost identical starts side by side
   - a live Lyapunov exponent estimate
   - Poincaré sections
   - flip-time maps (colouring a grid of start angles by how long the end takes to flip over)
5. ⬜ **Integrator comparison:** switch between Euler, RK4, adaptive RK45 / DOP853 and symplectic or implicit methods, and compare their energy drift live.
6. ⬜ **Better models:**
   - joint friction vs air drag
   - compound pendulums with distributed mass and moments of inertia
   - driven or forced pivots
   - springy links
   - slack strings
7. ⬜ **Validation and export:** CSV/JSON export of trajectories, an automated test suite (N = 1 reduction, energy conservation, convergence as Δt → 0), and comparison against video-tracked real pendulums.
8. ⬜ **Polish:** cross-platform build (CMake), saved presets, and recording to image or video.

### Applications

| Field | How the N-pendulum shows up |
|---|---|
| **Education** | Teaching nonlinear ODEs, energy, phase space, numerical methods and chaos, with the equations visible on screen. |
| **Chaos and dynamical systems research** | A testbed for Lyapunov exponents, Poincaré maps and the move from regular to chaotic motion as N grows. |
| **Robotics** | A serial robot arm is an N-link chain, with the same mass-matrix structure $M(q)\ddot q = \tau - C(q,\dot q) - G(q)$. Control and motion-planning methods can be tried here first. |
| **Biomechanics** | Legs, arms and the spine are often modelled as linked pendulums (walking gait, a gymnast on a bar, balance). |
| **Cables, ropes and chains** | As N → ∞ the chain approaches a hanging rope or cable, which is relevant to cranes, tethers and towed cables. |
| **Structural and mechanical engineering** | Tuned-mass dampers in tall buildings, swinging crane loads and anti-sway control. |
| **Numerical methods benchmarking** | A tough, energy-conserving, chaotic test case for comparing ODE integrators. |

---

## 6. Going further: what to study, and where human thinking is needed

AI can write ImGui panels, plotting code and boilerplate quickly. It can't decide whether the simulation is *correct* or *good*. The points below need real understanding, because a convincing-looking simulation can still be physically wrong.

### What to study

| Topic | Why it matters | Good sources |
|---|---|---|
| Lagrangian and Hamiltonian mechanics | Deriving the N-pendulum equations yourself, and knowing which quantities must be conserved | J. R. Taylor, *Classical Mechanics*; Goldstein, *Classical Mechanics* |
| Nonlinear dynamics and chaos | Understanding what you're seeing: bifurcations, separatrices, Lyapunov exponents, Poincaré sections | S. Strogatz, *Nonlinear Dynamics and Chaos* |
| Numerical ODE methods | Choosing and tuning integrators: stability, error order, adaptive step size, symplectic methods | Hairer, Nørsett & Wanner, *Solving ODEs I*; Hairer, Lubich & Wanner, *Geometric Numerical Integration* |
| Numerical linear algebra | Solving $M\ddot\theta = f$ quickly and stably, and recognising ill-conditioning | Trefethen & Bau, *Numerical Linear Algebra* |
| Multibody dynamics algorithms | Scaling to large N with $O(N)$ methods instead of $O(N^3)$ | R. Featherstone, *Rigid Body Dynamics Algorithms* |
| Floating-point arithmetic | Knowing when a tiny ΔE is physics and when it's rounding error | D. Goldberg, *What Every Computer Scientist Should Know About Floating-Point Arithmetic* |
| Real-time simulation loops | Fixed timesteps, accumulators, interpolation and the "spiral of death" | G. Fiedler, *Fix Your Timestep!* |
| Graphics and OpenGL | Rendering many pendulums and trails efficiently | learnopengl.com |

### Challenges that need judgement

1. **Deriving and checking the equations.** A single sign error in the N-pendulum equations still produces motion that looks chaotic and plausible. Check every derivation three ways: it should reduce to N = 1, conserve energy when damping is 0, and match the known double-pendulum equations for N = 2. Don't trust any derivation, including an AI's, until it passes all three.

2. **Defining "correct" for a chaotic system.** In a chaotic system any tiny error, even float rounding, grows exponentially, so after a few Lyapunov times *no* simulation matches the exact trajectory. Validation therefore can't be "compare the trajectory forever". It has to rely on:
   - invariants such as energy
   - convergence tests (halve Δt and check the results agree up to a predictable time)
   - statistical properties

   Choosing these criteria is a scientific decision, not a coding task.

3. **Choosing the integrator.** RK4 is accurate in the short term but drifts energy over long runs. Symplectic methods such as Verlet or leapfrog keep energy bounded, but the N-pendulum Hamiltonian isn't separable (the mass matrix depends on θ), so plain leapfrog doesn't apply directly. You need implicit symplectic methods (implicit midpoint, Gauss–Legendre) or adaptive high-order methods with error control. Picking between them means trading off accuracy, speed and stability.

4. **Stiffness and ill-conditioning.** Very unequal masses or lengths (a heavy top link with a feather-light tip, or a very short link) make the mass matrix nearly singular and the equations stiff. A fixed 1 ms step may silently go wrong. You need to detect this (condition number, step-doubling error estimates) and respond to it.

5. **Performance at large N.** A naive mass-matrix solve costs $O(N^3)$ per step, and RK4 needs 4 solves per step. For N = 100+ at 1000 steps/s you need the structure of the problem (Cholesky, or Featherstone's $O(N)$ articulated-body algorithm), and possibly multithreading or the GPU for butterfly mode.

6. **Modelling decisions.**
   - **Rigid rod or string?** The current model is a rigid rod. A real string goes slack when the tension becomes negative, and modelling that is a different, non-smooth problem.
   - **Damping.** Air drag on the bobs (absolute velocity) and friction at the joints (relative velocity between links) are different physics and give different equations.
   - **Point mass or compound pendulum.** Real lab pendulums have distributed mass, so you need moments of inertia.

   Each choice changes the equations, and deciding which matters depends on what you're trying to model.

7. **Telling a bug from real chaos.** When two runs differ, ask whether it's chaos, a bug, a timestep problem or undefined behaviour. Separating these takes controlled experiments and careful reasoning about the physics, not more code.

8. **Visualising high-dimensional state.** An N-pendulum lives in $2N$-dimensional phase space. Designing graphs that actually reveal structure (Poincaré sections, flip-time maps, projections) is a mix of physics insight and visual design.

9. **Matching reality.** Real pendulums have bearing friction, flexible rods and measurement noise. Comparing against video of a real double pendulum, and explaining the differences, is what separates a great simulation from a pretty one.

---

## Reference

- [func_Read_me.md](func_Read_me.md): a one-line explanation of every function in the codebase, grouped by file.

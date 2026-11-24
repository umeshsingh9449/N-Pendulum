#pragma once

#include <vector>

constexpr double PI         = 3.14159265358979323846;
constexpr double PHYSICS_DT = 1.0 / 1000.0; // fixed physics step (s): small and constant keeps RK4 accurate

// One snapshot of the pendulum, stored every frame for the graph and the bob trail
struct Sample {
    double t;         // time (s)
    double theta;     // angle from the vertical (deg)
    double omega;     // angular velocity (rad/s)
    double kinetic;   // kinetic energy (J)
    double potential; // potential energy (J)
    double total;     // total energy (J)
    double x, y;      // bob position relative to the pivot (m), y points up
};

// Fixed-size ring buffer: once full, new samples overwrite the oldest ones.
// 'data' + 'offset' can be handed straight to ImPlot (it understands ring buffers).
struct History {
    std::vector<Sample> data;
    int capacity;
    int offset = 0; // index of the oldest sample once the buffer is full

    explicit History(int capacity);
    void push(const Sample& s);
    void clear();
    int size() const;
    const Sample& fromEnd(int k) const; // k = 0 is the newest sample
};

// Single pendulum: a point mass (bob) on a massless rigid string
//
//      equation of motion:  θ'' = -(g/L)·sin θ - b·θ'
//
struct SinglePendulum {
    // ---- parameters ----
    double g       = 9.81; // gravity (m/s^2)
    double length  = 1.0;  // string length L (m)
    double mass    = 1.0;  // bob mass m (kg)
    double damping = 0.0;  // damping coefficient b (1/s)

    // ---- state ----
    double theta = 0.0; // angle from the downward vertical (rad), +ve = bob to the right
    double omega = 0.0; // angular velocity θ' (rad/s)
    double time  = 0.0; // simulated time (s)

    double angularAcceleration(double theta, double omega) const;
    void   step(double dt); // advance by dt using Runge-Kutta 4

    double kineticEnergy() const;    // ½·m·L²·ω²
    double potentialEnergy() const;  // m·g·L·(1 - cos θ), zero at the lowest point
    double tension() const;          // m·(g·cos θ + L·ω²)
    double smallAnglePeriod() const; // 2π·√(L/g)
};

// Everything the GUI reads and changes: the pendulum, its start conditions,
// play/pause state and the recorded history for the graph
struct Simulation {
    SinglePendulum pendulum;

    double initialAngleDeg = 45.0; // θ₀ (deg)
    double initialOmega    = 0.0;  // ω₀ (rad/s)

    bool  running     = true;
    float speed       = 1.0f;  // simulation speed multiplier
    float graphWindow = 10.0f; // seconds of history shown on the graph

    double  initialEnergy = 0.0; // energy at t = 0, used to show drift / damping loss
    History history{ 10000 }; // ~2.5 min at 60 fps

    void reset();                 // back to θ₀, ω₀ and t = 0
    void update(double frameDt);  // run as many fixed physics steps as this frame needs
    void record();                // push the current state into the history

private:
    double accumulator = 0.0; // leftover frame time not yet simulated
};

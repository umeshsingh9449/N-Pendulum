#include <include/Pendulum.h>
#include <cmath>
#include <algorithm>

// ------------------------------- History -------------------------------

History::History(int capacity) : capacity(capacity) {
    data.reserve(capacity);
}

void History::push(const Sample& s){
    if((int)data.size() < capacity){
        data.push_back(s);
    }
    else{
        // buffer full -> overwrite the oldest sample and move the start forward
        data[offset] = s;
        offset = (offset + 1) % capacity;
    }
}

void History::clear(){
    data.clear();
    offset = 0;
}

int History::size() const {
    return (int)data.size();
}

const Sample& History::fromEnd(int k) const {
    int n = size();
    return data[(offset + n - 1 - k) % n];
}

// ---------------------------- SinglePendulum ----------------------------

double SinglePendulum::angularAcceleration(double theta, double omega) const {
    return -(g / length) * std::sin(theta) - damping * omega;
}

// Runge-Kutta 4: samples the slope at 4 points inside the step and averages them.
// Much more accurate than plain Euler (which slowly gains energy and blows up).
void SinglePendulum::step(double dt){
    double k1_theta = omega;
    double k1_omega = angularAcceleration(theta, omega);

    double k2_theta = omega + 0.5 * dt * k1_omega;
    double k2_omega = angularAcceleration(theta + 0.5 * dt * k1_theta, k2_theta);

    double k3_theta = omega + 0.5 * dt * k2_omega;
    double k3_omega = angularAcceleration(theta + 0.5 * dt * k2_theta, k3_theta);

    double k4_theta = omega + dt * k3_omega;
    double k4_omega = angularAcceleration(theta + dt * k3_theta, k4_theta);

    theta += dt / 6.0 * (k1_theta + 2 * k2_theta + 2 * k3_theta + k4_theta);
    omega += dt / 6.0 * (k1_omega + 2 * k2_omega + 2 * k3_omega + k4_omega);
    time  += dt;
}

double SinglePendulum::kineticEnergy() const {
    return 0.5 * mass * length * length * omega * omega;
}

double SinglePendulum::potentialEnergy() const {
    return mass * g * length * (1.0 - std::cos(theta));
}

double SinglePendulum::tension() const {
    return mass * (g * std::cos(theta) + length * omega * omega);
}

double SinglePendulum::smallAnglePeriod() const {
    return 2.0 * PI * std::sqrt(length / g);
}

// ------------------------------ Simulation ------------------------------

void Simulation::reset(){
    pendulum.theta = initialAngleDeg * PI / 180.0;
    pendulum.omega = initialOmega;
    pendulum.time  = 0.0;
    accumulator    = 0.0;

    history.clear();
    initialEnergy = pendulum.kineticEnergy() + pendulum.potentialEnergy();
    record();
}

void Simulation::update(double frameDt){
    if(!running) return;

    // Clamp long frames (window dragged, breakpoint...) so the sim doesn't jump ahead
    accumulator += std::min(frameDt, 0.05) * speed;

    // Fixed timestep: physics always advances in equal PHYSICS_DT chunks,
    // no matter how fast or slow the frames are
    while(accumulator >= PHYSICS_DT){
        pendulum.step(PHYSICS_DT);
        accumulator -= PHYSICS_DT;
    }
    record();
}

void Simulation::record(){
    const SinglePendulum& p = pendulum;
    double ke = p.kineticEnergy();
    double pe = p.potentialEnergy();

    history.push({ p.time,
                   p.theta * 180.0 / PI,
                   p.omega,
                   ke, pe, ke + pe,
                   p.length * std::sin(p.theta),
                  -p.length * std::cos(p.theta) });
}

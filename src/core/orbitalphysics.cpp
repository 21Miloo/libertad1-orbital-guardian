#include "orbitalphysics.h"
#include <cmath>

Vector2D OrbitalPhysics::gravityAt(const Vector2D& position) const{

    Vector2D r(position-center);

    double d = r.length(); //magnitud del vector

    return r * (-gm / (d * d * d));
}

// Modifica la posicion y velocidad.
void OrbitalPhysics::step(Vector2D& position, Vector2D& velocity, double dt) const {

    velocity = velocity + gravityAt(position) * dt;
    position = position + velocity * dt;

}

double OrbitalPhysics::gmForOrbit(double radius, double period){

    return (4 * ( M_PI * M_PI) * (radius * radius * radius)) / (period * period);

}

// devuelve la velocidad circular del objeto
double OrbitalPhysics::circularSpeed(double radius) const {

   return std::sqrt(gm/radius);

}

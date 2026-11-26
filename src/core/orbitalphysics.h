#ifndef ORBITALPHYSICS_H
#define ORBITALPHYSICS_H

#include "vector2d.h"

class OrbitalPhysics{

private:

    Vector2D center;
    double gm; // gm -> G * M sale de masa por gravedad (formula de gravitacion universal)

public:

    // Constructor por lista de inicializacion
    OrbitalPhysics(const Vector2D& orbitCenter, double gmValue): center(orbitCenter), gm(gmValue) {}

    // Gravedad en un punto. (Devuelve la aceleracion)
    Vector2D gravityAt(const Vector2D& position) const;

    // Modifica la posicion y velocidad.
    void step(Vector2D& position, Vector2D& velocity, double dt) const;

    static double gmForOrbit(double radius, double period); // calcula el GM necesario para que una órbita de ese radio tarde ese periodo.
    // devuelve la velocidad circular del objeto
    double circularSpeed(double radius) const;

};

#endif // ORBITALPHYSICS_H

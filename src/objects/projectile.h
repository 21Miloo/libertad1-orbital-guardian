#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <QGraphicsScene>

#include "spaceobject.h"

// Disparo de la nave. Lo afecta la gravedad y desaparece al acabarse su
// tiempo de vida o al salir de la pantalla.
class Projectile : public SpaceObject {

private:
    double lifetime = 0.0;  // s que le quedan

public:
    static constexpr double RADIUS = 3.0;    // px
    static constexpr double SPEED = 220.0;   // px/s, relativo a la nave
    static constexpr double LIFETIME = 4.0;  // s

    explicit Projectile(QGraphicsScene &scene);

    // Disparar: sale con la posición y velocidad dadas y el tiempo de vida lleno
    void launch(const Vector2D &startPosition, const Vector2D &startVelocity);

    void update(double dt, const OrbitalPhysics &physics) override;
};

#endif // PROJECTILE_H

#ifndef DRONE_H
#define DRONE_H

#include <QGraphicsScene>
#include <QPolygonF>

#include "spaceobject.h"

// Dron de Soundwave. Además de la gravedad tiene un motor que lo empuja
// hacia su objetivo (seek steering) y aguanta varios disparos.
class Drone : public SpaceObject {

private:
    const Vector2D &target;  // posición que persigue (la del satélite); la actualiza la ventana
    int health = 0;
    int originSector = -1;   // sector de Soundwave desde el que salió (para que aprenda)
    double maxSpeed = 110.0; // px/s
    double thrust = 60.0;    // px/s²

    static QPolygonF arrowShape();
    Vector2D seek() const;

public:
    static constexpr double RADIUS = 9.0;        // px
    static constexpr int MAX_HEALTH = 2;         // disparos que aguanta
    static constexpr int DAMAGE = 2;             // vida que le quita al satélite

    Drone(QGraphicsScene &scene, const Vector2D &targetPosition);

    // Aparece quieto en la posición dada, con la vida llena
    void launch(const Vector2D &startPosition, int sector);

    // Inyectado por la dificultad. Lanza invalid_argument si <= 0
    void setPerformance(double maxSpeedValue, double thrustValue);

    int getOriginSector() const { return originSector; }

    // Recibe un disparo. Devuelve true si quedó destruido.
    bool takeHit();

    void update(double dt, const OrbitalPhysics &physics) override;
    void syncGraphics() override;
};

#endif // DRONE_H

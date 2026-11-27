#ifndef DEBRIS_H
#define DEBRIS_H

#include <QGraphicsScene>
#include <QPolygonF>

#include "spaceobject.h"

// Basura espacial: solo siente la gravedad. No tiene tiempo de vida ni se
// desactiva al salir de la pantalla, porque su órbita la trae de vuelta.
class Debris : public SpaceObject {

private:
    static QPolygonF rockShape();

public:
    static constexpr double RADIUS = 10.0;  // px, tamaño de la roca y radio de choque

    explicit Debris(QGraphicsScene &scene);

    // rotationDegrees: giro del dibujo, para que no todas las rocas se vean iguales
    void launch(const Vector2D &startPosition, const Vector2D &startVelocity, double rotationDegrees);

    void update(double dt, const OrbitalPhysics &physics) override;
};

#endif // DEBRIS_H

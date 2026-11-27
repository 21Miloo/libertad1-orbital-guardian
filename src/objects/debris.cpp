#include "debris.h"

Debris::Debris(QGraphicsScene &scene)
    : SpaceObject(scene.addPolygon(rockShape(), Qt::NoPen, QColor(140, 130, 120)),  // gris roca
                  RADIUS)
{
}


// Roca irregular centrada en el origen. Los puntos están en proporción al radio:
// cambiar RADIUS cambia el dibujo y el choque a la vez.
QPolygonF Debris::rockShape()
{
    const double r = RADIUS;
    QPolygonF shape;
    shape << QPointF(r, 0) << QPointF(0.45 * r, 0.85 * r) << QPointF(-0.55 * r, 0.85 * r)
          << QPointF(-r, 0.15 * r) << QPointF(-0.55 * r, -0.85 * r) << QPointF(0.45 * r, -0.7 * r);
    return shape;
}


void Debris::launch(const Vector2D &startPosition, const Vector2D &startVelocity, double rotationDegrees)
{
    item->setRotation(rotationDegrees);  // solo al aparecer, no cada frame
    activate(startPosition, startVelocity);
}


void Debris::update(double dt, const OrbitalPhysics &physics)
{
    physics.step(position, velocity, dt);
}

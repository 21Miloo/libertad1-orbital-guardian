#include "drone.h"

#include <QtMath>
#include <cmath>
#include <stdexcept>

namespace {
const QColor DRONE_COLOR(150, 90, 220);    // morado, el color de Soundwave
const QColor DAMAGED_COLOR(220, 80, 150);  // rosado: ya recibió un disparo
}


Drone::Drone(QGraphicsScene &scene, const Vector2D &targetPosition)
    : SpaceObject(scene.addPolygon(arrowShape(), Qt::NoPen, DRONE_COLOR), RADIUS),
      target(targetPosition)
{
}


// Flecha apuntando a la derecha (ángulo 0), como la nave, para poder girarla
// en la dirección en que se mueve
QPolygonF Drone::arrowShape()
{
    QPolygonF shape;
    shape << QPointF(10, 0) << QPointF(0, 7) << QPointF(-8, 4)
          << QPointF(-8, -4) << QPointF(0, -7);
    return shape;
}


void Drone::launch(const Vector2D &startPosition, int sector)
{
    health = MAX_HEALTH;
    originSector = sector;
    item->setBrush(DRONE_COLOR);
    activate(startPosition, Vector2D());  // arranca quieto; su motor lo acelera
}


// Fuera del hot path: solo al elegir dificultad
void Drone::setPerformance(double maxSpeedValue, double thrustValue)
{
    // Invariante de seek(): ambos > 0
    if (maxSpeedValue <= 0.0 || thrustValue <= 0.0) {
        throw std::invalid_argument("Drone: la velocidad y el empuje deben ser positivos");
    }
    maxSpeed = maxSpeedValue;
    thrust = thrustValue;
}


bool Drone::takeHit()
{
    --health;
    if (health <= 0) {
        deactivate();
        return true;
    }
    item->setBrush(DAMAGED_COLOR);
    return false;
}


void Drone::update(double dt, const OrbitalPhysics &physics)
{
    physics.step(position, velocity, dt, seek());
}


// Además de moverse, apunta hacia donde va. atan2 se calcula aquí, una vez
// por frame, y no en cada paso de física: solo importa para el dibujo.
void Drone::syncGraphics()
{
    SpaceObject::syncGraphics();  // la parte común: la posición
    item->setRotation(qRadiansToDegrees(std::atan2(velocity.y, velocity.x)));
}


// "Seek steering": el dron calcula la velocidad que le gustaría tener (directo al
// objetivo a velocidad máxima) y empuja para corregir la diferencia con la que tiene.
// El empuje está limitado, así que no gira en seco: se pasa, corrige y hace curvas.
// Costo: 2 raíces cuadradas por dron y por paso.
Vector2D Drone::seek() const
{
    const Vector2D toTarget = target - position;
    const double distance = toTarget.length();
    if (distance < 1e-6) return Vector2D();  // ya está encima: evita dividir entre cero

    const Vector2D desired = toTarget * (maxSpeed / distance);
    Vector2D steering = desired - velocity;

    const double strength = steering.length();
    if (strength > thrust) {
        steering = steering * (thrust / strength);  // recortar al empuje máximo
    }
    return steering;
}

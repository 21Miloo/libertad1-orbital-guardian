#include "projectile.h"
#include "core/constants.h"

Projectile::Projectile(QGraphicsScene &scene)
    : SpaceObject(scene.addEllipse(-RADIUS, -RADIUS, 2 * RADIUS, 2 * RADIUS,
                                   Qt::NoPen, QColor(255, 140, 0)),  // naranja
                  RADIUS)
{
}


void Projectile::launch(const Vector2D &startPosition, const Vector2D &startVelocity)
{
    lifetime = LIFETIME;
    activate(startPosition, startVelocity);
}


void Projectile::update(double dt, const OrbitalPhysics &physics)
{
    physics.step(position, velocity, dt);
    lifetime -= dt;

    const double margin = 50.0;  // px fuera de la pantalla antes de desactivarlo
    const bool outOfScreen = position.x < -margin || position.x > config::WINDOW_WIDTH + margin
                          || position.y < -margin || position.y > config::WINDOW_HEIGHT + margin;

    if (lifetime <= 0.0 || outOfScreen) deactivate();
}

#ifndef SPACEOBJECT_H
#define SPACEOBJECT_H

#include <QAbstractGraphicsShapeItem>

#include "core/vector2d.h"
#include "core/orbitalphysics.h"

// Clase base de todo lo que se mueve en órbita y puede chocar: disparos,
// basura y drones. Es abstracta: no se puede crear un SpaceObject "a secas",
// solo una de sus hijas, y cada hija decide cómo se mueve (update).
//
// Une las dos capas: la lógica (posición, velocidad, radio) y su dibujo
// en la escena (item). El ítem lo crea la hija y su dueña es la escena.
class SpaceObject {

protected:
    QAbstractGraphicsShapeItem *item;  // elipse o polígono; la escena lo destruye
    Vector2D position;                 // px
    Vector2D velocity;                 // px/s
    double radius;                     // px, radio del círculo de choque
    bool active = false;               // false = libre en el pool, escondido

public:
    SpaceObject(QAbstractGraphicsShapeItem *graphicsItem, double collisionRadius);
    virtual ~SpaceObject() = default;  // virtual: se destruyen a través de punteros a la base

    // dos objetos compartirían el mismo ítem de la escena
    SpaceObject(const SpaceObject &) = delete;
    SpaceObject &operator=(const SpaceObject &) = delete;

    // Un paso de física. Cada hija lo implementa a su manera
    virtual void update(double dt, const OrbitalPhysics &physics) = 0;

    // Pasa la posición calculada al dibujo. Se llama una vez por frame.
    // Las hijas que necesiten algo más (por ejemplo girar) lo sobrescriben.
    virtual void syncGraphics();

    void activate(const Vector2D &startPosition, const Vector2D &startVelocity);
    void deactivate();

    bool overlaps(const Vector2D &point, double otherRadius) const;
    bool overlaps(const SpaceObject &other) const;

    bool isActive() const { return active; }
    const Vector2D &getPosition() const { return position; }
    double getRadius() const { return radius; }
};

#endif // SPACEOBJECT_H

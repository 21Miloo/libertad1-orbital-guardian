#include "spaceobject.h"

#include <stdexcept>

SpaceObject::SpaceObject(QAbstractGraphicsShapeItem *graphicsItem, double collisionRadius)
    : item(graphicsItem), radius(collisionRadius)
{
    // Un objeto sin dibujo o con radio inválido rompería el juego más adelante
    // (puntero nulo, choques imposibles). Mejor fallar aquí, al crearlo.
    if (item == nullptr) {
        throw std::invalid_argument("SpaceObject: el ítem gráfico no puede ser nulo");
    }
    if (radius <= 0.0) {
        throw std::invalid_argument("SpaceObject: el radio debe ser positivo");
    }

    item->hide();  // nace libre en el pool
}


void SpaceObject::syncGraphics()
{
    item->setPos(position.x, position.y);
}


// Sacar un objeto del pool: se le da su estado inicial y se muestra
void SpaceObject::activate(const Vector2D &startPosition, const Vector2D &startVelocity)
{
    position = startPosition;
    velocity = startVelocity;
    active = true;

    item->setPos(position.x, position.y);
    item->show();
}


// Devolverlo al pool: solo se esconde, no se destruye
void SpaceObject::deactivate()
{
    active = false;
    item->hide();
}


// Dos círculos se tocan si la distancia entre centros es menor que la suma de radios.
// Se compara todo al cuadrado para no calcular la raíz.
bool SpaceObject::overlaps(const Vector2D &point, double otherRadius) const
{
    const double dx = position.x - point.x;
    const double dy = position.y - point.y;
    const double radii = radius + otherRadius;
    return dx * dx + dy * dy < radii * radii;
}


bool SpaceObject::overlaps(const SpaceObject &other) const
{
    return overlaps(other.position, other.radius);
}

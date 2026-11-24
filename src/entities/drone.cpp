#include "drone.h"

Drone::Drone(QGraphicsScene *scene, int x, int y, int left, int right) {
    body = scene->addRect(0, 0, 40, 25);
    body->setPos(x, y);
    body->setBrush(Qt::red);
    leftLimit=left;
    rightLimit=right;
    speed=2;

    vision = scene->addPolygon(QPolygonF());          // empty for now, we shape it every frame
    vision->setBrush(QColor(255, 255, 0, 60));        // transparent yellow
    vision->setPen(Qt::NoPen);
}

void Drone::updatePosition()
{
    body->moveBy(speed, 0);

    if (body->x() <= leftLimit || body->x() >= rightLimit) {
        speed = -speed;   // turn around
    }

    int direction = 1;
    if (speed < 0) direction = -1;

    double eyeX = body->x() + 20;   // middle of the drone
    double eyeY = body->y() + 12;

    QPolygonF cone;
    cone << QPointF(eyeX, eyeY)                           // tip, on the drone
         << QPointF(eyeX + 160 * direction, eyeY - 50)    // far corner, top
         << QPointF(eyeX + 160 * direction, eyeY + 50);   // far corner, bottom
    vision->setPolygon(cone);

}


bool Drone::canSee(QGraphicsItem *target)
{
    return vision->collidesWithItem(target);
}

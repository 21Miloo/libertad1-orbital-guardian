#include "drone.h"
#include <QtMath>

Drone::Drone(QGraphicsScene *scene, int x, int y, int left, int right) {
    body = scene->addRect(0, 0, 40, 25);
    body->setPos(x, y);
    body->setBrush(Qt::red);
    leftLimit=left;
    rightLimit=right;
    speed=2;

    // the cone looks to the right, then we rotate it
    QPolygonF cone;
    cone << QPointF(0, 0) << QPointF(160, -50) << QPointF(160, 50);
    vision = scene->addPolygon(cone);
    vision->setBrush(QColor(255, 255, 0, 60));        // transparent yellow
    vision->setPen(Qt::NoPen);
    state = PATROL;
}

void Drone::updatePosition(QGraphicsItem *player)
{
    // if he sees bumblebee he chases him, if not he patrols
    if (canSee(player)) state = CHASE;
    else state = PATROL;

    if (state == PATROL) {
        if (body->x() <= leftLimit) speed = 2;
        if (body->x() >= rightLimit) speed = -2;
        body->moveBy(speed, 0);
    }
    else if (state == CHASE) {
        if (player->x() > body->x()) speed = 2.5;
        else speed = -2.5;

        int moveY = 0;
        if (player->y() > body->y()) moveY = 1;
        if (player->y() < body->y()) moveY = -1;

        body->moveBy(speed, moveY);
    }

    // like a flashlight: move it to the drone and rotate it
    vision->setPos(body->x() + 20, body->y() + 12);

    double angle = 0;               // looking right
    if (speed < 0) angle = 180;     // looking left

    if (state == CHASE) {
        double dx = player->sceneBoundingRect().center().x() - vision->x();
        double dy = player->sceneBoundingRect().center().y() - vision->y();
        angle = qRadiansToDegrees(qAtan2(dy, dx));   // angle towards bumblebee
    }

    // turn slowly, max 2 degrees per frame
    double turn = angle - vision->rotation();
    if (turn > 180) turn -= 360;     // take the short way around
    if (turn < -180) turn += 360;
    if (turn > 2) turn = 2;
    if (turn < -2) turn = -2;
    vision->setRotation(vision->rotation() + turn);

}


bool Drone::canSee(QGraphicsItem *target)
{
    return vision->collidesWithItem(target);
}

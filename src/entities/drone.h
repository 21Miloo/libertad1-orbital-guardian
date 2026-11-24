#ifndef DRONE_H
#define DRONE_H

#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsPolygonItem>

enum DroneState { PATROL, CHASE };

class Drone
{
private:
    QGraphicsRectItem *body;   // the drone's rectangle in the scene
    double speed;
    int leftLimit;
    int rightLimit;
    QGraphicsPolygonItem *vision;
    DroneState state;
public:
    Drone(QGraphicsScene *scene, int x, int y, int left, int right);
    void updatePosition(QGraphicsItem *player);
    bool canSee(QGraphicsItem *target);
};

#endif // DRONE_H

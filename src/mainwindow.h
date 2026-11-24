#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QSet>
#include <QKeyEvent>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QTimer>
#include <QGraphicsEllipseItem>
#include "entities/drone.h"
struct Shot {
    QGraphicsEllipseItem *body;
    double velocityX;
    double velocityY;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    QSet<int> pressedKeys;
    QGraphicsScene *scene;
    QGraphicsView *view;
    QGraphicsRectItem *player;
    QTimer *timer;
    QList<QGraphicsRectItem*> obstacles;
    QList<Shot> shots;
    int facingDirection = 1;   // 1 = right, -1 = left
    const double restitution = 0.2;   // e: energy shot vs metal furniture
    const int floorTop = 300;
    const int floorBottom = 600;
    QGraphicsEllipseItem *shadow;
    Drone *drone;
private slots:
    void updateGame();
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
};
#endif // MAINWINDOW_H

#include "mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //we just create the Scene
    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, 800, 600);

    view = new QGraphicsView(scene,this);
    setCentralWidget(view);

    player = scene->addRect(0, 0, 40, 40);
    resize(820, 620);

    player->setBrush(Qt::yellow);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateGame);
    timer->start(16);

    // create the obstacles
    obstacles.append(scene->addRect(200, 150, 120, 40));
    obstacles.append(scene->addRect(500, 100, 40, 200));
    obstacles.append(scene->addRect(300, 400, 200, 50));

    for (QGraphicsRectItem *obstacle : obstacles) {
        obstacle->setBrush(Qt::gray);
    }
}

MainWindow::~MainWindow() {}

void MainWindow::keyPressEvent(QKeyEvent *event){
    if (event->isAutoRepeat())return;
    pressedKeys.insert(event->key());

    if (event->key() == Qt::Key_Space) {
        Shot newShot;
        newShot.body = scene->addEllipse(player->x() + 15, player->y() + 15, 10, 10);
        newShot.body->setBrush(QColor(255, 140, 0));   // orange
        newShot.velocityX = 8 * facingDirection;
        newShot.velocityY = 0;
        shots.append(newShot);
    }

    qDebug() << pressedKeys;

}


void MainWindow::keyReleaseEvent(QKeyEvent *event){
    if (event->isAutoRepeat())return;
    pressedKeys.remove(event->key());
    qDebug() << pressedKeys;
}

void MainWindow::updateGame(){
    const int speed = 5;   // pixels per frame
    int moveX = 0;
    int moveY = 0;

    if (pressedKeys.contains(Qt::Key_W)) moveY -= speed;   // up (Y grows downward)
    if (pressedKeys.contains(Qt::Key_S)) moveY += speed;   // down
    if (pressedKeys.contains(Qt::Key_A)) moveX -= speed;   // left
    if (pressedKeys.contains(Qt::Key_D)) moveX += speed;   // right

    if (moveX > 0) facingDirection = 1;
    if (moveX < 0) facingDirection = -1;

    player->moveBy(moveX, moveY);

    // move every shot; delete the ones that left the scene
    // (backwards, so removing one doesn't skip the next)
    for (int i = shots.size() - 1; i >= 0; i--) {
        Shot &shot = shots[i];
        shot.body->moveBy(shot.velocityX, shot.velocityY);

        if (!scene->sceneRect().intersects(shot.body->sceneBoundingRect())) {
            scene->removeItem(shot.body);
            delete shot.body;
            shots.removeAt(i);
        }
    }
}
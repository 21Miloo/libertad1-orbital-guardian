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

    scene->addRect(0, 0, 800, floorTop);
    scene->addRect(0, floorTop, 800, floorBottom - floorTop);

    shadow = scene->addEllipse(0, 0, 40, 10);
    shadow->setBrush(QColor(0, 0, 0, 90));
    shadow->setPen(Qt::NoPen);
    player = scene->addRect(0, 0, 40, 40);
    player->setPos(100,450);
    player->setTransformOriginPoint(20, 40);
    resize(820, 620);

    player->setBrush(Qt::yellow);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateGame);
    timer->start(16);

    // create the obstacles
    obstacles.append(scene->addRect(150, 320, 120, 40));
    obstacles.append(scene->addRect(600, 350, 40, 150));
    obstacles.append(scene->addRect(300, 400, 200, 50));

    for (QGraphicsRectItem *obstacle : obstacles) {
        obstacle->setBrush(Qt::gray);
        obstacle->setZValue(obstacle->sceneBoundingRect().bottom());
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



    // 2.5D: only the feet collide with the obstacle's footprint on the floor
    QRectF body = player->sceneBoundingRect();
    QRectF feetBox(body.left(), body.bottom() - 10, body.width(), 10);   // the last 10 px (his feet)

    for (QGraphicsRectItem *obstacle : obstacles) {
        QRectF o = obstacle->sceneBoundingRect();
        QRectF base(o.left(), o.bottom() - 15, o.width(), 15);   // the last 15 px (its footprint)

        if (feetBox.intersects(base)) {
            player->moveBy(-moveX, -moveY);
            break;   // undo only once
        }
    }

    double feet = player->sceneBoundingRect().bottom();
    if (feet < floorTop || feet > floorBottom) {
        player->moveBy(-moveX, -moveY);
    }

    player->setZValue(feet);
    double depth = (feet - floorTop) / (floorBottom - floorTop);   // 0 = back, 1 = front
    player->setScale(0.7 + 0.3 * depth);
    double shadowWidth = 40 * (0.7 + 0.3 * depth);
    double centerX = player->sceneBoundingRect().center().x();
    shadow->setRect(centerX - shadowWidth / 2, feet - 5, shadowWidth, 10);
    shadow->setZValue(feet - 1);
    // move every shot; delete the ones that left the scene
    // (backwards, so removing one doesn't skip the next)
    for (int i = shots.size() - 1; i >= 0; i--) {
        Shot &shot = shots[i];
        shot.body->moveBy(shot.velocityX, shot.velocityY);

        for (int j = 0; j < obstacles.size(); j++) {
            if (shot.body->collidesWithItem(obstacles[j])) {
                // 1. undo the move (get it out of the obstacle)
                shot.body->moveBy(-shot.velocityX, -shot.velocityY);

                // 2. which side did it hit? try moving only in X
                shot.body->moveBy(shot.velocityX, 0);
                bool hitLeftOrRight = shot.body->collidesWithItem(obstacles[j]);
                shot.body->moveBy(-shot.velocityX, 0);   // undo the test

                // 3. bounce: v⊥' = -e · v⊥   (the parallel one stays the same)
                if (hitLeftOrRight) {
                    shot.velocityX = -restitution * shot.velocityX;
                } else {
                    shot.velocityY = -restitution * shot.velocityY;
                }
                break;   // already bounced, no need to check the other obstacles
            }
        }

        // delete it if it left the scene or is almost stopped (< 1 px per frame)
        bool outOfScene = !scene->sceneRect().intersects(shot.body->sceneBoundingRect());
        bool tooSlow = qAbs(shot.velocityX) < 1 && qAbs(shot.velocityY) < 1;

        if (outOfScene || tooSlow) {
            scene->removeItem(shot.body);
            delete shot.body;
            shots.removeAt(i);
        }
    }
}
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

MainWindow::~MainWindow() {

}

void MainWindow::keyPressEvent(QKeyEvent *event){
    if (event->isAutoRepeat())return;
    pressedKeys.insert(event->key());
    qDebug() << pressedKeys;
}


void MainWindow::keyReleaseEvent(QKeyEvent *event){
    if (event->isAutoRepeat())return;
    pressedKeys.remove(event->key());
    qDebug() << pressedKeys;
}

void MainWindow::updateGame(){
    const int speed = 5;   // pixels per frame
    int dx = 0;
    int dy = 0;

    if (pressedKeys.contains(Qt::Key_W)) dy -= speed;   // up (Y grows downward)
    if (pressedKeys.contains(Qt::Key_S)) dy += speed;   // down
    if (pressedKeys.contains(Qt::Key_A)) dx -= speed;   // left
    if (pressedKeys.contains(Qt::Key_D)) dx += speed;   // right

    player->moveBy(dx, dy);
}
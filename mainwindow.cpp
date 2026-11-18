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
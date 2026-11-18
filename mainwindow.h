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

struct Shot {
    QGraphicsEllipseItem *item;
    double vx;
    double vy;
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
    int facing = 1;

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

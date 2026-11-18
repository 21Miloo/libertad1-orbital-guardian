#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSet>
#include <QKeyEvent>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsRectItem>

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    QSet<int> pressedKeys;
    QGraphicsScene *scene;
    QGraphicsView *view;
    QGraphicsRectItem *player;
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
};
#endif // MAINWINDOW_H

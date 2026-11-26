#ifndef ORBITWINDOW_H
#define ORBITWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsEllipseItem>
#include <QTimer>
#include <QElapsedTimer>

#include "core/orbitalphysics.h"



class OrbitWindow : public QMainWindow{

    Q_OBJECT

private:

    static constexpr double ORBIT_RADIUS = 300.0; // px
    static constexpr double ORBIT_PERIOD = 20.0; // segundos por vuelta
    static constexpr double EARTH_RADIUS = 80.0; // px
    static constexpr double SATELLITE_RADIUS = 8.0; // px


    QGraphicsScene *scene;
    QGraphicsView *view;
    QGraphicsEllipseItem *earth;
    QGraphicsEllipseItem *satellite;

    //traemos nuestra fisica de la orbita la cual incluye la clase vector tambien. Son atributos de la clase
    OrbitalPhysics physics;
    Vector2D satellitePosition;
    Vector2D satelliteVelocity;

    // Bucle del juego. Por valor: sin heap, se destruyen con la ventana
    QTimer timer;              // despierta al juego cada TICK_MS
    QElapsedTimer clock;       // mide el tiempo real entre ticks
    double accumulator = 0.0;  // tiempo real pendiente de simular (s)

private slots:
    void updateGame();

public:
    explicit OrbitWindow(QWidget *parent = nullptr);

};

#endif // ORBITWINDOW_H

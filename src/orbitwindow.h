#ifndef ORBITWINDOW_H
#define ORBITWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QTimer>
#include <QElapsedTimer>
#include <QKeyEvent>

#include "core/orbitalphysics.h"



class OrbitWindow : public QMainWindow{

    Q_OBJECT

private:

    static constexpr double ORBIT_RADIUS = 300.0; // px
    static constexpr double ORBIT_PERIOD = 20.0; // segundos por vuelta
    static constexpr double EARTH_RADIUS = 80.0; // px
    static constexpr double SATELLITE_RADIUS = 8.0; // px
    static constexpr double SHIP_ORBIT_RADIUS = 200.0; // px, órbita inicial de la nave
    static constexpr double SHIP_ROTATION_SPEED = 180.0; // grados por segundo
    static constexpr double SHIP_THRUST = 40.0;          // px/s², menor que la gravedad a 200 px (~67)

    // Predicción de trayectoria: 120 pasos de 1/30 s = 4 s hacia adelante,
    // suficiente para ver hacia dónde va la nave sin llenar la pantalla
    static constexpr int PREDICTION_STEPS = 120;
    static constexpr double PREDICTION_DT = 1.0 / 30.0;  // s


    QGraphicsScene *scene;
    QGraphicsView *view;
    QGraphicsEllipseItem *earth;
    QGraphicsEllipseItem *satellite;
    QGraphicsPolygonItem *ship;
    QGraphicsPathItem *trajectory;  // línea de predicción de la nave
    QPainterPath trajectoryPath;    // se reutiliza cada frame: clear() conserva la memoria

    //traemos nuestra fisica de la orbita la cual incluye la clase vector tambien. Son atributos de la clase
    OrbitalPhysics physics;
    Vector2D satellitePosition;
    Vector2D satelliteVelocity;
    Vector2D shipPosition;   // px
    Vector2D shipVelocity;   // px/s
    double shipAngle = 90.0; // grados; 0 = derecha, 90 = abajo (y crece hacia abajo)

    // Teclas de la nave. Solo son 3 y fijas: 3 bool en vez de un QSet
    // (sin heap ni hash, lectura directa)
    bool rotatingLeft = false;   // A
    bool rotatingRight = false;  // D
    bool thrusting = false;      // W

    // Bucle del juego. Por valor: sin heap, se destruyen con la ventana
    QTimer timer;              // despierta al juego cada TICK_MS
    QElapsedTimer clock;       // mide el tiempo real entre ticks
    double accumulator = 0.0;  // tiempo real pendiente de simular (s)

    void updateTrajectory();

private slots:
    void updateGame();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

public:
    explicit OrbitWindow(QWidget *parent = nullptr);

};

#endif // ORBITWINDOW_H

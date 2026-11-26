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

#include <array>

#include "core/orbitalphysics.h"
#include "core/constants.h"



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

    // Centro de la Tierra = centro de la ventana
    static constexpr double EARTH_X = config::WINDOW_WIDTH / 2.0;
    static constexpr double EARTH_Y = config::WINDOW_HEIGHT / 2.0;

    // Disparos
    static constexpr int MAX_PROJECTILES = 32;           // tamaño del pool
    static constexpr double PROJECTILE_SPEED = 220.0;    // px/s, relativo a la nave
    static constexpr double PROJECTILE_LIFETIME = 4.0;   // s antes de desaparecer
    static constexpr double PROJECTILE_RADIUS = 3.0;     // px
    static constexpr double FIRE_COOLDOWN = 0.2;         // s entre disparos (5 por segundo)

    // Un proyectil del pool. Se crean todos al iniciar; disparar es activar uno libre
    // y desaparecer es desactivarlo. Cero new/delete durante el juego.
    struct Projectile {
        QGraphicsEllipseItem *item = nullptr;  // dueña: la escena
        Vector2D position;
        Vector2D velocity;
        double lifetime = 0.0;   // s que le quedan
        bool active = false;
    };


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
    bool firing = false;         // Espacio (mantener = disparo continuo)

    // Pool de proyectiles: std::array de tamaño fijo, memoria contigua y sin heap
    std::array<Projectile, MAX_PROJECTILES> projectiles;
    double fireCooldown = 0.0;   // s que faltan para poder volver a disparar

    // Bucle del juego. Por valor: sin heap, se destruyen con la ventana
    QTimer timer;              // despierta al juego cada TICK_MS
    QElapsedTimer clock;       // mide el tiempo real entre ticks
    double accumulator = 0.0;  // tiempo real pendiente de simular (s)

    void updateTrajectory();
    void fireProjectile();
    void updateProjectiles(double dt);
    static bool isInsideEarth(const Vector2D &position);

private slots:
    void updateGame();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

public:
    explicit OrbitWindow(QWidget *parent = nullptr);

};

#endif // ORBITWINDOW_H

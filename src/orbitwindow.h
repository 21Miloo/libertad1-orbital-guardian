#ifndef ORBITWINDOW_H
#define ORBITWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsPathItem>
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QPainterPath>
#include <QTimer>
#include <QElapsedTimer>
#include <QKeyEvent>
#include <QColor>

#include <array>
#include <memory>
#include <vector>

#include "core/orbitalphysics.h"
#include "core/constants.h"
#include "core/difficulty.h"
#include "objects/projectile.h"
#include "objects/debris.h"
#include "objects/drone.h"
#include "agent/soundwaveagent.h"



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
    static constexpr double FIRE_COOLDOWN = 0.2;         // s entre disparos (5 por segundo)

    // Basura espacial
    static constexpr int MAX_DEBRIS = 12;                 // tamaño del pool
    static constexpr double DEBRIS_SPAWN_INTERVAL = 3.0;  // s entre apariciones
    static constexpr double DEBRIS_MIN_ORBIT = 240.0;     // px, cerca de la órbita del satélite
    static constexpr double DEBRIS_MAX_ORBIT = 360.0;     // px
    static constexpr double SAFE_SPAWN_DISTANCE = 150.0;  // px, no aparecer encima del satélite

    // Drones: su comportamiento lo define la dificultad
    static constexpr int MAX_DRONES = 6;                  // tamaño del pool

    // Victoria por tiempo
    static constexpr double TRANSMISSION_TIME = 90.0;     // s
    static constexpr double TRANSMISSION_BAR_WIDTH = 300.0; // px

    // Vida y choques
    static constexpr double SHIP_RADIUS = 9.0;            // px, círculo que envuelve el triángulo
    static constexpr int SATELLITE_MAX_HEALTH = 5;
    static constexpr int SHIP_MAX_HEALTH = 3;


    QGraphicsScene *scene;
    QGraphicsView *view;
    QGraphicsEllipseItem *earth;
    QGraphicsEllipseItem *satellite;
    QGraphicsPolygonItem *ship;
    QGraphicsPolygonItem *soundwaveItem;  // dibujo de Soundwave
    QGraphicsPathItem *trajectory;  // línea de predicción de la nave
    QPainterPath trajectoryPath;    // se reutiliza cada frame: clear() conserva la memoria

    //traemos nuestra fisica de la orbita la cual incluye la clase vector tambien. Son atributos de la clase
    OrbitalPhysics physics;
    Vector2D satellitePosition;
    Vector2D satelliteVelocity;
    SoundwaveAgent soundwave; // agente inteligente: decide desde dónde atacar
    Vector2D shipPosition;   // px
    Vector2D shipVelocity;   // px/s
    double shipAngle = 90.0; // grados; 0 = derecha, 90 = abajo (y crece hacia abajo)

    // Teclas de la nave. Solo son 3 y fijas: 3 bool en vez de un QSet
    // (sin heap ni hash, lectura directa)
    bool rotatingLeft = false;   // A
    bool rotatingRight = false;  // D
    bool thrusting = false;      // W
    bool firing = false;         // Espacio (mantener = disparo continuo)

    // Pools de objetos. Cada objeto se crea con new una sola vez al iniciar
    // (memoria dinámica, necesaria para el polimorfismo) y vive hasta que se
    // cierra la ventana. unique_ptr lo libera solo: sin delete a mano ni fugas.
    std::array<std::unique_ptr<Projectile>, MAX_PROJECTILES> projectiles;
    std::array<std::unique_ptr<Debris>, MAX_DEBRIS> debris;
    std::array<std::unique_ptr<Drone>, MAX_DRONES> drones;

    // Todos los objetos anteriores vistos como su clase base. No es dueño de nada:
    // sirve para moverlos, dibujarlos y revisarlos contra la Tierra en un solo
    // ciclo, sin importar de qué tipo sea cada uno (polimorfismo).
    std::vector<SpaceObject*> spaceObjects;

    double fireCooldown = 0.0;                   // s que faltan para poder volver a disparar
    double debrisTimer = DEBRIS_SPAWN_INTERVAL;  // s que faltan para la siguiente basura

    int satelliteHealth = SATELLITE_MAX_HEALTH;
    int shipHealth = SHIP_MAX_HEALTH;

    // Máquina de estados: gobierna input y loop
    enum class GameState { Menu, Playing, Over };
    GameState state = GameState::Menu;

    // Apunta a la tabla constexpr: cambiar de dificultad no copia nada
    const DifficultySettings *difficulty = &settingsFor(Difficulty::Normal);

    double transmissionTime = 0.0;  // s
    int transmissionPercent = -1;   // caché del HUD; -1 invalida

    // HUD: una barrita por cada punto de vida. Se crean al iniciar y solo
    // cambian de color al recibir daño; no se crea ni se escribe texto por frame.
    std::array<QGraphicsRectItem*, SATELLITE_MAX_HEALTH> satelliteBars;
    std::array<QGraphicsRectItem*, SHIP_MAX_HEALTH> shipBars;
    // Memoria de Soundwave dibujada sobre su órbita: un arco por sector, más grueso
    // y brillante cuanto mejor le ha ido atacando desde ahí. La forma de cada arco
    // se calcula una sola vez; después solo cambia el lápiz (grosor y color), y
    // solo cuando el agente decide o aprende, no cada frame.
    std::array<QGraphicsPathItem*, SoundwaveAgent::SECTORS> memoryArcs;
    // HUD con dirty check: ~100 redibujos por partida, no por paso
    QGraphicsRectItem *transmissionFill;
    QGraphicsSimpleTextItem *transmissionLabel;
    QGraphicsSimpleTextItem *difficultyLabel;

    // Panel padre: un hide() apaga el menú entero; Qt libera los hijos
    QGraphicsRectItem *menuPanel;

    QGraphicsSimpleTextItem *gameOverTitle;
    QGraphicsSimpleTextItem *gameOverHint;

    // Bucle del juego. Por valor: sin heap, se destruyen con la ventana
    QTimer timer;              // despierta al juego cada TICK_MS
    QElapsedTimer clock;       // mide el tiempo real entre ticks
    double accumulator = 0.0;  // tiempo real pendiente de simular (s)

    void updateTrajectory();
    void fireProjectile();
    void spawnDebris();
    void spawnDrone();
    void updateSpawners(double dt);
    void updateSoundwave(double dt);
    void checkCollisions();
    void damageSatellite(int amount = 1);
    void damageShip();
    void resetShip();
    void resetSatellite();
    void updateTransmission(double dt);
    void endGame(bool victory);
    void startGame(Difficulty chosen);
    void restartGame();
    void showMenu();
    void createHud();
    void createMenu();
    void updateHealthBars();
    void updateTransmissionHud();
    void syncSoundwaveItem();
    void createMemoryArcs();
    void updateMemoryArcs();

    static bool isInsideEarth(const Vector2D &position);
    static bool circlesOverlap(const Vector2D &a, double radiusA,
                               const Vector2D &b, double radiusB);
    static double randomRange(double min, double max);
    static QColor colorForHealth(const QColor &healthy, int health, int maxHealth);

private slots:
    void updateGame();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

public:
    explicit OrbitWindow(QWidget *parent = nullptr);

};

#endif // ORBITWINDOW_H

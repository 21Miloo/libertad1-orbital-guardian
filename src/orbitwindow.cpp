#include "orbitwindow.h"
#include "core/constants.h"

#include <QPainter>
#include <QPen>
#include <QtMath>
#include <cmath>

// Constructor
OrbitWindow::OrbitWindow(QWidget *parent): QMainWindow{parent}, physics(Vector2D(config:: WINDOW_WIDTH / 2.0, config::WINDOW_HEIGHT / 2.0),
              OrbitalPhysics::gmForOrbit(ORBIT_RADIUS, ORBIT_PERIOD)) {

    scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex); //Optimizamos procesamiento -> Sin Arbol BSP
    scene->setSceneRect(0, 0, config::WINDOW_WIDTH, config::WINDOW_HEIGHT); // Tamaño del mundo
    scene->setBackgroundBrush(Qt::black); // fondo de la escena negro

    // Centro de la tierra = es el centro de la escena
    const double centerX = config::WINDOW_WIDTH / 2.0;
    const double centerY = config::WINDOW_HEIGHT / 2.0;


    // Guia de la orbita (circulo punteado)
    QPen guidePen(QColor(255, 255, 255, 40)); // RGB. le damos un color blanco con transparencia
    guidePen.setStyle(Qt::DashLine); // Seteamos el estilo de la linea en punteado

    QGraphicsEllipseItem *guide = scene->addEllipse(
        -ORBIT_RADIUS, -ORBIT_RADIUS,            // esquina superior izquierda
        2 * ORBIT_RADIUS, 2 * ORBIT_RADIUS,      // ancho y alto = diámetro
        guidePen, Qt::NoBrush);                  // borde punteado, sin relleno
    guide->setPos(centerX, centerY);

    // --- Tierra ---
    earth = scene->addEllipse(-EARTH_RADIUS, -EARTH_RADIUS, 2 * EARTH_RADIUS, 2 * EARTH_RADIUS, Qt::NoPen, QColor(40, 110, 200));        // sin borde, relleno azul
    earth->setPos(centerX, centerY);

    // --- Satelite ---
    // Estado inicial: sobre la órbita, a la derecha, con velocidad circular hacia arriba
    satellitePosition = Vector2D(centerX + ORBIT_RADIUS, centerY);
    satelliteVelocity = Vector2D(0.0, -physics.circularSpeed(ORBIT_RADIUS));

    satellite = scene->addEllipse(-SATELLITE_RADIUS, -SATELLITE_RADIUS, 2 * SATELLITE_RADIUS, 2 * SATELLITE_RADIUS, Qt::NoPen, QColor(250, 200, 40));
    satellite->setPos(satellitePosition.x, satellitePosition.y);

    // --- Línea de predicción ---
    // Se crea antes que la nave y con Z menor para que quede dibujada debajo.
    QPen trajectoryPen(QColor(120, 200, 255, 150)); // azul claro semitransparente
    trajectoryPen.setWidthF(1.5);
    trajectoryPen.setStyle(Qt::DashLine);
    trajectory = scene->addPath(QPainterPath(), trajectoryPen, Qt::NoBrush);
    trajectory->setZValue(-1);
    trajectoryPath.reserve(PREDICTION_STEPS + 1); // reservar una sola vez: cero new por frame

    // --- Nave de Bumblebee ---
    // Triángulo centrado en el origen y apuntando a la derecha (ángulo 0).
    // Así setRotation lo gira sobre su centro.
    QPolygonF shipShape;
    shipShape << QPointF(12, 0) << QPointF(-8, -7) << QPointF(-8, 7);
    ship = scene->addPolygon(shipShape, Qt::NoPen, QColor(220, 220, 230));

    // Arranca a la izquierda de la Tierra. Para girar en el mismo sentido que el
    // satélite, a la izquierda la velocidad va hacia abajo (+y en pantalla).
    shipPosition = Vector2D(centerX - SHIP_ORBIT_RADIUS, centerY);
    shipVelocity = Vector2D(0.0, physics.circularSpeed(SHIP_ORBIT_RADIUS));

    ship->setPos(shipPosition.x, shipPosition.y);
    ship->setRotation(shipAngle); // apunta hacia abajo, en la dirección en que se mueve


    view = new QGraphicsView(scene, this);
    view->setFrameShape(QFrame::NoFrame);  // La vista sin borde
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // Quitamos las barras de desplazamiento de la vista.
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //

    //Antialiasing
    view->setRenderHint(QPainter:: Antialiasing); //Cuesta procesamiento, hace que los objetos no se vean con bordes de escalera. perdemos un poco de eficiencia por calidad Visual

    setCentralWidget(view); // pone la vista como contenido de la ventan
    setFixedSize(config:: WINDOW_WIDTH, config::WINDOW_HEIGHT); // El jugador no puede cambiar el tamaño de la ventana
    setWindowTitle(QStringLiteral("Libertad-1 · Nivel 2: Órbita")); // texto de la barra del titulo

    // --- Pool de proyectiles ---
    // Se crean todos de una vez, escondidos. Durante el juego solo se activan y desactivan.
    for (Projectile &p : projectiles) {
        p.item = scene->addEllipse(-PROJECTILE_RADIUS, -PROJECTILE_RADIUS,
                                   2 * PROJECTILE_RADIUS, 2 * PROJECTILE_RADIUS,
                                   Qt::NoPen, QColor(255, 140, 0)); // naranja
        p.item->hide(); // un ítem escondido no se dibuja
    }

    // Bucle del juego
    timer.setTimerType(Qt::PreciseTimer); // CoarseTimer puede atrasarse hasta un 5%
    connect(&timer, &QTimer::timeout, this, &OrbitWindow::updateGame);
    timer.start(config::TICK_MS);
    clock.start();

}


// Se llama en cada tick del timer.
// Paso fijo con acumulador: la física avanza al ritmo del tiempo real y siempre
// con pasos de DT, sin importar si el PC es lento o rápido.
void OrbitWindow::updateGame()
{
    // Tiempo real desde el último tick, en segundos (ns para no perder precisión)
    double elapsed = clock.nsecsElapsed() / 1e9;
    clock.restart();

    // Si el juego se congeló, no intentar recuperar todo de golpe
    if (elapsed > config::MAX_FRAME_TIME) {
        elapsed = config::MAX_FRAME_TIME;
    }

    accumulator += elapsed;
    while (accumulator >= config::DT) {
        physics.step(satellitePosition, satelliteVelocity, config::DT);

        // Rotación de la nave: en pantalla, ángulo positivo = sentido horario
        if (rotatingLeft)  shipAngle -= SHIP_ROTATION_SPEED * config::DT;
        if (rotatingRight) shipAngle += SHIP_ROTATION_SPEED * config::DT;

        // Empuje en la dirección a la que apunta la nave.
        // cos/sin solo se calculan si el motor está encendido.
        Vector2D thrust;
        if (thrusting) {
            const double radians = qDegreesToRadians(shipAngle);
            thrust = Vector2D(std::cos(radians), std::sin(radians)) * SHIP_THRUST;
        }

        physics.step(shipPosition, shipVelocity, config::DT, thrust);

        // Disparo continuo mientras se mantenga Espacio, con un tiempo mínimo entre tiros
        if (fireCooldown > 0.0) fireCooldown -= config::DT;
        if (firing && fireCooldown <= 0.0) {
            fireProjectile();
            fireCooldown = FIRE_COOLDOWN;
        }

        updateProjectiles(config::DT);
        accumulator -= config::DT;
    }

    // Dibujar una sola vez por tick, con la posición final
    satellite->setPos(satellitePosition.x, satellitePosition.y);
    ship->setPos(shipPosition.x, shipPosition.y);
    ship->setRotation(shipAngle);

    for (Projectile &p : projectiles) {
        if (p.active) p.item->setPos(p.position.x, p.position.y);
    }

    updateTrajectory();
}


// Activa el primer proyectil libre del pool. Si todos están en uso, no dispara.
void OrbitWindow::fireProjectile()
{
    for (Projectile &p : projectiles) {
        if (p.active) continue;

        const double radians = qDegreesToRadians(shipAngle);
        const Vector2D direction(std::cos(radians), std::sin(radians));

        // Sale de la punta de la nave (12 px adelante del centro) y hereda
        // la velocidad de la nave: física real, como disparar desde algo en movimiento
        p.position = shipPosition + direction * 12.0;
        p.velocity = shipVelocity + direction * PROJECTILE_SPEED;
        p.lifetime = PROJECTILE_LIFETIME;
        p.active = true;

        p.item->setPos(p.position.x, p.position.y);
        p.item->show();
        return;
    }
}


// Mueve los proyectiles activos con la misma gravedad y desactiva los que
// chocan con la Tierra, salen de la pantalla o se les acaba el tiempo.
void OrbitWindow::updateProjectiles(double dt)
{
    const double margin = 50.0; // px fuera de la pantalla antes de desactivarlo

    for (Projectile &p : projectiles) {
        if (!p.active) continue;

        physics.step(p.position, p.velocity, dt);
        p.lifetime -= dt;

        const bool outOfScreen = p.position.x < -margin || p.position.x > config::WINDOW_WIDTH + margin
                              || p.position.y < -margin || p.position.y > config::WINDOW_HEIGHT + margin;

        if (p.lifetime <= 0.0 || outOfScreen || isInsideEarth(p.position)) {
            p.active = false;
            p.item->hide();
        }
    }
}


// Compara la distancia al cuadrado: mismo resultado sin calcular la raíz cuadrada
bool OrbitWindow::isInsideEarth(const Vector2D &position)
{
    const double dx = position.x - EARTH_X;
    const double dy = position.y - EARTH_Y;
    return dx * dx + dy * dy < EARTH_RADIUS * EARTH_RADIUS;
}


// Simula hacia adelante una copia de la nave (sin motor) y dibuja por dónde pasará.
// Usa la misma física que el juego, así la línea nunca miente.
// Costo: 120 pasos por frame (~7 200 por segundo), despreciable incluso en un PC viejo.
void OrbitWindow::updateTrajectory()
{
    Vector2D position = shipPosition;   // copias: la nave real no se toca
    Vector2D velocity = shipVelocity;

    trajectoryPath.clear();
    trajectoryPath.moveTo(position.x, position.y);

    for (int i = 0; i < PREDICTION_STEPS; ++i) {
        physics.step(position, velocity, PREDICTION_DT);

        if (isInsideEarth(position)) break; // la línea se corta en el impacto

        trajectoryPath.lineTo(position.x, position.y);
    }

    trajectory->setPath(trajectoryPath);
}


// Teclado: solo se cambian los bool. El movimiento lo hace updateGame,
// así la nave responde igual sin importar la repetición automática del teclado.
void OrbitWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) return;

    switch (event->key()) {
    case Qt::Key_A: rotatingLeft = true;  break;
    case Qt::Key_D: rotatingRight = true; break;
    case Qt::Key_W: thrusting = true;     break;
    case Qt::Key_Space: firing = true;    break;
    default: QMainWindow::keyPressEvent(event); // otras teclas: comportamiento normal
    }
}

void OrbitWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) return;

    switch (event->key()) {
    case Qt::Key_A: rotatingLeft = false;  break;
    case Qt::Key_D: rotatingRight = false; break;
    case Qt::Key_W: thrusting = false;     break;
    case Qt::Key_Space: firing = false;    break;
    default: QMainWindow::keyReleaseEvent(event);
    }
}





/////////// Notas ////////////////
//QStringLiteral arma el QString mientras compilas: el texto ya queda guardado dentro del ejecutable en el formato interno de Qt. Al correr el programa no hay decodificación,
// ni new, ni copia, mientras que si usamos un string, QT lo convierte en QString y eso hace que Se lean los bytes, Se reserve memoria en el heap para el SString nuevo y
//Copia los caracteres
#include "orbitwindow.h"
#include "core/constants.h"

#include <QPainter>
#include <QPen>
#include <QtMath>
#include <QRandomGenerator>
#include <QFont>
#include <algorithm>
#include <cmath>

// Colores que se usan en varias funciones. En un namespace sin nombre:
// solo existen dentro de este .cpp y se construyen una sola vez al arrancar.
namespace {
const QColor SATELLITE_COLOR(250, 200, 40);  // amarillo
const QColor SHIP_COLOR(220, 220, 230);      // gris claro
const QColor HUD_TEXT_COLOR(200, 200, 210);
const QColor BAR_EMPTY_COLOR(60, 60, 70);    // barrita de vida perdida
}

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
    satellite = scene->addEllipse(-SATELLITE_RADIUS, -SATELLITE_RADIUS, 2 * SATELLITE_RADIUS, 2 * SATELLITE_RADIUS, Qt::NoPen, SATELLITE_COLOR);
    resetSatellite(); // estado inicial (la misma función se usa al reintentar)

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
    ship = scene->addPolygon(shipShape, Qt::NoPen, SHIP_COLOR);
    resetShip(); // posición y velocidad iniciales (la misma función se usa al reaparecer)


    view = new QGraphicsView(scene, this);
    view->setFrameShape(QFrame::NoFrame);  // La vista sin borde
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // Quitamos las barras de desplazamiento de la vista.
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //

    //Antialiasing
    view->setRenderHint(QPainter:: Antialiasing); //Cuesta procesamiento, hace que los objetos no se vean con bordes de escalera. perdemos un poco de eficiencia por calidad Visual

    setCentralWidget(view); // pone la vista como contenido de la ventan
    setFixedSize(config:: WINDOW_WIDTH, config::WINDOW_HEIGHT); // El jugador no puede cambiar el tamaño de la ventana
    setWindowTitle(QStringLiteral("Libertad-1 · Nivel 2: Órbita")); // texto de la barra del titulo

    // --- Pools de objetos ---
    // Cada objeto se crea con new (make_unique) una sola vez, aquí, escondido.
    // Durante el juego solo se activan y desactivan: cero new/delete por frame.
    spaceObjects.reserve(MAX_PROJECTILES + MAX_DEBRIS + MAX_DRONES);  // una sola reserva

    for (std::unique_ptr<Projectile> &projectile : projectiles) {
        projectile = std::make_unique<Projectile>(*scene);
        spaceObjects.push_back(projectile.get());
    }
    for (std::unique_ptr<Debris> &rock : debris) {
        rock = std::make_unique<Debris>(*scene);
        spaceObjects.push_back(rock.get());
    }
    for (std::unique_ptr<Drone> &drone : drones) {
        // El dron guarda una referencia a la posición del satélite: siempre ve
        // la posición actual sin que la ventana tenga que pasársela cada paso
        drone = std::make_unique<Drone>(*scene, satellitePosition);
        spaceObjects.push_back(drone.get());
    }

    createHud();

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

        updateSpawners(config::DT);

        // Un solo ciclo para todos: cada objeto se mueve a su manera
        // (proyectil, roca o dron) según la clase real a la que apunta
        for (SpaceObject *object : spaceObjects) {
            if (object->isActive()) object->update(config::DT, physics);
        }

        checkCollisions();
        accumulator -= config::DT;

        if (!timer.isActive()) break; // el juego terminó en este paso
    }

    // Dibujar una sola vez por tick, con la posición final
    satellite->setPos(satellitePosition.x, satellitePosition.y);
    ship->setPos(shipPosition.x, shipPosition.y);
    ship->setRotation(shipAngle);

    for (SpaceObject *object : spaceObjects) {
        if (object->isActive()) object->syncGraphics();
    }

    updateTrajectory();
}


// Activa el primer proyectil libre del pool. Si todos están en uso, no dispara.
void OrbitWindow::fireProjectile()
{
    for (std::unique_ptr<Projectile> &projectile : projectiles) {
        if (projectile->isActive()) continue;

        const double radians = qDegreesToRadians(shipAngle);
        const Vector2D direction(std::cos(radians), std::sin(radians));

        // Sale de la punta de la nave (12 px adelante del centro) y hereda
        // la velocidad de la nave: física real, como disparar desde algo en movimiento
        projectile->launch(shipPosition + direction * 12.0,
                           shipVelocity + direction * Projectile::SPEED);
        return;
    }
}


// Activa un pedazo de basura libre en una órbita al azar.
// Gira en sentido contrario al satélite para que sus caminos se crucen.
void OrbitWindow::spawnDebris()
{
    for (std::unique_ptr<Debris> &rock : debris) {
        if (rock->isActive()) continue;

        const double radius = randomRange(DEBRIS_MIN_ORBIT, DEBRIS_MAX_ORBIT);
        double angle = randomRange(0.0, 2.0 * M_PI);
        Vector2D direction(std::cos(angle), std::sin(angle));
        Vector2D position = Vector2D(EARTH_X, EARTH_Y) + direction * radius;

        // Si quedó muy cerca del satélite, se pasa al lado opuesto de la Tierra
        if (circlesOverlap(position, SAFE_SPAWN_DISTANCE, satellitePosition, 0.0)) {
            angle += M_PI;
            direction = Vector2D(std::cos(angle), std::sin(angle));
            position = Vector2D(EARTH_X, EARTH_Y) + direction * radius;
        }

        // Velocidad tangente: (-sin, cos) es el sentido contrario al satélite.
        // Entre 90% y 105% de la velocidad circular: órbita un poco ovalada,
        // así no todas se ven iguales.
        const double speed = physics.circularSpeed(radius) * randomRange(0.90, 1.05);
        rock->launch(position, Vector2D(-direction.y, direction.x) * speed, randomRange(0.0, 360.0));
        return;
    }
    // Pool lleno: no aparece nada hasta que se libere un espacio
}


// Activa un dron libre fuera de la pantalla, en una dirección al azar.
// Las esquinas de la ventana están a ~734 px del centro, así que un círculo
// de 760 px queda completamente por fuera en cualquier dirección.
void OrbitWindow::spawnDrone()
{
    for (std::unique_ptr<Drone> &drone : drones) {
        if (drone->isActive()) continue;

        const double angle = randomRange(0.0, 2.0 * M_PI);
        drone->launch(Vector2D(EARTH_X, EARTH_Y)
                      + Vector2D(std::cos(angle), std::sin(angle)) * DRONE_SPAWN_DISTANCE);
        return;
    }
}


// Cuenta el tiempo para la siguiente basura y el siguiente dron
void OrbitWindow::updateSpawners(double dt)
{
    debrisTimer -= dt;
    if (debrisTimer <= 0.0) {
        spawnDebris();
        debrisTimer = DEBRIS_SPAWN_INTERVAL;
    }

    droneTimer -= dt;
    if (droneTimer <= 0.0) {
        spawnDrone();
        droneTimer = DRONE_SPAWN_INTERVAL;
    }
}


// Revisa los choques del paso actual. Solo se comparan objetos activos.
// Peor caso: (12 basuras + 6 drones) x 32 disparos = 576 comparaciones, cada una
// son 2 restas, 3 multiplicaciones y una suma. Sin raíces cuadradas.
void OrbitWindow::checkCollisions()
{
    const Vector2D earthCenter(EARTH_X, EARTH_Y);

    // Cualquier objeto que toque la Tierra se quema en la atmósfera
    for (SpaceObject *object : spaceObjects) {
        if (object->isActive() && object->overlaps(earthCenter, EARTH_RADIUS)) {
            object->deactivate();
        }
    }

    // Basura: chocar con cualquier cosa la destruye
    for (std::unique_ptr<Debris> &rock : debris) {
        if (!rock->isActive()) continue;

        if (rock->overlaps(satellitePosition, SATELLITE_RADIUS)) {
            rock->deactivate();
            damageSatellite();
        } else if (rock->overlaps(shipPosition, SHIP_RADIUS)) {
            rock->deactivate();
            damageShip();
        } else {
            for (std::unique_ptr<Projectile> &projectile : projectiles) {
                if (projectile->isActive() && rock->overlaps(*projectile)) {
                    projectile->deactivate();
                    rock->deactivate();
                    break;  // un disparo basta, no seguir revisando los demás
                }
            }
        }
    }

    // Drones: chocar los destruye; un disparo solo les quita vida
    for (std::unique_ptr<Drone> &drone : drones) {
        if (!drone->isActive()) continue;

        if (drone->overlaps(satellitePosition, SATELLITE_RADIUS)) {
            drone->deactivate();
            damageSatellite(Drone::DAMAGE);
        } else if (drone->overlaps(shipPosition, SHIP_RADIUS)) {
            drone->deactivate();
            damageShip();
        } else {
            for (std::unique_ptr<Projectile> &projectile : projectiles) {
                if (projectile->isActive() && drone->overlaps(*projectile)) {
                    projectile->deactivate();
                    drone->takeHit();
                    break;
                }
            }
        }
    }

    // La nave se estrella contra la Tierra: pierde vida y vuelve a su órbita inicial
    if (circlesOverlap(shipPosition, SHIP_RADIUS, earthCenter, EARTH_RADIUS)) {
        damageShip();
        resetShip();
    }
}


// El color del satélite pasa de amarillo a rojo según la vida que le queda.
// Solo se cambia cuando recibe daño, no en cada frame.
void OrbitWindow::damageSatellite(int amount)
{
    if (satelliteHealth <= 0) return;

    satelliteHealth = std::max(0, satelliteHealth - amount); // nunca por debajo de 0
    satellite->setBrush(colorForHealth(SATELLITE_COLOR, satelliteHealth, SATELLITE_MAX_HEALTH));
    updateHealthBars();

    if (satelliteHealth == 0) endGame();
}


void OrbitWindow::damageShip()
{
    if (shipHealth <= 0) return;

    --shipHealth;
    ship->setBrush(colorForHealth(SHIP_COLOR, shipHealth, SHIP_MAX_HEALTH));
    updateHealthBars();

    if (shipHealth == 0) endGame();
}


// Pone la nave en su órbita inicial: a la izquierda de la Tierra. Para girar en el
// mismo sentido que el satélite, a la izquierda la velocidad va hacia abajo (+y).
void OrbitWindow::resetShip()
{
    shipPosition = Vector2D(EARTH_X - SHIP_ORBIT_RADIUS, EARTH_Y);
    shipVelocity = Vector2D(0.0, physics.circularSpeed(SHIP_ORBIT_RADIUS));
    shipAngle = 90.0; // apunta hacia abajo, en la dirección en que se mueve

    ship->setPos(shipPosition.x, shipPosition.y);
    ship->setRotation(shipAngle);
}


// Satélite sobre su órbita, a la derecha, con velocidad circular hacia arriba
void OrbitWindow::resetSatellite()
{
    satellitePosition = Vector2D(EARTH_X + ORBIT_RADIUS, EARTH_Y);
    satelliteVelocity = Vector2D(0.0, -physics.circularSpeed(ORBIT_RADIUS));
    satellite->setPos(satellitePosition.x, satellitePosition.y);
}


// Detiene el juego y muestra el mensaje de derrota (ya estaba creado, solo se muestra)
void OrbitWindow::endGame()
{
    timer.stop();
    gameOverTitle->show();
    gameOverHint->show();
}


// Vuelve todo al estado inicial sin crear nada nuevo: los pools se vacían
// escondiendo sus ítems, igual que durante el juego.
void OrbitWindow::restartGame()
{
    satelliteHealth = SATELLITE_MAX_HEALTH;
    shipHealth = SHIP_MAX_HEALTH;
    satellite->setBrush(SATELLITE_COLOR);
    ship->setBrush(SHIP_COLOR);
    updateHealthBars();

    resetSatellite();
    resetShip();

    for (SpaceObject *object : spaceObjects) {
        object->deactivate();
    }

    fireCooldown = 0.0;
    debrisTimer = DEBRIS_SPAWN_INTERVAL;
    droneTimer = DRONE_SPAWN_INTERVAL;
    accumulator = 0.0;

    gameOverTitle->hide();
    gameOverHint->hide();

    clock.restart();  // si no, el primer tick mediría todo el tiempo que estuvo detenido
    timer.start(config::TICK_MS);
}


// Crea los elementos del HUD una sola vez. Z alto para que queden encima de todo.
// QGraphicsSimpleTextItem en vez de QGraphicsTextItem: este último arma un
// documento de texto enriquecido completo (QTextDocument), que aquí no se necesita.
void OrbitWindow::createHud()
{
    const double hudZ = 10.0;
    const double barsX = 110.0;   // donde empiezan las barritas
    const double barWidth = 18.0;
    const double barHeight = 8.0;
    const double barGap = 4.0;

    QFont labelFont;
    labelFont.setPointSize(10);
    labelFont.setBold(true);

    // Etiquetas fijas: se escriben una vez y no vuelven a cambiar
    QGraphicsSimpleTextItem *satelliteLabel = scene->addSimpleText(QStringLiteral("SATÉLITE"), labelFont);
    satelliteLabel->setBrush(HUD_TEXT_COLOR);
    satelliteLabel->setPos(20, 16);
    satelliteLabel->setZValue(hudZ);

    QGraphicsSimpleTextItem *shipLabel = scene->addSimpleText(QStringLiteral("NAVE"), labelFont);
    shipLabel->setBrush(HUD_TEXT_COLOR);
    shipLabel->setPos(20, 38);
    shipLabel->setZValue(hudZ);

    // Barritas de vida, alineadas con el centro de cada etiqueta
    for (std::size_t i = 0; i < satelliteBars.size(); ++i) {
        satelliteBars[i] = scene->addRect(0, 0, barWidth, barHeight, Qt::NoPen, SATELLITE_COLOR);
        satelliteBars[i]->setPos(barsX + i * (barWidth + barGap), 21);
        satelliteBars[i]->setZValue(hudZ);
    }
    for (std::size_t i = 0; i < shipBars.size(); ++i) {
        shipBars[i] = scene->addRect(0, 0, barWidth, barHeight, Qt::NoPen, SHIP_COLOR);
        shipBars[i]->setPos(barsX + i * (barWidth + barGap), 43);
        shipBars[i]->setZValue(hudZ);
    }

    // Mensaje de derrota: se crea escondido y centrado en la ventana
    QFont titleFont;
    titleFont.setPointSize(32);
    titleFont.setBold(true);

    gameOverTitle = scene->addSimpleText(QStringLiteral("MISIÓN FALLIDA"), titleFont);
    gameOverTitle->setBrush(QColor(220, 40, 40));
    gameOverTitle->setPos((config::WINDOW_WIDTH - gameOverTitle->boundingRect().width()) / 2.0,
                          config::WINDOW_HEIGHT / 2.0 - 140);
    gameOverTitle->setZValue(hudZ);
    gameOverTitle->hide();

    QFont hintFont;
    hintFont.setPointSize(12);

    gameOverHint = scene->addSimpleText(QStringLiteral("Presiona R para reintentar"), hintFont);
    gameOverHint->setBrush(HUD_TEXT_COLOR);
    gameOverHint->setPos((config::WINDOW_WIDTH - gameOverHint->boundingRect().width()) / 2.0,
                         config::WINDOW_HEIGHT / 2.0 - 85);
    gameOverHint->setZValue(hudZ);
    gameOverHint->hide();
}


// Las barritas que quedan se pintan llenas y las perdidas en gris oscuro.
// Se llama solo cuando cambia la vida.
void OrbitWindow::updateHealthBars()
{
    for (std::size_t i = 0; i < satelliteBars.size(); ++i) {
        const bool full = static_cast<int>(i) < satelliteHealth;
        satelliteBars[i]->setBrush(full ? SATELLITE_COLOR : BAR_EMPTY_COLOR);
    }
    for (std::size_t i = 0; i < shipBars.size(); ++i) {
        const bool full = static_cast<int>(i) < shipHealth;
        shipBars[i]->setBrush(full ? SHIP_COLOR : BAR_EMPTY_COLOR);
    }
}


// Compara la distancia al cuadrado: mismo resultado sin calcular la raíz cuadrada
bool OrbitWindow::isInsideEarth(const Vector2D &position)
{
    const double dx = position.x - EARTH_X;
    const double dy = position.y - EARTH_Y;
    return dx * dx + dy * dy < EARTH_RADIUS * EARTH_RADIUS;
}


// Dos círculos se tocan si la distancia entre centros es menor que la suma de radios.
// Se compara todo al cuadrado para no calcular la raíz.
bool OrbitWindow::circlesOverlap(const Vector2D &a, double radiusA,
                                 const Vector2D &b, double radiusB)
{
    const double dx = a.x - b.x;
    const double dy = a.y - b.y;
    const double radii = radiusA + radiusB;
    return dx * dx + dy * dy < radii * radii;
}


// Número al azar en [min, max). QRandomGenerator es el generador de Qt:
// mejor distribución que rand() de C y sin tener que sembrarlo a mano.
double OrbitWindow::randomRange(double min, double max)
{
    return min + (max - min) * QRandomGenerator::global()->generateDouble();
}


// Mezcla el color sano con rojo: vida completa = color original, vida 0 = rojo.
QColor OrbitWindow::colorForHealth(const QColor &healthy, int health, int maxHealth)
{
    const QColor damaged(220, 40, 40);
    const double t = 1.0 - static_cast<double>(health) / maxHealth; // 0 = sano, 1 = destruido

    return QColor(qRound(healthy.red()   + (damaged.red()   - healthy.red())   * t),
                  qRound(healthy.green() + (damaged.green() - healthy.green()) * t),
                  qRound(healthy.blue()  + (damaged.blue()  - healthy.blue())  * t));
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
    case Qt::Key_R:
        if (!timer.isActive()) restartGame(); // solo funciona con el juego detenido
        break;
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
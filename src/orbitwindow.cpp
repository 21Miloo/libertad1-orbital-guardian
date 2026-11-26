#include "orbitwindow.h"
#include "core/constants.h"

#include <QPainter>
#include <QPen>

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


    view = new QGraphicsView(scene, this);
    view->setFrameShape(QFrame::NoFrame);  // La vista sin borde
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // Quitamos las barras de desplazamiento de la vista.
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //

    //Antialiasing
    view->setRenderHint(QPainter:: Antialiasing); //Cuesta procesamiento, hace que los objetos no se vean con bordes de escalera. perdemos un poco de eficiencia por calidad Visual

    setCentralWidget(view); // pone la vista como contenido de la ventan
    setFixedSize(config:: WINDOW_WIDTH, config::WINDOW_HEIGHT); // El jugador no puede cambiar el tamaño de la ventana
    setWindowTitle(QStringLiteral("Libertad-1 · Nivel 2: Órbita")); // texto de la barra del titulo

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
        accumulator -= config::DT;
    }

    // Dibujar una sola vez por tick, con la posición final
    satellite->setPos(satellitePosition.x, satellitePosition.y);
}





/////////// Notas ////////////////
//QStringLiteral arma el QString mientras compilas: el texto ya queda guardado dentro del ejecutable en el formato interno de Qt. Al correr el programa no hay decodificación,
// ni new, ni copia, mientras que si usamos un string, QT lo convierte en QString y eso hace que Se lean los bytes, Se reserve memoria en el heap para el SString nuevo y
//Copia los caracteres
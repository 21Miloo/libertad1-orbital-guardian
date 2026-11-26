#include <QApplication>
#include <QDebug>
#include <stdexcept>

#include "core/orbitalphysics.h"
#include "core/constants.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Aquí se va a crear y mostrar la ventana del juego

    // ===== Prueba de OrbitalPhysics =====
    const double radius = 300.0;
    const double period = 20.0;

    // TÚ: calcula gm usando el método static gmForOrbit
    double gm = OrbitalPhysics::gmForOrbit(radius, period);

    // TÚ: crea el objeto OrbitalPhysics con centro (640, 360) y ese gm
    OrbitalPhysics physics(Vector2D(640, 360), gm);

    // TÚ: posición inicial (centro + 300 en x) y velocidad inicial (0, -circularSpeed)
    Vector2D position(640 + 300, 360);
    Vector2D velocity(0, - physics.circularSpeed(radius));

    const int totalSteps = static_cast<int>(period / config::DT);   // 2400

    for (int i = 0; i <= totalSteps; ++i) {
        // Cada 600 pasos (= 5 segundos) imprime el estado
        if (i % 600 == 0) {
            Vector2D fromCenter = position - Vector2D(640, 360);
            qDebug() << "t =" << i * config::DT << "s"
                     << " pos = (" << position.x << "," << position.y << ")"
                     << " distancia =" << fromCenter.length();
        }

        // TÚ: avanza la física un paso con physics.step(...)
        physics.step(position, velocity, config::DT);

    }

    return app.exec();
}

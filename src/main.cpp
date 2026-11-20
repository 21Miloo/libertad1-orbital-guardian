#include <QApplication>
#include <QDebug>
#include <stdexcept>


#include "core/inventory.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Aquí se va a crear y mostrar la ventana del juego

    // ===== Prueba de Inventory (temporal) =====
    Inventory inventory;

    qDebug() << "Al inicio:" << inventory.collectedCount() << "piezas";

    // 1. TÚ: recoge 2 piezas distintas
    //    (ej: Structure y PowerSystem)

    inventory.collectPart(SatellitePart:: Structure);
    inventory.collectPart(SatellitePart:: PowerSystem);


    // 2. TÚ: imprime collectedCount() e isComplete()
    //    (sigue el ejemplo del qDebug de arriba)
    qDebug() << "Cantidad de " << inventory.collectedCount() << " piezas";
    qDebug() << "¿Estan completas? : " << inventory.isComplete();



    // 3. TÚ: recoge OTRA VEZ una de las que ya tenías
    //    e imprime collectedCount(). ¿Debería cambiar?
    inventory.collectPart(SatellitePart:: Structure);
    qDebug() << "Cantidad de " << inventory.collectedCount() << " piezas";

    // 4. TÚ: imprime isCollected() de una pieza que SÍ recogiste
    //    y de una que NO.
    bool result = inventory.isCollected(SatellitePart::PowerSystem);
    qDebug() << result;
    result = inventory.isCollected(SatellitePart::PropulsionSystem);
    qDebug() << result;



    // 6. TÚ: llama reset() e imprime collectedCount()
    inventory.reset();
    inventory.collectedCount();

    // ==========================================





    return app.exec();
}

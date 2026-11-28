#include <QApplication>
#include <QMessageBox>
#include <exception>

#include "core/inventory.h"
#include "orbitwindow.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Red de seguridad: un invariante roto al construir el juego (invalid_argument,
    // out_of_range) se reporta en vez de cerrar sin explicación.
    // Ojo: Qt no garantiza propagar excepciones a través del event loop;
    // dentro de los slots los datos ya vienen validados (tabla constexpr).
    try {
        // Inventario de prueba hasta integrar el nivel 1, que es quien lo llena
        Inventory inventory;
        inventory.unlockUpgrade(PlayerUpgrade::Blaster);
        inventory.unlockUpgrade(PlayerUpgrade::Shield);
        inventory.unlockUpgrade(PlayerUpgrade::Booster);

        OrbitWindow window(inventory);
        window.show();
        return app.exec();
    } catch (const std::exception &error) {
        QMessageBox::critical(nullptr, QStringLiteral("Libertad-1"),
                              QStringLiteral("Error inesperado:\n") + QString::fromUtf8(error.what()));
        return 1;
    }
}

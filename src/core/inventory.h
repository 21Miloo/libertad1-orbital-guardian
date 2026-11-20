#ifndef INVENTORY_H
#define INVENTORY_H

#include <array>

// Usamos un enum con las  piezas en vez de variable para cada una porque nos permite usar el mismo metodo para cada pieza y no tener que crear los mismos metodos para cada piece.
enum class SatellitePart {

    Structure,           // 0
    PowerSystem,         // 1
    CommunicationSystem, // 2
    GuidanceSystem,      // 3
    PropulsionSystem,    // 4
    OnboardComputer,     // 5
    ThermalSystem,       // 6
    Total                // 7

};

class Inventory {

private:

    // true: la pieza ya fue recogida.
    // false: la pieza todavía no ha sido recogida.
    std::array<bool, static_cast<std::size_t>(SatellitePart::Total)> collectedParts{}; // El {} inicializa todos los elementos en false


public:

    Inventory(); // Constructor


    //Metodos para verificar piezas

    void collectPart(SatellitePart part); // el nivel 1, cuando Bumblebee toca una pieza / Lanza std::out_of_range si part == SatellitePart::Total

    bool isCollected(SatellitePart part) const; // le ponemos el const para asegurarnos que el consultar no modificara su contenido.

    std::size_t collectedCount() const; // el HUD ("4/7")

    bool isComplete() const; //  el nivel 1, para saber cuándo pasar al nivel 2

    void reset();



};

#endif // INVENTORY_H

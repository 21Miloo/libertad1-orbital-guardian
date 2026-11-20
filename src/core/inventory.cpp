#include "inventory.h"
#include <algorithm>


// Constructor
Inventory::Inventory() = default;


// recoger pieza
void Inventory::collectPart(SatellitePart part){

// Se agrega la parte a el array collectedParts{}

    std::size_t index = static_cast<std::size_t>(part); // convertimos la parte tomada en el indice del enum
    collectedParts.at(index) = true; // con el indice del enum, ya sabemos que pieza es entonces lo hacemos true en el array collectedParts

}

// se tiene la pieza? buscamos en el array collectedParts por el indice y retornamos el valor el cual puede ser false o true.
bool Inventory::isCollected(SatellitePart part) const {

    std::size_t index = static_cast<std::size_t>(part);
    bool value = collectedParts.at(index);

    return value;
}

// nos da la cuenta de cuantos objetos hay cogidos.
std::size_t Inventory::collectedCount() const {

    return static_cast<std:: size_t>(std:: count(collectedParts.begin(), collectedParts.end(), true));

}

bool Inventory::isComplete() const{

  const bool hasAllParts = collectedCount() == collectedParts.size();

    return hasAllParts;
}


void Inventory:: reset(){

    collectedParts.fill(false);
    unlockedUpgrades.fill(false);

}


// Metodos de mejora del jugador

// Agrega la mejora a el arreglo inlockedUpgrades
void Inventory::unlockUpgrade(PlayerUpgrade upgrade){

    std::size_t index = static_cast<std::size_t>(upgrade);
    unlockedUpgrades.at(index) = true;


}

// Verifica si tiene la mejora
bool Inventory::hasUpgrade(PlayerUpgrade upgrade) const {

    std::size_t index = static_cast<std::size_t>(upgrade);
    bool value = unlockedUpgrades.at(index);

    return value;

}









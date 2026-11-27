#ifndef SOUNDWAVEAGENT_H
#define SOUNDWAVEAGENT_H

#include <array>

#include "core/vector2d.h"

// Agente inteligente de Soundwave. Patrulla una órbita alrededor de la Tierra y
// decide desde dónde lanzar sus drones. Sus cuatro tareas están separadas:
//
//   1. Percepción   perceive()  observa dónde están la nave y el satélite.
//   2. Razonamiento decide()    elige el sector de ataque más prometedor.
//   3. Acción       act()       se mueve hacia ese sector y lanza el dron.
//   4. Aprendizaje  learn()     recuerda qué sectores funcionaron y cuáles no.
//
// Es lógica pura: no sabe nada de la escena ni del dibujo. La ventana lo dibuja
// leyendo getPosition() y ejecuta el lanzamiento cuando act() lo indica.
class SoundwaveAgent {

public:
    static constexpr int SECTORS = 8;                 // la órbita dividida en 8 porciones de 45°
    static constexpr double ORBIT_RADIUS = 340.0;     // px, por fuera de la órbita del satélite
    static constexpr double MOVE_SPEED = 0.6;         // rad/s a lo largo de su órbita
    static constexpr double LAUNCH_INTERVAL = 8.0;    // s entre drones
    static constexpr double MIN_WEIGHT = 0.2;         // peso de un sector que siempre falla
    static constexpr double MAX_WEIGHT = 2.0;         // peso de un sector que siempre acierta

    // Lo que el agente sabe del mundo en el momento de decidir
    struct Perception {
        double shipAngle = 0.0;       // rad, ángulo de la nave alrededor de la Tierra
        double satelliteAngle = 0.0;  // rad, ángulo del satélite
    };

private:
    Vector2D center;              // centro de la Tierra
    double learningRate = 0.0;    // (0, 1]: peso de cada experiencia nueva

    Perception perception;
    std::array<double, SECTORS> weights;  // memoria: qué tan bueno ha sido cada sector

    double angle = 0.0;           // rad, posición actual en su órbita
    int targetSector = 0;         // sector elegido en la última decisión
    double launchCooldown = LAUNCH_INTERVAL;
    bool decisionPending = true;  // true = debe percibir y decidir antes de moverse

    static double wrapAngle(double radians);   // lleva un ángulo a [-pi, pi]
    static double sectorCenter(int sector);    // ángulo del centro de un sector
    double angleAround(const Vector2D &point) const;

public:
    SoundwaveAgent(const Vector2D &earthCenter, double learningRateValue);

    // 1. Percepción
    void perceive(const Vector2D &shipPosition, const Vector2D &satellitePosition);

    // 2. Razonamiento
    void decide();

    // 3. Acción: avanza dt segundos. Devuelve true cuando lanza un dron.
    bool act(double dt);

    // 4. Aprendizaje: resultado de un dron lanzado desde 'sector'
    void learn(int sector, bool success);

    // Vuelve a la posición inicial. La memoria se conserva a propósito:
    // al reintentar, Soundwave recuerda cómo jugaste antes.
    void resetPosition();

    // Prior uniforme: olvida todo lo aprendido
    void resetMemory();

    // Lanza invalid_argument fuera de (0, 1]
    void setLearningRate(double rate);

    bool needsDecision() const { return decisionPending; }
    int getTargetSector() const { return targetSector; }
    double getAngle() const { return angle; }
    Vector2D getPosition() const;
    const std::array<double, SECTORS> &getWeights() const { return weights; }
};

#endif // SOUNDWAVEAGENT_H

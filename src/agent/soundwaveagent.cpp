#include "soundwaveagent.h"

#include <QRandomGenerator>
#include <cmath>
#include <stdexcept>

SoundwaveAgent::SoundwaveAgent(const Vector2D &earthCenter, double learningRateValue)
    : center(earthCenter)
{
    setLearningRate(learningRateValue);
    resetMemory();
}


void SoundwaveAgent::setLearningRate(double rate)
{
    // 0 no aprende; > 1 sobrepasa el objetivo y oscila
    if (rate <= 0.0 || rate > 1.0) {
        throw std::invalid_argument("SoundwaveAgent: la tasa de aprendizaje debe estar en (0, 1]");
    }
    learningRate = rate;
}


void SoundwaveAgent::resetMemory()
{
    weights.fill(1.0);
}


// ---------- 1. Percepción ----------
// Solo se llama justo antes de decidir (una vez cada 8 s), no en cada paso:
// percibir cuesta dos atan2 y no hace falta hacerlo más seguido.
void SoundwaveAgent::perceive(const Vector2D &shipPosition, const Vector2D &satellitePosition)
{
    perception.shipAngle = angleAround(shipPosition);
    perception.satelliteAngle = angleAround(satellitePosition);
}


// ---------- 2. Razonamiento ----------
// A cada sector le da un puntaje que combina tres ideas:
//   - lo aprendido: sectores que funcionaron antes valen más (weights)
//   - lejos de la nave: al jugador le cuesta más llegar a defender
//   - cerca del satélite: el dron llega antes y hay menos tiempo para reaccionar
// Luego elige al azar, pero con probabilidad proporcional al puntaje (ruleta).
// Así casi siempre ataca por donde más le conviene, pero no es 100% predecible.
void SoundwaveAgent::decide()
{
    std::array<double, SECTORS> scores;
    double total = 0.0;

    for (int s = 0; s < SECTORS; ++s) {
        const double sectorAngle = sectorCenter(s);
        const double fromShip = std::abs(wrapAngle(sectorAngle - perception.shipAngle)) / M_PI;  // 0 = encima, 1 = opuesto
        const double fromSatellite = std::abs(wrapAngle(sectorAngle - perception.satelliteAngle)) / M_PI;

        const double shipFactor = 0.3 + 0.7 * fromShip;
        const double satelliteFactor = 0.5 + 0.5 * (1.0 - fromSatellite);

        scores[s] = weights[s] * shipFactor * satelliteFactor;
        total += scores[s];
    }

    double pick = QRandomGenerator::global()->generateDouble() * total;
    targetSector = SECTORS - 1;  // por si el redondeo deja pick un poco por encima del total
    for (int s = 0; s < SECTORS; ++s) {
        pick -= scores[s];
        if (pick <= 0.0) {
            targetSector = s;
            break;
        }
    }

    decisionPending = false;
}


// ---------- 3. Acción ----------
// Se mueve por su órbita hacia el sector elegido por el camino más corto.
// Cuando llegó y ya pasó el tiempo de recarga, lanza un dron.
bool SoundwaveAgent::act(double dt)
{
    const double difference = wrapAngle(sectorCenter(targetSector) - angle);
    const double step = MOVE_SPEED * dt;
    const bool arrived = std::abs(difference) <= step;

    if (arrived) {
        angle = sectorCenter(targetSector);
    } else {
        angle = wrapAngle(angle + (difference > 0.0 ? step : -step));
    }

    launchCooldown -= dt;
    if (arrived && launchCooldown <= 0.0) {
        launchCooldown = LAUNCH_INTERVAL;
        decisionPending = true;  // después de lanzar vuelve a pensar
        return true;
    }
    return false;
}


// ---------- 4. Aprendizaje ----------
// Promedio móvil exponencial: el peso se acerca un poco al valor que indica la
// experiencia (MAX si acertó, MIN si falló). Las experiencias recientes pesan más
// que las viejas, así que si el jugador cambia de estrategia, Soundwave se adapta.
// Ejemplo con tasa 0.3: un sector en 1.0 que acierta pasa a 1.0 + 0.3*(2.0-1.0) = 1.3.
void SoundwaveAgent::learn(int sector, bool success)
{
    if (sector < 0 || sector >= SECTORS) {
        throw std::out_of_range("SoundwaveAgent::learn: sector fuera de rango");
    }

    const double target = success ? MAX_WEIGHT : MIN_WEIGHT;
    weights[sector] += learningRate * (target - weights[sector]);
}


void SoundwaveAgent::resetPosition()
{
    angle = 0.0;
    launchCooldown = LAUNCH_INTERVAL;
    decisionPending = true;
}


Vector2D SoundwaveAgent::getPosition() const
{
    return center + Vector2D(std::cos(angle), std::sin(angle)) * ORBIT_RADIUS;
}


double SoundwaveAgent::wrapAngle(double radians)
{
    return std::remainder(radians, 2.0 * M_PI);  // resultado en [-pi, pi]
}


double SoundwaveAgent::sectorCenter(int sector)
{
    return wrapAngle((sector + 0.5) * (2.0 * M_PI / SECTORS));
}


double SoundwaveAgent::angleAround(const Vector2D &point) const
{
    return std::atan2(point.y - center.y, point.x - center.x);
}

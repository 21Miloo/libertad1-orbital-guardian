#ifndef DIFFICULTY_H
#define DIFFICULTY_H

#include <array>
#include <cstddef>

enum class Difficulty { Easy, Normal, Hard };

// Dificultad orientada a datos: cada nivel es una fila, no lógica.
// El juego solo lee estos valores; no existe ningún if por dificultad.
// Escala el comportamiento de los enemigos, no las cantidades.
struct DifficultySettings {
    const char *name;        // UTF-8
    double learningRate;     // velocidad de adaptación de Soundwave, (0, 1]
    double droneMaxSpeed;    // px/s
    double droneThrust;      // px/s², radio de giro del seek
    double debrisMinSpeed;   // factor f sobre v circular:
    double debrisMaxSpeed;   // excentricidad e = |f² - 1|
};

// constexpr: resuelta en compilación, costo cero al arrancar.
// e máxima de la basura: 0.06 / 0.19 / 0.36.
inline constexpr std::array<DifficultySettings, 3> DIFFICULTIES = {{
    { "FÁCIL",   0.15,  90.0, 45.0, 0.97, 1.02 },
    { "NORMAL",  0.30, 110.0, 60.0, 0.90, 1.05 },
    { "DIFÍCIL", 0.50, 130.0, 80.0, 0.80, 1.12 },
}};

inline const DifficultySettings &settingsFor(Difficulty difficulty)
{
    return DIFFICULTIES[static_cast<std::size_t>(difficulty)];
}

#endif // DIFFICULTY_H

#ifndef CONSTANTS_H
#define CONSTANTS_H

// Evitar choques de nombres. Sin namespace, tus constantes quedan en el espacio global,
// donde también viven las de Qt, las de la librería estándar y, en Windows, las de windows.h.
// Nombres genéricos como WIDTH, HEIGHT o DT pueden aparecer en otra librería.
namespace config {

constexpr int    WINDOW_WIDTH   = 1280;          // ancho de la ventana, en píxeles
constexpr int    WINDOW_HEIGHT  = 720;           // alto de la ventana, en píxeles
constexpr int    TICK_MS        = 16;            // cada cuántos milisegundos se actualiza el juego
constexpr double DT             = 1.0 / 120.0;   // paso de física, en segundos
constexpr double MAX_FRAME_TIME = 0.25;          // tiempo máximo aceptado entre dos actualizaciones, por si se queda pegado el juego

}

// Formula cinematica -------------------------------
// nueva posición = posición actual + velocidad × DT -> x = x + v · Δt
// --------------------------------------------------

// Con DT = 1/120 s ≈ 0.0083 s, la física se calcula 120 veces por segundo.
//Un ejemplo: si la nave va a 240 px/s, en cada paso avanza 240 × 0.0083 = 2 px.

// ¿Por qué 120 y no 60, que es la frecuencia de la pantalla?
// Porque en el nivel 2 la órbita se calcula con este paso. Un paso más pequeño hace que la órbita se mantenga estable y no se vaya abriendo con el tiempo.

#endif // CONSTANTS_H
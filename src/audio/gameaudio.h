#ifndef GAMEAUDIO_H
#define GAMEAUDIO_H

#include <QAudioOutput>
#include <QMediaPlayer>
#include <QSoundEffect>
#include <QUrl>

#include <array>
#include <cstddef>

// Fachada de audio: la ventana pide "qué" suena, no "cómo".
// Efectos: QSoundEffect, PCM precargado en memoria → latencia mínima, sin decodificar por disparo.
// Música: QMediaPlayer en streaming → no carga la pista entera.
class GameAudio {

public:
    enum class Effect { Shot, Explosion, Hit, Victory, Defeat, Count };

    GameAudio();

    void play(Effect effect);
    void playMenuMusic();
    void playGameMusic();
    void stopMusic();

private:
    static constexpr std::size_t EFFECT_COUNT = static_cast<std::size_t>(Effect::Count);

    std::array<QSoundEffect, EFFECT_COUNT> effects;  // por valor: cero heap propio
    QAudioOutput musicOutput;   // declarado antes que el player: se destruye después
    QMediaPlayer music;

    QUrl menuTrack;
    QUrl gameTrack;

    void playMusic(const QUrl &track);
    static QUrl firstAvailable(const QString &preferred, const QString &fallback);
};

#endif // GAMEAUDIO_H

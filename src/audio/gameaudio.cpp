#include "gameaudio.h"

#include <QFile>

namespace {
// Mismo orden que GameAudio::Effect
constexpr std::array<const char *, 5> EFFECT_FILES = {
    "qrc:/audio/sfx_shot.wav",
    "qrc:/audio/sfx_explosion.wav",
    "qrc:/audio/sfx_hit.wav",
    "qrc:/audio/sfx_victory.wav",
    "qrc:/audio/sfx_defeat.wav",
};
constexpr std::array<float, 5> EFFECT_VOLUMES = { 0.35f, 0.7f, 0.6f, 0.7f, 0.7f };

constexpr float MUSIC_VOLUME = 0.35f;
}

static_assert(EFFECT_FILES.size() == static_cast<std::size_t>(GameAudio::Effect::Count),
              "Un archivo por cada efecto");


GameAudio::GameAudio()
{
    // Carga única: los PCM quedan residentes, play() solo dispara
    for (std::size_t i = 0; i < effects.size(); ++i) {
        effects[i].setSource(QUrl(QString::fromLatin1(EFFECT_FILES[i])));
        effects[i].setVolume(EFFECT_VOLUMES[i]);
    }

    music.setAudioOutput(&musicOutput);
    music.setLoops(QMediaPlayer::Infinite);
    musicOutput.setVolume(MUSIC_VOLUME);

    // Pista de Transformers si está embebida; si no, la propia. El juego no depende de ella.
    menuTrack = firstAvailable(QStringLiteral(":/audio/music_menu.mp3"), QStringLiteral(":/audio/music_game.wav"));
    gameTrack = firstAvailable(QStringLiteral(":/audio/music_game.mp3"), QStringLiteral(":/audio/music_game.wav"));
}


void GameAudio::play(Effect effect)
{
    effects[static_cast<std::size_t>(effect)].play();
}


void GameAudio::playMenuMusic() { playMusic(menuTrack); }
void GameAudio::playGameMusic() { playMusic(gameTrack); }
void GameAudio::stopMusic()     { music.stop(); }


// Misma pista sonando: no reinicia (R no corta la música a la mitad)
void GameAudio::playMusic(const QUrl &track)
{
    if (music.source() == track && music.playbackState() == QMediaPlayer::PlayingState) return;

    if (music.source() != track) music.setSource(track);
    music.play();
}


// Resuelto una vez al arrancar, no en cada cambio de pista
QUrl GameAudio::firstAvailable(const QString &preferred, const QString &fallback)
{
    const QString &path = QFile::exists(preferred) ? preferred : fallback;
    return QUrl(QStringLiteral("qrc") + path);
}

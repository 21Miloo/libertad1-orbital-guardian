"""Síntesis de los sonidos propios del nivel 2.

Todo sale de ondas generadas aquí (seno, cuadrada, ruido) moldeadas con
envolventes: sin muestras de terceros, sin licencias.

Uso (desde la raíz del repo):  python tools/generate_sounds.py
Salida: recursos/audio/*.wav, mono, 16 bit, 22 050 Hz.

Solo usa la librería estándar: no hace falta instalar nada.
"""

import math
import random
import struct
import wave
from pathlib import Path

RATE = 22050                     # Hz; suficiente para efectos, la mitad de peso que 44.1 kHz
OUT = Path(__file__).resolve().parent.parent / "recursos" / "audio"
TAU = 2.0 * math.pi

random.seed(27)                  # determinista: regenerar da los mismos archivos


# ---------- utilidades ----------

def save(name, samples, gain=0.9):
    """Normaliza al pico y escribe WAV de 16 bit."""
    peak = max(1e-9, max(abs(s) for s in samples))
    scale = gain * 32767 / peak
    frames = b"".join(struct.pack("<h", int(s * scale)) for s in samples)
    with wave.open(str(OUT / name), "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(RATE)
        w.writeframes(frames)
    print(f"{name:18s} {len(samples) / RATE:5.2f} s")


def silence(seconds):
    return [0.0] * int(seconds * RATE)


def lowpass(samples, cutoff_at):
    """Filtro de un polo con corte variable: cutoff_at(t) en Hz."""
    out, y = [], 0.0
    for i, x in enumerate(samples):
        a = 1.0 - math.exp(-TAU * cutoff_at(i / RATE) / RATE)
        y += a * (x - y)
        out.append(y)
    return out


def tone(freq, seconds, shape="sine", attack=0.005, release=0.08):
    """Nota con envolvente lineal de ataque y exponencial de caída."""
    n = int(seconds * RATE)
    out, phase = [], 0.0
    for i in range(n):
        t = i / RATE
        phase += freq / RATE
        p = phase % 1.0
        if shape == "square":
            s = 1.0 if p < 0.5 else -1.0
        elif shape == "triangle":
            s = 4.0 * abs(p - 0.5) - 1.0
        else:
            s = math.sin(TAU * p) + 0.3 * math.sin(2 * TAU * p)  # seno con algo de brillo
        env = min(1.0, t / attack) * math.exp(-max(0.0, t - (seconds - release)) / (release / 4))
        out.append(s * env)
    return out


# ---------- efectos ----------

def shot():
    """Barrido exponencial 1400 → 300 Hz: el 'pew' clásico."""
    seconds, out, phase = 0.14, [], 0.0
    for i in range(int(seconds * RATE)):
        t = i / RATE
        freq = 1400 * (300 / 1400) ** (t / seconds)
        phase += freq / RATE
        s = 1.0 if phase % 1.0 < 0.5 else -1.0
        out.append(s * math.exp(-t / 0.05))
    return lowpass(out, lambda t: 5000)


def explosion():
    """Ruido blanco con filtro que se cierra + golpe grave de 55 Hz."""
    seconds = 0.8
    n = int(seconds * RATE)
    noise = [random.uniform(-1, 1) for _ in range(n)]
    rumble = lowpass(noise, lambda t: 2500 * math.exp(-t / 0.18) + 120)
    out = []
    for i, s in enumerate(rumble):
        t = i / RATE
        thump = math.sin(TAU * 55 * t) * math.exp(-t / 0.12)
        out.append((s * 2.5 + thump) * math.exp(-t / 0.25))
    return out


def hit():
    """Dos pitidos de alarma, agudo-grave: algo nuestro recibió daño."""
    return (tone(880, 0.09, "square", release=0.03)
            + tone(587, 0.14, "square", release=0.06))


def victory():
    """Arpegio mayor ascendente Do-Mi-Sol-Do."""
    notes = [(523.25, 0.13), (659.25, 0.13), (783.99, 0.13), (1046.5, 0.6)]
    out = []
    for f, d in notes:
        out += tone(f, d, release=d * 0.6)
    return out


def defeat():
    """Descenso menor lento: Sol-Mi♭-Do-Sol grave."""
    notes = [(392.0, 0.22), (311.1, 0.22), (261.6, 0.22), (196.0, 0.8)]
    out = []
    for f, d in notes:
        out += tone(f, d, "triangle", release=d * 0.7)
    return out


# ---------- música propia (respaldo y versión sin copyright) ----------

def music():
    """Pad espacial en bucle: Lam - Fa - Do - Sol, 4 s por acorde, 2 vueltas.

    Cada acorde son dientes de sierra levemente desafinados (efecto 'chorus')
    pasados por un pasa-bajos, más un bajo pulsante. La duración es múltiplo
    exacto del compás para que el bucle cierre sin salto.
    """
    chords = [
        (110.00, [220.00, 261.63, 329.63]),   # La menor
        (87.31,  [174.61, 220.00, 261.63]),   # Fa mayor
        (130.81, [196.00, 261.63, 329.63]),   # Do mayor
        (98.00,  [196.00, 246.94, 293.66]),   # Sol mayor
    ] * 2
    chord_len = 4.0
    n_chord = int(chord_len * RATE)
    out = []
    phases = [0.0] * 7
    for bass, notes in chords:
        voices = []
        for f in notes:
            voices += [f * 0.997, f * 1.003]          # pares desafinados
        for i in range(n_chord):
            t = i / RATE
            # Envolvente que respira: sube y baja dentro del acorde (bucle sin clic)
            env = math.sin(math.pi * t / chord_len) ** 0.6
            pad = 0.0
            for k, f in enumerate(voices):
                phases[k] = (phases[k] + f / RATE) % 1.0
                pad += 2.0 * phases[k] - 1.0          # diente de sierra
            pulse = 0.5 + 0.5 * math.cos(TAU * t / 0.666)   # ~90 bpm
            b = math.sin(TAU * bass * t) * (0.4 + 0.6 * pulse)
            out.append(pad * env * 0.12 + b * env * 0.5)
    return lowpass(out, lambda t: 900 + 500 * math.sin(TAU * t / 16.0))


if __name__ == "__main__":
    OUT.mkdir(parents=True, exist_ok=True)
    save("sfx_shot.wav", shot(), gain=0.5)
    save("sfx_explosion.wav", explosion(), gain=0.8)
    save("sfx_hit.wav", hit(), gain=0.5)
    save("sfx_victory.wav", victory(), gain=0.6)
    save("sfx_defeat.wav", defeat(), gain=0.6)
    save("music_game.wav", music(), gain=0.7)

#pragma once

#include <vector>
#include <random>
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Mariposas {

// conversion de Blender a OpenGL
inline glm::vec3 deBlender(float x, float y, float z) { return glm::vec3(x, z, -y); }

// ajustes
static const float RUMBO_MODELO = 0.0f;
static const float SIGNO_ALETEO = 1.0f;
static const float YAW_MARIPOSA_1 = -90.0f;

// limites del area de vuelo
static const glm::vec3 VUELO_MIN(-1.5f, 0.45f, -1.5f);
static const glm::vec3 VUELO_MAX( 4.0f, 1.40f,  0.95f);

// offsets de las alas respecto al cuerpo
static const glm::vec3 OFFSET_ALA_DER =
    deBlender(-0.817062f, 0.198031f, 0.925342f) - deBlender(-0.813777f, 0.198854f, 0.922936f);
static const glm::vec3 OFFSET_ALA_IZQ =
    deBlender(-0.817438f, 0.200224f, 0.924859f) - deBlender(-0.813777f, 0.198854f, 0.922936f);

static const float DOS_PI = 6.2831853f;
static const float PI_    = 3.1415927f;

struct Mariposa {
    glm::vec3 pos      = glm::vec3(0.0f);
    float yaw          = 0.0f;
    float escala       = 1.0f;
    bool  vuela        = false;

    // aleteo
    float freq         = 3.0f;
    float amp          = 0.6f;
    float apertura     = 0.3f;
    float fase         = 0.0f;

    // vuelo
    float velocidad    = 0.35f;
    glm::vec3 vel      = glm::vec3(0.0f);
    glm::vec3 destino  = glm::vec3(0.0f);
    glm::vec3 zigzag   = glm::vec3(0.0f);
    float tCambio      = 0.0f;
    float tZigzag      = 0.0f;
    std::mt19937 rng;
};

static float aleatorio(std::mt19937& g, float a, float b)
{
    std::uniform_real_distribution<float> d(a, b);
    return d(g);
}

// crea las 3 mariposas con sus valores iniciales
inline std::vector<Mariposa> crear()
{
    std::vector<Mariposa> v(3);

    // mariposa 1: posada, solo aletea
    v[0].pos      = deBlender(1.89717f, -0.944274f, 0.392372f);
    v[0].yaw      = glm::radians(YAW_MARIPOSA_1);
    v[0].vuela    = false;
    v[0].freq     = 3.5f;
    v[0].amp      = 0.55f;
    v[0].apertura = 0.45f;

    // mariposa 2: vuela
    v[1].pos      = deBlender(-0.813777f, 0.198854f, 0.922936f);
    v[1].vuela    = true;
    v[1].freq     = 7.0f;
    v[1].amp      = 0.85f;
    v[1].apertura = 0.15f;
    v[1].fase     = 1.3f;
    v[1].rng.seed(1234u);

    // mariposa 3: vuela con otro recorrido
    v[2].pos      = deBlender(0.761071f, 1.09549f, 0.028648f);
    v[2].vuela    = true;
    v[2].freq     = 6.2f;
    v[2].amp      = 0.80f;
    v[2].apertura = 0.15f;
    v[2].fase     = 3.1f;
    v[2].rng.seed(98765u);

    for (int i = 1; i < 3; ++i) {
        v[i].destino = v[i].pos;
        v[i].tCambio = 0.0f;
        v[i].tZigzag = 0.0f;
    }
    return v;
}

// mueve la mariposa (solo la que vuela)
inline void actualizar(Mariposa& m, float dt)
{
    if (!m.vuela) return;
    dt = std::min(dt, 0.05f);

    m.tCambio -= dt;
    m.tZigzag -= dt;

    // elegir nuevo destino
    if (m.tCambio <= 0.0f || glm::length(m.destino - m.pos) < 0.2f) {
        m.destino = glm::vec3(aleatorio(m.rng, VUELO_MIN.x, VUELO_MAX.x),
                              aleatorio(m.rng, VUELO_MIN.y, VUELO_MAX.y),
                              aleatorio(m.rng, VUELO_MIN.z, VUELO_MAX.z));
        m.tCambio   = aleatorio(m.rng, 2.5f, 6.0f);
        m.velocidad = aleatorio(m.rng, 0.25f, 0.55f);
    }

    // desvio aleatorio para que no vuele recto
    if (m.tZigzag <= 0.0f) {
        m.zigzag  = glm::vec3(aleatorio(m.rng, -1.0f, 1.0f),
                              aleatorio(m.rng, -0.6f, 0.6f),
                              aleatorio(m.rng, -1.0f, 1.0f));
        m.tZigzag = aleatorio(m.rng, 0.3f, 0.9f);
    }

    glm::vec3 haciaDestino = m.destino - m.pos;
    float len = glm::length(haciaDestino);
    glm::vec3 dir = (len > 1e-4f) ? haciaDestino / len : glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 mezcla = dir + m.zigzag * 0.7f;
    if (glm::length(mezcla) > 1e-4f) dir = glm::normalize(mezcla);

    // suavizado de la velocidad
    glm::vec3 velDeseada = dir * m.velocidad;
    float k = 1.0f - std::exp(-2.0f * dt);
    m.vel += (velDeseada - m.vel) * k;

    m.pos += m.vel * dt;
    m.pos = glm::clamp(m.pos, VUELO_MIN, VUELO_MAX);

    // la mariposa gira hacia donde va
    float vh = std::sqrt(m.vel.x * m.vel.x + m.vel.z * m.vel.z);
    if (vh > 0.03f) {
        float objetivo = std::atan2(-m.vel.z, m.vel.x) + RUMBO_MODELO;
        float d = objetivo - m.yaw;
        while (d >  PI_) d -= DOS_PI;
        while (d < -PI_) d += DOS_PI;
        m.yaw += d * (1.0f - std::exp(-4.0f * dt));
    }
}

struct Mats { glm::mat4 cuerpo, alaDer, alaIzq; };

// matrices del cuerpo y las 2 alas para el tiempo t
inline Mats construir(const Mariposa& m, float t)
{
    float ciclo = std::sin(DOS_PI * m.freq * t + m.fase);
    float a = (m.apertura + m.amp * ciclo) * SIGNO_ALETEO;

    glm::vec3 p = m.pos;
    if (m.vuela) p.y += 0.03f * std::sin(DOS_PI * 1.2f * t + m.fase);

    glm::mat4 base(1.0f);
    base = glm::translate(base, p);
    base = glm::rotate   (base, m.yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    base = glm::scale    (base, glm::vec3(m.escala));

    Mats r;
    r.cuerpo = base;
    r.alaDer = glm::rotate(glm::translate(base, OFFSET_ALA_DER), -a, glm::vec3(1.0f, 0.0f, 0.0f));
    r.alaIzq = glm::rotate(glm::translate(base, OFFSET_ALA_IZQ),  a, glm::vec3(1.0f, 0.0f, 0.0f));
    return r;
}

} // namespace Mariposas
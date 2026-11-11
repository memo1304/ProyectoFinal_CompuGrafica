#pragma once

#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Personaje {

// conversion de Blender a OpenGL
inline glm::vec3 deBlender(float x, float y, float z) { return glm::vec3(x, z, -y); }

// ajustes
static const float GIRO_MODELO         = 0.0f;
static const float SIGNO_BALANCEO      = 1.0f;
static const float ESCALA              = 1.0f;
static const float ALTURA_SUELO_REPOSO = 0.699024f;

// velocidad y largo de zancada
static const float VEL_CAMINAR      = 1.0f;
static const float LONGITUD_ZANCADA = 0.40f;

// amplitudes de la animacion (radianes)
static const float AMP_PIERNA   = 0.60f;
static const float FLEX_RODILLA = 0.90f;
static const float AMP_BRAZO    = 0.55f;
static const float CODO_BASE    = 0.15f;
static const float CODO_EXTRA   = 0.45f;
static const float TORSO_GIRO   = 0.08f;
static const float ADUCCION_BRAZOS = 0.0f;

static const float DOS_PI = 6.2831853f;

// pose de reposo
static const glm::vec3 TORSO     = deBlender(-2.52219f, 1.3657f,  0.888798f);
static const glm::vec3 HOMBRO_D  = deBlender(-2.53468f, 1.3136f,  1.03568f);
static const glm::vec3 HOMBRO_I  = deBlender(-2.53615f, 1.42185f, 1.03661f);
static const glm::vec3 CODO_D    = deBlender(-2.54122f, 1.27853f, 0.991688f);
static const glm::vec3 CODO_I    = deBlender(-2.5414f,  1.4569f,  0.992996f);
static const glm::vec3 PIERNA_D  = deBlender(-2.5262f,  1.34186f, 0.884531f);
static const glm::vec3 PIERNA_I  = deBlender(-2.52486f, 1.39205f, 0.884156f);
static const glm::vec3 RODILLA_D = deBlender(-2.52357f, 1.34304f, 0.818504f);
static const glm::vec3 RODILLA_I = deBlender(-2.52458f, 1.39385f, 0.818773f);

// donde aparece el personaje y origen del esqueleto
static const glm::vec3 INICIO = glm::vec3(TORSO.x, ALTURA_SUELO_REPOSO, TORSO.z);
static const glm::vec3 RAIZ   = glm::vec3(TORSO.x, ALTURA_SUELO_REPOSO, TORSO.z);

// offsets de cada pieza respecto a su padre
static const glm::vec3 L_TORSO       = TORSO    - RAIZ;
static const glm::vec3 L_PIERNA_D    = PIERNA_D - RAIZ;
static const glm::vec3 L_PIERNA_I    = PIERNA_I - RAIZ;
static const glm::vec3 OFF_HOMBRO_D  = HOMBRO_D - TORSO;
static const glm::vec3 OFF_HOMBRO_I  = HOMBRO_I - TORSO;
static const glm::vec3 OFF_CODO_D    = CODO_D   - HOMBRO_D;
static const glm::vec3 OFF_CODO_I    = CODO_I   - HOMBRO_I;
static const glm::vec3 OFF_RODILLA_D = RODILLA_D - PIERNA_D;
static const glm::vec3 OFF_RODILLA_I = RODILLA_I - PIERNA_I;

struct Estado {
    float yaw  = 0.0f;
    float fase = 0.0f;
    float peso = 0.0f;
};

// giro suave hacia el rumbo objetivo
inline void girarHacia(Estado& e, float yawObjetivo, float dt)
{
    float d = yawObjetivo - e.yaw;
    while (d >  3.14159265f) d -= DOS_PI;
    while (d < -3.14159265f) d += DOS_PI;
    e.yaw += d * (1.0f - std::exp(-12.0f * dt));
}

// la fase avanza segun la distancia recorrida, asi no patina si choca
inline void avanzarAnimacion(Estado& e, float distanciaRecorrida, float dt)
{
    if (dt <= 0.0f) return;
    float vel = distanciaRecorrida / dt;
    float objetivo = (vel > 0.05f) ? 1.0f : 0.0f;
    e.peso += (objetivo - e.peso) * (1.0f - std::exp(-10.0f * dt));
    e.fase  = std::fmod(e.fase + DOS_PI * distanciaRecorrida / LONGITUD_ZANCADA, DOS_PI);
}

struct Mats {
    glm::mat4 torso;
    glm::mat4 hombroDer, hombroIzq, codoDer, codoIzq;
    glm::mat4 piernaDer, piernaIzq, rodillaDer, rodillaIzq;
};

// pies = playerPos
inline Mats construir(const glm::vec3& pies, const Estado& e)
{
    const glm::vec3 eX(1.0f, 0.0f, 0.0f), eY(0.0f, 1.0f, 0.0f), eZ(0.0f, 0.0f, 1.0f);
    const float s = SIGNO_BALANCEO, w = e.peso;
    const float sn = std::sin(e.fase), cs = std::cos(e.fase);

    // angulos sobre Z, positivo = hacia adelante
    float muslo_d = w * s * AMP_PIERNA * sn;
    float muslo_i = -muslo_d;
    float rod_d   = -w * s * FLEX_RODILLA * std::max(0.0f,  cs);
    float rod_i   = -w * s * FLEX_RODILLA * std::max(0.0f, -cs);
    float bra_d   = -w * s * AMP_BRAZO * sn;
    float bra_i   = -bra_d;
    float cod_d   =  w * s * (CODO_BASE + CODO_EXTRA * std::max(0.0f, -cs));
    float cod_i   =  w * s * (CODO_BASE + CODO_EXTRA * std::max(0.0f,  cs));
    float torsion = -w * TORSO_GIRO * sn;

    float aduc = glm::radians(ADUCCION_BRAZOS);

    glm::mat4 raiz(1.0f);
    raiz = glm::translate(raiz, pies);
    raiz = glm::rotate   (raiz, e.yaw + GIRO_MODELO, eY);
    raiz = glm::scale    (raiz, glm::vec3(ESCALA));

    Mats m;

    // torso con cabeza
    m.torso = glm::rotate(glm::translate(raiz, L_TORSO), torsion, eY);

    // brazo derecho
    glm::mat4 hd = glm::translate(m.torso, OFF_HOMBRO_D);
    hd = glm::rotate(hd, bra_d, eZ);
    hd = glm::rotate(hd,  aduc, eX);
    m.hombroDer = hd;
    m.codoDer   = glm::rotate(glm::translate(hd, OFF_CODO_D), cod_d, eZ);

    // brazo izquierdo
    glm::mat4 hi = glm::translate(m.torso, OFF_HOMBRO_I);
    hi = glm::rotate(hi, bra_i, eZ);
    hi = glm::rotate(hi, -aduc, eX);
    m.hombroIzq = hi;
    m.codoIzq   = glm::rotate(glm::translate(hi, OFF_CODO_I), cod_i, eZ);

    // pierna derecha
    glm::mat4 pd = glm::rotate(glm::translate(raiz, L_PIERNA_D), muslo_d, eZ);
    m.piernaDer  = pd;
    m.rodillaDer = glm::rotate(glm::translate(pd, OFF_RODILLA_D), rod_d, eZ);

    // pierna izquierda
    glm::mat4 pi = glm::rotate(glm::translate(raiz, L_PIERNA_I), muslo_i, eZ);
    m.piernaIzq  = pi;
    m.rodillaIzq = glm::rotate(glm::translate(pi, OFF_RODILLA_I), rod_i, eZ);

    return m;
}

} // namespace Personaje
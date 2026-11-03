#pragma once
// ============================================================================
//  DanzaPescadores.h  -  VERSION POR KEYFRAMES
//  Animacion jerarquica (cinematica directa) para la Danza de los Pescadores.
//
//  Cada seccion del baile es una TABLA DE KEYFRAMES: una lista de poses
//  (beat + valor de cada articulacion). El programa solo interpola entre
//  una pose y la siguiente. No hay formulas de baile: las poses estan
//  definidas a mano en las tablas de abajo.
//
//  Jerarquia:
//    lancha (raiz: oleaje)
//      └─ cadera (giro del cuerpo, balanceo, flexion de rodillas)
//           ├─ piernaIzq / piernaDer
//           └─ torso (inclinacion, torsion)
//                ├─ cabeza
//                ├─ hombroIzq ─ codoIzq
//                └─ hombroDer ─ codoDer ─ red (pendulo)
// ============================================================================
#include <cmath>
#include <algorithm>
#include <vector>
#include <utility>
#include <initializer_list>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Danza {

// ───────────── Parametros musicales ─────────────
constexpr float PI                = 3.14159265f;
constexpr float BPM               = 100.0f;   // 1 beat = 0.6 s
constexpr float BEATS_PER_SECTION = 8.0f;
constexpr int   NUM_SECTIONS      = 8;        // 8 secciones en total
constexpr float BLEND_BEATS       = 1.0f;     // transicion suave entre secciones

// ───────────── Offsets de pivotes ─────────────
const glm::vec3 WAIST_OFFSET(0.0f, -0.05f, 0.0f);    // cintura respecto a cadera
const glm::vec3 NET_PIVOT_OFFSET(0.0f, 0.05f, 0.0f); // mano respecto al centro de la red

constexpr float LEG_GAIN = 0.45f; // Intensidad del movimiento de piernas
constexpr float FWD = -1.0f;      // Hacia donde miran los modelos: -1 = hacia -Z

// Dimensiones aproximadas de la lancha
constexpr float BOAT_HALF_LEN  = 0.8f;
constexpr float BOAT_HALF_WID  = 0.2f;
constexpr float BOAT_BOB_AMP   = 0.02f;
constexpr float BOAT_REST_LIFT = 0.0f;

// ───────────── Canales de animacion ─────────────
enum Ch {
    BOB_Y, BOAT_ROLL, BOAT_PITCH,
    BODY_YAW, HIP_ROLL, HIP_DROP,
    TORSO_BEND, TORSO_TWIST, TORSO_ROLL,
    HEAD_NOD, HEAD_TURN, HEAD_TILT,
    SHL_PITCH, SHL_ROLL, ELL_BEND,
    SHR_PITCH, SHR_ROLL, ELR_BEND,
    LEGL_PITCH, LEGL_ROLL, LEGR_PITCH, LEGR_ROLL,
    NET_X, NET_Z,
    NUM_CH
};

// Una pose = un valor por articulacion (lo que no se escribe vale 0)
struct Pose {
    float c[NUM_CH];
    Pose() { for (int i = 0; i < NUM_CH; ++i) c[i] = 0.0f; }
    float&       operator[](int i)       { return c[i]; }
    const float& operator[](int i) const { return c[i]; }
};

// ───────────── Utilidades ─────────────
inline float clamp01(float x) { return std::min(1.0f, std::max(0.0f, x)); }
inline float smooth(float x)  { x = clamp01(x); return x * x * (3.0f - 2.0f * x); }
inline float wrapPi(float a)  { while (a > PI) a -= 2*PI; while (a < -PI) a += 2*PI; return a; }

inline Pose lerpPose(const Pose& a, const Pose& b, float k) {
    Pose r;
    for (int i = 0; i < NUM_CH; ++i) r[i] = a[i] + (b[i] - a[i]) * k;
    return r;
}

// Mezcla entre secciones (el yaw se envuelve para no dar vueltas de mas)
inline Pose mix(const Pose& a, const Pose& b, float k) {
    Pose r = lerpPose(a, b, k);
    r[BODY_YAW] = a[BODY_YAW] + wrapPi(b[BODY_YAW] - a[BODY_YAW]) * k;
    return r;
}

// ═════════════ KEYFRAMES ═════════════

struct Key   { float beat; Pose pose; };
struct Track { std::vector<Key> keys; bool suave; }; // suave = ease in/out entre keys

// Crea un keyframe a partir de pares {canal, valor}. Si un canal se repite, se suma.
inline Key K(float beat, std::initializer_list<std::pair<int, float> > v)
{
    Key k;
    k.beat = beat;
    for (const auto& e : v) k.pose[e.first] += e.second;
    return k;
}

// Interpola la pose en 'beat' dentro de una tabla de keyframes
inline Pose evalTrack(const Track& t, float beat)
{
    const std::vector<Key>& k = t.keys;
    if (beat <= k.front().beat) return k.front().pose;
    if (beat >= k.back().beat)  return k.back().pose;

    size_t i = 0;
    while (i + 2 < k.size() && beat >= k[i + 1].beat) ++i;

    float u = (beat - k[i].beat) / (k[i + 1].beat - k[i].beat);
    if (t.suave) u = smooth(u);
    return lerpPose(k[i].pose, k[i + 1].pose, u);
}

// ───────────── Bloques reutilizables para escribir poses ─────────────
// Pie izquierdo / derecho levantado (a = intensidad del paso)
#define PL(a) {LEGL_PITCH,-0.80f*(a)},{LEGL_ROLL, 0.10f*(a)},{HIP_ROLL, 0.08f*(a)}
#define PR(a) {LEGR_PITCH,-0.80f*(a)},{LEGR_ROLL,-0.10f*(a)},{HIP_ROLL,-0.08f*(a)}

// ---- Seccion 0: Paso base ----
#define S0_BASE   {SHL_ROLL,0.15f},{SHR_ROLL,-0.15f},{ELL_BEND,-0.30f},{ELR_BEND,-0.50f},{TORSO_BEND,0.17f}
#define S0_ARMS_L {SHL_PITCH, 0.55f},{SHR_PITCH,-0.55f},{TORSO_ROLL, 0.08f}
#define S0_ARMS_R {SHL_PITCH,-0.55f},{SHR_PITCH, 0.55f},{TORSO_ROLL,-0.08f}

// ---- Seccion 1: Lanzar la red ----
#define S1_REST   {ELL_BEND,-0.30f}
#define S1_WIND   {TORSO_BEND,-0.30f},{TORSO_TWIST,0.55f},{HEAD_NOD,-0.15f},                 \
                  {SHR_PITCH,1.10f},{SHR_ROLL,-0.50f},{ELR_BEND,-0.40f},                      \
                  {SHL_PITCH,0.80f},{SHL_ROLL,0.30f},{ELL_BEND,-0.60f},{HIP_DROP,0.05f}
#define S1_THROW  {TORSO_BEND,0.80f},{TORSO_TWIST,-0.50f},{HEAD_NOD,0.25f},                  \
                  {SHR_PITCH,-2.40f},{SHR_ROLL,-0.30f},{ELR_BEND,-0.20f},                     \
                  {SHL_PITCH,-2.00f},{SHL_ROLL,0.20f},{ELL_BEND,-0.30f},{HIP_DROP,0.04f},     \
                  {LEGL_PITCH,-0.50f}

// ---- Seccion 2: Jalar la red (mano sobre mano) ----
#define S2_REST   {SHL_PITCH,-0.90f},{ELL_BEND,-0.80f},{SHR_PITCH,-0.90f},{ELR_BEND,-0.80f}, \
                  {TORSO_BEND,0.30f},{HIP_DROP,0.05f},{HEAD_NOD,0.15f},                       \
                  {LEGL_ROLL,0.18f},{LEGR_ROLL,-0.18f}
#define S2_COM    {TORSO_BEND,0.50f},{HIP_DROP,0.07f},{HEAD_NOD,0.15f},                      \
                  {LEGL_ROLL,0.18f},{LEGR_ROLL,-0.18f}
#define S2_JALA_L {SHL_PITCH,-1.40f},{ELL_BEND,0.0f},{SHR_PITCH,-0.40f},{ELR_BEND,-1.60f},   \
                  {TORSO_TWIST,0.30f},{BOAT_ROLL,0.03f}
#define S2_JALA_R {SHL_PITCH,-0.40f},{ELL_BEND,-1.60f},{SHR_PITCH,-1.40f},{ELR_BEND,0.0f},   \
                  {TORSO_TWIST,-0.30f},{BOAT_ROLL,-0.03f}

// ---- Seccion 3: Giro con brazos abiertos ----
#define S3_ARMS   {SHL_ROLL,2.0f},{SHR_ROLL,-2.0f},{ELL_BEND,-0.2f},{ELR_BEND,-0.2f},        \
                  {TORSO_BEND,-0.10f},{HEAD_NOD,-0.15f}
#define YAW_4PI   12.566371f

// ---- Seccion 4: Barrido de la red ----
#define S4_COM    {ELR_BEND,-0.2f},{SHL_PITCH,-0.2f},{ELL_BEND,-0.6f},{TORSO_BEND,0.15f}
#define S4_CENTRO {SHR_PITCH,-1.8f},{SHR_ROLL,-0.6f},{SHL_ROLL,1.2f}
#define S4_DER    {SHR_PITCH,-1.4f},{SHR_ROLL, 0.2f},{SHL_ROLL,0.8f},                        \
                  {TORSO_TWIST, 0.45f},{TORSO_ROLL, 0.10f},{HEAD_TURN, 0.5f}
#define S4_IZQ    {SHR_PITCH,-2.2f},{SHR_ROLL,-1.4f},{SHL_ROLL,1.6f},                        \
                  {TORSO_TWIST,-0.45f},{TORSO_ROLL,-0.10f},{HEAD_TURN,-0.5f}

// ---- Seccion 5: Remar ----
#define S5_NEUTRO {HIP_DROP,0.05f}
#define S5_JALA   {TORSO_BEND,-0.30f},{HIP_DROP,0.05f},{SHL_PITCH,-1.2f},{SHR_PITCH,-1.2f},  \
                  {ELL_BEND,-1.5f},{ELR_BEND,-1.5f}
#define S5_EMPUJA {TORSO_BEND,0.60f},{HIP_DROP,0.15f},{SHL_PITCH,0.8f},{SHR_PITCH,0.8f},     \
                  {ELL_BEND,-0.4f},{ELR_BEND,-0.4f},{HEAD_NOD,0.20f}

// ---- Seccion 6: Ofrenda (zapateado con la red al cielo) ----
#define S6_BASE   {SHR_PITCH,-2.5f},{SHR_ROLL,-0.3f},{ELR_BEND,-0.1f},                       \
                  {SHL_PITCH,-2.5f},{SHL_ROLL, 0.3f},{ELL_BEND,-0.1f},                        \
                  {TORSO_BEND,-0.3f},{HEAD_NOD,-0.5f}
#define S6_ZAP_L  PL(1.2f),{HIP_DROP,0.036f},{TORSO_ROLL, 0.05f},{SHL_ROLL, 0.05f},{SHR_ROLL, 0.05f}
#define S6_ZAP_R  PR(1.2f),{HIP_DROP,0.036f},{TORSO_ROLL,-0.05f},{SHL_ROLL,-0.05f},{SHR_ROLL,-0.05f}

// ---- Seccion 7: Reverencia y celebracion ----
#define S7_BOW    {TORSO_BEND,0.60f},{HEAD_NOD,0.20f},{HIP_DROP,0.06f},{SHL_PITCH,0.90f},{SHR_PITCH,0.90f}
#define S7_CELE   {TORSO_BEND,-0.25f},{HEAD_NOD,-0.30f}


// ═════════════ TABLAS DE POSES (aqui se edita el baile) ═════════════

inline std::vector<Track> crearSecciones()
{
    std::vector<Track> T;

    // ───────── 0. PASO BASE ─────────
    // Un paso por beat, alternando pie izq/der con balanceo de brazos,
    // torsion de torso y la cabeza mirando de lado a lado.
    T.push_back(Track{ {
        K(0.0f, { S0_BASE }),
        K(1.0f, { S0_BASE, PL(1.0f), S0_ARMS_L, {TORSO_TWIST, 0.25f}, {HEAD_TURN, 0.25f} }),
        K(2.0f, { S0_BASE, PR(1.0f), S0_ARMS_R,                       {HEAD_TURN, 0.35f} }),
        K(3.0f, { S0_BASE, PL(1.0f), S0_ARMS_L, {TORSO_TWIST,-0.25f}, {HEAD_TURN, 0.25f} }),
        K(4.0f, { S0_BASE, PR(1.0f), S0_ARMS_R                                            }),
        K(5.0f, { S0_BASE, PL(1.0f), S0_ARMS_L, {TORSO_TWIST, 0.25f}, {HEAD_TURN,-0.25f} }),
        K(6.0f, { S0_BASE, PR(1.0f), S0_ARMS_R,                       {HEAD_TURN,-0.35f} }),
        K(7.0f, { S0_BASE, PL(1.0f), S0_ARMS_L, {TORSO_TWIST,-0.25f}, {HEAD_TURN,-0.25f} }),
        K(8.0f, { S0_BASE })
    }, true });

    // ───────── 1. LANZAR LA RED ─────────
    // Dos lanzamientos: se prepara (brazos atras, torso girado),
    // lanza de golpe (brazos arriba y adelante) y regresa a reposo.
    T.push_back(Track{ {
        K(0.0f, { S1_REST }),
        K(1.8f, { S1_WIND }),      // preparacion 1
        K(2.4f, { S1_THROW }),     // lanzamiento 1
        K(4.0f, { S1_REST }),      // reposo
        K(5.8f, { S1_WIND }),      // preparacion 2
        K(6.4f, { S1_THROW }),     // lanzamiento 2
        K(8.0f, { S1_REST })
    }, true });

    // ───────── 2. JALAR LA RED ─────────
    // Mano sobre mano: un brazo jala mientras el otro suelta, alternando.
    T.push_back(Track{ {
        K(0.0f, { S2_REST }),
        K(0.5f, { S2_COM, S2_JALA_L, PL(0.25f) }),
        K(1.5f, { S2_COM, S2_JALA_R, PR(0.25f) }),
        K(2.5f, { S2_COM, S2_JALA_L, PL(0.25f) }),
        K(3.5f, { S2_COM, S2_JALA_R, PR(0.25f) }),
        K(4.5f, { S2_COM, S2_JALA_L, PL(0.25f) }),
        K(5.5f, { S2_COM, S2_JALA_R, PR(0.25f) }),
        K(6.5f, { S2_COM, S2_JALA_L, PL(0.25f) }),
        K(7.5f, { S2_COM, S2_JALA_R, PR(0.25f) }),
        K(8.0f, { S2_REST })
    }, true });

    // ───────── 3. GIRO ─────────
    // Brazos abiertos y dos vueltas completas (4*PI). Interpolacion lineal
    // (suave = false) para que el giro tenga velocidad constante.
    T.push_back(Track{ {
        K(0.0f, { }),
        K(0.5f, { {BODY_YAW, 0.0f} }),
        K(1.0f, { S3_ARMS, {BODY_YAW, 1.047f},  PL(0.8f), {TORSO_ROLL, 0.12f} }),
        K(2.0f, { S3_ARMS, {BODY_YAW, 3.142f},  PR(0.8f), {TORSO_ROLL,-0.12f} }),
        K(3.0f, { S3_ARMS, {BODY_YAW, 5.236f},  PL(0.8f), {TORSO_ROLL, 0.12f} }),
        K(4.0f, { S3_ARMS, {BODY_YAW, 7.330f},  PR(0.8f), {TORSO_ROLL,-0.12f} }),
        K(5.0f, { S3_ARMS, {BODY_YAW, 9.425f},  PL(0.8f), {TORSO_ROLL, 0.12f} }),
        K(6.0f, { S3_ARMS, {BODY_YAW, 11.519f}, PR(0.8f), {TORSO_ROLL,-0.12f} }),
        K(6.5f, { S3_ARMS, {BODY_YAW, YAW_4PI} }),
        K(7.0f, { S3_ARMS, {BODY_YAW, YAW_4PI}, PL(0.8f) }),
        K(8.0f, { {BODY_YAW, YAW_4PI} })
    }, false });

    // ───────── 4. BARRIDO DE LA RED ─────────
    // La red en alto se mueve de derecha a izquierda (4 beats por ciclo),
    // con el torso y la cabeza siguiendo el movimiento.
    T.push_back(Track{ {
        K(0.0f, { S4_COM, S4_CENTRO }),
        K(1.0f, { S4_COM, S4_DER,    PL(0.5f) }),
        K(2.0f, { S4_COM, S4_CENTRO, PR(0.5f) }),
        K(3.0f, { S4_COM, S4_IZQ,    PL(0.5f) }),
        K(4.0f, { S4_COM, S4_CENTRO, PR(0.5f) }),
        K(5.0f, { S4_COM, S4_DER,    PL(0.5f) }),
        K(6.0f, { S4_COM, S4_CENTRO, PR(0.5f) }),
        K(7.0f, { S4_COM, S4_IZQ,    PL(0.5f) }),
        K(8.0f, { S4_COM, S4_CENTRO })
    }, true });

    // ───────── 5. REMAR ─────────
    // Cada 2 beats: jala el remo hacia el pecho (torso atras) y
    // lo empuja hacia adelante (torso inclinado, cadera baja).
    T.push_back(Track{ {
        K(0.0f, { S5_NEUTRO }),
        K(0.5f, { S5_JALA,   PL(0.4f) }),
        K(1.5f, { S5_EMPUJA, PR(0.4f) }),
        K(2.5f, { S5_JALA,   PL(0.4f) }),
        K(3.5f, { S5_EMPUJA, PR(0.4f) }),
        K(4.5f, { S5_JALA,   PL(0.4f) }),
        K(5.5f, { S5_EMPUJA, PR(0.4f) }),
        K(6.5f, { S5_JALA,   PL(0.4f) }),
        K(7.5f, { S5_EMPUJA, PR(0.4f) }),
        K(8.0f, { S5_NEUTRO })
    }, true });

    // ───────── 6. OFRENDA ─────────
    // Brazos y red levantados al cielo, mirando arriba, mientras
    // zapatea rapido alternando los pies en cada beat.
    T.push_back(Track{ {
        K(0.0f, { S6_BASE }),
        K(1.0f, { S6_BASE, S6_ZAP_L }),
        K(2.0f, { S6_BASE, S6_ZAP_R }),
        K(3.0f, { S6_BASE, S6_ZAP_L }),
        K(4.0f, { S6_BASE, S6_ZAP_R }),
        K(5.0f, { S6_BASE, S6_ZAP_L }),
        K(6.0f, { S6_BASE, S6_ZAP_R }),
        K(7.0f, { S6_BASE, S6_ZAP_L }),
        K(8.0f, { S6_BASE })
    }, true });

    // ───────── 7. REVERENCIA FINAL ─────────
    // Se inclina con los brazos adelante (reverencia), se endereza y
    // celebra con los brazos abiertos moviendo la red y saltando.
    T.push_back(Track{ {
        K(0.0f,  { }),
        K(1.5f,  { S7_BOW }),                                                                   // reverencia
        K(4.0f,  { S7_BOW }),                                                                   // sostiene
        K(5.5f,  { }),                                                                          // se endereza
        K(6.5f,  { S7_CELE, {SHL_ROLL, 2.60f}, {SHR_ROLL,-2.60f} }),                            // brazos abiertos
        K(6.75f, { S7_CELE, {SHL_ROLL, 2.75f}, {SHR_ROLL,-2.75f}, {NET_Z, 0.40f}, PL(0.9f) }),  // celebra
        K(7.0f,  { S7_CELE, {SHL_ROLL, 2.45f}, {SHR_ROLL,-2.45f}, {NET_Z,-0.40f}, PR(0.9f) }),
        K(7.25f, { S7_CELE, {SHL_ROLL, 2.75f}, {SHR_ROLL,-2.75f}, {NET_Z, 0.40f}, PL(0.9f) }),
        K(7.5f,  { S7_CELE, {SHL_ROLL, 2.45f}, {SHR_ROLL,-2.45f}, {NET_Z,-0.40f}, PR(0.9f) }),
        K(8.0f,  { })                                                                           // reposo
    }, true });

    return T;
}

// ───────── OLEAJE: tambien por keyframes (se repite cada 8 beats) ─────────
// La lancha sube/baja y se inclina suavemente (un ciclo = 8 beats).
inline Track crearOleajeLancha()
{
    return Track{ {
        K(0.0f, { {BOB_Y, 0.0f},     {BOAT_ROLL, 0.0177f},  {BOAT_PITCH, 0.0f}     }),
        K(1.0f, { {BOB_Y, 0.0141f},  {BOAT_ROLL, 0.025f},   {BOAT_PITCH,-0.0085f}  }),
        K(2.0f, { {BOB_Y, 0.02f},    {BOAT_ROLL, 0.0177f},  {BOAT_PITCH,-0.012f}   }),
        K(3.0f, { {BOB_Y, 0.0141f},  {BOAT_ROLL, 0.0f},     {BOAT_PITCH,-0.0085f}  }),
        K(4.0f, { {BOB_Y, 0.0f},     {BOAT_ROLL,-0.0177f},  {BOAT_PITCH, 0.0f}     }),
        K(5.0f, { {BOB_Y,-0.0141f},  {BOAT_ROLL,-0.025f},   {BOAT_PITCH, 0.0085f}  }),
        K(6.0f, { {BOB_Y,-0.02f},    {BOAT_ROLL,-0.0177f},  {BOAT_PITCH, 0.012f}   }),
        K(7.0f, { {BOB_Y,-0.0141f},  {BOAT_ROLL, 0.0f},     {BOAT_PITCH, 0.0085f}  }),
        K(8.0f, { {BOB_Y, 0.0f},     {BOAT_ROLL, 0.0177f},  {BOAT_PITCH, 0.0f}     })
    }, true };
}

// La red se balancea en circulo (X y Z desfasados un cuarto de ciclo, 2 beats por vuelta).
inline Track crearOleajeRed()
{
    return Track{ {
        K(0.0f, { {NET_X, 0.08f}, {NET_Z, 0.0f}   }),
        K(0.5f, { {NET_X, 0.0f},  {NET_Z, 0.06f}  }),
        K(1.0f, { {NET_X,-0.08f}, {NET_Z, 0.0f}   }),
        K(1.5f, { {NET_X, 0.0f},  {NET_Z,-0.06f}  }),
        K(2.0f, { {NET_X, 0.08f}, {NET_Z, 0.0f}   }),
        K(2.5f, { {NET_X, 0.0f},  {NET_Z, 0.06f}  }),
        K(3.0f, { {NET_X,-0.08f}, {NET_Z, 0.0f}   }),
        K(3.5f, { {NET_X, 0.0f},  {NET_Z,-0.06f}  }),
        K(4.0f, { {NET_X, 0.08f}, {NET_Z, 0.0f}   }),
        K(4.5f, { {NET_X, 0.0f},  {NET_Z, 0.06f}  }),
        K(5.0f, { {NET_X,-0.08f}, {NET_Z, 0.0f}   }),
        K(5.5f, { {NET_X, 0.0f},  {NET_Z,-0.06f}  }),
        K(6.0f, { {NET_X, 0.08f}, {NET_Z, 0.0f}   }),
        K(6.5f, { {NET_X, 0.0f},  {NET_Z, 0.06f}  }),
        K(7.0f, { {NET_X,-0.08f}, {NET_Z, 0.0f}   }),
        K(7.5f, { {NET_X, 0.0f},  {NET_Z,-0.06f}  }),
        K(8.0f, { {NET_X, 0.08f}, {NET_Z, 0.0f}   })
    }, true };
}

// Las tablas se construyen una sola vez
inline const std::vector<Track>& secciones() { static const std::vector<Track> T = crearSecciones();   return T; }
inline const Track& oleajeLancha()           { static const Track T = crearOleajeLancha();            return T; }
inline const Track& oleajeRed()              { static const Track T = crearOleajeRed();               return T; }

#undef PL
#undef PR
#undef S0_BASE
#undef S0_ARMS_L
#undef S0_ARMS_R
#undef S1_REST
#undef S1_WIND
#undef S1_THROW
#undef S2_REST
#undef S2_COM
#undef S2_JALA_L
#undef S2_JALA_R
#undef S3_ARMS
#undef YAW_4PI
#undef S4_COM
#undef S4_CENTRO
#undef S4_DER
#undef S4_IZQ
#undef S5_NEUTRO
#undef S5_JALA
#undef S5_EMPUJA
#undef S6_BASE
#undef S6_ZAP_L
#undef S6_ZAP_R
#undef S7_BOW
#undef S7_CELE


// ═════════════ EVALUADOR ═════════════

// Pose de una seccion en un beat local (0..8), leida de su tabla de keyframes
inline Pose evalSection(int s, float b)
{
    int idx = std::max(0, std::min(NUM_SECTIONS - 1, s));
    return evalTrack(secciones()[idx], b);
}

// t = tiempo en segundos, delayBeats = retraso de este personaje, phase = desfase del oleaje
inline Pose evaluate(float t, float delayBeats, float phase)
{
    float tb    = t * BPM / 60.0f;                       // tiempo en beats
    float b     = std::max(0.0f, tb - delayBeats);
    float total = NUM_SECTIONS * BEATS_PER_SECTION;
    float cb    = std::fmod(b, total);
    int   s     = (int)(cb / BEATS_PER_SECTION);
    float lb    = cb - s * BEATS_PER_SECTION;

    Pose cur = evalSection(s, lb);

    // Transicion suave: al inicio de cada seccion se mezcla con la pose final de la anterior
    if (lb < BLEND_BEATS && (s > 0 || b >= total)) {
        int  ps   = (s + NUM_SECTIONS - 1) % NUM_SECTIONS;
        Pose prev = evalSection(ps, BEATS_PER_SECTION);
        cur = mix(prev, cur, smooth(lb / BLEND_BEATS));
    }

    // Oleaje y balanceo de la red (tambien keyframes; 'phase' los desfasa entre lanchas)
    float ob = std::fmod(tb + phase * BEATS_PER_SECTION / (2.0f * PI), BEATS_PER_SECTION);
    if (ob < 0.0f) ob += BEATS_PER_SECTION;

    Pose olaB = evalTrack(oleajeLancha(), ob);
    Pose olaR = evalTrack(oleajeRed(),    ob);
    for (int i = 0; i < NUM_CH; ++i) cur[i] += olaB[i] + olaR[i];

    return cur;
}

// ───────────── Matrices finales ─────────────
struct Mats {
    glm::mat4 lancha, cadera, torso, cabeza;
    glm::mat4 hombroIzq, codoIzq, hombroDer, codoDer;
    glm::mat4 piernaIzq, piernaDer, red;
};

inline glm::mat4 Rx(float a) { return glm::rotate(glm::mat4(1.0f), a, glm::vec3(1, 0, 0)); }
inline glm::mat4 Ry(float a) { return glm::rotate(glm::mat4(1.0f), a, glm::vec3(0, 1, 0)); }
inline glm::mat4 Rz(float a) { return glm::rotate(glm::mat4(1.0f), a, glm::vec3(0, 0, 1)); }
inline glm::mat4 Tr(const glm::vec3& v) { return glm::translate(glm::mat4(1.0f), v); }

inline glm::mat4 rotAbout(const glm::vec3& P, const glm::mat4& R) { return Tr(P) * R * Tr(-P); }

template <class P>
Mats build(const P& r, const Pose& a, const Pose& lag)
{
    Mats m;

    float lowest = std::fabs(a[BOAT_PITCH]) * BOAT_HALF_LEN +
                   std::fabs(a[BOAT_ROLL])  * BOAT_HALF_WID;
    float bob    = a[BOB_Y] + BOAT_BOB_AMP + lowest + BOAT_REST_LIFT;

    glm::mat4 Aboat = Tr(glm::vec3(0, bob, 0)) *
                      rotAbout(r.lancha, Rz(a[BOAT_ROLL]) * Rx(a[BOAT_PITCH]));

    glm::mat4 Ahip = Aboat * Tr(glm::vec3(0, -a[HIP_DROP], 0)) *
                     rotAbout(r.cadera, Ry(a[BODY_YAW]) * Rz(a[HIP_ROLL]));

    glm::vec3 waist = r.cadera + WAIST_OFFSET;
    glm::mat4 Atorso = Ahip *
        rotAbout(waist, Ry(a[TORSO_TWIST]) * Rz(a[TORSO_ROLL]) * Rx(FWD * a[TORSO_BEND]));

    glm::vec3 neck(r.cabeza.x, r.hombroIzq.y, r.cabeza.z);
    glm::mat4 Ahead = Atorso *
        rotAbout(neck, Ry(a[HEAD_TURN]) * Rz(a[HEAD_TILT]) * Rx(FWD * a[HEAD_NOD]));

    // ── RESTRICCION DINAMICA DE COLISION ──
    Pose a_adj = a;
    const float MAX_SAFE_PITCH = 0.15f;
    if (a_adj[SHR_PITCH] > MAX_SAFE_PITCH) {
        float excess = a_adj[SHR_PITCH] - MAX_SAFE_PITCH;
        a_adj[ELR_BEND] -= excess * 2.2f;
        a_adj[SHR_PITCH] -= excess * 0.4f;
    }
    if (a_adj[SHL_PITCH] > MAX_SAFE_PITCH) {
        float excess = a_adj[SHL_PITCH] - MAX_SAFE_PITCH;
        a_adj[ELL_BEND] -= excess * 2.2f;
        a_adj[SHL_PITCH] -= excess * 0.4f;
    }

    glm::mat4 AshL = Atorso * rotAbout(r.hombroIzq, Rz(a_adj[SHL_ROLL]) * Rx(FWD * a_adj[SHL_PITCH]));
    glm::mat4 AelL = AshL   * rotAbout(r.codoIzq,   Rx(FWD * a_adj[ELL_BEND]));
    glm::mat4 AshR = Atorso * rotAbout(r.hombroDer, Rz(a_adj[SHR_ROLL]) * Rx(FWD * a_adj[SHR_PITCH]));
    glm::mat4 AelR = AshR   * rotAbout(r.codoDer,   Rx(FWD * a_adj[ELR_BEND]));

    float dyaw = std::fabs(wrapPi(lag[BODY_YAW] - a[BODY_YAW]));
    float swX = std::max(-0.8f, std::min(0.8f, (lag[SHR_PITCH] - a[SHR_PITCH]) * 0.5f + a[NET_X]));
    float swZ = std::max(-0.8f, std::min(0.8f, (lag[SHR_ROLL]  - a[SHR_ROLL])  * 0.5f + a[NET_Z] - dyaw * 0.8f));
    glm::mat4 Ared = AelR * rotAbout(r.red + NET_PIVOT_OFFSET, Rz(swZ) * Rx(FWD * swX));

    float legLen = std::max(0.05f, r.cadera.y - r.lancha.y);
    float sc     = std::max(0.3f, (legLen - a[HIP_DROP]) / legLen);
    glm::mat4 S  = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, sc, 1.0f));

    glm::vec3 hipL = r.piernaIzq;
    glm::vec3 hipR = r.piernaDer;

    glm::mat4 AlegL = Ahip * rotAbout(hipL, Rz(LEG_GAIN * a[LEGL_ROLL]) * Rx(FWD * LEG_GAIN * a[LEGL_PITCH]) * S);
    glm::mat4 AlegR = Ahip * rotAbout(hipR, Rz(LEG_GAIN * a[LEGR_ROLL]) * Rx(FWD * LEG_GAIN * a[LEGR_PITCH]) * S);

    m.lancha    = Aboat  * Tr(r.lancha);
    m.cadera    = Ahip   * Tr(r.cadera);
    m.torso     = Atorso * Tr(r.torso);
    m.cabeza    = Ahead  * Tr(r.cabeza);
    m.hombroIzq = AshL   * Tr(r.hombroIzq);
    m.codoIzq   = AelL   * Tr(r.codoIzq);
    m.hombroDer = AshR   * Tr(r.hombroDer);
    m.codoDer   = AelR   * Tr(r.codoDer);
    m.piernaIzq = AlegL  * Tr(r.piernaIzq);
    m.piernaDer = AlegR  * Tr(r.piernaDer);
    m.red       = Ared   * Tr(r.red);
    return m;
}

} // namespace Danza
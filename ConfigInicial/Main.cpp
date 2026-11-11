//Proyecto Final
//Equipo 9
//319205459, 319276017
//Grupo 7
//Fecha de entrega 11 de noviembre de 2026

#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <cstdlib>
#include <thread>
#include <csignal>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"
#include "stb_easy_font.h" 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SOIL2/SOIL2.h"
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

#include "DanzaPescadores.h"
#include "Mariposas.h"
#include "Personaje.h"

// ───────────── Callbacks ─────────────
void KeyCallback(GLFWwindow*, int, int, int, int);
void MouseCallback(GLFWwindow*, double, double);
void DoMovement();

void playSound(const std::string& filename) {
#ifdef _WIN32
    std::thread([filename]() {
        std::string cmd = "powershell -c (New-Object Media.SoundPlayer '" + filename + "').PlaySync();";
        std::system(cmd.c_str());
    }).detach();
#elif __APPLE__
    std::string cmd = "afplay " + filename + " > /dev/null 2>&1 &";
    std::system(cmd.c_str());
#else
    std::string cmd = "aplay " + filename + " > /dev/null 2>&1 &";
    std::system(cmd.c_str());
#endif
}

void cleanupAudio() {
#ifdef __APPLE__
    std::system("pkill -f 'afplay.*campana' 2>/dev/null");
    std::system("pkill -f 'afplay.*janitzio' 2>/dev/null");
#endif
}

extern "C" void handleSigint(int) {
    exit(0);   
}

// ───────────── Configuración ─────────────
const GLuint WIDTH = 1600, HEIGHT = 900;
int SCREEN_WIDTH, SCREEN_HEIGHT;

Camera  camera(glm::vec3(-2.014f, 1.199f, -1.36796f)); 
GLfloat lastX = WIDTH / 2.0f;
GLfloat lastY = HEIGHT / 2.0f;
bool    keys[1024];
bool    firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// ── Velas que se envian al shader cada frame ──
const int NUM_FLAME_LIGHTS_CPU = 32;

// ═══════════════════════════════════════════════════════════
// SISTEMA DE CÁMARAS Y SUELO
// ═══════════════════════════════════════════════════════════
enum CameraMode { FIRST_PERSON, THIRD_PERSON_GTA, THIRD_PERSON_RESTRICTED, THIRD_PERSON_FREE };
const int TOTAL_CAMERA_MODES = 4;
CameraMode currentCameraMode = FIRST_PERSON;

glm::vec3 playerPos = glm::vec3(-2.63651f, 0.699024f, -1.4178f);
const float eyeHeight = 0.5f; 
const glm::vec3 STAIRS_START = glm::vec3(-2.63651f, 0.699024f, -1.4178f);

// ── Camara estilo GTA ──
const float GTA_DISTANCIA       = 1.10f;
const float GTA_DISTANCIA_MIN   = 0.30f;
const float GTA_ALTURA_OBJETIVO = 0.28f;
const float GTA_PITCH_MIN       = -50.0f;
const float GTA_PITCH_MAX       =  15.0f;
const float GTA_PITCH_INICIAL   = -15.0f;
const float PERSONAJE_RADIO     = 0.08f;

// ── Ambiente nocturno ──
const float LUZ_ESCENARIO = 0.45f;
const float BRILLO_CIELO  = 0.55f;
const float LUZ_PISO      = 1.00f;

// ── Luna (posicion unica para el modelo y el spotlight) ──
const glm::vec3 LUNA_POSICION(-10.0f, 18.0f, -20.0f);

Personaje::Estado personaje;

// ── FUNCIÓN DEL SUELO ──
float getFloorHeight(float x, float z) {
    const float MIN_X = -2.79187f;
    const float MAX_X = 0.064214f;
    const float MIN_Z = -1.97263f;  
    const float MAX_Z = -0.724842f; 

    if (x < MIN_X || x > MAX_X || z < MIN_Z || z > MAX_Z) {
        return 0.0f;
    }

    if (x <= -1.02154f) {
        return 0.699024f;
    }
    if (x > -1.02154f && x <= -0.612181f) {
        float t = (x - (-1.02154f)) / (-0.612181f - (-1.02154f));
        return 0.699024f + t * (0.299492f - 0.699024f);
    }
    if (x > -0.612181f && x <= MAX_X) {
        float t = (x - (-0.612181f)) / (MAX_X - (-0.612181f));
        return 0.299492f + t * (0.0f - 0.299492f);
    }
    return 0.0f;
}

// ═══════════════════════════════════════════════════════════
// COLISIONES CON PAREDES
// ═══════════════════════════════════════════════════════════
struct Pared { glm::vec2 a, b; };

const float PLAYER_RADIUS = 0.15f;

const std::vector<Pared> PAREDES = {
    { glm::vec2(-1.93240f, -0.724842f), glm::vec2(-1.93240f, -0.594801f) },
    { glm::vec2(-1.93240f, -0.594801f), glm::vec2(-2.00648f, -0.181302f) },
    { glm::vec2(-2.00648f, -0.181302f), glm::vec2(-2.00648f,  0.162231f) },
    { glm::vec2(-2.00648f,  0.162231f), glm::vec2(-1.93240f,  0.575730f) },
    { glm::vec2(-1.93240f,  0.575730f), glm::vec2(-2.05728f,  0.974691f) },
    { glm::vec2(-2.05728f,  0.974691f), glm::vec2(-0.214722f, 1.149100f) },
    { glm::vec2(-0.214722f, 1.149100f), glm::vec2( 2.53337f,  1.385660f) },
    { glm::vec2( 2.53337f,  1.385660f), glm::vec2( 3.94579f,  1.600840f) },
    { glm::vec2( 3.94579f,  1.600840f), glm::vec2( 4.45445f,  1.600840f) },
    { glm::vec2( 4.45445f,  1.600840f), glm::vec2( 4.45445f, -1.809860f) },
    { glm::vec2( 4.45445f, -1.809860f), glm::vec2( 0.064214f, -1.809860f) },
    { glm::vec2( 0.064214f, -1.809860f), glm::vec2( 0.064214f, -1.97263f) },
    { glm::vec2( 0.064214f, -1.97263f),  glm::vec2(-2.79187f,  -1.97263f) },
    { glm::vec2(-2.79187f,  -1.97263f),  glm::vec2(-2.79187f,  -0.724842f) },
    { glm::vec2(-2.79187f,  -0.724842f), glm::vec2( 0.064214f, -0.724842f) }
};

static glm::vec2 closestPointOnSegment(const glm::vec2& p, const glm::vec2& a, const glm::vec2& b)
{
    glm::vec2 ab = b - a;
    float len2 = glm::dot(ab, ab);
    if (len2 < 1e-8f) return a;
    float t = glm::clamp(glm::dot(p - a, ab) / len2, 0.0f, 1.0f);
    return a + t * ab;
}

static void resolveWallCollisions(glm::vec3& pos, float radius = PLAYER_RADIUS)
{
    glm::vec2 p(pos.x, pos.z);
    for (int iter = 0; iter < 3; ++iter) {
        for (const Pared& w : PAREDES) {
            glm::vec2 c = closestPointOnSegment(p, w.a, w.b);
            glm::vec2 d = p - c;
            float dist = glm::length(d);
            if (dist < radius) {
                if (dist > 1e-5f) {
                    p = c + d / dist * radius;
                } else {
                    glm::vec2 e = w.b - w.a;
                    glm::vec2 n = glm::normalize(glm::vec2(-e.y, e.x));
                    p = c + n * radius;
                }
            }
        }
    }
    pos.x = p.x;
    pos.z = p.y;
}

static glm::vec3 posicionCamaraGTA(const glm::vec3& target, const glm::vec3& front)
{
    glm::vec3 dir = -front;
    glm::vec2 o(target.x, target.z);
    glm::vec2 d(dir.x * GTA_DISTANCIA, dir.z * GTA_DISTANCIA);

    float tMin = 1.0f;
    for (const Pared& w : PAREDES) {
        glm::vec2 e  = w.b - w.a;
        glm::vec2 ao = w.a - o;
        float den = d.x * e.y - d.y * e.x;
        if (std::fabs(den) < 1e-8f) continue;
        float t = (ao.x * e.y - ao.y * e.x) / den;
        float u = (ao.x * d.y - ao.y * d.x) / den;
        if (t > 0.0f && t < tMin && u >= 0.0f && u <= 1.0f) tMin = t;
    }
    float dist = GTA_DISTANCIA;
    if (tMin < 1.0f) dist = std::max(GTA_DISTANCIA_MIN, tMin * GTA_DISTANCIA - 0.06f);

    glm::vec3 pos = target + dir * dist;

    float minY = getFloorHeight(pos.x, pos.z) + 0.06f;
    if (pos.y < minY) pos.y = minY;
    return pos;
}

// ═══════════════════════════════════════════════════════════
// CAMPANA
// ═══════════════════════════════════════════════════════════
const glm::vec3 BELL_POSITION    = glm::vec3(-2.0059f, 1.5588f, -1.3899f);
const glm::vec3 CLAPPER_POSITION = glm::vec3(-2.0064f, 1.4668f, -1.3872f);
const float     BELL_PIVOT_HEIGHT = 0.4f;
const glm::vec3 BELL_PIVOT        = BELL_POSITION + glm::vec3(0.0f, BELL_PIVOT_HEIGHT, 0.0f);
const float BELL_NEAR_DISTANCE  = 3.0f;
const float BELL_SWING_DURATION = 19.6f;
const float BELL_TOLL_TIMES[] = { 0.0f, 2.7f, 4.8f, 8.5f, 11.5f, 14.0f };
const int   BELL_TOLL_COUNT   = 6;
bool  bellSwingActive    = false;
float bellSwingStartTime = 0.0f;

// ═══════════════════════════════════════════════════════════
// TUMBAS INTERACTIVAS
// ═══════════════════════════════════════════════════════════
struct TumbaInteractiva {
    glm::vec3 posicion;
    int tecla;          
    std::string dato;
};

const TumbaInteractiva TUMBAS[10] = {
    { glm::vec3( 0.693586f, -0.000725f, -0.411303f), GLFW_KEY_1,
      "1. Para los purepechas, los muertos no se van para siempre: regresan cada noviembre. Esperarlos es decirles que siguen siendo parte de la familia." },

    { glm::vec3( 1.177570f, -0.015488f, -1.487300f), GLFW_KEY_2,
      "2. En Janitzio la espera se llama velacion. Las familias pasan la noche en vela junto a la tumba para recibir a las almas y acompanarlas hasta el amanecer." },

    { glm::vec3( 2.391750f,  0.014538f, -0.450766f), GLFW_KEY_3,
      "3. Las velas encendidas son la luz que guia a las almas desde el otro mundo hasta su tumba. Mientras mas velas, mas facil encuentran el camino." },

    { glm::vec3( 2.637270f,  0.006454f, -1.515720f), GLFW_KEY_4,
      "4. La flor de cempasuchil, con su color y aroma intensos, marca el camino de las almas. Sus petalos se riegan desde la entrada hasta la tumba." },

    { glm::vec3( 3.177310f, -0.001129f, -1.463610f), GLFW_KEY_5,
      "5. La ofrenda lleva la comida y la bebida favoritas del difunto. Se cree que las almas no comen: se alimentan del aroma y la esencia de los alimentos." },

    { glm::vec3( 3.492020f,  0.005087f, -0.969270f), GLFW_KEY_6,
      "6. El 1 de noviembre se espera a las almas de los ninos, los angelitos, con juguetes, dulces y atole. El 2 de noviembre llegan las de los adultos." },

    { glm::vec3( 3.93916f,   0.0f,      0.8662f),    GLFW_KEY_7,
      "7. El copal es un incienso prehispanico. Su humo limpia el ambiente y sirve de puente para que las almas lleguen a la ofrenda." },

    { glm::vec3( 2.990680f,  0.003724f,  1.186180f), GLFW_KEY_8,
      "8. La Danza de los Pescadores es una tradicion purepecha que rinde homenaje al lago y a quienes vivieron de el, incluidos los que ya no estan." },

    { glm::vec3( 1.962760f,  0.008751f,  0.746883f), GLFW_KEY_9,
      "9. Las mariposas monarca llegan a Michoacan por estas fechas, y muchas personas creen que son las almas de los difuntos que vienen de visita." },

    { glm::vec3( 0.418023f, -0.079952f,  0.732683f), GLFW_KEY_0,
      "10. Recordar a quienes se fueron es la esencia del Dia de Muertos: mientras alguien los espere con una ofrenda, siguen vivos en la memoria de su gente." }
};

std::string mensajeActivo = "";
float mensajeTimer = 0.0f;
const float DISTANCIA_TUMBA = 0.9f;

// ═══════════════════════════════════════════════════════════
// PESCADORES
// ═══════════════════════════════════════════════════════════
struct PescadorPose {
    glm::vec3 lancha, cabeza, torso, hombroIzq, codoIzq, hombroDer, codoDer;
    glm::vec3 cadera, piernaIzq, piernaDer, red;
};

const PescadorPose PESCADORES[3] = {
    {
        glm::vec3( 3.27893f, -2.29082f, 9.68666f), glm::vec3( 3.29460f, -1.89427f, 9.69836f),
        glm::vec3( 3.29252f, -2.05658f, 9.69458f), glm::vec3( 3.35132f, -1.97485f, 9.71129f),
        glm::vec3( 3.36824f, -2.00691f, 9.70975f), glm::vec3( 3.23667f, -1.96439f, 9.71338f),
        glm::vec3( 3.22378f, -2.00329f, 9.71241f), glm::vec3( 3.29318f, -2.05586f, 9.69561f),
        glm::vec3( 3.33037f, -2.17894f, 9.69834f), glm::vec3( 3.25794f, -2.17927f, 9.69636f),
        glm::vec3( 3.21595f, -2.10868f, 9.69014f)
    },
    {
        glm::vec3( 1.13466f, -2.29082f, 8.54121f), glm::vec3( 1.15033f, -1.89427f, 8.55291f),
        glm::vec3( 1.14825f, -2.05658f, 8.54913f), glm::vec3( 1.20705f, -1.97485f, 8.56584f),
        glm::vec3( 1.22397f, -2.00691f, 8.56430f), glm::vec3( 1.09240f, -1.96439f, 8.56793f),
        glm::vec3( 1.07951f, -2.00329f, 8.56696f), glm::vec3( 1.14891f, -2.05586f, 8.55016f),
        glm::vec3( 1.18610f, -2.17894f, 8.55290f), glm::vec3( 1.11367f, -2.17927f, 8.55091f),
        glm::vec3( 1.07168f, -2.10868f, 8.54469f)
    },
    {
        glm::vec3(-0.61301f, -2.29082f, 9.68666f), glm::vec3(-0.59734f, -1.89427f, 9.69836f),
        glm::vec3(-0.59942f, -2.05658f, 9.69458f), glm::vec3(-0.54062f, -1.97485f, 9.71129f),
        glm::vec3(-0.52370f, -2.00691f, 9.70975f), glm::vec3(-0.65527f, -1.96439f, 9.71338f),
        glm::vec3(-0.66816f, -2.00329f, 9.71241f), glm::vec3(-0.59875f, -2.05586f, 9.69561f),
        glm::vec3(-0.56157f, -2.17894f, 9.69834f), glm::vec3(-0.63400f, -2.17927f, 9.69636f),
        glm::vec3(-0.67599f, -2.10868f, 9.69014f)
    }
};
const int TOTAL_PESCADORES = 3;

// ═══════════════════════════════════════════════════════════
// VELADORAS
// ═══════════════════════════════════════════════════════════
const float VELADORA_OPACIDAD = 0.55f;

const glm::vec3 VELADORAS_AZULES[] = {
    glm::vec3(3.8508f, 0.0447f, 1.2191f),
    glm::vec3(4.0498f, 0.0447f, 1.2191f),
    glm::vec3(1.2253f, 0.1644f, -0.2572f),
    glm::vec3(2.0575f, 0.0476f, 0.4577f),
    glm::vec3(1.2253f, 0.1644f, -0.5958f),
    glm::vec3(1.0680f, 0.0772f, -0.5958f),
    glm::vec3(1.0680f, 0.0772f, -0.2602f),
    glm::vec3(0.8767f, 0.0772f, -0.2602f),
    glm::vec3(0.8767f, 0.0772f, -0.5853f),
    glm::vec3(1.9333f, 0.0476f, 0.4577f),
    glm::vec3(3.8508f, 0.0447f, 0.8833f),
    glm::vec3(4.0498f, 0.0447f, 0.8833f),
    glm::vec3(2.0575f, 0.0720f, 0.8924f),
    glm::vec3(1.9333f, 0.0720f, 0.8924f)
};

const glm::vec3 VELADORAS_ROJAS[] = {
    glm::vec3(-0.0962f, 0.1513f, 0.6829f),
    glm::vec3(0.2375f, 0.0655f, 0.9972f),
    glm::vec3(0.0569f, 0.0655f, 0.6723f),
    glm::vec3(0.0569f, 0.0655f, 0.9986f),
    glm::vec3(0.3328f, -0.0131f, 1.0077f),
    glm::vec3(0.4179f, -0.0131f, 1.0077f),
    glm::vec3(0.5069f, -0.0131f, 1.0077f),
    glm::vec3(0.5858f, -0.0131f, 1.0077f),
    glm::vec3(0.6710f, -0.0131f, 1.0077f),
    glm::vec3(0.7599f, -0.0131f, 1.0077f),
    glm::vec3(0.3328f, -0.0131f, 0.6536f),
    glm::vec3(0.4179f, -0.0131f, 0.6536f),
    glm::vec3(0.5069f, -0.0131f, 0.6536f),
    glm::vec3(0.5858f, -0.0131f, 0.6536f),
    glm::vec3(0.6710f, -0.0131f, 0.6536f),
    glm::vec3(0.7599f, -0.0131f, 0.6536f),
    glm::vec3(0.8858f, -0.0131f, 0.9682f),
    glm::vec3(0.8858f, -0.0131f, 0.8393f),
    glm::vec3(0.8858f, -0.0131f, 0.7091f),
    glm::vec3(0.2375f, 0.0655f, 0.6723f),
    glm::vec3(-0.0962f, 0.1513f, 0.9972f)
};

const glm::vec3 VELADORAS2_ROJAS[] = {
    glm::vec3(3.9173f, 0.0211f, -1.3924f),
    glm::vec3(3.8521f, 0.0211f, -1.3924f),
    glm::vec3(3.7830f, 0.0211f, -1.3924f),
    glm::vec3(3.7268f, 0.0211f, -1.3924f),
    glm::vec3(3.8521f, 0.0211f, -1.4525f),
    glm::vec3(3.8521f, 0.0211f, -1.3360f),
    glm::vec3(2.5280f, 0.0070f, -1.2567f),
    glm::vec3(2.6131f, 0.0070f, -1.2567f),
    glm::vec3(2.7021f, 0.0070f, -1.2567f),
    glm::vec3(2.7898f, 0.0070f, -1.2567f),
    glm::vec3(2.8749f, 0.0070f, -1.2567f),
    glm::vec3(2.9639f, 0.0070f, -1.2567f),
    glm::vec3(3.0937f, 0.0070f, -1.2567f),
    glm::vec3(3.2078f, 0.0070f, -1.3095f),
    glm::vec3(3.2259f, 0.0070f, -1.4630f),
    glm::vec3(3.2259f, 0.0070f, -1.5599f),
    glm::vec3(3.2259f, 0.0070f, -1.6742f),
    glm::vec3(3.2259f, 0.0070f, -1.7625f),
    glm::vec3(2.8478f, 0.0619f, -1.5610f),
    glm::vec3(2.9129f, 0.0619f, -1.5610f),
    glm::vec3(2.9821f, 0.0619f, -1.5610f),
    glm::vec3(3.0382f, 0.0471f, -1.5610f),
    glm::vec3(2.9129f, 0.0619f, -1.5008f),
    glm::vec3(2.9129f, 0.0619f, -1.6174f)
};

const glm::vec3 VELADORAS_AMARILLAS[] = {
    glm::vec3(2.2622f, -0.0068f, -0.2605f),
    glm::vec3(2.3925f, -0.0068f, -0.2605f),
    glm::vec3(2.5239f, -0.0068f, -0.2605f),
    glm::vec3(2.6840f, -0.0068f, -0.2605f),
    glm::vec3(2.7445f, -0.0068f, -0.3216f),
    glm::vec3(2.7445f, -0.0068f, -0.4533f),
    glm::vec3(2.7445f, -0.0068f, -0.5814f),
    glm::vec3(2.2622f, -0.0068f, -0.6618f),
    glm::vec3(2.3925f, -0.0068f, -0.6618f),
    glm::vec3(2.5239f, -0.0068f, -0.6618f),
    glm::vec3(2.6840f, -0.0068f, -0.6618f),
    glm::vec3(3.7965f, -0.0068f, -0.4637f),
    glm::vec3(3.9232f, -0.0068f, -0.4637f),
    glm::vec3(4.0778f, -0.0068f, -0.4637f),
    glm::vec3(4.2045f, -0.0068f, -0.4637f),
    glm::vec3(4.0778f, -0.0068f, 0.1149f),
    glm::vec3(4.2045f, -0.0068f, 0.1149f),
    glm::vec3(3.7968f, -0.0068f, 0.1149f),
    glm::vec3(3.9235f, -0.0068f, 0.1149f),
    glm::vec3(3.2156f, 0.0077f, 1.2081f),
    glm::vec3(3.2650f, 0.0077f, 1.2081f),
    glm::vec3(3.3165f, 0.0077f, 1.2081f),
    glm::vec3(3.3599f, 0.0077f, 1.2081f),
    glm::vec3(3.2650f, 0.0077f, 1.1526f),
    glm::vec3(3.2650f, 0.0077f, 1.2632f),
    glm::vec3(2.2622f, 0.0119f, -0.6618f),
    glm::vec3(0.9310f, -0.0020f, -1.4412f),
    glm::vec3(0.9310f, -0.0020f, -1.4884f),
    glm::vec3(0.9310f, -0.0020f, -1.5369f),
    glm::vec3(0.9310f, -0.0020f, -1.5825f),
    glm::vec3(0.9310f, -0.0020f, -1.3912f),
    glm::vec3(0.8855f, -0.0020f, -1.3912f),
    glm::vec3(0.8377f, -0.0020f, -1.3912f),
    glm::vec3(0.8347f, -0.0020f, -1.4412f),
    glm::vec3(0.8347f, -0.0020f, -1.4884f),
    glm::vec3(0.8347f, -0.0020f, -1.5369f),
    glm::vec3(0.8347f, -0.0020f, -1.5825f),
    glm::vec3(0.8855f, -0.0020f, -1.5843f)
};

// ═══════════════════════════════════════════════════════════
// FLAMAS
// ═══════════════════════════════════════════════════════════
const glm::vec3 FLAMAS_NUEVAS[] = {
    glm::vec3(-0.0961f, 0.1930f, 0.6831f),
    glm::vec3(-0.0961f, 0.1930f, 0.9971f),
    glm::vec3(0.0567f, 0.1072f, 0.9971f),
    glm::vec3(0.2375f, 0.1077f, 0.9971f),
    glm::vec3(0.2375f, 0.1077f, 0.6717f),
    glm::vec3(0.0567f, 0.1077f, 0.6717f),
    glm::vec3(0.3327f, 0.0291f, 1.0076f),
    glm::vec3(0.4178f, 0.0291f, 1.0076f),
    glm::vec3(0.5068f, 0.0291f, 1.0076f),
    glm::vec3(0.5858f, 0.0291f, 1.0076f),
    glm::vec3(0.6709f, 0.0291f, 1.0076f),
    glm::vec3(0.7599f, 0.0291f, 1.0076f),
    glm::vec3(0.3327f, 0.0291f, 0.6536f),
    glm::vec3(0.4178f, 0.0291f, 0.6536f),
    glm::vec3(0.5068f, 0.0291f, 0.6536f),
    glm::vec3(0.5858f, 0.0291f, 0.6536f),
    glm::vec3(0.6709f, 0.0291f, 0.6536f),
    glm::vec3(0.7599f, 0.0291f, 0.6536f),
    glm::vec3(0.8858f, 0.0291f, 0.9682f),
    glm::vec3(0.8858f, 0.0291f, 0.8392f),
    glm::vec3(0.8858f, 0.0291f, 0.7091f),
    glm::vec3(3.8509f, 0.1141f, 1.2192f),
    glm::vec3(4.0503f, 0.1141f, 1.2192f),
    glm::vec3(1.2254f, 0.2338f, -0.2571f),
    glm::vec3(2.0580f, 0.1170f, 0.4577f),
    glm::vec3(2.2623f, 0.0626f, -0.2605f),
    glm::vec3(2.3926f, 0.0626f, -0.2605f),
    glm::vec3(2.5241f, 0.0626f, -0.2605f),
    glm::vec3(2.6841f, 0.0626f, -0.2605f),
    glm::vec3(2.7446f, 0.0626f, -0.3215f),
    glm::vec3(2.7446f, 0.0626f, -0.4532f),
    glm::vec3(2.7446f, 0.0626f, -0.5813f),
    glm::vec3(2.2623f, 0.0626f, -0.6617f),
    glm::vec3(2.3926f, 0.0626f, -0.6617f),
    glm::vec3(2.5241f, 0.0626f, -0.6617f),
    glm::vec3(2.6841f, 0.0626f, -0.6617f),
    glm::vec3(3.9172f, 0.0432f, -1.3925f),
    glm::vec3(3.8521f, 0.0432f, -1.3925f),
    glm::vec3(3.7829f, 0.0432f, -1.3925f),
    glm::vec3(3.7268f, 0.0432f, -1.3925f),
    glm::vec3(3.8521f, 0.0432f, -1.4526f),
    glm::vec3(3.8521f, 0.0432f, -1.3360f),
    glm::vec3(1.2254f, 0.2338f, -0.5957f),
    glm::vec3(1.0682f, 0.1466f, -0.5957f),
    glm::vec3(1.0682f, 0.1466f, -0.2601f),
    glm::vec3(0.8769f, 0.1466f, -0.2601f),
    glm::vec3(0.8769f, 0.1466f, -0.5852f),
    glm::vec3(1.9338f, 0.1170f, 0.4577f),
    glm::vec3(3.7966f, 0.0626f, -0.4637f),
    glm::vec3(3.9233f, 0.0626f, -0.4637f),
    glm::vec3(4.0779f, 0.0626f, -0.4637f),
    glm::vec3(4.2046f, 0.0626f, -0.4637f),
    glm::vec3(4.0779f, 0.0626f, 0.1149f),
    glm::vec3(4.2046f, 0.0626f, 0.1149f),
    glm::vec3(3.7969f, 0.0626f, 0.1149f),
    glm::vec3(3.9236f, 0.0626f, 0.1149f),
    glm::vec3(2.5279f, 0.0291f, -1.2568f),
    glm::vec3(2.6130f, 0.0291f, -1.2568f),
    glm::vec3(2.7020f, 0.0291f, -1.2568f),
    glm::vec3(2.7897f, 0.0291f, -1.2568f),
    glm::vec3(2.8748f, 0.0291f, -1.2568f),
    glm::vec3(2.9638f, 0.0291f, -1.2568f),
    glm::vec3(3.0936f, 0.0291f, -1.2568f),
    glm::vec3(3.2077f, 0.0291f, -1.3095f),
    glm::vec3(3.2258f, 0.0291f, -1.4630f),
    glm::vec3(3.2258f, 0.0291f, -1.5599f),
    glm::vec3(3.2258f, 0.0291f, -1.6743f),
    glm::vec3(3.2258f, 0.0291f, -1.7626f),
    glm::vec3(2.8479f, 0.0840f, -1.5609f),
    glm::vec3(2.9130f, 0.0840f, -1.5609f),
    glm::vec3(2.9822f, 0.0840f, -1.5609f),
    glm::vec3(3.0383f, 0.0692f, -1.5609f),
    glm::vec3(2.9130f, 0.0840f, -1.5008f),
    glm::vec3(2.9130f, 0.0840f, -1.6173f),
    glm::vec3(3.8509f, 0.1141f, 0.8834f),
    glm::vec3(4.0503f, 0.1141f, 0.8834f),
    glm::vec3(2.0580f, 0.1414f, 0.8925f),
    glm::vec3(1.9338f, 0.1414f, 0.8925f),
    glm::vec3(3.2157f, 0.0771f, 1.2081f),
    glm::vec3(3.2651f, 0.0771f, 1.2081f),
    glm::vec3(3.3166f, 0.0771f, 1.2081f),
    glm::vec3(3.3601f, 0.0771f, 1.2081f),
    glm::vec3(3.2651f, 0.0771f, 1.2632f),
    glm::vec3(3.2651f, 0.0771f, 1.1527f),
    glm::vec3(0.8381f, 0.0660f, -1.3915f),
    glm::vec3(0.8857f, 0.0662f, -1.3907f),
    glm::vec3(0.9314f, 0.0662f, -1.3907f),
    glm::vec3(0.8347f, 0.0660f, -1.4888f),
    glm::vec3(0.9314f, 0.0662f, -1.4880f),
    glm::vec3(0.8347f, 0.0660f, -1.5368f),
    glm::vec3(0.9314f, 0.0662f, -1.5360f),
    glm::vec3(0.8347f, 0.0660f, -1.5823f),
    glm::vec3(0.9314f, 0.0662f, -1.5815f),
    glm::vec3(0.8859f, 0.0662f, -1.5839f),
    glm::vec3(0.8347f, 0.0660f, -1.4411f),
    glm::vec3(0.9314f, 0.0662f, -1.4408f)
};

const glm::vec3 FLAMAS_ANTERIORES[] = {
    glm::vec3(0.000f, 0.224f, 0.554f), glm::vec3(0.463f, 0.224f, 0.554f), glm::vec3(0.823f, 0.224f, 0.554f),
    glm::vec3(0.836f, 0.224f, 1.057f), glm::vec3(0.463f, 0.224f, 1.058f), glm::vec3(0.027f, 0.224f, 1.067f),
    glm::vec3(1.702f, 0.224f, 0.423f), glm::vec3(1.702f, 0.224f, 0.704f), glm::vec3(1.702f, 0.224f, 0.959f),
    glm::vec3(2.298f, 0.224f, 0.423f), glm::vec3(2.298f, 0.224f, 0.704f), glm::vec3(2.298f, 0.224f, 0.959f),
    glm::vec3(2.846f, 0.224f, 0.927f), glm::vec3(3.127f, 0.224f, 0.927f), glm::vec3(3.382f, 0.224f, 0.927f),
    glm::vec3(2.846f, 0.224f, 1.418f), glm::vec3(3.127f, 0.224f, 1.436f), glm::vec3(3.382f, 0.224f, 1.436f),
    glm::vec3(3.757f, 0.224f, 1.316f), glm::vec3(3.757f, 0.224f, 1.035f), glm::vec3(3.757f, 0.224f, 0.780f),
    glm::vec3(4.154f, 0.224f, 1.316f), glm::vec3(4.154f, 0.224f, 1.035f), glm::vec3(4.154f, 0.224f, 0.780f),
    glm::vec3(2.743f, 0.224f, -0.258f), glm::vec3(2.462f, 0.224f, -0.258f), glm::vec3(2.207f, 0.224f, -0.258f),
    glm::vec3(2.743f, 0.224f, -0.655f), glm::vec3(2.462f, 0.224f, -0.655f), glm::vec3(2.207f, 0.224f, -0.655f),
    glm::vec3(3.731f, 0.224f, 0.127f), glm::vec3(4.011f, 0.224f, 0.127f), glm::vec3(4.267f, 0.224f, 0.127f),
    glm::vec3(3.731f, 0.224f, -0.469f), glm::vec3(4.011f, 0.224f, -0.469f), glm::vec3(4.267f, 0.224f, -0.469f),
    glm::vec3(3.023f, 0.224f, -1.263f), glm::vec3(2.743f, 0.224f, -1.263f), glm::vec3(2.487f, 0.224f, -1.263f),
    glm::vec3(3.215f, 0.224f, -1.424f), glm::vec3(3.215f, 0.224f, -1.633f), glm::vec3(1.330f, 0.224f, -1.237f),
    glm::vec3(1.049f, 0.224f, -1.237f), glm::vec3(0.794f, 0.224f, -1.237f), glm::vec3(1.330f, 0.224f, -1.736f),
    glm::vec3(1.049f, 0.224f, -1.721f), glm::vec3(0.794f, 0.224f, -1.721f), glm::vec3(0.342f, 0.224f, -0.149f),
    glm::vec3(0.622f, 0.224f, -0.149f), glm::vec3(0.878f, 0.224f, -0.149f), glm::vec3(-1.818f, 0.224f, 0.376f),
    glm::vec3(0.342f, 0.224f, -0.694f), glm::vec3(0.622f, 0.224f, -0.694f), glm::vec3(0.878f, 0.224f, -0.694f),
    glm::vec3(-1.818f, 0.224f, -0.375f), glm::vec3(0.632f, 0.224f, -1.584f), glm::vec3(0.637f, 0.224f, -1.432f),
    glm::vec3(-1.578f, 0.224f, 0.376f), glm::vec3(-1.578f, 0.224f, -0.375f), glm::vec3(-1.322f, 0.224f, 0.376f),
    glm::vec3(-1.322f, 0.224f, -0.375f), glm::vec3(-1.082f, 0.224f, 0.376f), glm::vec3(-1.082f, 0.224f, -0.375f),
    glm::vec3(-0.846f, 0.224f, 0.376f), glm::vec3(-0.846f, 0.224f, -0.375f), glm::vec3(-0.600f, 0.224f, 0.376f),
    glm::vec3(-0.600f, 0.224f, -0.375f), glm::vec3(4.192f, 0.224f, -1.136f), glm::vec3(3.912f, 0.224f, -1.136f),
    glm::vec3(3.656f, 0.224f, -1.136f), glm::vec3(4.192f, 0.224f, -1.653f), glm::vec3(3.912f, 0.224f, -1.653f),
    glm::vec3(3.656f, 0.224f, -1.653f)
};

std::vector<glm::vec3> posicionesFlamas;
int totalFlamas = 0;

static void construirFlamas()
{
    posicionesFlamas.assign(std::begin(FLAMAS_ANTERIORES), std::end(FLAMAS_ANTERIORES));
    posicionesFlamas.insert(posicionesFlamas.end(), std::begin(FLAMAS_NUEVAS), std::end(FLAMAS_NUEVAS));
    totalFlamas = (int)posicionesFlamas.size();
}

// ───────────── Helpers ─────────────
static void setVec3 (GLuint p, const std::string& n, float x, float y, float z)
{ glUniform3f(glGetUniformLocation(p, n.c_str()), x, y, z); }
static void setFloat(GLuint p, const std::string& n, float v)
{ glUniform1f(glGetUniformLocation(p, n.c_str()), v); }
static void setEmission(GLuint p, float r, float g, float b)
{ glUniform3f(glGetUniformLocation(p, "emissive"), r, g, b); }

std::string wrapText(const std::string& text, float maxWidth)
{
    std::istringstream iss(text);
    std::string word, line, result;
    while (iss >> word) {
        std::string test = line.empty() ? word : line + " " + word;
        if (!line.empty() && stb_easy_font_width((char*)test.c_str()) > maxWidth) {
            result += line + "\n";
            line = word;
        } else {
            line = test;
        }
    }
    result += line;
    return result;
}

// ═══════════════════════════════════════════════════════════
// MAIN
// ═══════════════════════════════════════════════════════════
int main()
{
    atexit(cleanupAudio);
    signal(SIGINT, handleSigint);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Proyecto final equipo 9 - 319205459, 319276017", nullptr, nullptr);
    if (!window) { std::cout << "Failed GLFW window\n"; glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { std::cout << "Failed GLEW\n"; return -1; }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
    Shader textShader("Shader/text.vs", "Shader/text.frag");

    // Modelos
    Model cielo    ((char*)"ModelsProyectos/cielo.obj");
    Model escenario((char*)"ModelsProyectos/terreno.obj");
    Model flama    ((char*)"ModelsProyectos/flama.obj");
    Model luna     ((char*)"ModelsProyectos/Moon.obj");
    Model puerta1  ((char*)"ModelsProyectos/puerta1.obj");
    Model puerta2  ((char*)"ModelsProyectos/puerta2.obj");
    Model campana  ((char*)"ModelsProyectos/campana.obj");
    Model badoja   ((char*)"ModelsProyectos/badajo.obj");
    Model lancha     ((char*)"ModelsProyectos/lancha.obj");
    Model cabeza     ((char*)"ModelsProyectos/cabeza.obj");
    Model torso      ((char*)"ModelsProyectos/torso.obj");
    Model hombroIzq  ((char*)"ModelsProyectos/hombro-izquierdo.obj");
    Model codoIzq    ((char*)"ModelsProyectos/codo-izquierdo.obj");
    Model hombroDer  ((char*)"ModelsProyectos/hombro-derecho.obj");
    Model codoDer    ((char*)"ModelsProyectos/codo-derecho.obj");
    Model cadera     ((char*)"ModelsProyectos/cadera.obj");
    Model piernaIzq  ((char*)"ModelsProyectos/pierna-izquierda.obj");
    Model piernaDer  ((char*)"ModelsProyectos/pierna-derecha.obj");
    Model redPesc    ((char*)"ModelsProyectos/red.obj");

    Model velAzul     ((char*)"ModelsProyectos/veladora-azul.obj");
    Model velRoja     ((char*)"ModelsProyectos/veladora-roja.obj");
    Model velRoja2    ((char*)"ModelsProyectos/veladora2-roja.obj");
    Model velAmarilla ((char*)"ModelsProyectos/veladora-amarilla.obj");

    Model mariposaCuerpo ((char*)"ModelsProyectos/cuerpo-mariposa.obj");
    Model mariposaAlaDer ((char*)"ModelsProyectos/ala-derecha.obj");
    Model mariposaAlaIzq ((char*)"ModelsProyectos/ala-izquierda.obj");

    Model pjTorso       ((char*)"ModelsProyectos/torso-ca.obj");
    Model pjHombroDer   ((char*)"ModelsProyectos/hombro-derecho-ca.obj");
    Model pjHombroIzq   ((char*)"ModelsProyectos/hombro-izquierdo-ca.obj");
    Model pjCodoDer     ((char*)"ModelsProyectos/codo-derecho-ca.obj");
    Model pjCodoIzq     ((char*)"ModelsProyectos/codo-izquierdo-ca.obj");
    Model pjPiernaDer   ((char*)"ModelsProyectos/pierna-derecha-ca.obj");
    Model pjPiernaIzq   ((char*)"ModelsProyectos/pierna-izquierda-ca.obj");
    Model pjRodillaDer  ((char*)"ModelsProyectos/rodilla-derecha-ca.obj");
    Model pjRodillaIzq  ((char*)"ModelsProyectos/rodilla-izquierda-ca.obj");

    lightingShader.Use();
    glUniform1i(glGetUniformLocation(lightingShader.Program, "material.diffuse"),  0);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "material.specular"), 1);

    construirFlamas();

    {
        GLint nUniforms = 0;
        glGetProgramiv(lightingShader.Program, GL_ACTIVE_UNIFORMS, &nUniforms);
        for (GLint i = 0; i < nUniforms; ++i) {
            char name[128]; GLsizei len = 0; GLint size = 0; GLenum type = 0;
            glGetActiveUniform(lightingShader.Program, i, sizeof(name), &len, &size, &type, name);
            if (std::string(name).rfind("flamePos", 0) == 0 && size < NUM_FLAME_LIGHTS_CPU) {
                std::cout << "AVISO: lighting.frag solo tiene espacio para " << size
                          << " flamas pero se envian " << NUM_FLAME_LIGHTS_CPU << std::endl;
            }
        }
    }

    GLuint textVAO, textVBO, textEBO;
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glGenBuffers(1, &textEBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, 99999, NULL, GL_DYNAMIC_DRAW);
    std::vector<unsigned int> indices(9999 * 6);
    for(int i = 0; i < 9999; i++) {
        indices[i*6 + 0] = i*4 + 0; indices[i*6 + 1] = i*4 + 1; indices[i*6 + 2] = i*4 + 2;
        indices[i*6 + 3] = i*4 + 2; indices[i*6 + 4] = i*4 + 3; indices[i*6 + 5] = i*4 + 0;
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, (void*)0);
    glBindVertexArray(0);

    std::vector<Mariposas::Mariposa> mariposas = Mariposas::crear();

    std::vector<std::pair<float, int>> flameDists;
    flameDists.reserve(totalFlamas);
    glm::vec3 nearestFlames[NUM_FLAME_LIGHTS_CPU];

    // ── Textura de relleno para la unidad 1 (especular) ──
    // Si un modelo no tiene textura especular, se usa este pixel negro
    // para que el shader lea negro en lugar de "nada".
    GLuint fallbackSpecular;
    {
        glGenTextures(1, &fallbackSpecular);
        glBindTexture(GL_TEXTURE_2D, fallbackSpecular);
        unsigned char blackPixel[4] = {0, 0, 0, 255};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, blackPixel);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    camera.SetYaw(0.0f);

    while (!glfwWindowShouldClose(window))
    {
        GLfloat now = (GLfloat)glfwGetTime();
        deltaTime = now - lastFrame;
        lastFrame = now;

        glfwPollEvents();
        DoMovement();

        glm::vec3 renderCamPos;
        glm::mat4 view;
        glm::vec3 front = camera.GetFront();

        if (currentCameraMode == FIRST_PERSON) {
            renderCamPos = playerPos + glm::vec3(0.0f, eyeHeight, 0.0f);
            view = glm::lookAt(renderCamPos, renderCamPos + front, glm::vec3(0.0f, 1.0f, 0.0f));
        } else if (currentCameraMode == THIRD_PERSON_GTA) {
            glm::vec3 target = playerPos + glm::vec3(0.0f, GTA_ALTURA_OBJETIVO, 0.0f);
            renderCamPos = posicionCamaraGTA(target, front);
            view = glm::lookAt(renderCamPos, target, glm::vec3(0.0f, 1.0f, 0.0f));
        } else {
            glm::vec3 target = playerPos + glm::vec3(0.0f, eyeHeight, 0.0f);
            renderCamPos = target - front * 3.0f + glm::vec3(0.0f, 0.5f, 0.0f);
            view = glm::lookAt(renderCamPos, target, glm::vec3(0.0f, 1.0f, 0.0f));
        }

        glClearColor(0.02f, 0.03f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Asegurar que la unidad 1 siempre tenga algo valido
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, fallbackSpecular);
        glActiveTexture(GL_TEXTURE0);

        lightingShader.Use();
        GLuint prog = lightingShader.Program;

        setVec3(prog, "viewPos", renderCamPos.x, renderCamPos.y, renderCamPos.z);

        // ── Luz direccional (luz de luna sobre el escenario) ──
        setVec3 (prog, "dirLight.direction", -0.2f, -1.0f, -0.3f);
        setVec3 (prog, "dirLight.ambient",    0.10f * LUZ_ESCENARIO, 0.15f * LUZ_ESCENARIO, 0.25f * LUZ_ESCENARIO);
        setVec3 (prog, "dirLight.diffuse",    0.30f * LUZ_ESCENARIO, 0.35f * LUZ_ESCENARIO, 0.55f * LUZ_ESCENARIO);
        setVec3 (prog, "dirLight.specular",   0.20f * LUZ_ESCENARIO, 0.25f * LUZ_ESCENARIO, 0.35f * LUZ_ESCENARIO);

        // ── SpotLight de la luna ──
        glm::vec3 lunaTarget( 2.0f, 0.0f, 0.0f);
        glm::vec3 lunaDir = glm::normalize(lunaTarget - LUNA_POSICION);
        setVec3 (prog, "spotLight.position",    LUNA_POSICION.x, LUNA_POSICION.y, LUNA_POSICION.z);
        setVec3 (prog, "spotLight.direction",   lunaDir.x, lunaDir.y, lunaDir.z);
        setFloat(prog, "spotLight.cutOff",      glm::cos(glm::radians(30.0f)));
        setFloat(prog, "spotLight.outerCutOff", glm::cos(glm::radians(45.0f)));
        setFloat(prog, "spotLight.constant",    1.0f);
        setFloat(prog, "spotLight.linear",      0.005f);
        setFloat(prog, "spotLight.quadratic",   0.0005f);
        setVec3 (prog, "spotLight.ambient",     0.08f, 0.10f, 0.18f);
        setVec3 (prog, "spotLight.diffuse",     0.80f, 0.90f, 1.10f);
        setVec3 (prog, "spotLight.specular",    0.30f, 0.40f, 0.60f);

        flameDists.clear();
        for (int i = 0; i < totalFlamas; ++i) {
            glm::vec3 d = renderCamPos - posicionesFlamas[i];
            flameDists.push_back({ d.x * d.x + d.y * d.y + d.z * d.z, i });
        }
        std::partial_sort(flameDists.begin(),
            flameDists.begin() + NUM_FLAME_LIGHTS_CPU,
            flameDists.end());

        for (int i = 0; i < NUM_FLAME_LIGHTS_CPU; ++i)
            nearestFlames[i] = posicionesFlamas[flameDists[i].second];

        glUniform3fv(glGetUniformLocation(prog, "flamePos"),
            NUM_FLAME_LIGHTS_CPU, glm::value_ptr(nearestFlames[0]));
        glUniform1f(glGetUniformLocation(prog, "uTime"), now);

        setFloat(prog, "material.shininess", 32.0f);

        glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 500.0f);
        glUniformMatrix4fv(glGetUniformLocation(prog, "view"),       1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(prog, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        GLint modelLoc = glGetUniformLocation(prog, "model");
        GLint transLoc = glGetUniformLocation(prog, "transparency");

        setVec3(prog, "floorGlow", 0.12f * LUZ_PISO, 0.05f * LUZ_PISO, 0.02f * LUZ_PISO);

        // CIELO NOCTURNO
        glDisable(GL_DEPTH_TEST);
        setEmission(prog, BRILLO_CIELO, BRILLO_CIELO, BRILLO_CIELO);
        glUniform1i(transLoc, 0);

        glm::mat4 modelCielo = glm::mat4(1.0f);
        modelCielo = glm::translate(modelCielo, renderCamPos);
        modelCielo = glm::scale(modelCielo, glm::vec3(20.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCielo));
        cielo.Draw(lightingShader);

        glEnable(GL_DEPTH_TEST);

        // ESCENARIO
        setEmission(prog, 0.0f, 0.0f, 0.0f);
        glUniform1i(transLoc, 1);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        escenario.Draw(lightingShader);

        // FLAMAS
        setEmission(prog, 0.85f, 0.40f, 0.10f);
        for (int i = 0; i < totalFlamas; ++i) {
            float t = now + i * 0.35f;
            float pulso = std::sin(t * 5.1f) * 0.15f + std::sin(t * 7.7f) * 0.08f + std::sin(t * 12.3f) * 0.03f;
            float escalaY  = 1.0f + pulso;
            float escalaXZ = 1.0f - pulso * 0.3f;
            float tiltX = std::sin(t * 3.3f) * 0.25f + std::sin(t * 5.9f) * 0.10f;
            float tiltZ = std::cos(t * 3.1f) * 0.20f + std::cos(t * 6.7f) * 0.08f;

            glm::mat4 mLlama = glm::mat4(1.0f);
            mLlama = glm::translate(mLlama, posicionesFlamas[i]);
            mLlama = glm::rotate(mLlama, tiltX, glm::vec3(1, 0, 0));
            mLlama = glm::rotate(mLlama, tiltZ, glm::vec3(0, 0, 1));
            mLlama = glm::scale  (mLlama, glm::vec3(escalaXZ, escalaY, escalaXZ));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mLlama));
            flama.Draw(lightingShader);
        }

        // LUNA (con brillo propio)
        setEmission(prog, 0.85f, 0.90f, 1.05f);
        glUniform1i(transLoc, 0);
        model = glm::mat4(1.0f);
        model = glm::translate(model, LUNA_POSICION);
        model = glm::scale    (model, glm::vec3(0.6f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        luna.Draw(lightingShader);

        // PUERTAS
        setEmission(prog, 0.0f, 0.0f, 0.0f);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.014f, 1.0331f, -1.7952f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        puerta1.Draw(lightingShader);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.0125f, 1.0337f, -0.988f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        puerta2.Draw(lightingShader);

        // CAMPANA
        float bellAngle = 0.0f, clapperAngle = 0.0f;
        if (bellSwingActive) {
            float t = now - bellSwingStartTime;
            if (t > BELL_SWING_DURATION) bellSwingActive = false;
            else {
                float lastToll = BELL_TOLL_TIMES[0];
                for (int i = 0; i < BELL_TOLL_COUNT; ++i) {
                    if (t >= BELL_TOLL_TIMES[i]) lastToll = BELL_TOLL_TIMES[i];
                    else break;
                }
                float localT = t - lastToll;
                float env    = std::exp(-1.3f * localT);
                bellAngle    = 0.35f * std::sin(2.2f * localT) * env;
                clapperAngle = bellAngle + 0.08f * std::sin(6.5f * localT) * env;
            }
        }
        model = glm::mat4(1.0f);
        model = glm::translate(model, BELL_PIVOT);
        model = glm::rotate   (model, bellAngle, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, BELL_POSITION - BELL_PIVOT);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        campana.Draw(lightingShader);
        model = glm::mat4(1.0f);
        model = glm::translate(model, BELL_PIVOT);
        model = glm::rotate   (model, clapperAngle, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, CLAPPER_POSITION - BELL_PIVOT);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        badoja.Draw(lightingShader);

        // PESCADORES
        Model* piezasModelos[10] = {
            &cadera, &torso, &cabeza, &hombroIzq, &codoIzq,
            &hombroDer, &codoDer, &piernaIzq, &piernaDer, &redPesc
        };

        const float delay = 0.0f;
        const float phase = 0.0f;
        Danza::Pose pose    = Danza::evaluate(now, delay, phase);
        Danza::Pose lagPose = Danza::evaluate(now - 0.05f, delay, phase);

        for (int i = 0; i < TOTAL_PESCADORES; ++i) {
            Danza::Mats mats = Danza::build(PESCADORES[i], pose, lagPose);

            glm::mat4 M[10] = {
                mats.cadera, mats.torso, mats.cabeza,
                mats.hombroIzq, mats.codoIzq, mats.hombroDer, mats.codoDer,
                mats.piernaIzq, mats.piernaDer, mats.red
            };

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mats.lancha));
            lancha.Draw(lightingShader);

            for (int k = 0; k < 10; ++k) {
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(M[k]));
                piezasModelos[k]->Draw(lightingShader);
            }
        }

        // MARIPOSAS
        setEmission(prog, 0.12f, 0.08f, 0.04f);
        glUniform1i(transLoc, 1);
        for (Mariposas::Mariposa& m : mariposas) {
            Mariposas::actualizar(m, deltaTime);
            Mariposas::Mats mm = Mariposas::construir(m, now);

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mm.cuerpo));
            mariposaCuerpo.Draw(lightingShader);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mm.alaDer));
            mariposaAlaDer.Draw(lightingShader);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mm.alaIzq));
            mariposaAlaIzq.Draw(lightingShader);
        }
        setEmission(prog, 0.0f, 0.0f, 0.0f);

        // PERSONAJE (solo con la camara GTA)
        if (currentCameraMode == THIRD_PERSON_GTA) {
            setEmission(prog, 0.05f, 0.045f, 0.04f);
            glUniform1i(transLoc, 0);

            Personaje::Mats pm = Personaje::construir(playerPos, personaje);
            const glm::mat4 PM[9] = {
                pm.torso, pm.hombroDer, pm.hombroIzq, pm.codoDer, pm.codoIzq,
                pm.piernaDer, pm.piernaIzq, pm.rodillaDer, pm.rodillaIzq
            };
            Model* piezasPJ[9] = {
                &pjTorso, &pjHombroDer, &pjHombroIzq, &pjCodoDer, &pjCodoIzq,
                &pjPiernaDer, &pjPiernaIzq, &pjRodillaDer, &pjRodillaIzq
            };
            for (int k = 0; k < 9; ++k) {
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(PM[k]));
                piezasPJ[k]->Draw(lightingShader);
            }
            setEmission(prog, 0.0f, 0.0f, 0.0f);
        }

        // VELADORAS (translucidas)
        setEmission(prog, 0.0f, 0.0f, 0.0f);
        glUniform1i(transLoc, 1);
        glEnable(GL_BLEND);
        glBlendColor(0.0f, 0.0f, 0.0f, VELADORA_OPACIDAD);
        glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
        glDepthMask(GL_FALSE);

        auto dibujarVeladoras = [&](Model& m, const glm::vec3* posiciones, int cantidad) {
            for (int i = 0; i < cantidad; ++i) {
                glm::mat4 mv = glm::translate(glm::mat4(1.0f), posiciones[i]);
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mv));
                m.Draw(lightingShader);
            }
        };
        dibujarVeladoras(velAzul,     VELADORAS_AZULES,    (int)(sizeof(VELADORAS_AZULES)    / sizeof(glm::vec3)));
        dibujarVeladoras(velRoja,     VELADORAS_ROJAS,     (int)(sizeof(VELADORAS_ROJAS)     / sizeof(glm::vec3)));
        dibujarVeladoras(velRoja2,    VELADORAS2_ROJAS,    (int)(sizeof(VELADORAS2_ROJAS)    / sizeof(glm::vec3)));
        dibujarVeladoras(velAmarilla, VELADORAS_AMARILLAS, (int)(sizeof(VELADORAS_AMARILLAS) / sizeof(glm::vec3)));

        glDepthMask(GL_TRUE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // TEXTO UI
        if (mensajeTimer > 0.0f) {
            mensajeTimer -= deltaTime;
            
            glm::mat4 textProj = glm::ortho(0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0.0f, -1.0f, 1.0f);
            textShader.Use();
            glUniformMatrix4fv(glGetUniformLocation(textShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(textProj));
            glUniform2f(glGetUniformLocation(textShader.Program, "scale"), 3.0f, 3.0f);
            glUniform3f(glGetUniformLocation(textShader.Program, "textColor"), 1.0f, 1.0f, 1.0f);
            
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            static char buffer[99999]; 
            int num_quads = stb_easy_font_print(20.0f, 20.0f, (char*)mensajeActivo.c_str(), NULL, buffer, sizeof(buffer));

            glBindVertexArray(textVAO);
            glBindBuffer(GL_ARRAY_BUFFER, textVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, num_quads * 4 * 16, buffer);
            glDrawElements(GL_TRIANGLES, num_quads * 6, GL_UNSIGNED_INT, 0);
            
            glBindVertexArray(0);
            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
        }

        glDisable(GL_BLEND);
        glfwSwapBuffers(window);
    }

    glfwTerminate();

    cleanupAudio();

    return 0;
}

// ───────────── Controles ─────────────
void DoMovement()
{
    float velocity = 2.5f * deltaTime;
    glm::vec3 f = camera.GetFront();

    if (currentCameraMode == THIRD_PERSON_FREE) {
        float vel = 3.0f * deltaTime;
        glm::vec3 fwd = glm::normalize(f);
        glm::vec3 rgt = glm::normalize(glm::cross(fwd, glm::vec3(0.0f, 1.0f, 0.0f)));

        if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])    playerPos += fwd * vel;
        if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])  playerPos -= fwd * vel;
        if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])  playerPos -= rgt * vel;
        if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) playerPos += rgt * vel;
        if (keys[GLFW_KEY_SPACE])      playerPos.y += vel;
        if (keys[GLFW_KEY_LEFT_SHIFT]) playerPos.y -= vel;
        return;
    }

    if (currentCameraMode == THIRD_PERSON_GTA) {
        glm::vec3 fwd = glm::normalize(glm::vec3(f.x, 0.0f, f.z));
        glm::vec3 rgt = glm::normalize(glm::cross(fwd, glm::vec3(0.0f, 1.0f, 0.0f)));

        glm::vec3 dir(0.0f);
        if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])    dir += fwd;
        if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])  dir -= fwd;
        if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])  dir -= rgt;
        if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) dir += rgt;

        glm::vec3 antes = playerPos;

        if (glm::length(dir) > 0.001f) {
            dir = glm::normalize(dir);
            glm::vec3 move = dir * Personaje::VEL_CAMINAR * deltaTime;

            int steps = std::max(1, (int)std::ceil(glm::length(move) / 0.02f));
            for (int i = 0; i < steps; ++i) {
                playerPos += move / (float)steps;
                resolveWallCollisions(playerPos, PERSONAJE_RADIO);
            }

            Personaje::girarHacia(personaje, std::atan2(-dir.z, dir.x), deltaTime);
        }

        playerPos.y = getFloorHeight(playerPos.x, playerPos.z);

        float recorrido = glm::length(glm::vec2(playerPos.x - antes.x, playerPos.z - antes.z));
        Personaje::avanzarAnimacion(personaje, recorrido, deltaTime);
        return;
    }

    glm::vec3 forward = glm::normalize(glm::vec3(f.x, 0.0f, f.z));
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));

    glm::vec3 move(0.0f);
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])    move += forward * velocity;
    if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])  move -= forward * velocity;
    if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])  move -= right * velocity;
    if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) move += right * velocity;

    int steps = std::max(1, (int)std::ceil(glm::length(move) / 0.05f));
    for (int i = 0; i < steps; ++i) {
        playerPos += move / (float)steps;
        resolveWallCollisions(playerPos);
    }

    if (currentCameraMode == THIRD_PERSON_RESTRICTED) {
        playerPos.x = glm::clamp(playerPos.x, -3.5f, 4.5f);
        playerPos.z = glm::clamp(playerPos.z, -2.5f, 2.0f);
    }

    playerPos.y = getFloorHeight(playerPos.x, playerPos.z);
}

void KeyCallback(GLFWwindow* w, int key, int, int action, int)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(w, GL_TRUE);
    }

    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        currentCameraMode = (CameraMode)((currentCameraMode + 1) % TOTAL_CAMERA_MODES);

        if (currentCameraMode == THIRD_PERSON_GTA) {
            playerPos = Personaje::INICIO;
            personaje = Personaje::Estado();
            camera.SetPitchLimits(GTA_PITCH_MIN, GTA_PITCH_MAX);
            camera.SetYaw(0.0f);
            camera.SetPitch(GTA_PITCH_INICIAL);
            std::cout << "Camara: Tercera Persona estilo GTA." << std::endl;
        } else {
            camera.SetPitchLimits(-89.0f, 89.0f);

            if (currentCameraMode == THIRD_PERSON_FREE) {
                playerPos = STAIRS_START;
                camera.SetYaw(0.0f); 
                std::cout << "Camara: Tercera Persona Libre." << std::endl;
            } else if (currentCameraMode == THIRD_PERSON_RESTRICTED) {
                std::cout << "Camara: Tercera Persona Restringida." << std::endl;
            } else {
                playerPos = STAIRS_START;
                std::cout << "Camara: Primera Persona" << std::endl;
            }
        }
    }

    if (key == GLFW_KEY_E && action == GLFW_PRESS) {
        // Si la campana ya esta sonando, ignorar la tecla
        if (bellSwingActive) {
            std::cout << "La campana ya esta sonando..." << std::endl;
            return;
        }

        glm::vec3 playerHead = playerPos + glm::vec3(0.0f, eyeHeight, 0.0f);
        if (glm::length(playerHead - BELL_POSITION) < BELL_NEAR_DISTANCE) {
            bellSwingActive    = true;
            bellSwingStartTime = (float)glfwGetTime();
            playSound("Sounds/campana.wav");
        }
    }

    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        std::cout << "playerPos: " << playerPos.x << ", "
                  << playerPos.y << ", " << playerPos.z << std::endl;
    }

    if (action == GLFW_PRESS) {
        int teclasTumbas[10] = { GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5, 
                                 GLFW_KEY_6, GLFW_KEY_7, GLFW_KEY_8, GLFW_KEY_9, GLFW_KEY_0 };
        
        for (int i = 0; i < 10; ++i) {
            if (key == teclasTumbas[i]) {
                glm::vec2 playerXZ(playerPos.x, playerPos.z);
                glm::vec2 tumbaXZ(TUMBAS[i].posicion.x, TUMBAS[i].posicion.z);
                
                float dist = glm::length(playerXZ - tumbaXZ);
                
                if (dist < DISTANCIA_TUMBA) {
                    mensajeActivo = wrapText(TUMBAS[i].dato, (SCREEN_WIDTH - 120) / 3.0f);
                    mensajeTimer = 10.0f;
                } else {
                    std::cout << "Muy lejos de la Tumba " << (i+1) 
                              << ". Distancia actual: " << dist 
                              << " (Necesitas estar a menos de " << DISTANCIA_TUMBA << ")" << std::endl;
                }
            }
        }
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)        keys[key] = true;
        else if (action == GLFW_RELEASE) keys[key] = false;
    }
}

void MouseCallback(GLFWwindow*, double xPos, double yPos)
{
    if (firstMouse) { lastX = xPos; lastY = yPos; firstMouse = false; }
    GLfloat xo = xPos - lastX;
    GLfloat yo = lastY - yPos;
    lastX = xPos; lastY = yPos;
    camera.ProcessMouseMovement(xo, yo);
}
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdlib>                    // ← AÑADIDO: para std::system

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SOIL2/SOIL2.h"
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

// ───────────── Callbacks ─────────────
void KeyCallback(GLFWwindow*, int, int, int, int);
void MouseCallback(GLFWwindow*, double, double);
void DoMovement();

// ───────────── Configuración ─────────────
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

Camera  camera(glm::vec3(0.0f, 1.0f, 5.0f));
GLfloat lastX = WIDTH / 2.0f;
GLfloat lastY = HEIGHT / 2.0f;
bool    keys[1024];
bool    firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

#define NUM_FLAME_LIGHTS 8    // debe coincidir con el #define del .frag

// ═══════════════════════════════════════════════════════════
// CAMPANA — posición, pivote y estado del balanceo
// ═══════════════════════════════════════════════════════════
const glm::vec3 BELL_POSITION    = glm::vec3(-2.0059f, 1.5588f, -1.3899f);
const glm::vec3 CLAPPER_POSITION = glm::vec3(-2.0064f, 1.4668f, -1.3872f);

const float     BELL_PIVOT_HEIGHT = 0.4f;
const glm::vec3 BELL_PIVOT        = BELL_POSITION + glm::vec3(0.0f, BELL_PIVOT_HEIGHT, 0.0f);

const float BELL_NEAR_DISTANCE  = 3.0f;   // ← AÑADIDA: distancia para poder tocar la campana
const float BELL_SWING_DURATION = 17.0f;  // duración total del balanceo

// Tiempos exactos de cada campanazo (medidos del audio)
const float BELL_TOLL_TIMES[] = { 0.0f, 2.8f, 5.8f, 8.2f, 11.3f, 14.0f };
const int   BELL_TOLL_COUNT   = 6;

bool  bellSwingActive    = false;
float bellSwingStartTime = 0.0f;

// ═══════════════════════════════════════════════════════════
// POSICIONES EXACTAS DE LAS FLAMAS
// ═══════════════════════════════════════════════════════════
glm::vec3 posicionesFlamas[] = {
    glm::vec3(0.000f, 0.224f, 0.554f),
    glm::vec3(0.463f, 0.224f, 0.554f),
    glm::vec3(0.823f, 0.224f, 0.554f),
    glm::vec3(0.836f, 0.224f, 1.057f),
    glm::vec3(0.463f, 0.224f, 1.058f),
    glm::vec3(0.027f, 0.224f, 1.067f),
    glm::vec3(1.702f, 0.224f, 0.423f),
    glm::vec3(1.702f, 0.224f, 0.704f),
    glm::vec3(1.702f, 0.224f, 0.959f),
    glm::vec3(2.298f, 0.224f, 0.423f),
    glm::vec3(2.298f, 0.224f, 0.704f),
    glm::vec3(2.298f, 0.224f, 0.959f),
    glm::vec3(2.846f, 0.224f, 0.927f),
    glm::vec3(3.127f, 0.224f, 0.927f),
    glm::vec3(3.382f, 0.224f, 0.927f),
    glm::vec3(2.846f, 0.224f, 1.418f),
    glm::vec3(3.127f, 0.224f, 1.436f),
    glm::vec3(3.382f, 0.224f, 1.436f),
    glm::vec3(3.757f, 0.224f, 1.316f),
    glm::vec3(3.757f, 0.224f, 1.035f),
    glm::vec3(3.757f, 0.224f, 0.780f),
    glm::vec3(4.154f, 0.224f, 1.316f),
    glm::vec3(4.154f, 0.224f, 1.035f),
    glm::vec3(4.154f, 0.224f, 0.780f),
    glm::vec3(2.743f, 0.224f, -0.258f),
    glm::vec3(2.462f, 0.224f, -0.258f),
    glm::vec3(2.207f, 0.224f, -0.258f),
    glm::vec3(2.743f, 0.224f, -0.655f),
    glm::vec3(2.462f, 0.224f, -0.655f),
    glm::vec3(2.207f, 0.224f, -0.655f),
    glm::vec3(3.731f, 0.224f, 0.127f),
    glm::vec3(4.011f, 0.224f, 0.127f),
    glm::vec3(4.267f, 0.224f, 0.127f),
    glm::vec3(3.731f, 0.224f, -0.469f),
    glm::vec3(4.011f, 0.224f, -0.469f),
    glm::vec3(4.267f, 0.224f, -0.469f),
    glm::vec3(3.023f, 0.224f, -1.263f),
    glm::vec3(2.743f, 0.224f, -1.263f),
    glm::vec3(2.487f, 0.224f, -1.263f),
    glm::vec3(3.215f, 0.224f, -1.424f),
    glm::vec3(3.215f, 0.224f, -1.633f),
    glm::vec3(1.330f, 0.224f, -1.237f),
    glm::vec3(1.049f, 0.224f, -1.237f),
    glm::vec3(0.794f, 0.224f, -1.237f),
    glm::vec3(1.330f, 0.224f, -1.736f),
    glm::vec3(1.049f, 0.224f, -1.721f),
    glm::vec3(0.794f, 0.224f, -1.721f),
    glm::vec3(0.342f, 0.224f, -0.149f),
    glm::vec3(0.622f, 0.224f, -0.149f),
    glm::vec3(0.878f, 0.224f, -0.149f),
    glm::vec3(-1.818f, 0.224f, 0.376f),
    glm::vec3(0.342f, 0.224f, -0.694f),
    glm::vec3(0.622f, 0.224f, -0.694f),
    glm::vec3(0.878f, 0.224f, -0.694f),
    glm::vec3(-1.818f, 0.224f, -0.375f),
    glm::vec3(0.632f, 0.224f, -1.584f),
    glm::vec3(0.637f, 0.224f, -1.432f),
    glm::vec3(-1.578f, 0.224f, 0.376f),
    glm::vec3(-1.578f, 0.224f, -0.375f),
    glm::vec3(-1.322f, 0.224f, 0.376f),
    glm::vec3(-1.322f, 0.224f, -0.375f),
    glm::vec3(-1.082f, 0.224f, 0.376f),
    glm::vec3(-1.082f, 0.224f, -0.375f),
    glm::vec3(-0.846f, 0.224f, 0.376f),
    glm::vec3(-0.846f, 0.224f, -0.375f),
    glm::vec3(-0.600f, 0.224f, 0.376f),
    glm::vec3(-0.600f, 0.224f, -0.375f),
    glm::vec3(4.192f, 0.224f, -1.136f),
    glm::vec3(3.912f, 0.224f, -1.136f),
    glm::vec3(3.656f, 0.224f, -1.136f),
    glm::vec3(4.192f, 0.224f, -1.653f),
    glm::vec3(3.912f, 0.224f, -1.653f),
    glm::vec3(3.656f, 0.224f, -1.653f)
};
int totalFlamas = sizeof(posicionesFlamas) / sizeof(glm::vec3);

// ───────────── Helpers para uniforms ─────────────
static void setVec3 (GLuint p, const std::string& n, float x, float y, float z)
{ glUniform3f(glGetUniformLocation(p, n.c_str()), x, y, z); }

static void setFloat(GLuint p, const std::string& n, float v)
{ glUniform1f(glGetUniformLocation(p, n.c_str()), v); }

static void setEmission(GLuint p, float r, float g, float b)
{ glUniform3f(glGetUniformLocation(p, "emissive"), r, g, b); }

// ═══════════════════════════════════════════════════════════
// MAIN
// ═══════════════════════════════════════════════════════════
int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT,
                                          "Proyecto final equipo 9",
                                          nullptr, nullptr);
    if (!window) { std::cout << "Failed GLFW window\n"; glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { std::cout << "Failed GLEW\n"; return -1; }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");

    // ── Modelos ──
    Model escenario((char*)"ModelsProyectos/prueba.obj");
    Model flama    ((char*)"ModelsProyectos/flama.obj");
    Model luna     ((char*)"ModelsProyectos/Moon.obj");
    Model puerta1  ((char*)"ModelsProyectos/puerta1.obj");
    Model puerta2  ((char*)"ModelsProyectos/puerta2.obj");
    Model campana  ((char*)"ModelsProyectos/campana.obj");
    Model badoja   ((char*)"ModelsProyectos/badajo.obj");

    lightingShader.Use();
    glUniform1i(glGetUniformLocation(lightingShader.Program, "material.diffuse"),  0);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "material.specular"), 1);

    std::cout << "Flamas totales: " << totalFlamas << "\n";

    // ═══════════════════════ GAME LOOP ═══════════════════════
    while (!glfwWindowShouldClose(window))
    {
        GLfloat now = (GLfloat)glfwGetTime();
        deltaTime = now - lastFrame;
        lastFrame = now;

        glfwPollEvents();
        DoMovement();

        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        lightingShader.Use();
        GLuint prog = lightingShader.Program;

        // ── Cámara ──
        glm::vec3 camPos = camera.GetPosition();
        setVec3(prog, "viewPos", camPos.x, camPos.y, camPos.z);

        // ── Luz direccional (luna) ──
        setVec3 (prog, "dirLight.direction", -0.2f, -1.0f, -0.3f);
        setVec3 (prog, "dirLight.ambient",    0.10f, 0.15f, 0.25f);
        setVec3 (prog, "dirLight.diffuse",    0.30f, 0.35f, 0.55f);
        setVec3 (prog, "dirLight.specular",   0.20f, 0.25f, 0.35f);

        // ── SpotLight de la luna (linterna azulada suave) ──
        // NOTA: si mueves el modelo de la luna, sincroniza también lunaPos aquí
        glm::vec3 lunaPos   (-10.0f, 5.0f, -12.0f);
        glm::vec3 lunaTarget(  2.0f, 0.0f,   0.0f);
        glm::vec3 lunaDir = glm::normalize(lunaTarget - lunaPos);

        setVec3 (prog, "spotLight.position",    lunaPos.x, lunaPos.y, lunaPos.z);
        setVec3 (prog, "spotLight.direction",   lunaDir.x, lunaDir.y, lunaDir.z);
        setFloat(prog, "spotLight.cutOff",      glm::cos(glm::radians(30.0f)));
        setFloat(prog, "spotLight.outerCutOff", glm::cos(glm::radians(45.0f)));
        setFloat(prog, "spotLight.constant",    1.0f);
        setFloat(prog, "spotLight.linear",      0.005f);
        setFloat(prog, "spotLight.quadratic",   0.0005f);
        setVec3 (prog, "spotLight.ambient",     0.08f, 0.10f, 0.18f);
        setVec3 (prog, "spotLight.diffuse",     0.80f, 0.90f, 1.10f);
        setVec3 (prog, "spotLight.specular",    0.30f, 0.40f, 0.60f);

        // ── Point lights: 8 flamas más cercanas a la cámara ──
        {
            std::vector<std::pair<float,int>> ord;
            ord.reserve(totalFlamas);
            for (int i = 0; i < totalFlamas; ++i)
                ord.emplace_back(glm::distance(camPos, posicionesFlamas[i]), i);

            int k = std::min<int>(NUM_FLAME_LIGHTS, totalFlamas);
            std::partial_sort(ord.begin(), ord.begin() + k, ord.end());

            for (int j = 0; j < NUM_FLAME_LIGHTS; ++j) {
                std::string base = "pointLights[" + std::to_string(j) + "]";
                int idx = (j < (int)ord.size()) ? ord[j].second : 0;
                glm::vec3 p = posicionesFlamas[idx];

                float seed    = now + idx * 0.35f;
                float flicker = 0.85f
                              + 0.10f * std::sin(seed * 5.1f)
                              + 0.05f * std::sin(seed * 12.3f);

                setVec3 (prog, base + ".position",  p.x, p.y + 0.08f, p.z);
                setFloat(prog, base + ".constant",  1.0f);
                setFloat(prog, base + ".linear",    0.20f);
                setFloat(prog, base + ".quadratic", 0.18f);
                setVec3 (prog, base + ".ambient",   0.20f*flicker, 0.06f*flicker, 0.00f);
                setVec3 (prog, base + ".diffuse",   1.20f*flicker, 0.55f*flicker, 0.15f*flicker);
                setVec3 (prog, base + ".specular",  1.00f*flicker, 0.60f*flicker, 0.20f*flicker);
            }
        }

        setFloat(prog, "material.shininess", 32.0f);

        // ── Matrices ──
        glm::mat4 projection = glm::perspective(camera.GetZoom(),
            (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 500.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glUniformMatrix4fv(glGetUniformLocation(prog, "view"),       1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(prog, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        GLint modelLoc = glGetUniformLocation(prog, "model");
        GLint transLoc = glGetUniformLocation(prog, "transparency");

        // ══════ ESCENARIO ══════
        setEmission(prog, 0.0f, 0.0f, 0.0f);
        glUniform1i(transLoc, 1);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        escenario.Draw(lightingShader);

        // ══════ FLAMAS ANIMADAS ══════
        setEmission(prog, 0.0f, 0.0f, 0.0f);
        glUniform1i(transLoc, 1);

        for (int i = 0; i < totalFlamas; ++i)
        {
            float t = now + i * 0.35f;

            float pulso = std::sin(t * 5.1f) * 0.15f
                        + std::sin(t * 7.7f) * 0.08f
                        + std::sin(t * 12.3f) * 0.03f;

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

        // ══════ LUNA (opaca, con brillo propio) ══════
        setEmission(prog, 0.85f, 0.90f, 1.05f);
        glUniform1i(transLoc, 0);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-10.0f, 18.0f, -20.0f));
        model = glm::scale    (model, glm::vec3(0.6f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        luna.Draw(lightingShader);

        // ══════ PUERTAS ══════
        setEmission(prog, 0.0f, 0.0f, 0.0f);
        glUniform1i(transLoc, 0);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.014f, 1.0331f, -1.7952f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        puerta1.Draw(lightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.0125f, 1.0337f, -0.988f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        puerta2.Draw(lightingShader);

        // ══════ CAMPANA Y BADAJO (con balanceo) ══════
        setEmission(prog, 0.0f, 0.0f, 0.0f);
        glUniform1i(transLoc, 0);

        float bellAngle    = 0.0f;
        float clapperAngle = 0.0f;

        if (bellSwingActive) {
            float t = now - bellSwingStartTime;
            if (t > BELL_SWING_DURATION) {
                bellSwingActive = false;
            } else {
                // Último campanazo que ya ocurrió
                float lastToll = BELL_TOLL_TIMES[0];
                for (int i = 0; i < BELL_TOLL_COUNT; ++i) {
                    if (t >= BELL_TOLL_TIMES[i]) lastToll = BELL_TOLL_TIMES[i];
                    else break;
                }

                float localT = t - lastToll;
                float env    = std::exp(-1.3f * localT);

                bellAngle    = 0.35f * std::sin(2.2f * localT) * env;

                // El badajo sigue a la campana + oscila ligeramente adentro
                float clapperLag = 0.08f;   // cuánto se separa del centro de la campana
                clapperAngle = bellAngle + clapperLag * std::sin(6.5f * localT) * env;
            }
        }

        // ── Campana: rotar alrededor del pivote (eje Z) ──
        model = glm::mat4(1.0f);
        model = glm::translate(model, BELL_PIVOT);
        model = glm::rotate   (model, bellAngle, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, BELL_POSITION - BELL_PIVOT);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        campana.Draw(lightingShader);

        // ── Badajo: mismo pivote, ángulo desfasado ──
        model = glm::mat4(1.0f);
        model = glm::translate(model, BELL_PIVOT);
        model = glm::rotate   (model, clapperAngle, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, CLAPPER_POSITION - BELL_PIVOT);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        badoja.Draw(lightingShader);

        glDisable(GL_BLEND);
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

// ───────────── Controles ─────────────
void DoMovement()
{
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])    camera.ProcessKeyboard(FORWARD,  deltaTime);
    if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])  camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])  camera.ProcessKeyboard(LEFT,     deltaTime);
    if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) camera.ProcessKeyboard(RIGHT,    deltaTime);
}

void KeyCallback(GLFWwindow* w, int key, int, int action, int)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(w, GL_TRUE);

    // ── E: tocar la campana si estás cerca ──
    if (key == GLFW_KEY_E && action == GLFW_PRESS) {
        float dist = glm::length(camera.GetPosition() - BELL_POSITION);
        if (dist < BELL_NEAR_DISTANCE) {
            bellSwingActive    = true;
            bellSwingStartTime = (float)glfwGetTime();
            std::system("afplay Sounds/campana.wav > /dev/null 2>&1 &");
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
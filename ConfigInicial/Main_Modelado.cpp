//Practica 4
//Guillermo Espinoza Sánchez
//Fecha de entrega: 10 de agosto de 2026
//Número de cuenta: 319205459

#include<iostream>

//#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Shaders
#include "Shader.h"

void Inputs(GLFWwindow* window);

const GLint WIDTH = 800, HEIGHT = 600;
float movX = 0.0f;
float movY = 0.0f;
float movZ = -5.0f;
float rot = 0.0f;

int main() {
    glfwInit();

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Modelado geometrico - Guillermo Espinoza Sánchez", nullptr, nullptr);

    int screenWidth, screenHeight;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

    if (nullptr == window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;

    if (GLEW_OK != glewInit()) {
        std::cout << "Failed to initialise GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    glViewport(0, 0, screenWidth, screenHeight);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader ourShader("Shader/core.vs", "Shader/core.frag");

    // AMARILLO (1.0f, 0.8f, 0.0f)
    float verticesAmarillo[] = {
        -0.5f, -0.5f, 0.5f, 1.0f, 0.8f, 0.0f,   0.5f, -0.5f, 0.5f, 1.0f, 0.8f, 0.0f,   0.5f,  0.5f, 0.5f, 1.0f, 0.8f, 0.0f,
         0.5f,  0.5f, 0.5f, 1.0f, 0.8f, 0.0f,  -0.5f,  0.5f, 0.5f, 1.0f, 0.8f, 0.0f,  -0.5f, -0.5f, 0.5f, 1.0f, 0.8f, 0.0f,
        -0.5f, -0.5f,-0.5f, 1.0f, 0.8f, 0.0f,   0.5f, -0.5f,-0.5f, 1.0f, 0.8f, 0.0f,   0.5f,  0.5f,-0.5f, 1.0f, 0.8f, 0.0f,
         0.5f,  0.5f,-0.5f, 1.0f, 0.8f, 0.0f,  -0.5f,  0.5f,-0.5f, 1.0f, 0.8f, 0.0f,  -0.5f, -0.5f,-0.5f, 1.0f, 0.8f, 0.0f,
         0.5f, -0.5f, 0.5f, 1.0f, 0.8f, 0.0f,   0.5f, -0.5f,-0.5f, 1.0f, 0.8f, 0.0f,   0.5f,  0.5f,-0.5f, 1.0f, 0.8f, 0.0f,
         0.5f,  0.5f,-0.5f, 1.0f, 0.8f, 0.0f,   0.5f,  0.5f, 0.5f, 1.0f, 0.8f, 0.0f,   0.5f, -0.5f, 0.5f, 1.0f, 0.8f, 0.0f,
        -0.5f,  0.5f, 0.5f, 1.0f, 0.8f, 0.0f,  -0.5f,  0.5f,-0.5f, 1.0f, 0.8f, 0.0f,  -0.5f, -0.5f,-0.5f, 1.0f, 0.8f, 0.0f,
        -0.5f, -0.5f,-0.5f, 1.0f, 0.8f, 0.0f,  -0.5f, -0.5f, 0.5f, 1.0f, 0.8f, 0.0f,  -0.5f,  0.5f, 0.5f, 1.0f, 0.8f, 0.0f,
        -0.5f, -0.5f,-0.5f, 1.0f, 0.8f, 0.0f,   0.5f, -0.5f,-0.5f, 1.0f, 0.8f, 0.0f,   0.5f, -0.5f, 0.5f, 1.0f, 0.8f, 0.0f,
         0.5f, -0.5f, 0.5f, 1.0f, 0.8f, 0.0f,  -0.5f, -0.5f, 0.5f, 1.0f, 0.8f, 0.0f,  -0.5f, -0.5f,-0.5f, 1.0f, 0.8f, 0.0f,
        -0.5f,  0.5f,-0.5f, 1.0f, 0.8f, 0.0f,   0.5f,  0.5f,-0.5f, 1.0f, 0.8f, 0.0f,   0.5f,  0.5f, 0.5f, 1.0f, 0.8f, 0.0f,
         0.5f,  0.5f, 0.5f, 1.0f, 0.8f, 0.0f,  -0.5f,  0.5f, 0.5f, 1.0f, 0.8f, 0.0f,  -0.5f,  0.5f,-0.5f, 1.0f, 0.8f, 0.0f
    };

    // NEGRO (0.1f, 0.1f, 0.1f)
    float verticesNegro[] = {
        -0.5f, -0.5f, 0.5f, 0.1f, 0.1f, 0.1f,   0.5f, -0.5f, 0.5f, 0.1f, 0.1f, 0.1f,   0.5f,  0.5f, 0.5f, 0.1f, 0.1f, 0.1f,
         0.5f,  0.5f, 0.5f, 0.1f, 0.1f, 0.1f,  -0.5f,  0.5f, 0.5f, 0.1f, 0.1f, 0.1f,  -0.5f, -0.5f, 0.5f, 0.1f, 0.1f, 0.1f,
        -0.5f, -0.5f,-0.5f, 0.1f, 0.1f, 0.1f,   0.5f, -0.5f,-0.5f, 0.1f, 0.1f, 0.1f,   0.5f,  0.5f,-0.5f, 0.1f, 0.1f, 0.1f,
         0.5f,  0.5f,-0.5f, 0.1f, 0.1f, 0.1f,  -0.5f,  0.5f,-0.5f, 0.1f, 0.1f, 0.1f,  -0.5f, -0.5f,-0.5f, 0.1f, 0.1f, 0.1f,
         0.5f, -0.5f, 0.5f, 0.1f, 0.1f, 0.1f,   0.5f, -0.5f,-0.5f, 0.1f, 0.1f, 0.1f,   0.5f,  0.5f,-0.5f, 0.1f, 0.1f, 0.1f,
         0.5f,  0.5f,-0.5f, 0.1f, 0.1f, 0.1f,   0.5f,  0.5f, 0.5f, 0.1f, 0.1f, 0.1f,   0.5f, -0.5f, 0.5f, 0.1f, 0.1f, 0.1f,
        -0.5f,  0.5f, 0.5f, 0.1f, 0.1f, 0.1f,  -0.5f,  0.5f,-0.5f, 0.1f, 0.1f, 0.1f,  -0.5f, -0.5f,-0.5f, 0.1f, 0.1f, 0.1f,
        -0.5f, -0.5f,-0.5f, 0.1f, 0.1f, 0.1f,  -0.5f, -0.5f, 0.5f, 0.1f, 0.1f, 0.1f,  -0.5f,  0.5f, 0.5f, 0.1f, 0.1f, 0.1f,
        -0.5f, -0.5f,-0.5f, 0.1f, 0.1f, 0.1f,   0.5f, -0.5f,-0.5f, 0.1f, 0.1f, 0.1f,   0.5f, -0.5f, 0.5f, 0.1f, 0.1f, 0.1f,
         0.5f, -0.5f, 0.5f, 0.1f, 0.1f, 0.1f,  -0.5f, -0.5f, 0.5f, 0.1f, 0.1f, 0.1f,  -0.5f, -0.5f,-0.5f, 0.1f, 0.1f, 0.1f,
        -0.5f,  0.5f,-0.5f, 0.1f, 0.1f, 0.1f,   0.5f,  0.5f,-0.5f, 0.1f, 0.1f, 0.1f,   0.5f,  0.5f, 0.5f, 0.1f, 0.1f, 0.1f,
         0.5f,  0.5f, 0.5f, 0.1f, 0.1f, 0.1f,  -0.5f,  0.5f, 0.5f, 0.1f, 0.1f, 0.1f,  -0.5f,  0.5f,-0.5f, 0.1f, 0.1f, 0.1f
    };

    // AZUL (0.2f, 0.6f, 1.0f)
    float verticesAzul[] = {
        -0.5f, -0.5f, 0.5f, 0.2f, 0.6f, 1.0f,   0.5f, -0.5f, 0.5f, 0.2f, 0.6f, 1.0f,   0.5f,  0.5f, 0.5f, 0.2f, 0.6f, 1.0f,
         0.5f,  0.5f, 0.5f, 0.2f, 0.6f, 1.0f,  -0.5f,  0.5f, 0.5f, 0.2f, 0.6f, 1.0f,  -0.5f, -0.5f, 0.5f, 0.2f, 0.6f, 1.0f,
        -0.5f, -0.5f,-0.5f, 0.2f, 0.6f, 1.0f,   0.5f, -0.5f,-0.5f, 0.2f, 0.6f, 1.0f,   0.5f,  0.5f,-0.5f, 0.2f, 0.6f, 1.0f,
         0.5f,  0.5f,-0.5f, 0.2f, 0.6f, 1.0f,  -0.5f,  0.5f,-0.5f, 0.2f, 0.6f, 1.0f,  -0.5f, -0.5f,-0.5f, 0.2f, 0.6f, 1.0f,
         0.5f, -0.5f, 0.5f, 0.2f, 0.6f, 1.0f,   0.5f, -0.5f,-0.5f, 0.2f, 0.6f, 1.0f,   0.5f,  0.5f,-0.5f, 0.2f, 0.6f, 1.0f,
         0.5f,  0.5f,-0.5f, 0.2f, 0.6f, 1.0f,   0.5f,  0.5f, 0.5f, 0.2f, 0.6f, 1.0f,   0.5f, -0.5f, 0.5f, 0.2f, 0.6f, 1.0f,
        -0.5f,  0.5f, 0.5f, 0.2f, 0.6f, 1.0f,  -0.5f,  0.5f,-0.5f, 0.2f, 0.6f, 1.0f,  -0.5f, -0.5f,-0.5f, 0.2f, 0.6f, 1.0f,
        -0.5f, -0.5f,-0.5f, 0.2f, 0.6f, 1.0f,  -0.5f, -0.5f, 0.5f, 0.2f, 0.6f, 1.0f,  -0.5f,  0.5f, 0.5f, 0.2f, 0.6f, 1.0f,
        -0.5f, -0.5f,-0.5f, 0.2f, 0.6f, 1.0f,   0.5f, -0.5f,-0.5f, 0.2f, 0.6f, 1.0f,   0.5f, -0.5f, 0.5f, 0.2f, 0.6f, 1.0f,
         0.5f, -0.5f, 0.5f, 0.2f, 0.6f, 1.0f,  -0.5f, -0.5f, 0.5f, 0.2f, 0.6f, 1.0f,  -0.5f, -0.5f,-0.5f, 0.2f, 0.6f, 1.0f,
        -0.5f,  0.5f,-0.5f, 0.2f, 0.6f, 1.0f,   0.5f,  0.5f,-0.5f, 0.2f, 0.6f, 1.0f,   0.5f,  0.5f, 0.5f, 0.2f, 0.6f, 1.0f,
         0.5f,  0.5f, 0.5f, 0.2f, 0.6f, 1.0f,  -0.5f,  0.5f, 0.5f, 0.2f, 0.6f, 1.0f,  -0.5f,  0.5f,-0.5f, 0.2f, 0.6f, 1.0f
    };

    // ROJO (0.9f, 0.1f, 0.2f)
    float verticesRojo[] = {
        -0.5f, -0.5f, 0.5f, 0.9f, 0.1f, 0.2f,   0.5f, -0.5f, 0.5f, 0.9f, 0.1f, 0.2f,   0.5f,  0.5f, 0.5f, 0.9f, 0.1f, 0.2f,
         0.5f,  0.5f, 0.5f, 0.9f, 0.1f, 0.2f,  -0.5f,  0.5f, 0.5f, 0.9f, 0.1f, 0.2f,  -0.5f, -0.5f, 0.5f, 0.9f, 0.1f, 0.2f,
        -0.5f, -0.5f,-0.5f, 0.9f, 0.1f, 0.2f,   0.5f, -0.5f,-0.5f, 0.9f, 0.1f, 0.2f,   0.5f,  0.5f,-0.5f, 0.9f, 0.1f, 0.2f,
         0.5f,  0.5f,-0.5f, 0.9f, 0.1f, 0.2f,  -0.5f,  0.5f,-0.5f, 0.9f, 0.1f, 0.2f,  -0.5f, -0.5f,-0.5f, 0.9f, 0.1f, 0.2f,
         0.5f, -0.5f, 0.5f, 0.9f, 0.1f, 0.2f,   0.5f, -0.5f,-0.5f, 0.9f, 0.1f, 0.2f,   0.5f,  0.5f,-0.5f, 0.9f, 0.1f, 0.2f,
         0.5f,  0.5f,-0.5f, 0.9f, 0.1f, 0.2f,   0.5f,  0.5f, 0.5f, 0.9f, 0.1f, 0.2f,   0.5f, -0.5f, 0.5f, 0.9f, 0.1f, 0.2f,
        -0.5f,  0.5f, 0.5f, 0.9f, 0.1f, 0.2f,  -0.5f,  0.5f,-0.5f, 0.9f, 0.1f, 0.2f,  -0.5f, -0.5f,-0.5f, 0.9f, 0.1f, 0.2f,
        -0.5f, -0.5f,-0.5f, 0.9f, 0.1f, 0.2f,  -0.5f, -0.5f, 0.5f, 0.9f, 0.1f, 0.2f,  -0.5f,  0.5f, 0.5f, 0.9f, 0.1f, 0.2f,
        -0.5f, -0.5f,-0.5f, 0.9f, 0.1f, 0.2f,   0.5f, -0.5f,-0.5f, 0.9f, 0.1f, 0.2f,   0.5f, -0.5f, 0.5f, 0.9f, 0.1f, 0.2f,
         0.5f, -0.5f, 0.5f, 0.9f, 0.1f, 0.2f,  -0.5f, -0.5f, 0.5f, 0.9f, 0.1f, 0.2f,  -0.5f, -0.5f,-0.5f, 0.9f, 0.1f, 0.2f,
        -0.5f,  0.5f,-0.5f, 0.9f, 0.1f, 0.2f,   0.5f,  0.5f,-0.5f, 0.9f, 0.1f, 0.2f,   0.5f,  0.5f, 0.5f, 0.9f, 0.1f, 0.2f,
         0.5f,  0.5f, 0.5f, 0.9f, 0.1f, 0.2f,  -0.5f,  0.5f, 0.5f, 0.9f, 0.1f, 0.2f,  -0.5f,  0.5f,-0.5f, 0.9f, 0.1f, 0.2f
    };

    GLuint VAOs[4], VBOs[4];
    glGenVertexArrays(4, VAOs);
    glGenBuffers(4, VBOs);

    // VAO[0] - AMARILLO
    glBindVertexArray(VAOs[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesAmarillo), verticesAmarillo, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // VAO[1] - NEGRO
    glBindVertexArray(VAOs[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesNegro), verticesNegro, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // VAO[2] - AZUL
    glBindVertexArray(VAOs[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesAzul), verticesAzul, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // VAO[3] - ROJO
    glBindVertexArray(VAOs[3]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesRojo), verticesRojo, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (GLfloat)screenWidth / (GLfloat)screenHeight, 0.1f, 100.0f);

    while (!glfwWindowShouldClose(window)) {

        Inputs(window);
        glfwPollEvents();

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.Use();
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);

        view = glm::translate(view, glm::vec3(movX, movY, movZ));
        view = glm::rotate(view, glm::radians(rot), glm::vec3(0.0f, 1.0f, 0.0f));

        GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
        GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
        GLint projecLoc = glGetUniformLocation(ourShader.Program, "projection");

        glUniformMatrix4fv(projecLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        // PARTES AMARILLAS
        glBindVertexArray(VAOs[0]);

        // -- Cabeza
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.85f, 1.0f));
        model = glm::scale(model, glm::vec3(1.6f, 1.4f, 1.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Franja 2 (Cuerpo Amarilla)
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.7f, -0.2f));
        model = glm::scale(model, glm::vec3(1.6f, 1.4f, 0.4f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        // Franja 4 (Cuerpo Amarilla, final del cuerpo)
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.7f, -1.0f));
        model = glm::scale(model, glm::vec3(1.6f, 1.4f, 0.4f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // PARTES ROJAS (pupila)
        glBindVertexArray(VAOs[3]);

        // Pupila Izquierdo - Fondo Rojo
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.69f, 0.75f, 1.62f));
        model = glm::scale(model, glm::vec3(0.3f, 0.2f, 0.05f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        // Pupila Izquierda - Lateral 
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.82f, 0.75f, 1.54f));
        model = glm::scale(model, glm::vec3(0.05f, 0.2f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        // Pupila Derecho - Fondo Rojo
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.69f, 0.75f, 1.62f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.05f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
		// Pupila Derecha - Lateral 
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.82f, 0.75f, 1.54f));
		model = glm::scale(model, glm::vec3(-0.05f, 0.2f, 0.2f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);

        // PARTES NEGRAS (Cuerpo, ojos, Antenas, Patas)
        glBindVertexArray(VAOs[1]);

        // -- Ojos 
        // Ojo Izquierdo 
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.60f, 0.95f, 1.61f));
        model = glm::scale(model, glm::vec3(0.4f, 0.6f, 0.05f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Ojo Izquierdo - Lateral 
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.81f, 0.95f, 1.4f));
        model = glm::scale(model, glm::vec3(0.05f, 0.6f, 0.4f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Ojo Derecho
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.60f, 0.95f, 1.61f));
        model = glm::scale(model, glm::vec3(-0.4f, 0.6f, 0.05f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

		// Ojo Derecho - Lateral 
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.81f, 0.95f, 1.4f));
		model = glm::scale(model, glm::vec3(-0.05f, 0.6f, 0.4f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);

        // Franja 1 (Cuerpo Negra)
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.7f, 0.2f));
        model = glm::scale(model, glm::vec3(1.6f, 1.4f, 0.4f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        // Franja 3 (Cuerpo Negra)
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.7f, -0.6f));
        model = glm::scale(model, glm::vec3(1.6f, 1.4f, 0.4f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // -- Antenas
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.4f, 1.85f, 1.1f));
        model = glm::scale(model, glm::vec3(0.2f, 0.6f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.4f, 2.25f, 1.33f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.28f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.4f, 1.85f, 1.1f));
        model = glm::scale(model, glm::vec3(0.2f, 0.6f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.4f, 2.25f, 1.33f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.28f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        // -- Patas
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.5f, -0.1f, 0.2f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.5f, -0.3f, 0.4f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.5f, -0.1f, 0.2f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.5f, -0.3f, 0.4f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.5f, -0.1f, -0.9f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.5f, -0.3f, -0.7f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.5f, -0.1f, -0.9f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.5f, -0.3f, -0.7f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        // -- Aguijón
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.7f, -1.4f));
        model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // PARTES AZULES
        glBindVertexArray(VAOs[2]);

        // -- Alas
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.6f, 1.55f, -0.2f));
        model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.35f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.6f, 1.80f, -0.5f));
        model = glm::scale(model, glm::vec3(0.3f, 0.2f, 0.5f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.6f, 1.55f, -0.2f));
        model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.35f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.6f, 1.80f, -0.5f));
        model = glm::scale(model, glm::vec3(0.3f, 0.2f, 0.5f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(4, VAOs);
    glDeleteBuffers(4, VBOs);

    glfwTerminate();
    return EXIT_SUCCESS;
}

void Inputs(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        movX += 0.03f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        movX -= 0.03f;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        movY += 0.03f;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        movY -= 0.03f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        movZ -= 0.03f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        movZ += 0.03f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        rot += 0.5f;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        rot -= 0.5f;
}
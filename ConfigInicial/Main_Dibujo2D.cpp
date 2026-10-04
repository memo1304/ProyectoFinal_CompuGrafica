//practica#2
//Espinoza Sáncgez Guillermo
//Fecha de entrega: 27 de agosto del 2026
//Número de cuenta 319205459

#include<iostream>

//#define GLEW_STATIC

#include <GL/glew.h>

#include <GLFW/glfw3.h>

// Shaders
#include "Shader.h"

void resize(GLFWwindow* window, int width, int height);

const GLint WIDTH = 800, HEIGHT = 600;


int main() {
    glfwInit();
    //Verificaci�n de compatibilidad
    /*glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);*/

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Dibujo de Primitivas en 2D", NULL, NULL);
    glfwSetFramebufferSizeCallback(window, resize);

    //Verificaci�n de errores de creacion  ventana
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();

        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;

    //Verificaci�n de errores de inicializaci�n de glew

    if (GLEW_OK != glewInit()) {
        std::cout << "Failed to initialise GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    // Imprimimos informacin de OpenGL del sistema
    std::cout << "> Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "> Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "> Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "> SL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;


    // Define las dimensiones del viewport
    //glViewport(0, 0, screenWidth, screenHeight);

    Shader ourShader("Shader/core.vs", "Shader/core.frag");

    // Set up vertex data (and buffer(s)) and attribute pointers
    float vertices[] = {
        // --- CARA (Naranja) 
        -0.7f,  0.2f, 0.0f,      1.0f, 0.5f, 0.0f,
        0.7f,  0.2f, 0.0f,      1.0f, 0.5f, 0.0f,
        0.0f, -0.6f, 0.0f,      1.0f, 0.5f, 0.0f,

        // --- OJO IZQUIERDO (Negro) 
        -0.25f, -0.05f, 0.01f,   0.0f, 0.0f, 0.0f,
        -0.15f, -0.05f, 0.01f,   0.0f, 0.0f, 0.0f,
        -0.15f, -0.15f, 0.01f,   0.0f, 0.0f, 0.0f,
        -0.25f, -0.15f, 0.01f,   0.0f, 0.0f, 0.0f,

        // --- OJO DERECHO (Negro) 
        0.15f, -0.05f, 0.01f,   0.0f, 0.0f, 0.0f,
        0.25f, -0.05f, 0.01f,   0.0f, 0.0f, 0.0f,
        0.25f, -0.15f, 0.01f,   0.0f, 0.0f, 0.0f,
        0.15f, -0.15f, 0.01f,   0.0f, 0.0f, 0.0f,

        // --- BRILLO OJO IZQUIERDO (Blanco) 
        -0.22f, -0.08f, 0.02f,   1.0f, 1.0f, 1.0f,  
        -0.18f, -0.08f, 0.02f,   1.0f, 1.0f, 1.0f,  
        -0.18f, -0.12f, 0.02f,   1.0f, 1.0f, 1.0f,   
        -0.22f, -0.12f, 0.02f,   1.0f, 1.0f, 1.0f,   

        // --- BRILLO OJO DERECHO (Blanco) 
        0.18f, -0.08f, 0.02f,   1.0f, 1.0f, 1.0f,   
        0.22f, -0.08f, 0.02f,   1.0f, 1.0f, 1.0f,   
        0.22f, -0.12f, 0.02f,   1.0f, 1.0f, 1.0f,   
        0.18f, -0.12f, 0.02f,   1.0f, 1.0f, 1.0f,   

        // --- CHAPETE IZQUIERDO (Rojo) 
        -0.30f, -0.18f, 0.01f,   0.9f, 0.2f, 0.2f,  
        -0.22f, -0.18f, 0.01f,   0.9f, 0.2f, 0.2f,   
        -0.22f, -0.26f, 0.01f,   0.9f, 0.2f, 0.2f,   
        -0.30f, -0.26f, 0.01f,   0.9f, 0.2f, 0.2f,   

        // --- CHAPETE DERECHO (Rojo) 
        0.22f, -0.18f, 0.01f,   0.9f, 0.2f, 0.2f,   
        0.30f, -0.18f, 0.01f,   0.9f, 0.2f, 0.2f,   
        0.30f, -0.26f, 0.01f,   0.9f, 0.2f, 0.2f,   
        0.22f, -0.26f, 0.01f,   0.9f, 0.2f, 0.2f,  

        // --- NARIZ (Negro) 
        -0.06f, -0.45f, 0.01f,   0.0f, 0.0f, 0.0f, 
        0.06f, -0.45f, 0.01f,   0.0f, 0.0f, 0.0f,  
        0.00f, -0.55f, 0.01f,   0.0f, 0.0f, 0.0f,  

        // --- OREJA IZQUIERDA (Naranja) 
        -0.70f,  0.20f, -0.01f,  1.0f, 0.5f, 0.0f,   
        -0.15f,  0.20f, -0.01f,  1.0f, 0.5f, 0.0f,  
        -0.15f,  0.75f, -0.01f,  1.0f, 0.5f, 0.0f,  

        // --- OREJA DERECHA (Naranja) 
        0.15f,  0.20f, -0.01f,  1.0f, 0.5f, 0.0f,  
        0.70f,  0.20f, -0.01f,  1.0f, 0.5f, 0.0f,  
        0.15f,  0.75f, -0.01f,  1.0f, 0.5f, 0.0f  
    };

    unsigned int indices[] = {
        // Cara
        0, 1, 2,

        // Ojo Izquierdo
        3, 6, 5,
        3, 5, 4,

        // Ojo Derecho
        7, 10, 9,
        7, 9, 8,

        // Brillo Ojo Izquierdo
        11, 14, 13,
        11, 13, 12,

        // Brillo Ojo Derecho
        15, 18, 17,
        15, 17, 16,

        // Chapete Izquierdo
        19, 22, 21,
        19, 21, 20,

        // Chapete Derecho
        23, 26, 25,
        23, 25, 24,

        // Nariz
        27, 28, 29,

        // Oreja Izquierda
        30, 31, 32,

        // Oreja Derecha
        33, 34, 35
    };



    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Enlazar  Vertex Array Object
    glBindVertexArray(VAO);

    //2.- Copiamos nuestros arreglo de vertices en un buffer de vertices para que OpenGL lo use
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // 3.Copiamos nuestro arreglo de indices en  un elemento del buffer para que OpenGL lo use
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 4. Despues colocamos las caracteristicas de los vertices

    //Posicion
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    //Color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);


    glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs)



    while (!glfwWindowShouldClose(window))
    {
        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();

        // Render
        // Clear the colorbuffer
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);


        // Draw our first triangle
        ourShader.Use();
        glBindVertexArray(VAO);


        //glPointSize(1);
        //glDrawArrays(GL_POINTS,0,1);

        //glDrawArrays(GL_LINES,0,2);
        //glDrawArrays(GL_LINE_LOOP,0,4);

        //glDrawArrays(GL_TRIANGLES,0,3);
        glDrawElements(GL_TRIANGLES, 48, GL_UNSIGNED_INT, 0);



        glBindVertexArray(0);

        // Swap the screen buffers
        glfwSwapBuffers(window);
    }



    glfwTerminate();
    return EXIT_SUCCESS;
}

void resize(GLFWwindow* window, int width, int height)
{
    // Set the Viewport to the size of the created window
    glViewport(0, 0, width, height);
    //glViewport(0, 0, screenWidth, screenHeight);
}
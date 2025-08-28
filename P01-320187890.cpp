#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <vector>

#ifdef _WIN32
#include <process.h>   // _getpid
#define getpid _getpid
#else
#include <unistd.h>    // getpid, read
#include <fcntl.h>     // open
#endif

#include <glew.h>
#include <glfw3.h>

int gVertexCount = 0; //

// Dimensiones de la ventana
const int WIDTH = 800, HEIGHT = 800;
GLuint VAO, VBO, shader;

// ====== SHADERS ======
// Vertex Shader
static const char* vShader = "                       \n\
#version 330                                            \n\
layout (location = 0) in vec3 pos;                      \n\
void main()                                             \n\
{                                                       \n\
    gl_Position = vec4(pos, 1.0);                       \n\
}";

// Fragment Shader
static const char* fShader = "                       \n\
#version 330                                            \n\
out vec4 color;                                         \n\
void main()                                             \n\
{                                                       \n\
    color = vec4(1.0, 0.0, 0.0, 1.0);                   \n\
}";

// ====== RNG con alta entropía =====
//Usamos rand pero con una semilla y actualizado con id del proceso y tiempo actual
static void seed_rng_simple(void) {
    unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)_getpid();
    srand(seed);
}

static void random_color(float* r, float* g, float* b) {
    *r = (float)rand() / (float)RAND_MAX;
    *g = (float)rand() / (float)RAND_MAX;
    *b = (float)rand() / (float)RAND_MAX;
}

// ====== Geometría ======
void CrearTriangulo()
{
    GLfloat vertices[] = {
         // === Letra C ===
        // Barra izquierda
        - 0.9f, -0.6f, 0.0f,   -0.8f, -0.6f, 0.0f,   -0.8f,  0.6f, 0.0f,
        -0.9f, -0.6f, 0.0f,   -0.8f,  0.6f, 0.0f,   -0.9f,  0.6f, 0.0f,

        // Barra superior
        -0.9f,  0.5f, 0.0f,   -0.5f,  0.5f, 0.0f,   -0.5f,  0.6f, 0.0f,
        -0.9f,  0.5f, 0.0f,   -0.5f,  0.6f, 0.0f,   -0.9f,  0.6f, 0.0f,

        // Barra inferior
        -0.9f, -0.6f, 0.0f,   -0.5f, -0.6f, 0.0f,   -0.5f, -0.5f, 0.0f,
        -0.9f, -0.6f, 0.0f,   -0.5f, -0.5f, 0.0f,   -0.9f, -0.5f, 0.0f,

        // === Letra M ===
        // Vertical izquierda
        -0.4f, -0.6f, 0.0f,   -0.3f, -0.6f, 0.0f,   -0.3f,  0.6f, 0.0f,
        -0.4f, -0.6f, 0.0f,   -0.3f,  0.6f, 0.0f,   -0.4f,  0.6f, 0.0f,

        // Vertical derecha
         0.1f, -0.6f, 0.0f,    0.2f, -0.6f, 0.0f,    0.2f,  0.6f, 0.0f,
         0.1f, -0.6f, 0.0f,    0.2f,  0.6f, 0.0f,    0.1f,  0.6f, 0.0f,

        // V interna izquierda
        -0.3f,  0.6f, 0.0f,   -0.1f, -0.6f, 0.0f,   -0.05f, 0.6f, 0.0f,

        // V interna derecha
         0.2f,  0.6f, 0.0f,   -0.1f, -0.6f, 0.0f,   -0.05f, 0.6f, 0.0f,

        // === Letra R ===
        // Vertical izquierda
         0.3f, -0.6f, 0.0f,   0.4f, -0.6f, 0.0f,   0.4f,  0.6f, 0.0f,
         0.3f, -0.6f, 0.0f,   0.4f,  0.6f, 0.0f,   0.3f,  0.6f, 0.0f,

        // Parte superior
         0.4f,  0.5f, 0.0f,   0.7f,  0.5f, 0.0f,   0.7f,  0.6f, 0.0f,
         0.4f,  0.5f, 0.0f,   0.7f,  0.6f, 0.0f,   0.4f,  0.6f, 0.0f,

        // Media horizontal
         0.4f, -0.05f, 0.0f,   0.65f, -0.05f, 0.0f,   0.65f, 0.05f, 0.0f,
         0.4f, -0.05f, 0.0f,   0.65f,  0.05f, 0.0f,   0.4f,  0.05f, 0.0f,

         // Lateral derecho de la R
         0.65f,  0.05f, 0.0f,   0.75f,  0.05f, 0.0f,   0.75f,  0.6f, 0.0f,
         0.65f,  0.05f, 0.0f,   0.75f,  0.6f, 0.0f,   0.65f,  0.6f, 0.0f,

        // Diagonal pierna
         0.4f, -0.05f, 0.0f,   0.7f, -0.6f, 0.0f,   0.55f, -0.6f, 0.0f
    };

    int numVertices = sizeof(vertices) / (3 * sizeof(GLfloat));

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Guarda el número de vértices global para usar en glDrawArrays
    gVertexCount = numVertices;
}

// ====== Shaders helpers ======
void AddShader(GLuint theProgram, const char* shaderCode, GLenum shaderType)
{
    GLuint theShader = glCreateShader(shaderType);
    const GLchar* theCode[1] = { shaderCode };
    GLint codeLength[1] = { (GLint)strlen(shaderCode) };

    glShaderSource(theShader, 1, theCode, codeLength);
    glCompileShader(theShader);

    GLint result = 0;
    GLchar eLog[1024] = { 0 };

    glGetShaderiv(theShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        // FIX: usar glGetShaderInfoLog para errores de compilación del shader
        glGetShaderInfoLog(theShader, sizeof(eLog), NULL, eLog);
        printf("Error al compilar el shader %d: %s\n", shaderType, eLog);
        glDeleteShader(theShader);
        return;
    }

    glAttachShader(theProgram, theShader);
    glDeleteShader(theShader); // Se puede borrar tras adjuntarlo
}

void CompileShaders() {
    shader = glCreateProgram();
    if (!shader)
    {
        printf("Error creando el programa de shaders\n");
        return;
    }

    AddShader(shader, vShader, GL_VERTEX_SHADER);
    AddShader(shader, fShader, GL_FRAGMENT_SHADER);

    GLint result = 0;
    GLchar eLog[1024] = { 0 };

    glLinkProgram(shader);
    glGetProgramiv(shader, GL_LINK_STATUS, &result);
    if (!result)
    {
        glGetProgramInfoLog(shader, sizeof(eLog), NULL, eLog);
        printf("Error al linkear: %s\n", eLog);
        return;
    }

    glValidateProgram(shader);
    glGetProgramiv(shader, GL_VALIDATE_STATUS, &result);
    if (!result)
    {
        glGetProgramInfoLog(shader, sizeof(eLog), NULL, eLog);
        printf("Error al validar: %s\n", eLog);
        return;
    }
}

int main()
{
    // Inicialización de GLFW
    if (!glfwInit())
    {
        printf("Falló inicializar GLFW\n");
        glfwTerminate();
        return 1;
    }

    // Versión/contexto OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Crear ventana
    GLFWwindow* mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "Primer ventana", NULL, NULL);
    if (!mainWindow)
    {
        printf("Fallo en crearse la ventana con GLFW\n");
        glfwTerminate();
        return 1;
    }

    int BufferWidth, BufferHeight;
    glfwGetFramebufferSize(mainWindow, &BufferWidth, &BufferHeight);
    glfwMakeContextCurrent(mainWindow);

    // VSync (opcional): suaviza el loop
    glfwSwapInterval(1);

    // GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Falló inicialización de GLEW\n");
        glfwDestroyWindow(mainWindow);
        glfwTerminate();
        return 1;
    }

    // Viewport
    glViewport(0, 0, BufferWidth, BufferHeight);

    // RNG con alta entropía para evitar mismas secuencias entre ejecuciones
    //seed_rng_secure();

    // Preparar escena
    CrearTriangulo();
    CompileShaders();

    // Color de fondo inicial y temporizador
    float bgR = 0.0f, bgG = 0.0f, bgB = 0.0f;
    random_color(&bgR, &bgG, &bgB);

    double lastChange = glfwGetTime();
    const double period = 2.0; //Actualización de nuevo color en segundos.

    // Loop principal
    while (!glfwWindowShouldClose(mainWindow))
    {
        glfwPollEvents();

        // Cambiar color cada 2 segundos
        double now = glfwGetTime();
        if ((now - lastChange) >= period) {
            random_color(&bgR, &bgG, &bgB);
            lastChange = now;
        }

        // Limpiar la ventana con el color aleatorio actual
        glClearColor(bgR, bgG, bgB, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Dibujar triángulo (rojo) sobre el fondo aleatorio
        glUseProgram(shader);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, gVertexCount);
        glBindVertexArray(0);
        glUseProgram(0);

        glfwSwapBuffers(mainWindow);


    }

    // Limpieza opcional
    if (VBO) glDeleteBuffers(1, &VBO);
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (shader) glDeleteProgram(shader);

    glfwDestroyWindow(mainWindow);
    glfwTerminate();
    return 0;
}
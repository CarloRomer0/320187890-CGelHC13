#pragma once
#include <stdio.h>
#include <glew.h>
#include <glfw3.h>
#include <glm.hpp>                // para glm::vec3
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

class Window
{
public:
    Window();
    Window(GLint windowWidth, GLint windowHeight);
    int Initialise();

    GLfloat getBufferWidth() { return bufferWidth; }
    GLfloat getBufferHeight() { return bufferHeight; }
    GLfloat getXChange();
    GLfloat getYChange();
    GLfloat getmuevex() { return muevex; }
    bool    getShouldClose() { return glfwWindowShouldClose(mainWindow); }
    bool* getsKeys() { return keys; }
    void    swapBuffers() { return glfwSwapBuffers(mainWindow); }

    // (getters/ajustadores que ya usas para otras cosas; puedes dejarlos tal cual)
    float getPataFL() const { float v = articulacion1; return v < -45.0f ? -45.0f : (v > 45.0f ? 45.0f : v); }
    float getPataFR() const { float v = articulacion2; return v < -45.0f ? -45.0f : (v > 45.0f ? 45.0f : v); }
    float getPataRL() const { float v = articulacion3; return v < -45.0f ? -45.0f : (v > 45.0f ? 45.0f : v); }
    float getPataRR() const { float v = articulacion4; return v < -45.0f ? -45.0f : (v > 45.0f ? 45.0f : v); }
    float getJaw()   const { float v = articulacion5; return v < 0.0f ? 0.0f : (v > 35.0f ? 35.0f : v); }

    void adjustPataFL(float d) { articulacion1 += d; if (articulacion1 > 45.0f) articulacion1 = 45.0f; if (articulacion1 < -45.0f) articulacion1 = -45.0f; }
    void adjustPataFR(float d) { articulacion2 += d; if (articulacion2 > 45.0f) articulacion2 = 45.0f; if (articulacion2 < -45.0f) articulacion2 = -45.0f; }
    void adjustPataRL(float d) { articulacion3 += d; if (articulacion3 > 45.0f) articulacion3 = 45.0f; if (articulacion3 < -45.0f) articulacion3 = -45.0f; }
    void adjustPataRR(float d) { articulacion4 += d; if (articulacion4 > 45.0f) articulacion4 = 45.0f; if (articulacion4 < -45.0f) articulacion4 = -45.0f; }
    void adjustJaw(float d) { articulacion5 += d; if (articulacion5 > 35.0f) articulacion5 = 35.0f; if (articulacion5 < 0.0f) articulacion5 = 0.0f; }

    // --- Carro (si lo usas desde aquí) ---
    float carWheelAng;
    float carHoodAng;
    float carPosZ;
    float carYaw;

    ~Window();

    // === Helpers helicóptero ===
    template<typename CarT>
    void ProcessGameplayInput(CarT& car, float deltaTime) {
        const bool* k = this->getsKeys();
        const float hoodSpeed = 25.0f; // grados/seg
        if (k[GLFW_KEY_T]) car.openHood(+hoodSpeed * deltaTime);
        if (k[GLFW_KEY_G]) car.openHood(-hoodSpeed * deltaTime);
    }

    template<typename OffT>
    void ProcessDebugTuning(OffT& off) {
        const bool* k = this->getsKeys();
        const float nud = 0.1f;

        // Cofre: HoodLocal
        if (k[GLFW_KEY_1]) off.hoodLocal.x += nud;
        if (k[GLFW_KEY_2]) off.hoodLocal.x -= nud;
        if (k[GLFW_KEY_3]) off.hoodLocal.y += nud;
        if (k[GLFW_KEY_4]) off.hoodLocal.y -= nud;
        if (k[GLFW_KEY_5]) off.hoodLocal.z += nud;
        if (k[GLFW_KEY_6]) off.hoodLocal.z -= nud;

        // Cofre: HoodPivot
        if (k[GLFW_KEY_7]) off.hoodPivot.x += nud;
        if (k[GLFW_KEY_8]) off.hoodPivot.x -= nud;
        if (k[GLFW_KEY_9]) off.hoodPivot.y += nud;
        if (k[GLFW_KEY_0]) off.hoodPivot.y -= nud;
        if (k[GLFW_KEY_MINUS]) off.hoodPivot.z += nud;
        if (k[GLFW_KEY_EQUAL]) off.hoodPivot.z -= nud;
    }

    // === Helicóptero: adelante/atrás===
    template<typename HeliT>
    void ProcessHelicopterInput(HeliT& heli, float deltaTime) {
        const bool* k = this->getsKeys();
        const float d = heli.speed * deltaTime;
        const float r = glm::radians(heli.yawDeg);
        const glm::vec3 fwd = glm::normalize(glm::vec3(-cosf(r), 0.0f, sinf(r)));

        if (k[GLFW_KEY_UP])    heli.pos += fwd * d;   // adelante 
        if (k[GLFW_KEY_DOWN])  heli.pos -= fwd * d;   // atrás
    }




    // === Spotlight del heli con ligera inclinación ===
    template<typename HeliT, typename SpotLightT>
    void UpdateHelicopterSpotlight(const HeliT& heli, SpotLightT& sl) {
        const glm::vec3 spotPos = heli.pos + glm::vec3(0.0f, -0.5f, 0.0f);
        const float r = glm::radians(heli.yawDeg);
        const glm::vec3 fwd = glm::normalize(glm::vec3(-cosf(r), 0.0f, sinf(r)));
        const glm::vec3 spotDir = glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f) * 0.85f + fwd * 0.35f);
        sl.SetFlash(spotPos, spotDir);
    }



private:
    GLFWwindow* mainWindow;
    GLint width, height;

    GLfloat rotax, rotay, rotaz, articulacion1, articulacion2, articulacion3, articulacion4, articulacion5, articulacion6;
    bool keys[1024];
    GLint bufferWidth, bufferHeight;
    void createCallbacks();
    GLfloat lastX;
    GLfloat lastY;
    GLfloat xChange;
    GLfloat yChange;
    GLfloat muevex;
    bool mouseFirstMoved;

    static void ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode);
    static void ManejaMouse(GLFWwindow* window, double xPos, double yPos);
};


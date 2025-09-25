#pragma once
#include<stdio.h>
#include<glew.h>
#include<glfw3.h>

class Window
{
public:
	Window();
	Window(GLint windowWidth, GLint windowHeight);
	int Initialise();
	GLfloat getBufferWidth() { return bufferWidth; }
	GLfloat getBufferHeight() { return bufferHeight; }
	bool getShouldClose() {
		return  glfwWindowShouldClose(mainWindow);}
	bool* getsKeys() { return keys; }
	GLfloat getXChange();
	GLfloat getYChange();
	void swapBuffers() { return glfwSwapBuffers(mainWindow); }
	GLfloat getrotay() { return rotay; }
	GLfloat getrotax() { return rotax; }
	GLfloat getrotaz() { return rotaz; }
    
	// === Getters de ángulo de patas con límite ±45° (inline, sin GLM) ===
	float getPataFL() const { float v = articulacion1; return v < -45.0f ? -45.0f : (v > 45.0f ? 45.0f : v); }
	float getPataFR() const { float v = articulacion2; return v < -45.0f ? -45.0f : (v > 45.0f ? 45.0f : v); }
	float getPataRL() const { float v = articulacion3; return v < -45.0f ? -45.0f : (v > 45.0f ? 45.0f : v); }
	float getPataRR() const { float v = articulacion4; return v < -45.0f ? -45.0f : (v > 45.0f ? 45.0f : v); }
	float getJaw()   const { float v = articulacion5; return v < 0.0f ? 0.0f : (v > 35.0f ? 35.0f : v); }


	// === Ajustadores públicos (sumar/restar con límites) ===
// Patas: ±45°
	void adjustPataFL(float d) { articulacion1 += d; if (articulacion1 > 45.0f) articulacion1 = 45.0f; if (articulacion1 < -45.0f) articulacion1 = -45.0f; }
	void adjustPataFR(float d) { articulacion2 += d; if (articulacion2 > 45.0f) articulacion2 = 45.0f; if (articulacion2 < -45.0f) articulacion2 = -45.0f; }
	void adjustPataRL(float d) { articulacion3 += d; if (articulacion3 > 45.0f) articulacion3 = 45.0f; if (articulacion3 < -45.0f) articulacion3 = -45.0f; }
	void adjustPataRR(float d) { articulacion4 += d; if (articulacion4 > 45.0f) articulacion4 = 45.0f; if (articulacion4 < -45.0f) articulacion4 = -45.0f; }
	void adjustJaw(float d) { articulacion5 += d; if (articulacion5 > 35.0f) articulacion5 = 35.0f; if (articulacion5 < 0.0f)  articulacion5 = 0.0f; }
	
	// --- Carro ---
	float carWheelAng;
	float carHoodAng;
	float carPosZ;
	float carYaw;

	~Window();
private: 
	GLFWwindow *mainWindow;
	GLint width, height;
	GLfloat rotax,rotay,rotaz, articulacion1, articulacion2, articulacion3, articulacion4, articulacion5, articulacion6;
	bool keys[1024];
	GLint bufferWidth, bufferHeight;
	GLfloat lastX;
	GLfloat lastY;
	GLfloat xChange;
	GLfloat yChange;
	bool mouseFirstMoved;
	void createCallbacks();
	static void ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode);
	static void ManejaMouse(GLFWwindow* window, double xPos, double yPos);
	
};


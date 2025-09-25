/*
Práctica 5: Optimización y Carga de Modelos
*/
//para cargar imagen
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <math.h>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>
//para probar el importer
//#include<assimp/Importer.hpp>

#include "Window.h"
#include "Mesh.h"
#include "Shader_m.h"
#include "Camera.h"
#include "Sphere.h"
#include"Model.h"
#include "Skybox.h"

const float toRadians = 3.14159265f / 180.0f;
//float angulocola = 0.0f;
Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;

Camera camera;
Model Goddard_M;

Skybox skybox;

//Sphere cabeza = Sphere(0.5, 20, 20);
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;


// Vertex Shader
static const char* vShader = "shaders/shader_m.vert";

// Fragment Shader
static const char* fShader = "shaders/shader_m.frag";





void CreateObjects()
{
	unsigned int indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};

	GLfloat vertices[] = {
		//	x      y      z			u	  v			nx	  ny    nz
			-1.0f, -1.0f, -0.6f,	0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 1.0f,		0.5f, 0.0f,		0.0f, 0.0f, 0.0f,
			1.0f, -1.0f, -0.6f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,		0.5f, 1.0f,		0.0f, 0.0f, 0.0f
	};

	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat floorVertices[] = {
		-10.0f, 0.0f, -10.0f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, -10.0f,	10.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-10.0f, 0.0f, 10.0f,	0.0f, 10.0f,	0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, 10.0f,		10.0f, 10.0f,	0.0f, -1.0f, 0.0f
	};

	
	Mesh *obj1 = new Mesh();
	obj1->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj1);

	Mesh *obj2 = new Mesh();
	obj2->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj2);

	Mesh *obj3 = new Mesh();
	obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(obj3);


}


void CreateShaders()
{
	Shader *shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}

void RenderLeg(
	Model& legModel,
	GLuint uniformModel,
	const glm::mat4& base,
	const glm::vec3& hipLocalPos,
	float swingDeg,          // (-45..+45)
	bool mirrorX,            // true = pata derecha
	const glm::vec3& legOriginOffset = glm::vec3(0.0f)  
) {
	glm::mat4 M = base;
	M = glm::translate(M, hipLocalPos);
	M = glm::rotate(M, glm::radians(90.0f),  glm::vec3(0,1,0));  
	M = glm::rotate(M, glm::radians(swingDeg), glm::vec3(1.0f, 0.0f, 0.0f));

	if (mirrorX) {
		M = glm::scale(M, glm::vec3(-1.0f, 1.0f, 1.0f));
	}

	M = glm::translate(M, legOriginOffset);

	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(M));
	legModel.RenderModel();
}


int main()
{
	mainWindow = Window(1366, 768); // 1280, 1024 or 1024, 768
	mainWindow.Initialise();

	CreateObjects();
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 0.5f, 7.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.3f, 1.0f);

	Goddard_M = Model();
	Model Cuerpo_M;
	Model Mandibula_M;
	Model PataFront_M;
	Model PataRear_M;
	//Goddard_M.LoadModel("Models/mandibula.obj");
	//Goddard_M.LoadModel("Models/eraManibulaPeroMeEquivoque.obj");
	Cuerpo_M.LoadModel("Models/cuerpo.obj");
	Mandibula_M.LoadModel("Models/mandibula.obj");
	PataFront_M.LoadModel("Models/pata_delantera.obj");
	PataRear_M.LoadModel("Models/pata_trasera.obj");

	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");

	skybox = Skybox(skyboxFaces);

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0;
	GLuint uniformColor = 0;
	//glm::mat4 projection = glm::perspective(glm::radians(45.0f), (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);
	glm::mat4 projection = glm::perspective(
		glm::radians(45.0f),
		static_cast<float>(mainWindow.getBufferWidth()) / static_cast<float>(mainWindow.getBufferHeight()),
		0.1f,
		1000.0f
	);

	glm::mat4 model(1.0);
	glm::mat4 modelaux(1.0);
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

	////Loop mientras no se cierra la ventana
	while (!mainWindow.getShouldClose())
	{

		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		//Recibir eventos del usuario
		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		// === CONTROLES CONTINUOS DE ARTICULACIONES (por frame) ===
		const bool* k = mainWindow.getsKeys();
		const float degPerSec = 90.0f; // velocidad (grados/seg)
		const float step = degPerSec * deltaTime;

		// FL (delantera izquierda)
		if (k[GLFW_KEY_F]) mainWindow.adjustPataFL(+step);
		if (k[GLFW_KEY_V]) mainWindow.adjustPataFL(-step);

		// FR (delantera derecha)
		if (k[GLFW_KEY_G]) mainWindow.adjustPataFR(+step);
		if (k[GLFW_KEY_B]) mainWindow.adjustPataFR(-step);

		// RL (trasera izquierda)
		if (k[GLFW_KEY_H]) mainWindow.adjustPataRL(+step);
		if (k[GLFW_KEY_N]) mainWindow.adjustPataRL(-step);

		// RR (trasera derecha)
		if (k[GLFW_KEY_J]) mainWindow.adjustPataRR(+step);
		if (k[GLFW_KEY_M]) mainWindow.adjustPataRR(-step);

		// Mandíbula
		if (k[GLFW_KEY_K]) mainWindow.adjustJaw(+step); // abrir
		if (k[GLFW_KEY_I]) mainWindow.adjustJaw(-step); // cerrar


		// Clear the window
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//Se dibuja el Skybox
		skybox.DrawSkybox(camera.calculateViewMatrix(), projection);

		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformColor = shaderList[0].getColorLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		// INICIA DIBUJO DEL PISO
		color = glm::vec3(0.5f, 0.5f, 0.5f); //piso de color gris
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -5.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[2]->RenderMesh();

		//------------*INICIA DIBUJO DE NUESTROS DEMÁS OBJETOS-------------------*
		// Color base para el modelo 
		color = glm::vec3(0.0f, 0.0f, 0.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));

		// === BASE DEL PERSONAJE ===
		glm::mat4 modelBase(1.0f);
		modelBase = glm::translate(modelBase, glm::vec3(0.0f, -2.0f, -1.5f)); // posición en el mundo
		// modelBase = glm::scale(modelBase, glm::vec3(1.0f));                // ajusta escala global si lo ves muy grande/pequeño

		// === 1) CUERPO ===
		color = glm::vec3(0.85f, 0.85f, 0.88f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelBase));
		Cuerpo_M.RenderModel();
		
		float aJaw = mainWindow.getJaw();
		float aFL = mainWindow.getPataFL();
		float aFR = mainWindow.getPataFR();
		float aRL = mainWindow.getPataRL();
		float aRR = mainWindow.getPataRR();
		

		// ====== MANDÍBULA ======
		{
			const glm::vec3 jawPivotLocal = glm::vec3(2.50f, -1.20f, -2.30f);
			const glm::vec3 jawOriginToPivot = glm::vec3(0.00f, -2.0f, 0.0f);

			const float preRY = 90.0f;   
			const float preRZ = 0.0f;   

			const glm::vec3 hingeLocalAxis = glm::vec3(1.0f, 0.0f, 0.0f); 
			const float jawSign = +1.0f;  

			glm::mat4 J = modelBase;

			J = glm::translate(J, jawPivotLocal);

			if (preRY != 0.0f) J = glm::rotate(J, glm::radians(preRY), glm::vec3(0, 1, 0));
			if (preRZ != 0.0f) J = glm::rotate(J, glm::radians(preRZ), glm::vec3(0, 0, 1));

			J = glm::translate(J, -jawOriginToPivot);
			J = glm::rotate(J, glm::radians(jawSign * mainWindow.getJaw()), glm::normalize(hingeLocalAxis));

			J = glm::translate(J, +jawOriginToPivot);
			if (preRZ != 0.0f) J = glm::rotate(J, glm::radians(-preRZ), glm::vec3(0, 0, 1));
			if (preRY != 0.0f) J = glm::rotate(J, glm::radians(-preRY), glm::vec3(0, 1, 0));
			glm::vec3 colorJaw = glm::vec3(0.95f, 0.55f, 0.20f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(colorJaw));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(J));
			Mandibula_M.RenderModel();
		}

		// === PATAS (derecha = al FRENTE, izquierda = ATRÁS) ===
		glm::vec3 hipRightA(+0.9f, -0.3f, +0.90f); 
		glm::vec3 hipRightB(+0.9f, -0.3f, -0.80f);  
		glm::vec3 hipLeftA(-0.9f, -1.2f, 1.20f);  
		glm::vec3 hipLeftB(-0.9f, -1.2f, -0.80f);  

		float angRightA = aFR;  
		float angRightB = aRR;  
		float angLeftA = aFL; 
		float angLeftB = aRL;  

		// DERECHA = FRENTE  → usa malla de PATA DELANTERA y mirrorX = true
		color = glm::vec3(0.30f, 0.85f, 0.40f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		RenderLeg(PataFront_M, uniformModel, modelBase, hipRightA, angRightA, /*mirrorX=*/true);

		color = glm::vec3(0.25f, 0.45f, 0.95f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		RenderLeg(PataFront_M, uniformModel, modelBase, hipRightB, angRightB, /*mirrorX=*/true);

		// IZQUIERDA = ATRÁS → usa malla de PATA TRASERA y mirrorX = false
		color = glm::vec3(0.60f, 0.40f, 0.90f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		RenderLeg(PataRear_M, uniformModel, modelBase, hipLeftA, angLeftA,  /*mirrorX=*/false);

		color = glm::vec3(0.90f, 0.25f, 0.35f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		RenderLeg(PataRear_M, uniformModel, modelBase, hipLeftB, angLeftB,  /*mirrorX=*/false);
		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}

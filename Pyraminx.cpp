#include "Pyraminx.h"
#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>
#include <gtc\random.hpp>
#include <cmath>
#include <vector>

Pyraminx::Pyraminx(Shader shader, Shader shaderColor, Camera& camera, unsigned int basePieces) 
	: m_shader(shader), m_shaderColor(shaderColor), m_camera(camera), m_basePieces(basePieces) {

	float totalWidth = basePieces * m_pieceWidth;
	m_baseHeight = std::sqrt(totalWidth * totalWidth - ((totalWidth * totalWidth) / 4.0f));
	m_pieceHeight = std::sqrt(m_pieceWidth * m_pieceWidth - ((m_pieceWidth * m_pieceWidth) / 4.0f));
	m_baseCentroid = (0.0f + 0.0f - m_baseHeight) / 3.0f;
}

void Pyraminx::Render() {
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.0f));

	RenderPiece(0.0f, 0.0f, 0.0f, model);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-m_pieceWidth / 2.0f, m_pieceHeight, -2.15f));
	RenderPiece(0.0f, 0.0f, 0.0f, model);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-m_pieceWidth / 2.0f, 0.0f, -2.15f));
	RenderPiece(m_pieceWidth, -m_pieceHeight, m_pieceWidth / 2.0f + 0.0175, model);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-m_pieceWidth / 2.0f, 0.0f, -2.15f));
	RenderPiece(0.0f, -m_pieceHeight, m_pieceWidth / 2.0f + 0.0175, model);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.0f));
	RenderPiece(-m_pieceWidth, 0.0f, 0.0f, model);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-m_pieceWidth / 2.0f, 0.0f, -1.875f));
	RenderPiece(-m_pieceWidth, -m_pieceHeight, 0.0f, model);

	// Green face
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.05f));

	RenderPiece(-m_pieceWidth / 2.0f + 0.015, 0.0f, -m_pieceWidth + 0.1f, model);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.2f));
	RenderPiece(-m_pieceWidth / 2.0f + 0.02, -m_pieceHeight, -m_pieceWidth - 0.02, model);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.2f));
	RenderPiece(-m_pieceWidth / 2.0f + 0.25, -m_pieceHeight, -m_pieceWidth / 2.0f + 0.1475, model);

	// blue face
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-m_pieceWidth / 2.0f, 0.0f, -2.29f));
	RenderPiece(-0.25, -m_pieceHeight, 0.0f, model);

	// Centers
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.0f));
	RenderCenter(model);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-m_pieceWidth, 0.0f, -2.0f));
	RenderCenter(model);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-m_pieceWidth / 2.0f, m_pieceHeight, -2.15f));
	RenderCenter(model);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-m_pieceWidth / 2.0f, 0.0f, -2.0f - m_pieceWidth + 0.06));
	RenderCenter(model);
}

void Pyraminx::RenderPiece(float offsetX, float offsetY, float offsetZ, glm::mat4 model) {
	unsigned int indices_pieza[] = {
		0, 1, 2,
		3, 4, 5,
		6, 7, 8,
		9, 10, 11,
		12, 13, 14,
		15, 16, 17,
		18, 19, 20,
		21, 22, 23
	};

	float padding = 0.05, paddingY = 0.03;
	GLfloat vertices_pieza[] = {
		0.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ,																							0.0f, 0.0f, 0.0f,
		m_pieceWidth + offsetX, 0.0f + offsetY, 0.0f + offsetZ,																			0.0f, 0.0f, 0.0f,
		m_pieceWidth / 2.0f + offsetX, m_pieceHeight + offsetY, -(m_pieceHeight / 3.0f) + offsetZ,					0.0f, 0.0f, 0.0f,

		m_pieceWidth + offsetX, 0.0f + offsetY, 0.0f + offsetZ,										0.0f, 0.0f, 0.0f,
		m_pieceWidth / 2.0f + offsetX, 0.0f + offsetY, -m_pieceHeight + offsetZ,										0.0f, 0.0f, 0.0f,
		m_pieceWidth / 2.0f + offsetX, m_pieceHeight + offsetY, -(m_pieceHeight / 3.0f) + offsetZ,	0.0f, 0.0f, 0.0f,

		0.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ,																					0.0f, 0.0f, 0.0f,
		m_pieceWidth / 2.0f + offsetX, 0.0f + offsetY, -m_pieceHeight + offsetZ,										0.0f, 0.0f, 0.0f,
		m_pieceWidth / 2.0f + offsetX, m_pieceHeight + offsetY, -(m_pieceHeight / 3.0f) + offsetZ,	0.0f, 0.0f, 0.0f,

		0.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ,																					0.0f, 0.0f, 0.0f,
		m_pieceWidth + offsetX, 0.0f + offsetY, 0.0f + offsetZ,										0.0f, 0.0f, 0.0f,
		m_pieceWidth / 2.0f + offsetX, 0.0f + offsetY, -m_pieceHeight + offsetZ,										0.0f, 0.0f, 0.0f,

		// Sticker rojo
		0.0f + padding + offsetX, 0.0f + paddingY + offsetY, -0.01f + offsetZ,																1.0f, 0.0f, 0.0f,
		m_pieceWidth - padding + offsetX, 0.0f + paddingY + offsetY, -0.01f + offsetZ, 										1.0f, 0.0f, 0.0f,
		m_pieceWidth / 2.0f + offsetX, m_pieceHeight - padding + offsetY, -(m_pieceHeight / 3.0f) + 0.02 + offsetZ,				1.0f, 0.0f, 0.0f,

		// Sticker verde
		m_pieceWidth - 0.02f + offsetX, 0.0f + paddingY + offsetY, -0.04 + offsetZ,									0.0f, 1.0f, 0.0f,
		m_pieceWidth / 2.0f + 0.02 + offsetX, 0.0f + paddingY + offsetY, -m_pieceHeight + 0.035 + offsetZ,			0.0f, 1.0f, 0.0f,
		m_pieceWidth / 2.0f + 0.02 + offsetX, m_pieceHeight - padding + offsetY, -(m_pieceHeight / 3.0f) - 0.02 + offsetZ,			0.0f, 1.0f, 0.0f,

		// Sticker azul
		0.0f + 0.02f + offsetX, 0.0f + paddingY + offsetY, -0.04 + offsetZ,									0.0f, 0.0f, 1.0f,
		m_pieceWidth / 2.0f - 0.02 + offsetX, 0.0f + paddingY + offsetY, -m_pieceHeight + 0.035 + offsetZ,			0.0f, 0.0f, 1.0f,
		m_pieceWidth / 2.0f - 0.02 + offsetX, m_pieceHeight - padding + offsetY, -(m_pieceHeight / 3.0f) - 0.02 + offsetZ,			0.0f, 0.0f, 1.0f,

		// Sticker amarillo
		0.0f + padding + offsetX, 0.0f - 0.001 + offsetY, 0.0f - 0.02 + offsetZ,									1.0f, 1.0f, 0.0f,
		m_pieceWidth - padding + offsetX, 0.0f - 0.001 + offsetY, 0.0f - 0.02 + offsetZ,					1.0f, 1.0f, 0.0f,
		m_pieceWidth / 2.0f + offsetX, 0.0f - 0.001 + offsetY, -m_pieceHeight + 0.05 + offsetZ,	1.0f, 1.0f, 0.0f,
	};

	Mesh* pieza = new Mesh();
	pieza->CreateMesh(vertices_pieza, indices_pieza, 144, 24);

	m_shaderColor.useShader();
	GLuint uniformModel = m_shaderColor.getModelLocation();
	GLuint uniformProjection = m_shaderColor.getProjectLocation();
	GLuint uniformView = m_shaderColor.getViewLocation();
	glm::mat4 projection = glm::perspective(glm::radians(60.0f), 16.0f / 9.0f, 0.1f, 100.0f);
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(m_camera.calculateViewMatrix()));

	pieza->RenderMesh();

	delete pieza;
}

void Pyraminx::RenderCenter(glm::mat4 model) {
	unsigned int indices_centro[] = {
		0, 1, 2,
		3, 4, 5,
		6, 7, 8,
		9, 10, 11
	};

	GLfloat vertices_centro[] = {
		0.0f, 0.0f, 0.0f,							1.0f, 0.0f, 0.0f,
		m_pieceWidth, 0.0f, 0.0f,			1.0f, 0.0f, 0.0f,
		m_pieceWidth / 2.0f, -m_pieceHeight, m_pieceWidth / 3.0f - 0.03f,			1.0f, 0.0f, 0.0f,

		m_pieceWidth, 0.0f, 0.0f,							0.0f, 1.0f, 0.0f,
		m_pieceWidth / 2.0f, 0.0f, -m_pieceWidth + 0.06,			0.0f , 1.0f, 0.0f,
		m_pieceWidth, -m_pieceHeight, -m_pieceWidth / 2.0f - 0.06,		0.0f, 1.0f, 0.0f,
		
		0.0f, 0.0f, 0.0f,							0.0f, 0.0f, 1.0f,
		m_pieceWidth / 2.0f, 0.0f, -m_pieceWidth + 0.06,			0.0f , 0.0f, 1.0f,
		0.0f, -m_pieceHeight, -m_pieceWidth / 2.0f - 0.06,		0.0f, 0.0f, 1.0f,

		m_pieceWidth / 2.0f, -m_pieceHeight, 0.0f + 0.11,				1.0f, 1.0f, 0.0f,
		0.0f, -m_pieceHeight, -m_pieceHeight + 0.11,							1.0f, 1.0f, 0.0f,
		m_pieceWidth, -m_pieceHeight, -m_pieceHeight + 0.11,			1.0f, 1.0f, 0.0f
	};

	Mesh* centro = new Mesh();
	centro->CreateMesh(vertices_centro, indices_centro, 72, 12);

	m_shaderColor.useShader();
	GLuint uniformModel = m_shaderColor.getModelLocation();
	GLuint uniformProjection = m_shaderColor.getProjectLocation();
	GLuint uniformView = m_shaderColor.getViewLocation();
	glm::mat4 projection = glm::perspective(glm::radians(60.0f), 16.0f / 9.0f, 0.1f, 100.0f);
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(m_camera.calculateViewMatrix()));

	centro->RenderMesh();

	delete centro;
}
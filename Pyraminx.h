#pragma once
#include <glew.h>
#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"

class Pyraminx {
public:
	Pyraminx(Shader shader, Shader shaderColor, Camera& camera, unsigned int basePieces);
	void Render();
private:
	void RenderPiece(float, float, float, glm::mat4);
	void RenderCenter(glm::mat4);

private:
	const float m_padding = 0.2f;
	const float m_pieceWidth = 0.5f;
	float m_pieceHeight;
	float m_baseHeight;
	float m_baseCentroid;
	unsigned int m_basePieces;
	Shader m_shader;
	Shader m_shaderColor;
	Camera& m_camera;
};
#version 330

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in vec3 norm;

out vec4 vCol;
out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out vec4 vColor;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;
uniform vec3 color;

// NUEVOS uniforms para subtextura
uniform vec2 uOffset;   // esquina sup. izq. del recorte
uniform vec2 uSubSize;  // tamaño del recorte (ancho, alto) en UV

void main()
{
    gl_Position = projection * view * model * vec4(pos, 1.0);
    vCol   = vec4(0.0, 1.0, 0.0, 1.0f);
    vColor = vec4(color, 1.0f);

    // tex viene en [0..1] del quad → lo escalamos y lo movemos
    TexCoord = uOffset + tex * uSubSize;

    Normal = mat3(transpose(inverse(model))) * norm;
    FragPos = (model * vec4(pos, 1.0)).xyz;
}

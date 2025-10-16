/*
Práctica 7: Iluminación 1
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
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Sphere.h"
#include "Model.h"
#include "Skybox.h"

//para iluminación
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"
const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;

Camera camera;

Texture brickTexture;
Texture dirtTexture;
Texture plainTexture;
Texture pisoTexture;
Texture AgaveTexture;

Model Kitt_M;
Model Llanta_M;
Model Blackhawk_M;
Model Goddard_M;
Skybox skybox;

// Lámpara del parque
Model StreetLamp_M;
// Altura real del .obj ~14.53; queremos ~3.2 en escena
const float kLampModelHeight = 14.53f;
const float kLampTargetHeight = 3.20f;
const float kLampScale = kLampTargetHeight / kLampModelHeight; // ≈ 0.22f

// Posición en el mundo (mueve a tu gusto)
const glm::vec3 streetLampPos = glm::vec3(6.0f, -1.0f, -10.0f);

//materiales
Material Material_brillante;
Material Material_opaco;

//Sphere cabeza = Sphere(0.5, 20, 20);
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

// luz direccional
DirectionalLight mainLight;
//para declarar varias luces de tipo pointlight
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

// Vertex Shader
static const char* vShader = "shaders/shader_light.vert";

// Shader
static const char* fShader = "shaders/shader_light.frag";

/*===============Carro=======================*/

struct CarOffsetsRaw {
    glm::vec3 wheelFL;      // front-left
    glm::vec3 wheelFR;      // front-right
    glm::vec3 wheelRL;      // rear-left
    glm::vec3 wheelRR;      // rear-right
    glm::vec3 wheelCenter;  // centro real del mesh de llanta 
    glm::vec3 hoodPivot;    // bisagra del cofre 
    glm::vec3 hoodLocal;    // corrección local del mesh del cofre
    glm::vec3 headLLocal;   // faro izquierdo (local del coche)
    glm::vec3 headRLocal;   // faro derecho  (local del coche)

    // --- Parabrisas ---
    glm::vec3 wsLocal;      // posición local del parabrisas respecto al cuerpo
    float     wsYawDeg;     // rotación en Y del parabrisas
};

struct Car {
    // Modelos
    Model body, hood, wheel, windshield;

    // Estado
    float scale = 7.4f;            // tamaño grande (proporcional al perro)
    glm::vec3 pos = glm::vec3(0);
    float yawDeg = 0.0f;           // giro Y
    float wheelAng = 0.0f;         // giro de llantas
    float hoodAng = 0.0f;          // apertura del cofre
    float hoodMax = 50.0f;         // límite superior del cofre

    // Offsets en unidad de modelo (raw) 
    CarOffsetsRaw raw;
    struct {
        glm::vec3 FL, FR, RL, RR, Ctr;
        glm::vec3 HoodPivot, HoodLocal;
        glm::vec3 WsLocal;
        glm::vec3 HeadL, HeadR;
    } comp;
    float wsYawDeg = 180.0f;

    // Config
    void setScale(float s) { scale = s; recomputeComp(); }
    void setBasePosition(const glm::vec3& p) { pos = p; }
    void setOffsetsRaw(const CarOffsetsRaw& o) { raw = o; recomputeComp(); }
    void setHoodMax(float deg) { hoodMax = deg; }

    // Movimiento
    void moveWorldZ(float dz) { pos.z += dz; }
    void addYaw(float ddeg) { yawDeg += ddeg; }
    void moveLocalForward(float d) {
        float r = glm::radians(yawDeg);
        pos += glm::vec3(-sinf(r), 0.0f, -cosf(r)) * d;
    }
    void spinWheels(float ddeg) { wheelAng += ddeg; }
    void openHood(float ddeg) {
        hoodAng += ddeg;
        if (hoodAng < 0.0f) hoodAng = 0.0f;
        if (hoodAng > hoodMax) hoodAng = hoodMax;
    }

    //faros (para spotlights)
    void getHeadlightWorld(bool left, glm::vec3& outPos, glm::vec3& outDir) const {
        glm::mat4 M(1.0f);
        M = glm::translate(M, pos);
        M = glm::rotate(M, glm::radians(yawDeg), glm::vec3(0, 1, 0));
        M = glm::scale(M, glm::vec3(scale));

        glm::vec3 loc = left ? comp.HeadL : comp.HeadR;
        outPos = glm::vec3(M * glm::vec4(loc, 1.0f));

        // “adelante” del coche es +Z local -> solo rotación Y
        glm::mat4 R(1.0f);
        R = glm::rotate(R, glm::radians(yawDeg), glm::vec3(0, 1, 0));
        outDir = glm::vec3(R * glm::vec4(0, 0, 1, 0));
    }

    // Render
    void render(GLint uModel, GLint uColor) {
        glm::mat4 M(1.0f);
        M = glm::translate(M, pos);
        M = glm::rotate(M, glm::radians(yawDeg), glm::vec3(0, 1, 0));
        M = glm::scale(M, glm::vec3(scale));

        // CUERPO 
        glUniform3fv(uColor, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(M));
        body.RenderModel();

        // COFRE 
        {
            glm::mat4 H = M;
            H = glm::translate(H, comp.HoodLocal);
            H = glm::translate(H, comp.HoodPivot);
            H = glm::rotate(H, glm::radians(-hoodAng), glm::vec3(1, 0, 0));
            H = glm::translate(H, -comp.HoodPivot);

            glUniform3fv(uColor, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(H));
            hood.RenderModel();
        }

        // PARABRISAS 
        {
            glm::mat4 W = M;
            W = glm::translate(W, comp.WsLocal);
            W = glm::rotate(W, glm::radians(wsYawDeg), glm::vec3(0, 1, 0));

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);

            glUniform3fv(uColor, 1, glm::value_ptr(glm::vec3(1.0f)));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(W));
            windshield.RenderModel();

            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }

        // LLANTAS 
        auto drawWheel = [&](const glm::vec3& p) {
            glm::mat4 W = M;
            W = glm::translate(W, p);
            W = glm::translate(W, comp.Ctr);
            W = glm::rotate(W, glm::radians(wheelAng), glm::vec3(1, 0, 0));
            W = glm::translate(W, -comp.Ctr);
            glUniform3fv(uColor, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(W));
            wheel.RenderModel();
            };
        drawWheel(comp.FL);
        drawWheel(comp.FR);
        drawWheel(comp.RL);
        drawWheel(comp.RR);
    }

private:
    // Compensa offsets 
    void recomputeComp() {
        auto LS = [&](const glm::vec3& v) { return v / scale; };
        comp.FL = LS(raw.wheelFL);
        comp.FR = LS(raw.wheelFR);
        comp.RL = LS(raw.wheelRL);
        comp.RR = LS(raw.wheelRR);
        comp.Ctr = LS(raw.wheelCenter);
        comp.HoodPivot = LS(raw.hoodPivot);
        comp.HoodLocal = LS(raw.hoodLocal);

        comp.WsLocal = LS(raw.wsLocal); // parabrisas
        wsYawDeg = raw.wsYawDeg;

        comp.HeadL = LS(raw.headLLocal); // faros
        comp.HeadR = LS(raw.headRLocal);
    }
};


//función de calculo de normales por promedio de vértices 
void calcAverageNormals(unsigned int* indices, unsigned int indiceCount, GLfloat* vertices, unsigned int verticeCount,
    unsigned int vLength, unsigned int normalOffset)
{
    for (size_t i = 0; i < indiceCount; i += 3)
    {
        unsigned int in0 = indices[i] * vLength;
        unsigned int in1 = indices[i + 1] * vLength;
        unsigned int in2 = indices[i + 2] * vLength;
        glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
        glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
        glm::vec3 normal = glm::cross(v1, v2);
        normal = glm::normalize(normal);

        in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
        vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
        vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
        vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
    }

    for (size_t i = 0; i < verticeCount / vLength; i++)
    {
        unsigned int nOffset = i * vLength + normalOffset;
        glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
        vec = glm::normalize(vec);
        vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
    }
}


void CreateObjects()
{
    unsigned int indices[] = {
        0, 3, 1,
        1, 3, 2,
        2, 3, 0,
        0, 1, 2
    };

    GLfloat vertices[] = {
        //  x      y      z         u     v         nx    ny    nz
        -1.0f, -1.0f, -0.6f,    0.0f, 0.0f,      0.0f, 0.0f, 0.0f,
         0.0f, -1.0f,  1.0f,    0.5f, 0.0f,      0.0f, 0.0f, 0.0f,
         1.0f, -1.0f, -0.6f,    1.0f, 0.0f,      0.0f, 0.0f, 0.0f,
         0.0f,  1.0f,  0.0f,    0.5f, 1.0f,      0.0f, 0.0f, 0.0f
    };

    unsigned int floorIndices[] = {
        0, 2, 1,
        1, 2, 3
    };

    GLfloat floorVertices[] = {
        -10.0f, 0.0f, -10.0f,   0.0f, 0.0f,      0.0f, -1.0f, 0.0f,
         10.0f, 0.0f, -10.0f,  10.0f, 0.0f,      0.0f, -1.0f, 0.0f,
        -10.0f, 0.0f,  10.0f,   0.0f, 10.0f,     0.0f, -1.0f, 0.0f,
         10.0f, 0.0f,  10.0f,  10.0f, 10.0f,     0.0f, -1.0f, 0.0f
    };

    unsigned int vegetacionIndices[] = {
       0, 1, 2,
       0, 2, 3,
       4, 5, 6,
       4, 6, 7
    };

    GLfloat vegetacionVertices[] = {
        -0.5f, -0.5f, 0.0f,     0.0f, 0.0f,      0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,     1.0f, 0.0f,      0.0f, 0.0f, 0.0f,
         0.5f,  0.5f, 0.0f,     1.0f, 1.0f,      0.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, 0.0f,     0.0f, 1.0f,      0.0f, 0.0f, 0.0f,

         0.0f, -0.5f, -0.5f,    0.0f, 0.0f,      0.0f, 0.0f, 0.0f,
         0.0f, -0.5f,  0.5f,    1.0f, 0.0f,      0.0f, 0.0f, 0.0f,
         0.0f,  0.5f,  0.5f,    1.0f, 1.0f,      0.0f, 0.0f, 0.0f,
         0.0f,  0.5f, -0.5f,    0.0f, 1.0f,      0.0f, 0.0f, 0.0f,
    };

    Mesh* obj1 = new Mesh();
    obj1->CreateMesh(vertices, indices, 32, 12);
    meshList.push_back(obj1);

    Mesh* obj2 = new Mesh();
    obj2->CreateMesh(vertices, indices, 32, 12);
    meshList.push_back(obj2);

    Mesh* obj3 = new Mesh();
    obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
    meshList.push_back(obj3);

    Mesh* obj4 = new Mesh();
    obj4->CreateMesh(vegetacionVertices, vegetacionIndices, 64, 12);
    meshList.push_back(obj4);

    calcAverageNormals(indices, 12, vertices, 32, 8, 5);
    calcAverageNormals(vegetacionIndices, 12, vegetacionVertices, 64, 8, 5);
}

// Helper de pata 
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
    M = glm::rotate(M, glm::radians(90.0f), glm::vec3(0, 1, 0));
    M = glm::rotate(M, glm::radians(swingDeg), glm::vec3(1.0f, 0.0f, 0.0f));

    if (mirrorX) M = glm::scale(M, glm::vec3(-1.0f, 1.0f, 1.0f));

    M = glm::translate(M, legOriginOffset);

    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(M));
    legModel.RenderModel();
}

void CreateShaders()
{
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);
}

int main()
{
    mainWindow = Window(1366, 768);
    mainWindow.Initialise();

    CreateObjects();
    CreateShaders();

    camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.3f, 0.5f);

    // Modelos perro
    Goddard_M = Model();
    Model Cuerpo_M;
    Model Mandibula_M;
    Model PataFront_M;
    Model PataRear_M;

    Cuerpo_M.LoadModel("Models/cuerpo.obj");
    Mandibula_M.LoadModel("Models/mandibula.obj");
    PataFront_M.LoadModel("Models/pata_delantera.obj");
    PataRear_M.LoadModel("Models/pata_trasera.obj");

    brickTexture = Texture("Textures/brick.png");
    brickTexture.LoadTextureA();
    dirtTexture = Texture("Textures/dirt.png");
    dirtTexture.LoadTextureA();
    plainTexture = Texture("Textures/plain.png");
    plainTexture.LoadTextureA();
    pisoTexture = Texture("Textures/piso.tga");
    pisoTexture.LoadTextureA();
    AgaveTexture = Texture("Textures/Agave.tga");
    AgaveTexture.LoadTextureA();

    Kitt_M = Model();
    Kitt_M.LoadModel("Models/kitt_optimizado.obj");
    Llanta_M = Model();
    Llanta_M.LoadModel("Models/llanta_optimizada.obj");
    Blackhawk_M = Model();
    Blackhawk_M.LoadModel("Models/uh60.obj");

    // Lámpara (asegúrate de tener Lampara.obj/.mtl en Models/)
    Model Lampara_M;
    Lampara_M.LoadModel("Models/Lampara.obj");
    //Nueva Lampara
    StreetLamp_M.LoadModel("Models/StreetLamp.obj");


    std::vector<std::string> skyboxFaces;
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");

    skybox = Skybox(skyboxFaces);

    Material_brillante = Material(4.0f, 256);
    Material_opaco = Material(0.3f, 4);

    //luz direccional, sólo 1 y siempre debe de existir
    mainLight = DirectionalLight(1.0f, 1.0f, 1.0f,
        0.3f, 0.3f,
        0.0f, 0.0f, -1.0f);

    //contador de luces puntuales
    unsigned int pointLightCount = 0;
    //Declaración de primer luz puntual
    pointLights[0] = PointLight(1.0f, 0.0f, 0.0f,
        0.0f, 1.0f,
        -6.0f, 1.5f, 1.5f,
        1.0f, 0.2f, 0.1f); // atenuación (ajustado: const=1.0)
    pointLightCount++;

    unsigned int spotLightCount = 0;
    //linterna (camara)
    spotLights[0] = SpotLight(1.0f, 1.0f, 1.0f,
        0.0f, 2.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        5.0f);
    spotLightCount++;

    //luz fija
    spotLights[1] = SpotLight(0.0f, 1.0f, 0.0f,
        1.0f, 2.0f,
        5.0f, 10.0f, 0.0f,
        0.0f, -5.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        15.0f);
    spotLightCount++;

    // === Instancia del Carro  ===
    Car car;
    car.body.LoadModel("Models/CuerpoCarro.obj");
    car.hood.LoadModel("Models/CofreTextura.obj");
    car.wheel.LoadModel("Models/Llanta.obj");
    car.windshield.LoadModel("Models/Parabrisas.obj");

    car.setScale(7.4f);
    car.setBasePosition(glm::vec3(0.0f, -2.2f, -14.0f));

    CarOffsetsRaw off;
    // ==== Offsets de arranque  =====
    off.wheelFL = glm::vec3(+47.7f, 0.0f, +13.35f);//frontal izq
    off.wheelFR = glm::vec3(+36.0f, 0.0f, +13.35f);//frontal der
    off.wheelRL = glm::vec3(+47.7f, 0.0f, -8.4f);//trasera izq
    off.wheelRR = glm::vec3(+36.0f, 0.0f, -8.4f);//trasera der

    off.wheelCenter = glm::vec3(0.0f, 0.0f, -0.12f);

    // Cofre (ajuste para giro natural)
    off.hoodPivot = glm::vec3(0.0f, 3.00f, 0.65f);
    off.hoodLocal = glm::vec3(2.70f, 0.35f, 4.55f);

    // Parabrisas 
    off.wsLocal = glm::vec3(3.0f, 0.0f, 4.8f);
    off.wsYawDeg = 180.0f;

    // Faros
    off.headLLocal = glm::vec3(+10.0f, +6.5f, +15.0f); // izquierdo
    off.headRLocal = glm::vec3(-2.0f, +6.5f, +15.0f);  // derecho

    car.setOffsetsRaw(off);
    car.setHoodMax(50.0f); // límite realista de apertura

    // === Faros del coche (spotlights) ===
    spotLights[2] = SpotLight(
        0.25f, 0.6f, 1.0f,
        0.1f, 6.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        1.0f, 0.009f, 0.00032f,
        16.0f
    );
    spotLightCount++;

    spotLights[3] = SpotLight(
        0.25f, 0.6f, 1.0f,
        0.1f, 6.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        1.0f, 0.009f, 0.00032f,
        16.0f
    );
    spotLightCount++;

    // === Estado del Helicóptero y Lámpara ===
    struct HelicopterState {
        glm::vec3 pos{ -8.0f, 6.0f, -12.0f };
        float yawDeg{ 0.0f };
        float speed{ 4.0f };
        int   spotIdx{ -1 };
    } heli;

    const glm::vec3 lampPos = glm::vec3(6.0f, 0.0f, -10.0f);

    // === Spotlight del helicóptero (amarilla) ===
    {
        const GLfloat edgeDeg = 30.0f;
        int idx = spotLightCount++;
        heli.spotIdx = idx;
        spotLights[idx] = SpotLight(
            1.0f, 0.95f, 0.60f,
            0.15f, 4.0f,
            heli.pos.x, heli.pos.y, heli.pos.z,
            0.0f, -1.0f, 0.0f,
            1.0f, 0.045f, 0.0072f,
            edgeDeg
        );
    }

    // === Punto de luz blanca de la lámpara ===
    {
        glm::vec3 bulb = lampPos + glm::vec3(0.0f, 2.0f, 0.0f);
        pointLights[pointLightCount] = PointLight(
            1.0f, 1.0f, 1.0f,
            0.0f, 1.0f,
            bulb.x, bulb.y, bulb.z,
            1.0f, 0.09f, 0.032f
        );
        pointLightCount++;
    }

    // === Punto de luz de la lámpara (blanca) ===
    {
        // Bombilla: un poco por debajo del extremo superior del modelo (~13.9)
        glm::vec3 bulb = streetLampPos + glm::vec3(0.0f, kLampScale * 13.9f, 0.0f);

        pointLights[pointLightCount] = PointLight(
            1.0f, 1.0f, 1.0f,        // color
            0.05f, 1.6f,             // ambient leve, diffuse brillante
            bulb.x, bulb.y, bulb.z,  // posición
            1.0f, 0.045f, 0.0075f    // atenuación media (bonita caída)
        );
        pointLightCount++;
    }



    GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
        uniformSpecularIntensity = 0, uniformShininess = 0;
    GLuint uniformColor = 0;

    glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

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

        // Clear the window
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        skybox.DrawSkybox(camera.calculateViewMatrix(), projection);

        shaderList[0].UseShader();
        uniformModel = shaderList[0].GetModelLocation();
        uniformProjection = shaderList[0].GetProjectionLocation();
        uniformView = shaderList[0].GetViewLocation();
        uniformEyePosition = shaderList[0].GetEyePositionLocation();
        uniformColor = shaderList[0].getColorLocation();

        //información en el shader de intensidad especular y brillo
        uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
        uniformShininess = shaderList[0].GetShininessLocation();

        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
        glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

        // luz ligada a la cámara de tipo flash
        glm::vec3 lowerLight = camera.getCameraPosition();
        lowerLight.y -= 0.3f;
        spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());

        // === INPUT: abrir/cerrar cofre ===
        mainWindow.ProcessGameplayInput(car, deltaTime);

        // === Posicion faros con el coche ===
        {
            glm::vec3 pL, dL, pR, dR;
            car.getHeadlightWorld(true, pL, dL);
            car.getHeadlightWorld(false, pR, dR);
            spotLights[2].SetFlash(pL, dL);
            spotLights[3].SetFlash(pR, dR);
        }

        // ===Ajuste===
        {
            mainWindow.ProcessDebugTuning(off);
            car.setOffsetsRaw(off);
        }

        // === Input helicóptero (fuera de main) ===
        mainWindow.ProcessHelicopterInput(heli, deltaTime);

        // Spotlight del heli sigue su posición/orientación
        if (heli.spotIdx >= 0) {
            mainWindow.UpdateHelicopterSpotlight(heli, spotLights[heli.spotIdx]);
        }

        //información al shader de fuentes de iluminación
        shaderList[0].SetDirectionalLight(&mainLight);
        shaderList[0].SetPointLights(pointLights, pointLightCount);
        shaderList[0].SetSpotLights(spotLights, spotLightCount);

        glm::mat4 model(1.0);
        glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

        // Piso
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        pisoTexture.UseTexture();
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[2]->RenderMesh();

        // === CARRO COMPLETO ===
        car.setBasePosition(glm::vec3(0.0f, -2.2f, -14.0f - mainWindow.getmuevex()));
        car.render(uniformModel, uniformColor);

        // === HELICÓPTERO === (con estado)
        model = glm::mat4(1.0);
        model = glm::translate(model, heli.pos);
        model = glm::rotate(model, glm::radians(heli.yawDeg), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
        model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Blackhawk_M.RenderModel();

        // === LÁMPARA (modelo) ===
        const glm::vec3 lampPos = glm::vec3(6.0f, 0.0f, -10.0f);
        model = glm::mat4(1.0);
        model = glm::translate(model, lampPos);
        model = glm::scale(model, glm::vec3(0.6f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Lampara_M.RenderModel();

        // Agave
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(0.0f, 1.0f, -4.0f));
        model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

        //blending: transparencia o traslucidez
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        AgaveTexture.UseTexture();
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[3]->RenderMesh();
        glDisable(GL_BLEND);

        // ===STREET LAMP===
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, streetLampPos);
            model = glm::scale(model, glm::vec3(kLampScale));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
            StreetLamp_M.RenderModel();

        }
        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}



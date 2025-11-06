/*  (… cabecera idéntica; no la recorto para que pegues tal cual)  */
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <array>
#include <map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "Window.h"
#include "Mesh.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Model.h"
#include "Skybox.h"

// Iluminación
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"

const float toRadians = 3.14159265f / 180.0f;

/*================== Globals base ==================*/
Window mainWindow;
std::vector<Mesh*>  meshList;
std::vector<Shader> shaderList;
Camera camera;

// Shaders
static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";

// Texturas
Texture pisoTexture;
Texture LetrasBobTex;
Texture texPuertasMadera, texMarcoPiedra;
// Textura extra (balón)
Texture balonTexture;

// Cielo
Skybox skybox;

// Iluminación
DirectionalLight mainLight;
PointLight  pointLights[MAX_POINT_LIGHTS];
SpotLight   spotLights[MAX_SPOT_LIGHTS];

// Materiales
Material Material_brillante, Material_opaco;

// Timing
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

/*================== MARQUESINA (letras) ==================*/
struct Glyph { float u0, v0, u1, v1, width; };
std::map<char, Glyph> gFont;
const char* kMarquesinaText = " PROYECTO CGEIHC ";
int   kMarquesinaLen = 0;
float marquesinaTimer = 0.0f;
int   marquesinaOffset = 0;
const float marquesinaInterval = 0.25f;

/*================== Objetos / Modelos ==================*/
Model Crustaceo_M;
Model MarcoPuerta, Letrero;
Model StreetLamp_M;
Model Gary_M;

// === Modelos nuevos ===
Model puerta;
Model banca;
Model ring;
Model lampara;
Model lampara1;
Model casa2;
Model edificios;
Model balon;
Model casas;

/*================== Puerta / Marco / Letrero ==================*/
glm::vec3 gPuertaBasePos = glm::vec3(5.0f, -1.0f, 70.0f);
glm::mat4 gMarcoModel(1.0f), gLetreroModel(1.0f);

/*================== Luces fijas ==================*/
const float     kLampScale = 2.0f;
const glm::vec3 streetLampPos = glm::vec3(6.0f, -1.3f, -40.0f);
glm::vec3       gLampWorldPos = glm::vec3(0.0f, -2.2f, -30.0f);
float           gLampScale = 45.0f;

/*================== Piso ==================*/
const float floorScale = 30.0f;

/*================== Gary (estado) ==================*/
struct GaryState {
    glm::vec3 pos{ -20.0f, -0.9f, 0.0f };
    float     yawDeg{ 0.0f };
    float     scale{ 7.0f };
    float     step{ 0.8f };
    float     k{ 2.0f * 3.14159265f / 12.0f };
    float     ampZ{ 2.0f };
    float     ampY{ 0.06f };
    float     baseY{ -0.9f };
} gGary;

/*================== Keyframes (Gary) ==================*/
struct Keyframe { glm::vec3 pos, rot; float t; };
std::vector<Keyframe> gKF;
bool   gKF_Play = false;
float  gKF_PlayTime = 0.0f;
float  gKF_SegmentDur = 0.8f;
std::string gKF_LastPath = "keyframes_gary.txt";

/*================== Utilidades Keyframes ==================*/
static std::string NowTimestamp() {
    std::time_t t = std::time(nullptr); std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    tm = *std::localtime(&t);
#endif
    std::ostringstream oss; oss << std::put_time(&tm, "%Y%m%d_%H%M%S");
    return oss.str();
}
static void KF_Print() {
    printf("=== KEYFRAMES (%zu) ===\n", gKF.size());
    for (size_t i = 0; i < gKF.size(); ++i) {
        auto& k = gKF[i];
        printf("[%zu] t=%.3f pos(%.3f,%.3f,%.3f) rot(%.3f,%.3f,%.3f)\n",
            i, k.t, k.pos.x, k.pos.y, k.pos.z, k.rot.x, k.rot.y, k.rot.z);
    }
    printf("========================\n");
}
static void KF_Save(const std::string& path = "") {
    std::string out = path.empty() ? ("keyframes_" + NowTimestamp() + ".txt") : path;
    gKF_LastPath = out; std::ofstream f(out);
    if (!f) { fprintf(stderr, "No pude abrir: %s\n", out.c_str()); return; }
    f << "# index\tt(s)\tpos.x\tpos.y\tpos.z\trot.x\trot.y\trot.z\n";
    for (size_t i = 0; i < gKF.size(); ++i) {
        auto& k = gKF[i];
        f << i << "\t" << std::fixed << std::setprecision(3) << k.t << "\t"
            << k.pos.x << "\t" << k.pos.y << "\t" << k.pos.z << "\t"
            << k.rot.x << "\t" << k.rot.y << "\t" << k.rot.z << "\n";
    }
    printf("Keyframes guardados en: %s\n", out.c_str());
}
static void KF_CaptureCurrent(float tRel = -1.0f) {
    float t = (tRel >= 0.0f) ? tRel : (gKF.empty() ? 0.0f : gKF.back().t + gKF_SegmentDur);
    gKF.push_back({ gGary.pos, glm::vec3(0.0f, gGary.yawDeg, 0.0f), t });
    printf("Capturado KF[%zu] t=%.3f\n", gKF.size() - 1, t);
}
static float Lerp(float a, float b, float u) { return a + (b - a) * u; }
static glm::vec3 Lerp3(const glm::vec3& A, const glm::vec3& B, float u) {
    return glm::vec3(Lerp(A.x, B.x, u), Lerp(A.y, B.y, u), Lerp(A.z, B.z, u));
}
static glm::vec3 LerpRot(const glm::vec3& A, const glm::vec3& B, float u) { return Lerp3(A, B, u); }
static void KF_UpdateAndApply(float dt) {
    if (!gKF_Play || gKF.size() < 2) return;
    gKF_PlayTime += dt;
    size_t i1 = 0, i2 = 1;
    for (size_t i = 1; i < gKF.size(); ++i) {
        if (gKF_PlayTime <= gKF[i].t) { i2 = i; i1 = i - 1; break; }
        if (i == gKF.size() - 1) { gGary.pos = gKF.back().pos; gGary.yawDeg = gKF.back().rot.y; gKF_Play = false; printf("Reproduccion finalizada.\n"); return; }
    }
    const Keyframe& A = gKF[i1]; const Keyframe& B = gKF[i2];
    float span = glm::max(0.0001f, B.t - A.t);
    float u = glm::clamp((gKF_PlayTime - A.t) / span, 0.0f, 1.0f);
    gGary.pos = Lerp3(A.pos, B.pos, u);
    gGary.yawDeg = LerpRot(A.rot, B.rot, u).y;
}
static void KF_GenerateSineReturn(int samples = 24) {
    if (gKF.size() < 2) { printf("Necesitas ≥2 KF de ida.\n"); return; }
    if (samples < 2) samples = 2;
    const glm::vec3 Pstart = gKF.back().pos, Pend = gKF.front().pos;
    float t = gKF.back().t;
    float x0 = Pstart.x, x1 = Pend.x;
    for (int i = 0; i < samples; ++i) {
        float u = (float)i / (samples - 1);
        float x = glm::mix(x0, x1, u);
        float z = gGary.ampZ * sinf(gGary.k * x);
        float y = gGary.baseY + gGary.ampY * sinf(2.0f * gGary.k * x);
        float xNext = glm::mix(x0, x1, glm::clamp(u + 1.0f / (samples - 1), 0.0f, 1.0f));
        float yawAlong = (xNext - x) >= 0.0f ? 0.0f : 180.0f;
        gKF.push_back({ glm::vec3(x,y,z), glm::vec3(0.0f,yawAlong,0.0f), t });
        t += gKF_SegmentDur;
    }
    printf("Regreso senoidal generado.\n");
}

/*================== Helper de movimiento por salto ==================*/
static inline void MoveGaryStep(float dx) {
    float x = gGary.pos.x + dx;
    float z = gGary.ampZ * sinf(gGary.k * x);
    float y = gGary.baseY + gGary.ampY * sinf(2.0f * gGary.k * x);
    y = glm::max(y, -1.0f + 0.001f);
    gGary.yawDeg = (dx >= 0.0f) ? 0.0f : 180.0f;
    gGary.pos = glm::vec3(x, y, z);
}

/*================== Objetos simples ==================*/
void CreateObjects() {
    unsigned int floorIndices[] = { 0,2,1, 1,2,3 };
    GLfloat floorVertices[] = {
        -10.0f,0.0f,-10.0f, 0.0f,0.0f, 0,-1,0,
         10.0f,0.0f,-10.0f,10.0f,0.0f, 0,-1,0,
        -10.0f,0.0f, 10.0f, 0.0f,10.0f,0,-1,0,
         10.0f,0.0f, 10.0f,10.0f,10.0f,0,-1,0
    };
    Mesh* floor = new Mesh(); floor->CreateMesh(floorVertices, floorIndices, 32, 6); meshList.push_back(floor);

    unsigned int quadIdx[] = { 0,1,2, 0,2,3 };
    GLfloat quadV[] = {
        -0.5f,0.0f, 0.5f, 0.0f,0.0f, 0,-1,0,
         0.5f,0.0f  ,0.5f, 1.0f,0.0f, 0,-1,0,
         0.5f,0.0f ,-0.5f, 1.0f,1.0f, 0,-1,0,
        -0.5f,0.0f,-0.5f, 0.0f,1.0f, 0,-1,0
    };
    Mesh* quad = new Mesh(); quad->CreateMesh(quadV, quadIdx, 32, 6); meshList.push_back(quad);
}

/*================== Shaders ==================*/
void CreateShaders() {
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);
}

/*================== main ==================*/
int main() {
    mainWindow = Window(1366, 768);
    mainWindow.Initialise();

    CreateObjects();
    CreateShaders();

    camera = Camera(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), -60.0f, 0.0f, 9.5f, 0.8f);

    // Texturas
    pisoTexture = Texture("Textures/Suelo.jpg");               pisoTexture.LoadTextureA();
    texPuertasMadera = Texture("Textures/PuertasMadera.png");       texPuertasMadera.LoadTextureA();
    texMarcoPiedra = Texture("Textures/MarcoPiedra.png");         texMarcoPiedra.LoadTextureA();
    LetrasBobTex = Texture("Textures/LetrasBob.png");           LetrasBobTex.LoadTextureA();
    // Balón (por si el OBJ no trae MTL)
    balonTexture = Texture("Textures/10536_soccerball_V1_diffuse.jpg"); balonTexture.LoadTextureA();

    // Fuente (marquesina) – igual que antes
    {
        const float TW = 512.0f, TH = 512.0f;
        auto makeGlyph = [&](int xl, int yt, int xr, int yb)->Glyph {
            Glyph g; g.u0 = xl / TW; g.u1 = xr / TW;
            float vTop = 1.0f - (float)yt / TH, vBot = 1.0f - (float)yb / TH;
            g.v0 = vBot; g.v1 = vTop; g.width = float(xr - xl) / 50.0f; return g;
            };
        gFont['C'] = makeGlyph(191, 10, 238, 78); gFont['G'] = makeGlyph(43, 113, 93, 184);
        gFont['E'] = makeGlyph(346, 11, 389, 78); gFont['H'] = makeGlyph(121, 117, 171, 183);
        gFont['I'] = makeGlyph(197, 118, 219, 184); gFont['O'] = makeGlyph(194, 217, 244, 282);
        gFont['P'] = makeGlyph(273, 216, 317, 284); gFont['R'] = makeGlyph(421, 218, 470, 293);
        gFont['T'] = makeGlyph(101, 318, 147, 386); gFont['Y'] = makeGlyph(182, 423, 232, 497);
        gFont[' '] = { 0,0,0,0,0.35f };
        kMarquesinaLen = (int)strlen(kMarquesinaText);
    }

    // Modelos base
    Crustaceo_M.LoadModel("Models/crustaceo.obj");
    StreetLamp_M.LoadModel("Models/StreetLamp.obj");
    MarcoPuerta.LoadModel("Models/MarcoPuerta.obj");
    Letrero.LoadModel("Models/Letrero.obj");
    Gary_M.LoadModel("Models/GaryBob.obj");

    // === Cargar modelos nuevos ===
    banca.LoadModel("Models/Banca.fbx");
    ring.LoadModel("Models/ringT.obj");
    lampara1.LoadModel("Models/Lamp_Text.obj");     
    lampara.LoadModel("Models/lampara.fbx");       
    puerta.LoadModel("Models/puerta.obj");
    casa2.LoadModel("Models/casa2.fbx");
    edificios.LoadModel("Models/edificiosEjemplo.fbx");
    balon.LoadModel("Models/10536_soccerball_V1_iterations-2.obj");
    casas.LoadModel("Models/casas.fbx");

    // Skybox
    std::vector<std::string> skyFaces = {
        "Textures/Skybox/cupertin-lake_rt.tga","Textures/Skybox/cupertin-lake_lf.tga",
        "Textures/Skybox/cupertin-lake_dn.tga","Textures/Skybox/cupertin-lake_up.tga",
        "Textures/Skybox/cupertin-lake_bk.tga","Textures/Skybox/cupertin-lake_ft.tga"
    };
    skybox = Skybox(skyFaces);

    // Materiales
    Material_brillante = Material(4.0f, 256);
    Material_opaco = Material(0.3f, 4);

    // Luz direccional
    mainLight = DirectionalLight(1, 1, 1, 0.3f, 0.3f, 0, 0, -1);

    unsigned int pointLightCount = 0, spotLightCount = 0;
    pointLights[pointLightCount++] = PointLight(0.6f, 0.2f, 0.8f, 0.1f, 2.2f, 0, 0, 0, 1.0f, 0.022f, 0.0019f);

    const int camSpotIdx = spotLightCount;
    spotLights[spotLightCount++] = SpotLight(1, 1, 1, 0, 2, 0, 0, 0, 0, -1, 0, 1.0f, 0.09f, 0.032f, 12.0f);

    int streetPointIdx = pointLightCount;
    pointLights[pointLightCount++] = PointLight(
        1, 1, 1, 0.12f, 2.4f,
        streetLampPos.x, streetLampPos.y + kLampScale * 13.9f, streetLampPos.z,
        1.0f, 0.022f, 0.0019f
    );

    gMarcoModel = glm::scale(glm::translate(glm::mat4(1), gPuertaBasePos), glm::vec3(1.2f));
    gLetreroModel = glm::scale(glm::translate(glm::mat4(1), gPuertaBasePos + glm::vec3(-10.0f, 30.0f, -7.0f)), glm::vec3(10.0f));

    // Uniforms
    GLuint uProj = 0, uModel = 0, uView = 0, uEye = 0, uSpec = 0, uShine = 0, uColor = 0, uOffset = 0, uSub = 0;
    glm::mat4 projection = glm::perspective(45.0f,
        (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 2000.0f);

    static std::array<bool, 1024> prevKey{}; prevKey.fill(false);
    auto keyOnce = [&](int key)->bool {
        bool cur = mainWindow.getsKeys()[key];
        bool fire = cur && !prevKey[key];
        prevKey[key] = cur; return fire;
        };
    auto movementEnabled = [&]() { return !gKF_Play; };

    while (!mainWindow.getShouldClose()) {
        GLfloat now = glfwGetTime(); deltaTime = now - lastTime; lastTime = now;

        marquesinaTimer += deltaTime;
        if (marquesinaTimer >= 0.25f) { marquesinaTimer = 0.0f; marquesinaOffset = (marquesinaOffset + 1) % kMarquesinaLen; }

        glfwPollEvents();
        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

        if (movementEnabled()) {
            if (keyOnce(GLFW_KEY_RIGHT)) MoveGaryStep(+gGary.step);
            if (keyOnce(GLFW_KEY_LEFT)) MoveGaryStep(-gGary.step);
            if (keyOnce(GLFW_KEY_PAGE_UP)) { gGary.baseY += 0.02f; MoveGaryStep(0.0f); }
            if (keyOnce(GLFW_KEY_PAGE_DOWN)) { gGary.baseY -= 0.02f; MoveGaryStep(0.0f); }
        }
        if (keyOnce(GLFW_KEY_K)) KF_CaptureCurrent();
        if (keyOnce(GLFW_KEY_L)) { KF_Print(); KF_Save(); }
        if (keyOnce(GLFW_KEY_C)) { gKF.clear(); gKF_Play = false; gKF_PlayTime = 0; printf("KF limpiados.\n"); }
        if (keyOnce(GLFW_KEY_J)) { if (gKF.size() >= 2) { gKF_Play = true; gKF_PlayTime = 0; printf("Play KF\n"); } }
        if (keyOnce(GLFW_KEY_G)) { KF_GenerateSineReturn(24); KF_Print(); KF_Save("keyframes_gary.txt"); }
        KF_UpdateAndApply(deltaTime);

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        skybox.DrawSkybox(camera.calculateViewMatrix(), projection);

        shaderList[0].UseShader();
        uModel = shaderList[0].GetModelLocation();
        uProj = shaderList[0].GetProjectionLocation();
        uView = shaderList[0].GetViewLocation();
        uEye = shaderList[0].GetEyePositionLocation();
        uSpec = shaderList[0].GetSpecularIntensityLocation();
        uShine = shaderList[0].GetShininessLocation();
        uColor = shaderList[0].getColorLocation();
        uOffset = shaderList[0].getOffsetLocation();
        uSub = shaderList[0].getSubSizeLocation();

        glUniformMatrix4fv(uProj, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
        glUniform3f(uEye, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

        glm::vec3 lower = camera.getCameraPosition(); lower.y -= 0.3f;
        spotLights[camSpotIdx].SetFlash(lower, camera.getCameraDirection());

        const glm::mat4 streetLampModelR = glm::scale(glm::translate(glm::mat4(1), streetLampPos), glm::vec3(kLampScale));
        const glm::vec3 streetBulbWorld = glm::vec3(streetLampModelR * glm::vec4(0, 13.9f, 0, 1));
        pointLights[streetPointIdx] = PointLight(1, 1, 1, 0.12f, 2.4f, streetBulbWorld.x, streetBulbWorld.y, streetBulbWorld.z, 1.0f, 0.022f, 0.0019f);

        shaderList[0].SetDirectionalLight(&mainLight);
        shaderList[0].SetPointLights(pointLights, pointLightCount);
        shaderList[0].SetSpotLights(spotLights, spotLightCount);

        // Piso
        {
            glUniform2fv(uOffset, 1, glm::value_ptr(glm::vec2(0, 0)));
            glUniform2fv(uSub, 1, glm::value_ptr(glm::vec2(1, 1)));
            glm::mat4 M = glm::scale(glm::translate(glm::mat4(1), glm::vec3(0, -1, 0)), glm::vec3(floorScale, 1, floorScale));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(M));
            glUniform3fv(uColor, 1, glm::value_ptr(glm::vec3(1)));
            pisoTexture.UseTexture();
            Material_opaco.UseMaterial(uSpec, uShine);
            meshList[0]->RenderMesh();
        }

        // StreetLamp
        { glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(streetLampModelR)); StreetLamp_M.RenderModel(); }

        // Crustáceo
        {
            glm::mat4 M = glm::translate(glm::mat4(1), glm::vec3(100.0f, -2.2f, -18.0f));
            M = glm::scale(M, glm::vec3(8.8f));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(M));
            Crustaceo_M.RenderModel();
        }

        // Marco
        {
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(gMarcoModel));
            texMarcoPiedra.UseTexture(); Material_opaco.UseMaterial(uSpec, uShine);
            MarcoPuerta.RenderModel();
        }

        // Letrero
        {
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(gLetreroModel));
            texPuertasMadera.UseTexture(); Material_opaco.UseMaterial(uSpec, uShine);
            Letrero.RenderModel();
        }

        // Marquesina
        {
            glm::mat4 base = glm::mat4(1);
            base = glm::translate(base, gPuertaBasePos + glm::vec3(-6.0f, 35.0f, 5.5f));
            base = glm::rotate(base, glm::radians(90.0f), glm::vec3(1, 0, 0));
            glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            LetrasBobTex.UseTexture(); Material_opaco.UseMaterial(uSpec, uShine);
            float cursorX = 0.0f; const int visible = 15; float globalScale = 1.0f;
            for (int i = 0; i < visible; ++i) {
                int idx = (marquesinaOffset + i) % kMarquesinaLen;
                char ch = kMarquesinaText[idx]; auto it = gFont.find(ch);
                if (it == gFont.end()) { cursorX += 0.35f; continue; }
                const Glyph& g = it->second; float subU = g.u1 - g.u0, subV = g.v1 - g.v0;
                glm::mat4 M = glm::translate(base, glm::vec3(cursorX, 0, 0));
                M = glm::scale(M, glm::vec3(globalScale));
                glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(M));
                glUniform3fv(uColor, 1, glm::value_ptr(glm::vec3(1)));
                glUniform2fv(uOffset, 1, glm::value_ptr(glm::vec2(g.u0, g.v0)));
                glUniform2fv(uSub, 1, glm::value_ptr(glm::vec2(subU, subV)));
                meshList[1]->RenderMesh();
                cursorX += g.width;
            }
            glDisable(GL_BLEND);
            // RESET uniforms del atlas (arreglo mínimo)
            glUniform2fv(uOffset, 1, glm::value_ptr(glm::vec2(0, 0)));
            glUniform2fv(uSub, 1, glm::value_ptr(glm::vec2(1, 1)));
        }

        // Gary
        {
            glm::mat4 M = glm::translate(glm::mat4(1), gGary.pos);
            M = glm::rotate(M, glm::radians(gGary.yawDeg), glm::vec3(0, 1, 0));
            M = glm::scale(M, glm::vec3(3.0));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(M));
            glUniform3fv(uColor, 1, glm::value_ptr(glm::vec3(1)));
            Gary_M.RenderModel();
        }

        /* ===================== MODELOS NUEVOS ===================== */

        // lampara1 
        {
            glm::mat4 M = glm::mat4(1);
            M = glm::translate(M, glm::vec3(100.0f, -1.0f, 15.0f));
            M = glm::scale(M, glm::vec3(3.0f));
            M = glm::rotate(M, -90.0f * toRadians, glm::vec3(0, 1, 0));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(M));
            lampara1.RenderModel();
        }

        // casa2
        {
            glm::mat4 M = glm::mat4(1);
            M = glm::translate(M, glm::vec3(100.0f, 0.0f, 100.0f));
            M = glm::scale(M, glm::vec3(2.5f));
            M = glm::rotate(M, -90.0f * toRadians, glm::vec3(1, 0, 0));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(M));
            casa2.RenderModel();
        }

        // edificios
        {
            glm::mat4 M = glm::mat4(1);
            M = glm::translate(M, glm::vec3(100.0f, -1.0f, -60.0f));
            M = glm::scale(M, glm::vec3(10.0f));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(M));
            edificios.RenderModel();
        }

        // casas
        {
            glm::mat4 M = glm::mat4(1);
            M = glm::translate(M, glm::vec3(60.0f, -1.0f, -80.0f));
            M = glm::scale(M, glm::vec3(8.0f));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(M));
            casas.RenderModel();
        }

        // puerta (Monsters Inc style)
        {
            glm::mat4 M = glm::mat4(1);
            M = glm::translate(M, glm::vec3(10.0f, 0.0f, -10.0f));
            M = glm::scale(M, glm::vec3(0.08f));
            M = glm::rotate(M, 90.0f * toRadians, glm::vec3(1, 0, 0));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(M));
            puerta.RenderModel();
        }

        // banca 
        {
            glm::mat4 M = glm::mat4(1);
            M = glm::translate(M, glm::vec3(-10.0f, 0.0f, 10.0f));
            M = glm::scale(M, glm::vec3(10.0f));
            M = glm::rotate(M, -90.0f * toRadians, glm::vec3(0, 0, 1));
            M = glm::rotate(M, -90.0f * toRadians, glm::vec3(0, 1, 0));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(M));
            banca.RenderModel();
            
        }

        // lampara 
        {
            glm::mat4 M = glm::mat4(1);
            M = glm::translate(M, glm::vec3(100.0f, -2.0f, 30.0f));
            M = glm::scale(M, glm::vec3(1.5f));
            M = glm::rotate(M, -90.0f * toRadians, glm::vec3(0, 1, 0));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(M));
            lampara.RenderModel();
        }

        // ring
        {
            glm::mat4 M = glm::mat4(1);
            M = glm::translate(M, glm::vec3(10.0f, 0.0f, 0.0f));
            M = glm::scale(M, glm::vec3(0.1f));
            M = glm::rotate(M, 90.0f * toRadians, glm::vec3(0, 1, 0));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(M));
            ring.RenderModel();
        }

        // balón 
        {
            glm::mat4 M = glm::mat4(1);
            M = glm::translate(M, glm::vec3(5.0f, 0.2f, 1.0f));
            M = glm::scale(M, glm::vec3(0.4f));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(M));
            balonTexture.UseTexture();  // coméntalo si el OBJ ya trae su material
            balon.RenderModel();
        }

        /* =================== FIN MODELOS NUEVOS =================== */

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}

#version 330

in vec4 vCol;
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in vec4 vColor;

out vec4 color;

const int MAX_POINT_LIGHTS = 5;
const int MAX_SPOT_LIGHTS  = 8;

struct Light {
    vec3 color;
    float ambientIntensity;
    float diffuseIntensity;
};

struct DirectionalLight {
    Light base;
    vec3 direction;
};

struct PointLight {
    Light base;
    vec3 position;
    float constant;
    float linear;
    float exponent;
};

struct SpotLight {
    PointLight base;
    vec3 direction;
    float edge;
};

struct Material {
    float specularIntensity;
    float shininess;
};

uniform int pointLightCount;
uniform int spotLightCount;

uniform DirectionalLight directionalLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight  spotLights[MAX_SPOT_LIGHTS];

uniform sampler2D theTexture;
uniform Material  material;
uniform vec3      eyePosition;

vec4 CalcLightByDirection(Light light, vec3 direction)
{
    vec4 ambientcolor = vec4(light.color, 1.0f) * light.ambientIntensity;
    float diffuseFactor = max(dot(normalize(Normal), normalize(direction)), 0.0f);
    vec4 diffusecolor = vec4(light.color * light.diffuseIntensity * diffuseFactor, 1.0f);

    vec4 specularcolor = vec4(0, 0, 0, 0);
    if(diffuseFactor > 0.0f)
    {
        vec3 fragToEye = normalize(eyePosition - FragPos);
        vec3 reflectedVertex = normalize(reflect(direction, normalize(Normal)));

        float specularFactor = dot(fragToEye, reflectedVertex);
        if(specularFactor > 0.0f)
        {
            specularFactor = pow(specularFactor, material.shininess);
            specularcolor = vec4(light.color * material.specularIntensity * specularFactor, 1.0f);
        }
    }

    return (ambientcolor + diffusecolor + specularcolor);
}

vec4 CalcDirectionalLight() { return CalcLightByDirection(directionalLight.base, directionalLight.direction); }

vec4 CalcPointLight(PointLight pLight)
{
    vec3 direction = FragPos - pLight.position;
    float distance = length(direction);
    direction = normalize(direction);

    vec4 c = CalcLightByDirection(pLight.base, direction);
    float att = pLight.exponent * distance * distance +
                pLight.linear   * distance +
                pLight.constant;

    return c / att;
}

vec4 CalcSpotLight(SpotLight sLight)
{
    vec3 rayDirection = normalize(FragPos - sLight.base.position);
    float slFactor = dot(rayDirection, sLight.direction);

    if(slFactor > sLight.edge)
    {
        vec4 c = CalcPointLight(sLight.base);
        return c * (1.0f - (1.0f - slFactor)*(1.0f/(1.0f - sLight.edge)));
    }
    else
        return vec4(0,0,0,0);
}

vec4 CalcPointLights()
{
    vec4 total = vec4(0,0,0,0);
    for(int i=0;i<pointLightCount;i++)
        total += CalcPointLight(pointLights[i]);
    return total;
}

vec4 CalcSpotLights()
{
    vec4 total = vec4(0,0,0,0);
    for(int i=0;i<spotLightCount;i++)
        total += CalcSpotLight(spotLights[i]);
    return total;
}

void main()
{
    vec4 tex = texture(theTexture, TexCoord);

    // si el atlas trae alfa, no lo dibuja
    if (tex.a < 0.1)
        discard;

    vec4 finalcolor = CalcDirectionalLight();
    finalcolor += CalcPointLights();
    finalcolor += CalcSpotLights();

    color = tex * vColor * finalcolor;
}


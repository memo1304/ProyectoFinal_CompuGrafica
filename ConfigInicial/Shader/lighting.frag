#version 330 core

// ──── IMPORTANTE ────
// El CPU calcula cada frame las velas más cercanas a la cámara
// y envía SOLO esas. El shader NO busca vecinas, solo ilumina.
#define NUM_FLAME_LIGHTS 32      // cuántas velas recibe del CPU por frame
#define FLAME_INTENSITY  0.65f   // intensidad global (1.0 = como antes)

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirLight {
    vec3 direction;
    vec3 ambient, diffuse, specular;
};

struct SpotLight {
    vec3 position, direction;
    float cutOff, outerCutOff;
    float constant, linear, quadratic;
    vec3 ambient, diffuse, specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
out vec4 color;

uniform vec3      viewPos;
uniform DirLight  dirLight;
uniform SpotLight spotLight;
uniform Material  material;
uniform int       transparency;
uniform vec3      emissive;

// ★ Solo 32 velas (las más cercanas, calculadas en CPU)
uniform vec3  flamePos[NUM_FLAME_LIGHTS];
uniform float uTime;

vec3 CalcDirLight  (DirLight  L, vec3 N, vec3 V);
vec3 CalcSpotLight (SpotLight L, vec3 N, vec3 P, vec3 V);
vec3 CalcAllFlames (vec3 N, vec3 P, vec3 V);

vec3 sampleAlbedo()
{
    vec3 a = texture(material.diffuse, TexCoords).rgb;
    return max(a, vec3(0.12));
}

void main()
{
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);

    vec3 result = CalcDirLight(dirLight, N, V);
    result += CalcAllFlames(N, FragPos, V);
    result += CalcSpotLight(spotLight, N, FragPos, V);

    vec3 albedo = texture(material.diffuse, TexCoords).rgb;
    result = max(result, emissive * albedo);

    color = vec4(result, texture(material.diffuse, TexCoords).a);
    if (color.a < 0.1 && transparency == 1)
        discard;
}

vec3 CalcDirLight(DirLight L, vec3 N, vec3 V)
{
    vec3 Ld = normalize(-L.direction);
    float d = max(dot(N, Ld), 0.0);
    vec3 R = reflect(-Ld, N);
    float s = pow(max(dot(V, R), 0.0), material.shininess);

    vec3 albedo = sampleAlbedo();
    return (L.ambient * albedo)
         + (L.diffuse * d * albedo)
         + (L.specular * s * vec3(texture(material.specular, TexCoords)));
}

// Ahora solo itera sobre las 32 velas YA ordenadas por proximidad al CPU
vec3 CalcAllFlames(vec3 N, vec3 P, vec3 V)
{
    vec3 albedo  = sampleAlbedo();
    vec3 specTex = vec3(texture(material.specular, TexCoords));
    vec3 total   = vec3(0.0);

    for (int i = 0; i < NUM_FLAME_LIGHTS; i++)
    {
        vec3  toL  = (flamePos[i] + vec3(0.0, 0.08, 0.0)) - P;
        float dist = max(length(toL), 0.0001);

        float seed    = uTime + float(i) * 0.35;
        float flicker = 0.85 + 0.10 * sin(seed * 5.1) + 0.05 * sin(seed * 12.3);

        vec3  Ld = toL / dist;
        float d  = max(dot(N, Ld), 0.0);
        vec3  R  = reflect(-Ld, N);
        float s  = pow(max(dot(V, R), 0.0), material.shininess);

        float att = 1.0 / (1.0 + 0.20 * dist + 0.18 * dist * dist);

        vec3 amb = vec3(0.20, 0.06, 0.00) * flicker;
        vec3 dif = vec3(1.20, 0.55, 0.15) * flicker;
        vec3 spc = vec3(1.00, 0.60, 0.20) * flicker;

        total += att * ( (amb * albedo)
                       + (dif * d * albedo)
                       + (spc * s * specTex) );
    }
    return total * FLAME_INTENSITY;
}

vec3 CalcSpotLight(SpotLight L, vec3 N, vec3 P, vec3 V)
{
    vec3 Ld = normalize(L.position - P);
    float d = max(dot(N, Ld), 0.0);
    vec3 R = reflect(-Ld, N);
    float s = pow(max(dot(V, R), 0.0), material.shininess);

    float dist = length(L.position - P);
    float att = 1.0 / (L.constant + L.linear * dist + L.quadratic * dist * dist);

    float theta = dot(Ld, normalize(-L.direction));
    float eps = L.cutOff - L.outerCutOff;
    float spot = clamp((theta - L.outerCutOff) / eps, 0.0, 1.0);

    vec3 albedo = sampleAlbedo();
    return att * spot * ( (L.ambient * albedo)
                        + (L.diffuse * d * albedo)
                        + (L.specular * s * vec3(texture(material.specular, TexCoords))) );
}
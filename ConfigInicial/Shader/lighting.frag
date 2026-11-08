#version 330 core

#define NUM_FLAMES   169      // debe ser igual a totalFlamas en main.cpp
#define NUM_NEAREST  8       // cuantas velas iluminan cada punto (igual que antes)
#define FLAME_INTENSITY 0.65 // intensidad de TODAS las velas: 1.0 = como antes, menor = mas tenue

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

// ★ NUEVO: posicion de cada vela y tiempo (para el parpadeo)
uniform vec3  flamePos[NUM_FLAMES];
uniform float uTime;

vec3 CalcDirLight  (DirLight  L, vec3 N, vec3 V);
vec3 CalcSpotLight (SpotLight  L, vec3 N, vec3 P, vec3 V);
vec3 CalcAllFlames (vec3 N, vec3 P, vec3 V);   // ★ NUEVO

// Helper: sample albedo con fallback si la textura está vacía/negra
vec3 sampleAlbedo()
{
    vec3 a = texture(material.diffuse, TexCoords).rgb;
    return max(a, vec3(0.12));   // nunca negro absoluto
}

void main()
{
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);

    vec3 result = CalcDirLight(dirLight, N, V);

    // ★ CAMBIO: antes aqui habia un for con las 8 pointLights
    result += CalcAllFlames(N, FragPos, V);

    result += CalcSpotLight(spotLight, N, FragPos, V);

    // brillo propio (emissive)
    vec3 albedo = texture(material.diffuse, TexCoords).rgb;
    result = max(result, emissive * albedo);   // la luna se ilumina a sí misma

    color = vec4(result, texture(material.diffuse, TexCoords).a);
    if (color.a < 0.1 && transparency == 1)
        discard;
}

vec3 CalcDirLight(DirLight L, vec3 N, vec3 V)
{
    vec3 Ld   = normalize(-L.direction);
    float d   = max(dot(N, Ld), 0.0);
    vec3 R    = reflect(-Ld, N);
    float s   = pow(max(dot(V, R), 0.0), material.shininess);

    vec3 albedo = sampleAlbedo();
    return (L.ambient * albedo)
         + (L.diffuse * d * albedo)
         + (L.specular * s * vec3(texture(material.specular, TexCoords)));
}

// ★ NUEVO: para cada fragmento se buscan sus 8 velas mas cercanas y se suma su luz.
//   Mismos colores, atenuacion y parpadeo que las luces puntuales anteriores.
vec3 CalcAllFlames(vec3 N, vec3 P, vec3 V)
{
    // 1) Buscar las NUM_NEAREST velas mas cercanas a este punto (lista ordenada por distancia)
    float bestD2[NUM_NEAREST];
    int   bestI [NUM_NEAREST];
    for (int k = 0; k < NUM_NEAREST; k++) { bestD2[k] = 1e20; bestI[k] = 0; }

    for (int i = 0; i < NUM_FLAMES; i++)
    {
        vec3  dv = (flamePos[i] + vec3(0.0, 0.08, 0.0)) - P;
        float d2 = dot(dv, dv);
        if (d2 < bestD2[NUM_NEAREST - 1])
        {
            int j = NUM_NEAREST - 1;
            while (j > 0 && bestD2[j - 1] > d2)
            {
                bestD2[j] = bestD2[j - 1];
                bestI [j] = bestI [j - 1];
                j--;
            }
            bestD2[j] = d2;
            bestI [j] = i;
        }
    }

    // 2) Sumar la luz de esas velas
    vec3 albedo  = sampleAlbedo();                                // se lee una sola vez
    vec3 specTex = vec3(texture(material.specular, TexCoords));   // se lee una sola vez
    vec3 total   = vec3(0.0);

    for (int k = 0; k < NUM_NEAREST; k++)
    {
        int   idx  = bestI[k];
        vec3  toL  = (flamePos[idx] + vec3(0.0, 0.08, 0.0)) - P;
        float dist = max(length(toL), 0.0001);

        float seed    = uTime + float(idx) * 0.35;
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
    vec3 Ld   = normalize(L.position - P);
    float d   = max(dot(N, Ld), 0.0);
    vec3 R    = reflect(-Ld, N);
    float s   = pow(max(dot(V, R), 0.0), material.shininess);

    float dist = length(L.position - P);
    float att  = 1.0 / (L.constant + L.linear * dist + L.quadratic * dist * dist);

    float theta = dot(Ld, normalize(-L.direction));
    float eps   = L.cutOff - L.outerCutOff;
    float spot  = clamp((theta - L.outerCutOff) / eps, 0.0, 1.0);

    vec3 albedo = sampleAlbedo();
    return att * spot * ( (L.ambient * albedo)
                        + (L.diffuse * d * albedo)
                        + (L.specular * s * vec3(texture(material.specular, TexCoords))) );
}

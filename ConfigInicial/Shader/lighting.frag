#version 330 core

#define NUMBER_OF_POINT_LIGHTS 8

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirLight {
    vec3 direction;
    vec3 ambient, diffuse, specular;
};

struct PointLight {
    vec3 position;
    float constant, linear, quadratic;
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
uniform PointLight pointLights[NUMBER_OF_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform Material  material;
uniform int       transparency;
uniform vec3      emissive; 

vec3 CalcDirLight  (DirLight  L, vec3 N, vec3 V);
vec3 CalcPointLight(PointLight L, vec3 N, vec3 P, vec3 V);
vec3 CalcSpotLight (SpotLight  L, vec3 N, vec3 P, vec3 V);

// Helper: sample albedo con fallback si la textura está vacía/negra
vec3 sampleAlbedo()
{
    vec3 a = texture(material.diffuse, TexCoords).rgb;
    return max(a, vec3(0.12));   // <-- SALVAVIDAS: nunca negro absoluto
}

void main()
{
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);

    vec3 result = CalcDirLight(dirLight, N, V);

    for (int i = 0; i < NUMBER_OF_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], N, FragPos, V);

    result += CalcSpotLight(spotLight, N, FragPos, V);

    // ── NUEVO: brillo propio (emissive) ──
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

vec3 CalcPointLight(PointLight L, vec3 N, vec3 P, vec3 V)
{
    vec3 Ld   = normalize(L.position - P);
    float d   = max(dot(N, Ld), 0.0);
    vec3 R    = reflect(-Ld, N);
    float s   = pow(max(dot(V, R), 0.0), material.shininess);

    float dist = length(L.position - P);
    float att  = 1.0 / (L.constant + L.linear * dist + L.quadratic * dist * dist);

    vec3 albedo = sampleAlbedo();
    return att * ( (L.ambient * albedo)
                 + (L.diffuse * d * albedo)
                 + (L.specular * s * vec3(texture(material.specular, TexCoords))) );
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
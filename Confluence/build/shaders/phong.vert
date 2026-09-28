#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;

uniform mat4 uModel, uView, uProjection;
uniform mat3 uNormalMatrix;      // (M^-1)^T -- correct under non-uniform scale
uniform int  uShadingMode;       // 0=Flat, 1=Gouraud, 2=Phong

struct Light {
    int   type;            // 0 = directional, 1 = point
    vec3  direction;
    vec3  position;
    vec3  diffuse, specular;
    vec3  attenuation;     // (a0, a1, a2)
    float enabled;
};
uniform Light uLights[2];   // [0] = sun, [1] = sky fill
uniform vec3  uGlobalAmbient;
uniform vec3  uViewPos;
uniform vec3  uKa, uKd, uKs, uEmission;
uniform float uShininess;
uniform int   uUseBlinn;
uniform int   uTermMask;         // bit 0 ambient, 1 diffuse, 2 specular

out vec3 vFragPos;
out vec3 vNormal;
out vec3 vColor;
out vec3 vGouraudColor;

vec3 computeLighting(vec3 N, vec3 fragPos, vec3 albedo) {
    vec3 V = normalize(uViewPos - fragPos);

    vec3 result = vec3(0.0);
    if ((uTermMask & 1) != 0) result += uKa * albedo * uGlobalAmbient;
    result += uEmission;

    for (int i = 0; i < 2; ++i) {
        if (uLights[i].enabled < 0.5) continue;

        vec3  L;
        float attenuation = 1.0;
        if (uLights[i].type == 0) {
            L = normalize(-uLights[i].direction);
        } else {
            vec3 toLight = uLights[i].position - fragPos;
            float d = length(toLight);
            L = toLight / d;
            vec3 a = uLights[i].attenuation;
            attenuation = 1.0 / (a.x + a.y * d + a.z * d * d);
        }

        float NdotL = max(dot(N, L), 0.0);
        if ((uTermMask & 2) != 0)
            result += attenuation * uKd * albedo * uLights[i].diffuse * NdotL;

        if (NdotL > 0.0 && (uTermMask & 4) != 0) {
            float spec;
            if (uUseBlinn == 1) {
                vec3 H = normalize(L + V);
                spec = pow(max(dot(N, H), 0.0), uShininess);
            } else {
                vec3 R = reflect(-L, N);
                spec = pow(max(dot(R, V), 0.0), uShininess);
            }
            result += attenuation * uKs * uLights[i].specular * spec;
        }
    }
    return result;
}

void main() {
    vFragPos = vec3(uModel * vec4(aPos, 1.0));
    vNormal  = uNormalMatrix * aNormal;
    vColor   = aColor;

    vGouraudColor = vec3(0.0);
    if (uShadingMode == 1)   // Gouraud: evaluate illumination HERE, at the vertex
        vGouraudColor = computeLighting(normalize(vNormal), vFragPos, aColor);

    gl_Position = uProjection * uView * vec4(vFragPos, 1.0);
}

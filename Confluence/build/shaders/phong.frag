#version 330 core
in vec3 vFragPos;
in vec3 vNormal;
in vec3 vColor;
in vec3 vGouraudColor;
out vec4 FragColor;

uniform int  uShadingMode;       // 0=Flat, 1=Gouraud, 2=Phong

struct Light {
    int   type;
    vec3  direction;
    vec3  position;
    vec3  diffuse, specular;
    vec3  attenuation;
    float enabled;
};
uniform Light uLights[2];
uniform vec3  uGlobalAmbient;
uniform vec3  uViewPos;
uniform vec3  uKa, uKd, uKs, uEmission;
uniform float uShininess;
uniform int   uUseBlinn;
uniform int   uTermMask;

// Aerial perspective: distant geometry fades towards the horizon haze colour.
uniform vec3  uFogColor;
uniform float uFogDensity;

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
    vec3 color;
    if (uShadingMode == 0) {
        vec3 faceN = normalize(cross(dFdx(vFragPos), dFdy(vFragPos)));  // Flat: per-face normal
        color = computeLighting(faceN, vFragPos, vColor);
    } else if (uShadingMode == 1) {
        color = vGouraudColor;                                          // Gouraud: interpolated colour
    } else {
        color = computeLighting(normalize(vNormal), vFragPos, vColor);  // Phong: interpolated normal
    }

    float d = length(vFragPos - uViewPos) * uFogDensity;
    color = mix(color, uFogColor, 1.0 - exp(-d * d));
    FragColor = vec4(color, 1.0);
}

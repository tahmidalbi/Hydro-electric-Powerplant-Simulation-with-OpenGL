// =====================================================================
// lighting_common.glsl  --  ONE lighting formula for every shader stage
// =====================================================================
// Shader.cpp pastes this file into phong.vert, phong.frag and flat.geom
// right after their "#version" line (GLSL 3.30 has no #include).
//
// That is the whole point of Step 3/4: Flat, Gouraud and Phong all use the
// SAME lighting equation below. They differ only in WHERE it is evaluated:
//   Flat    -> once per triangle   (flat.geom, at the triangle centre)
//   Gouraud -> once per vertex     (phong.vert), colours blended across
//   Phong   -> once per pixel      (phong.frag), normals blended across
//
// The Phong LIGHTING MODEL (this formula) is not the same thing as Phong
// SHADING (evaluating it per pixel). Gouraud shading uses this same model.

#define MAX_LIGHTS 3   // must match MAX_LIGHTS in src/core/Light.h

struct Light {
    int   type;            // 0 = directional (sun), 1 = point (lamp)
    vec3  direction;       // directional: the way the light travels
    vec3  position;        // point: where the lamp is
    vec3  diffuse, specular;
    vec3  attenuation;     // point: (a0, a1, a2) in 1 / (a0 + a1*d + a2*d*d)
    float enabled;         // 1 on, 0 off (L key)
};
uniform Light uLights[MAX_LIGHTS];   // [0] sun, [1] sky fill, [2] inspection lamp

uniform vec3  uGlobalAmbient;        // background light from everywhere
uniform vec3  uViewPos;              // camera position (world)
uniform vec3  uKa, uKd, uKs, uEmission;   // material
uniform float uShininess;
uniform int   uUseBlinn;             // B key: 0 Phong reflection vector, 1 Blinn half vector
uniform int   uTermMask;             // K key: bit 0 ambient, bit 1 diffuse, bit 2 specular

// N: unit surface normal, fragPos: world position, albedo: surface colour.
vec3 computeLighting(vec3 N, vec3 fragPos, vec3 albedo) {
    vec3 V = normalize(uViewPos - fragPos);          // towards the camera

    vec3 result = vec3(0.0);

    // Ambient: a small, even amount of light so shadowed sides stay readable.
    if ((uTermMask & 1) != 0) result += uKa * albedo * uGlobalAmbient;

    // Emission: the surface glows by itself (lamp bulb, clouds). Always on.
    result += uEmission;

    for (int i = 0; i < MAX_LIGHTS; ++i) {
        if (uLights[i].enabled < 0.5) continue;

        vec3  L;                  // unit vector from the surface towards the light
        float attenuation = 1.0;
        if (uLights[i].type == 0) {
            L = normalize(-uLights[i].direction);
        } else {
            vec3 toLight = uLights[i].position - fragPos;
            float d = length(toLight);
            L = toLight / d;
            vec3 a = uLights[i].attenuation;
            attenuation = 1.0 / (a.x + a.y * d + a.z * d * d);   // weaker further away
        }

        // Diffuse (Lambert): brightest when the surface faces the light.
        float NdotL = max(dot(N, L), 0.0);
        if ((uTermMask & 2) != 0)
            result += attenuation * uKd * albedo * uLights[i].diffuse * NdotL;

        // Specular: a shiny highlight that depends on where the camera is.
        if (NdotL > 0.0 && (uTermMask & 4) != 0) {
            float spec;
            if (uUseBlinn == 1) {
                vec3 H = normalize(L + V);                 // Blinn: half vector
                spec = pow(max(dot(N, H), 0.0), uShininess);
            } else {
                vec3 R = reflect(-L, N);                   // Phong: mirror direction
                spec = pow(max(dot(R, V), 0.0), uShininess);
            }
            result += attenuation * uKs * uLights[i].specular * spec;
        }
    }
    return result;
}

// Everything that happens to a surface AFTER lighting (waves, water
// reflection, flowing streaks, highlight, fog) is in surface_common.glsl,
// which Shader.cpp pastes right after this file.

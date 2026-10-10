// water.vert - moves water vertices:
//   * vertices lying on the free surface (height == uSurfaceY) get rolling
//     waves (sum of sine waves) -> lakes and channels move;
//   * mode 2 (water inside a pipe): vertices beyond the water "front" are
//     pulled towards the pipe axis, giving the rushing water a rounded nose.
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

uniform mat4 uModel;
uniform mat4 uNormalMat;
uniform mat4 uView;
uniform mat4 uProj;
uniform float uTime;
uniform int   uWaterMode;
uniform float uWaveAmp;
uniform float uWaveFreq;
uniform float uSurfaceY;
uniform float uFront;
uniform float uNoseLen;
uniform float uTubeRadius;
uniform float uFlowSpeed;

out vec3 vWorldPos;
out vec3 vNormal;
out vec2 vUV;
out float vNose;

float waveHeight(vec2 p) {
    float f = uWaveFreq;
    float t = uTime;
    vec2 q = p - vec2(uFlowSpeed * t, 0.0);   // waves travel with the current (+x)
    return uWaveAmp * (0.50 * sin(q.x * f + t * 1.3)
                     + 0.30 * sin(q.y * f * 1.4 - t * 1.1 + q.x * f * 0.5)
                     + 0.20 * sin((q.x + q.y) * f * 2.3 + t * 2.1));
}

void main() {
    vec3 p = aPos;
    vec3 n = aNormal;
    vNose = 0.0;
    if (uWaterMode == 2 && uNoseLen > 0.0) {
        float d = aUV.y - uFront;
        if (d > 0.0) {
            float k = clamp(d / uNoseLen, 0.0, 1.0);
            vec3 centre = p - n * uTubeRadius;
            p = centre + n * uTubeRadius * sqrt(max(1.0 - k * k, 0.0));
            vNose = k;
        }
    }
    vec4 wp = uModel * vec4(p, 1.0);
    if (uWaveAmp > 0.0 && abs(wp.y - uSurfaceY) < 0.02) wp.y += waveHeight(wp.xz);

    vWorldPos = wp.xyz;
    vNormal = mat3(uNormalMat) * n;
    vUV = aUV;
    gl_Position = uProj * uView * wp;
}

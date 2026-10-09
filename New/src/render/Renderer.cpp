#include "Renderer.h"
#include <glad/gl.h>

bool Renderer::init() {
    bool ok = true;
    ok &= lit.load("lit.vert", "lit.frag");             // Phong (per-pixel) lighting
    ok &= gouraud.load("gouraud.vert", "gouraud.frag"); // Gouraud (per-vertex) lighting
    ok &= water.load("water.vert", "water.frag");
    ok &= depth.load("depth.vert", "depth.frag");       // shadow map pass
    ok &= sky.load("fullscreen.vert", "sky.frag");
    ok &= particle.load("particle.vert", "particle.frag");

    // depth texture the sun "sees" (shadow map)
    glGenFramebuffers(1, &shadowFbo);
    glGenTextures(1, &shadowTex);
    glBindTexture(GL_TEXTURE_2D, shadowTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, shadowSize, shadowSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenVertexArrays(1, &emptyVao);
    return ok;
}

// Upload the uniforms every lit shader shares (camera, sun, sky, fog ...).
void Renderer::applyCommon(const Shader& s) {
    s.use();
    s.set("uView", viewM);
    s.set("uProj", projM);
    s.set("uCamPos", cam.finalEye());
    s.set("uTime", time);
    s.set("uSunDir", normalize(L.sunDir));
    s.set("uSunColor", L.sunColor);
    s.set("uSkyTop", L.skyTop);
    s.set("uSkyHorizon", L.skyHorizon);
    s.set("uGroundColor", L.ground);
    s.set("uAmbient", L.ambient);
    s.set("uFogColor", L.fogColor);
    s.set("uFogDensity", L.fogDensity);
    s.set("uSkyMode", L.skyMode);
    s.set("uUnderwater", L.underwater ? 1 : 0);
    s.set("uWaterLevel", L.waterLevel);
    s.set("uCaustics", L.caustics);
    s.set("uCausticRect", L.causticRect);
    s.set("uExposure", L.exposure);
    s.set("uNumPoint", L.numPoint);
    if (L.numPoint > 0) {
        s.setArray("uPointPos", L.pointPos, L.numPoint);
        s.setArray("uPointColor", L.pointColor, L.numPoint);
    }
    s.set("uShadowsOn", L.shadows ? 1 : 0);
    s.set("uLightSpace", lightSpace);
    s.set("uShadowBias", shadowBias);
    s.set("uShadowNormalOffset", shadowNormalOffset);
    s.set("uShadowMap", 5);
    s.set("uCamRight", camRight);
    s.set("uCamUp", camUp);
    s.set("uCamForward", camForward);
    s.set("uTanHalfFov", std::tan(radians(cam.fovDeg) * 0.5f));
    s.set("uAspect", (float)width / (float)height);
    s.set("uResolution", Vec2((float)width, (float)height));
}

void Renderer::beginFrame(const Camera& c, const Lighting& lighting, float t, int w, int h) {
    cam = c;
    L = lighting;
    time = t;
    width = w;
    height = h;
    viewM = cam.view();
    projM = cam.proj((float)w / (float)h);
    Vec3 eye = cam.finalEye();
    camForward = normalize(cam.target + cam.pan - eye);
    camRight = normalize(cross(camForward, Vec3(0, 1, 0)));
    camUp = cross(camRight, camForward);
}

void Renderer::beginShadowPass(const Vec3& centre, float radius) {
    Vec3 sun = normalize(L.sunDir);
    Vec3 up = std::fabs(sun.y) > 0.95f ? Vec3(1, 0, 0) : Vec3(0, 1, 0);
    Mat4 lv = lookAt(centre + sun * radius * 2.0f, centre, up);
    Mat4 lp = ortho(-radius, radius, -radius, radius, 0.1f, radius * 4.0f);
    lightSpace = lp * lv;
    // bias values scale with the size of the area the shadow map covers
    float texel = 2.0f * radius / shadowSize;
    shadowNormalOffset = texel * 3.0f;
    shadowBias = texel * 2.0f / (radius * 4.0f);

    shadowPass = true;
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFbo);
    glViewport(0, 0, shadowSize, shadowSize);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.5f, 2.0f);
    depth.use();
    depth.set("uLightSpace", lightSpace);
}

void Renderer::endShadowPass() {
    shadowPass = false;
    glDisable(GL_POLYGON_OFFSET_FILL);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
}

void Renderer::prepareMainPass() {
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, shadowTex);
    glActiveTexture(GL_TEXTURE0);
    // every lit shader needs this frame's camera, sun and light matrix
    applyCommon(lit);
    applyCommon(gouraud);
    applyCommon(water);
    applyCommon(particle);
    applyCommon(sky);
}

void Renderer::drawFullscreen() {
    glBindVertexArray(emptyVao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Renderer::drawSky() {
    applyCommon(sky);
    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    drawFullscreen();
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
}

void Renderer::drawMesh(const Mesh& m, const Mat4& model, const Material& mat) {
    if (shadowPass) {
        if (!mat.castShadow || mat.additive || mat.alpha < 0.99f) return;
        depth.set("uModel", model);
        depth.set("uPattern", mat.pattern);
        depth.set("uPatternScale", mat.patternScale);
        m.draw();
        return;
    }
    bool useGouraud = (shadingMode == SHADE_ALL_GOURAUD) || (shadingMode == SHADE_MIXED && mat.gouraud);
    const Shader& s = useGouraud ? gouraud : lit;
    s.use();
    s.set("uModel", model);
    s.set("uNormalMat", normalMatrix(model));
    s.set("uColor", mat.color);
    s.set("uSpecular", mat.specular);
    s.set("uShininess", mat.shininess);
    s.set("uReflect", mat.reflect);
    s.set("uPattern", mat.pattern);
    s.set("uPatternScale", mat.patternScale);
    s.set("uEmissive", mat.emissive);
    s.set("uAlpha", mat.alpha);
    s.set("uTwoSided", mat.twoSided ? 1 : 0);

    bool blended = mat.alpha < 0.999f || mat.additive;
    if (blended) {
        glEnable(GL_BLEND);
        if (mat.additive) glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        else glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
    }
    m.draw();
    if (blended) {
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }
}

void Renderer::drawWater(const Mesh& m, const Mat4& model, const WaterStyle& w) {
    if (shadowPass) return;     // water does not cast shadows
    const Shader& s = water;
    s.use();
    s.set("uModel", model);
    s.set("uNormalMat", normalMatrix(model));
    s.set("uWaterMode", w.mode);
    s.set("uDeep", w.deep);
    s.set("uShallow", w.shallow);
    s.set("uAlpha", w.alpha);
    s.set("uFoam", w.foam);
    s.set("uFlowSpeed", w.flowSpeed);
    s.set("uRipple", w.ripple);
    s.set("uRippleStrength", w.rippleStrength);
    s.set("uWaveAmp", w.waveAmp);
    s.set("uWaveFreq", w.waveFreq);
    s.set("uSurfaceY", w.surfaceY);
    s.set("uFront", w.front);
    s.set("uTail", w.tail);
    s.set("uNoseLen", w.noseLen);
    s.set("uTubeRadius", w.tubeRadius);
    s.set("uSheetWidth", w.sheetWidth);
    s.set("uGlow", w.glow);
    s.set("uFoamSpot", w.foamSpot);
    s.set("uFoamSpot2", w.foamSpot2);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    m.draw();
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

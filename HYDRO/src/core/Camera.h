// =============================================================================
//  Camera.h  -  a look-at camera.
//
//  Two ways to drive it:
//    * CINEMATIC: each scene sets eye/target every frame (a scripted dolly
//      shot, like the camera moves in the video).
//    * FREE (user interaction): mouse drag orbits around the target, the
//      wheel zooms, W/A/S/D/Q/E pan the target.
// =============================================================================
#pragma once
#include "MathLib.h"

struct Camera {
    Vec3 eye{0, 5, 20};
    Vec3 target{0, 0, 0};
    float fovDeg = 50.0f;
    float nearZ = 0.2f, farZ = 3000.0f;

    // user orbit offsets (added on top of the cinematic shot)
    float yaw = 0, pitch = 0, zoom = 1.0f;
    Vec3 pan{0, 0, 0};

    Mat4 view() const { return lookAt(finalEye(), target + pan, Vec3(0, 1, 0)); }
    Mat4 proj(float aspect) const { return perspective(radians(fovDeg), aspect, nearZ, farZ); }

    // Eye after the user's orbit/zoom has been applied around the target.
    Vec3 finalEye() const {
        Vec3 off = eye - target;
        float dist = length(off) * zoom;
        float baseYaw = std::atan2(off.x, off.z);
        float basePitch = std::asin(clampf(off.y / std::fmax(length(off), 1e-4f), -1.0f, 1.0f));
        float yw = baseYaw + yaw;
        float pt = clampf(basePitch + pitch, radians(-80.0f), radians(85.0f));
        Vec3 dir(std::sin(yw) * std::cos(pt), std::sin(pt), std::cos(yw) * std::cos(pt));
        return target + pan + dir * dist;
    }
    void resetUser() { yaw = 0; pitch = 0; zoom = 1; pan = Vec3(0, 0, 0); }
};

// Smoothly blends between camera key positions (for scripted shots).
struct CamKey { float t; Vec3 eye, target; };
void evalCamPath(const CamKey* keys, int count, float t, Vec3& eye, Vec3& target);

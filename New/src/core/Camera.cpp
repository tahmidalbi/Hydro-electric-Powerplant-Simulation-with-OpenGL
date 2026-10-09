#include "Camera.h"

// Catmull-Rom spline through p1..p2 (p0 and p3 are the neighbours).
// It passes through every key and has no "stop" at the keys, so the camera
// glides like a real dolly / crane shot.
static Vec3 catmull(const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& p3, float u) {
    float u2 = u * u, u3 = u2 * u;
    return (p1 * 2.0f + (p2 - p0) * u + (p0 * 2.0f - p1 * 5.0f + p2 * 4.0f - p3) * u2 +
            (p1 * 3.0f - p0 - p2 * 3.0f + p3) * u3) * 0.5f;
}

void evalCamPath(const CamKey* k, int n, float t, Vec3& eye, Vec3& target) {
    if (n == 1 || t <= k[0].t) { eye = k[0].eye; target = k[0].target; return; }
    if (t >= k[n - 1].t) { eye = k[n - 1].eye; target = k[n - 1].target; return; }
    int i = 0;
    while (i + 1 < n && t > k[i + 1].t) i++;
    float u = (t - k[i].t) / (k[i + 1].t - k[i].t);
    // ease in on the very first segment and ease out on the last one
    if (i == 0) u = u * u * (3.0f - 2.0f * u) * 0.5f + u * 0.5f;
    if (i + 2 == n) u = u * u * (3.0f - 2.0f * u) * 0.5f + u * 0.5f;
    int i0 = i > 0 ? i - 1 : i, i3 = i + 2 < n ? i + 2 : i + 1;
    eye = catmull(k[i0].eye, k[i].eye, k[i + 1].eye, k[i3].eye, u);
    target = catmull(k[i0].target, k[i].target, k[i + 1].target, k[i3].target, u);
}

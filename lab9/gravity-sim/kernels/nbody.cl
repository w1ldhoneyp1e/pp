typedef float4 Body;

__kernel void step_nbody(__global Body *positions,
                         __global Body *velocities,
                         const int count,
                         const float dt,
                         const float gravity,
                         const float softening)
{
    const int i = get_global_id(0);
    if (i >= count) {
        return;
    }

    Body pi = positions[i];
    float3 acc = (float3)(0.0f, 0.0f, 0.0f);

    for (int j = 0; j < count; ++j) {
        Body pj = positions[j];
        float3 r = (float3)(pj.x - pi.x, pj.y - pi.y, pj.z - pi.z);
        float dist2 = dot(r, r) + softening * softening;
        float inv_dist = native_rsqrt(dist2);
        float inv_dist3 = inv_dist * inv_dist * inv_dist;

        acc += gravity * pj.w * inv_dist3 * r;
    }

    Body vi = velocities[i];
    vi.x += acc.x * dt;
    vi.y += acc.y * dt;
    vi.z += acc.z * dt;

    pi.x += vi.x * dt;
    pi.y += vi.y * dt;
    pi.z += vi.z * dt;

    positions[i] = pi;
    velocities[i] = vi;
}

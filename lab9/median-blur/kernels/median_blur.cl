#define MAX_RADIUS 7
#define MAX_WINDOW ((MAX_RADIUS * 2 + 1) * (MAX_RADIUS * 2 + 1))

inline int clamp_int(int value, int low, int high)
{
    return max(low, min(value, high));
}

inline uchar luminance(uchar4 color)
{
    return (uchar)((54 * (int)color.x + 183 * (int)color.y + 19 * (int)color.z) >> 8);
}

__kernel void median_blur(__global const uchar4 *input,
                          __global uchar4 *output,
                          int width,
                          int height,
                          int radius,
                          __local uchar4 *tile,
                          __local int2 *windowOffsets)
{
    const int localX = get_local_id(0);
    const int localY = get_local_id(1);
    const int localWidth = get_local_size(0);
    const int localHeight = get_local_size(1);
    const int groupX = get_group_id(0) * localWidth;
    const int groupY = get_group_id(1) * localHeight;
    const int globalX = get_global_id(0);
    const int globalY = get_global_id(1);
    const int threadIndex = localY * localWidth + localX;
    const int threadCount = localWidth * localHeight;
    const int tileWidth = localWidth + radius * 2;
    const int tileHeight = localHeight + radius * 2;
    const int tileArea = tileWidth * tileHeight;
    const int diameter = radius * 2 + 1;
    const int windowArea = diameter * diameter;

    for (int index = threadIndex; index < tileArea; index += threadCount) {
        const int tileX = index % tileWidth;
        const int tileY = index / tileWidth;
        const int sourceX = clamp_int(groupX + tileX - radius, 0, width - 1);
        const int sourceY = clamp_int(groupY + tileY - radius, 0, height - 1);
        tile[index] = input[sourceY * width + sourceX];
    }

    for (int index = threadIndex; index < windowArea; index += threadCount) {
        windowOffsets[index] = (int2)(index % diameter - radius, index / diameter - radius);
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    if (globalX >= width || globalY >= height) {
        return;
    }

    uchar values[MAX_WINDOW];
    uchar4 colors[MAX_WINDOW];
    int count = 0;

    for (int index = 0; index < windowArea; ++index) {
        const int2 offset = windowOffsets[index];
        const int tileIndex = (localY + radius + offset.y) * tileWidth
                            + (localX + radius + offset.x);
        const uchar4 color = tile[tileIndex];
        const uchar value = luminance(color);

        int insertAt = count;
        while (insertAt > 0 && values[insertAt - 1] > value) {
            values[insertAt] = values[insertAt - 1];
            colors[insertAt] = colors[insertAt - 1];
            --insertAt;
        }
        values[insertAt] = value;
        colors[insertAt] = color;
        ++count;
    }

    output[globalY * width + globalX] = colors[count / 2];
}

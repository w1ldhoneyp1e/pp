#define RADIX 256
#define BLOCK_SIZE 256

uint GetDigit(int value, uint shift)
{
    uint key = ((uint)value) ^ 0x80000000u;
    return (key >> shift) & 0xffu;
}

__kernel void BuildHistogram(__global const int *input,
                             __global uint *histograms,
                             uint count,
                             uint shift)
{
    const uint block = get_group_id(0);
    const uint localId = get_local_id(0);
    const uint index = block * BLOCK_SIZE + localId;

    __local uint localHistogram[RADIX];

    localHistogram[localId] = 0u;
    barrier(CLK_LOCAL_MEM_FENCE);

    if (index < count)
    {
        const uint digit = GetDigit(input[index], shift);
        atomic_inc(&localHistogram[digit]);
    }

    barrier(CLK_LOCAL_MEM_FENCE);
    histograms[block * RADIX + localId] = localHistogram[localId];
}

__kernel void ScatterByDigit(__global const int *input,
                             __global int *output,
                             __global const uint *offsets,
                             uint count,
                             uint shift)
{
    const uint block = get_group_id(0);
    const uint localId = get_local_id(0);
    const uint index = block * BLOCK_SIZE + localId;

    __local int localValues[BLOCK_SIZE];
    __local uint localDigits[BLOCK_SIZE];

    if (index < count)
    {
        localValues[localId] = input[index];
        localDigits[localId] = GetDigit(input[index], shift);
    }
    else
    {
        localValues[localId] = 0;
        localDigits[localId] = 0xffffffffu;
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    if (index < count)
    {
        const uint digit = localDigits[localId];
        uint rank = 0u;

        for (uint i = 0u; i < localId; ++i)
        {
            if (localDigits[i] == digit)
            {
                ++rank;
            }
        }

        output[offsets[block * RADIX + digit] + rank] = localValues[localId];
    }
}

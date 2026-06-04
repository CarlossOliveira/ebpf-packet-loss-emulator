#ifndef MATH_UTILS_BPF_H
#define MATH_UTILS_BPF_H

#include <bpf/bpf_helpers.h>

#define PI ((__u64)3141592653589793238ULL)
#define FIXED_POINT_SCALE ((__u64)1000000000000000000ULL)

static __always_inline __u64 sqrt_u64(__u64 x)
{
    __u64 op = x;
    __u64 res = 0;
    __u64 bit = 1ULL << 62;

    while (bit > op)
        bit >>= 2;

    while (bit != 0)
    {
        if (op >= res + bit)
        {
            op -= res + bit;
            res += bit << 1;
        }
        res >>= 1;
        bit >>= 2;
    }
    return res;
};

static __always_inline __u64 pow_u64(__u64 x, __u64 n)
{
    __u64 result = FIXED_POINT_SCALE;

    if (n <= 0)
        return result;

    for (__u64 i = 0; i < n; i++)
        result = (result * x) / FIXED_POINT_SCALE;

    return result;
}
#endif // !MATH_UTILS_BPF_H
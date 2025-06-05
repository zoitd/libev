#ifndef ECB_H
#define ECB_H

/* 16 bits major, 16 bits minor */
#define ECB_VERSION 0x00010005

#ifdef _WIN32
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
#if __GNUC__
typedef signed long long int64_t;
typedef unsigned long long uint64_t;
#else /* _MSC_VER || __BORLANDC__ */
typedef signed __int64 int64_t;
typedef unsigned __int64 uint64_t;
#endif
#ifdef _WIN64
#define ECB_PTRSIZE 8
typedef uint64_t uintptr_t;
typedef int64_t intptr_t;
#else
#define ECB_PTRSIZE 4
typedef uint32_t uintptr_t;
typedef int32_t intptr_t;
#endif
#else
#include <inttypes.h>
#if (defined INTPTR_MAX ? INTPTR_MAX : ULONG_MAX) > 0xffffffffU
#define ECB_PTRSIZE 8
#else
#define ECB_PTRSIZE 4
#endif
#endif

#define ECB_GCC_AMD64 (__amd64 || __amd64__ || __x86_64 || __x86_64__)
#define ECB_MSVC_AMD64 (_M_AMD64 || _M_X64)

/* work around x32 idiocy by defining proper macros */
#if ECB_GCC_AMD64 || ECB_MSVC_AMD64
#if _ILP32
#define ECB_AMD64_X32 1
#else
#define ECB_AMD64 1
#endif
#endif

/* many compilers define _GNUC_ to some versions but then only implement
 * what their idiot authors think are the "more important" extensions,
 * causing enormous grief in return for some better fake benchmark numbers.
 * or so.
 * we try to detect these and simply assume they are not gcc - if they have
 * an issue with that they should have done it right in the first place.
 */
// 列出不兼容GCC的编译器
#if !defined __GNUC_MINOR__ || defined __INTEL_COMPILER || defined __SUNPRO_C || defined __SUNPRO_CC || defined __llvm__ || defined __clang__
#define ECB_GCC_VERSION(major, minor) 0
#else
#define ECB_GCC_VERSION(major, minor) (__GNUC__ > (major) || (__GNUC__ == (major) && __GNUC_MINOR__ >= (minor)))
#endif

#define ECB_CLANG_VERSION(major, minor) (__clang_major__ > (major) || (__clang_major__ == (major) && __clang_minor__ >= (minor)))

#if __clang__ && defined __has_builtin
#define ECB_CLANG_BUILTIN(x) __has_builtin(x)
#else
#define ECB_CLANG_BUILTIN(x) 0
#endif

#if __clang__ && defined __has_extension
#define ECB_CLANG_EXTENSION(x) __has_extension(x)
#else
#define ECB_CLANG_EXTENSION(x) 0
#endif

#define ECB_CPP (__cplusplus + 0)
#define ECB_CPP11 (__cplusplus >= 201103L)

#if ECB_CPP
#define ECB_C 0
#define ECB_STDC_VERSION 0
#else
#define ECB_C 1
#define ECB_STDC_VERSION __STDC_VERSION__
#endif

#define ECB_C99 (ECB_STDC_VERSION >= 199901L)
#define ECB_C11 (ECB_STDC_VERSION >= 201112L)

#if ECB_CPP
#define ECB_EXTERN_C extern "C"
#define ECB_EXTERN_C_BEG \
    ECB_EXTERN_C         \
    {
#define ECB_EXTERN_C_END }
#else
#define ECB_EXTERN_C extern
#define ECB_EXTERN_C_BEG
#define ECB_EXTERN_C_END
#endif

/*****************************************************************************/

/* ECB_NO_THREADS - ecb is not used by multiple threads, ever */
/* ECB_NO_SMP     - ecb might be used in multiple threads, but only on a single cpu */

#if ECB_NO_THREADS
#define ECB_NO_SMP 1
#endif

#if ECB_NO_SMP
#define ECB_MEMORY_FENCE \
    do                   \
    {                    \
    } while (0)
#endif

/* http://www-01.ibm.com/support/knowledgecenter/SSGH3R_13.1.0/com.ibm.xlcpp131.aix.doc/compiler_ref/compiler_builtins.html */
#if __xlC__ && ECB_CPP
#include <builtins.h>
#endif

#if 1400 <= _MSC_VER
#include <intrin.h> /* fence functions _ReadBarrier, also bit search functions _BitScanReverse */
#endif

#ifndef ECB_MEMORY_FENCE
#if ECB_GCC_VERSION(2, 5) || defined __INTEL_COMPILER || (__llvm__ && __GNUC__) || __SUNPRO_C >= 0x5110 || __SUNPRO_CC >= 0x5110
#if __i386 || __i386__
#define ECB_MEMORY_FENCE __asm__ __volatile__("lock; orb $0, -1(%%esp)" : : : "memory")
#define ECB_MEMORY_FENCE_ACQUIRE __asm__ __volatile__("" : : : "memory")
#define ECB_MEMORY_FENCE_RELEASE __asm__ __volatile__("")
#elif ECB_GCC_AMD64
#define ECB_MEMORY_FENCE __asm__ __volatile__("mfence" : : : "memory")
#define ECB_MEMORY_FENCE_ACQUIRE __asm__ __volatile__("" : : : "memory")
#define ECB_MEMORY_FENCE_RELEASE __asm__ __volatile__("")
#elif __powerpc__ || __ppc__ || __powerpc64__ || __ppc64__
#define ECB_MEMORY_FENCE __asm__ __volatile__("sync" : : : "memory")
#elif defined __ARM_ARCH_2__ || defined __ARM_ARCH_3__ || defined __ARM_ARCH_3M__ || defined __ARM_ARCH_4__ || defined __ARM_ARCH_4T__ || defined __ARM_ARCH_5__ || defined __ARM_ARCH_5E__ || defined __ARM_ARCH_5T__ || defined __ARM_ARCH_5TE__ || defined __ARM_ARCH_5TEJ__
/* should not need any, unless running old code on newer cpu - arm doesn't support that */
#elif defined __ARM_ARCH_6__ || defined __ARM_ARCH_6J__ || defined __ARM_ARCH_6K__ || defined __ARM_ARCH_6ZK__ || defined __ARM_ARCH_6T2__
#define ECB_MEMORY_FENCE __asm__ __volatile__("mcr p15,0,%0,c7,c10,5" : : "r"(0) : "memory")
#elif defined __ARM_ARCH_7__ || defined __ARM_ARCH_7A__ || defined __ARM_ARCH_7R__ || defined __ARM_ARCH_7M__
#define ECB_MEMORY_FENCE __asm__ __volatile__("dmb" : : : "memory")
#elif __aarch64__
#define ECB_MEMORY_FENCE __asm__ __volatile__("dmb ish" : : : "memory")
#elif (__sparc || __sparc__) && !(__sparc_v8__ || defined __sparcv8)
#define ECB_MEMORY_FENCE __asm__ __volatile__("membar #LoadStore | #LoadLoad | #StoreStore | #StoreLoad" : : : "memory")
#define ECB_MEMORY_FENCE_ACQUIRE __asm__ __volatile__("membar #LoadStore | #LoadLoad" : : : "memory")
#define ECB_MEMORY_FENCE_RELEASE __asm__ __volatile__("membar #LoadStore             | #StoreStore")
#elif defined __s390__ || defined __s390x__
#define ECB_MEMORY_FENCE __asm__ __volatile__("bcr 15,0" : : : "memory")
#elif defined __mips__
/* GNU/Linux emulates sync on mips1 architectures, so we force its use */
/* anybody else who still uses mips1 is supposed to send in their version, with detection code. */
#define ECB_MEMORY_FENCE __asm__ __volatile__(".set mips2; sync; .set mips0" : : : "memory")
#elif defined __alpha__
#define ECB_MEMORY_FENCE __asm__ __volatile__("mb" : : : "memory")
#elif defined __hppa__
#define ECB_MEMORY_FENCE __asm__ __volatile__("" : : : "memory")
#define ECB_MEMORY_FENCE_RELEASE __asm__ __volatile__("")
#elif defined __ia64__
#define ECB_MEMORY_FENCE __asm__ __volatile__("mf" : : : "memory")
#elif defined __m68k__
#define ECB_MEMORY_FENCE __asm__ __volatile__("" : : : "memory")
#elif defined __m88k__
#define ECB_MEMORY_FENCE __asm__ __volatile__("tb1 0,%%r0,128" : : : "memory")
#elif defined __sh__
#define ECB_MEMORY_FENCE __asm__ __volatile__("" : : : "memory")
#endif
#endif
#endif

#ifndef ECB_MEMORY_FENCE
#if ECB_GCC_VERSION(4, 7)
/* see comment below (stdatomic.h) about the C11 memory model. */
#define ECB_MEMORY_FENCE __atomic_thread_fence(__ATOMIC_SEQ_CST)
#define ECB_MEMORY_FENCE_ACQUIRE __atomic_thread_fence(__ATOMIC_ACQUIRE)
#define ECB_MEMORY_FENCE_RELEASE __atomic_thread_fence(__ATOMIC_RELEASE)

#elif ECB_CLANG_EXTENSION(c_atomic)
/* see comment below (stdatomic.h) about the C11 memory model. */
#define ECB_MEMORY_FENCE __c11_atomic_thread_fence(__ATOMIC_SEQ_CST)
#define ECB_MEMORY_FENCE_ACQUIRE __c11_atomic_thread_fence(__ATOMIC_ACQUIRE)
#define ECB_MEMORY_FENCE_RELEASE __c11_atomic_thread_fence(__ATOMIC_RELEASE)

#elif ECB_GCC_VERSION(4, 4) || defined __INTEL_COMPILER || defined __clang__
#define ECB_MEMORY_FENCE __sync_synchronize()
#elif _MSC_VER >= 1500 /* VC++ 2008 */
/* apparently, microsoft broke all the memory barrier stuff in Visual Studio 2008... */
#pragma intrinsic(_ReadBarrier, _WriteBarrier, _ReadWriteBarrier)
#define ECB_MEMORY_FENCE \
    _ReadWriteBarrier(); \
    MemoryBarrier()
#define ECB_MEMORY_FENCE_ACQUIRE \
    _ReadWriteBarrier();         \
    MemoryBarrier() /* according to msdn, _ReadBarrier is not a load fence */
#define ECB_MEMORY_FENCE_RELEASE \
    _WriteBarrier();             \
    MemoryBarrier()
#elif _MSC_VER >= 1400 /* VC++ 2005 */
#pragma intrinsic(_ReadBarrier, _WriteBarrier, _ReadWriteBarrier)
#define ECB_MEMORY_FENCE _ReadWriteBarrier()
#define ECB_MEMORY_FENCE_ACQUIRE _ReadWriteBarrier() /* according to msdn, _ReadBarrier is not a load fence */
#define ECB_MEMORY_FENCE_RELEASE _WriteBarrier()
#elif defined _WIN32
#include <WinNT.h>
#define ECB_MEMORY_FENCE MemoryBarrier() /* actually just xchg on x86... scary */
#elif __SUNPRO_C >= 0x5110 || __SUNPRO_CC >= 0x5110
#include <mbarrier.h>
#define ECB_MEMORY_FENCE __machine_rw_barrier()
#define ECB_MEMORY_FENCE_ACQUIRE __machine_r_barrier()
#define ECB_MEMORY_FENCE_RELEASE __machine_w_barrier()
#elif __xlC__
#define ECB_MEMORY_FENCE __sync()
#endif
#endif

#ifndef ECB_MEMORY_FENCE
#if ECB_C11 && !defined __STDC_NO_ATOMICS__
/* we assume that these memory fences work on all variables/all memory accesses, */
/* not just C11 atomics and atomic accesses */
#include <stdatomic.h>
/* Unfortunately, neither gcc 4.7 nor clang 3.1 generate any instructions for */
/* any fence other than seq_cst, which isn't very efficient for us. */
/* Why that is, we don't know - either the C11 memory model is quite useless */
/* for most usages, or gcc and clang have a bug */
/* I *currently* lean towards the latter, and inefficiently implement */
/* all three of ecb's fences as a seq_cst fence */
/* Update, gcc-4.8 generates mfence for all c++ fences, but nothing */
/* for all __atomic_thread_fence's except seq_cst */
#define ECB_MEMORY_FENCE atomic_thread_fence(memory_order_seq_cst)
#endif
#endif

#ifndef ECB_MEMORY_FENCE
#if !ECB_AVOID_PTHREADS
/*
 * if you get undefined symbol references to pthread_mutex_lock,
 * or failure to find pthread.h, then you should implement
 * the ECB_MEMORY_FENCE operations for your cpu/compiler
 * OR provide pthread.h and link against the posix thread library
 * of your system.
 */
#include <pthread.h>
#define ECB_NEEDS_PTHREADS 1
#define ECB_MEMORY_FENCE_NEEDS_PTHREADS 1

static pthread_mutex_t ecb_mf_lock = PTHREAD_MUTEX_INITIALIZER;
#define ECB_MEMORY_FENCE                    \
    do                                      \
    {                                       \
        pthread_mutex_lock(&ecb_mf_lock);   \
        pthread_mutex_unlock(&ecb_mf_lock); \
    } while (0)
#endif
#endif

#if !defined ECB_MEMORY_FENCE_ACQUIRE && defined ECB_MEMORY_FENCE
#define ECB_MEMORY_FENCE_ACQUIRE ECB_MEMORY_FENCE
#endif

#if !defined ECB_MEMORY_FENCE_RELEASE && defined ECB_MEMORY_FENCE
#define ECB_MEMORY_FENCE_RELEASE ECB_MEMORY_FENCE
#endif

/*****************************************************************************/

#if ECB_CPP
#define ecb_inline static inline
#elif ECB_GCC_VERSION(2, 5)
#define ecb_inline static __inline__
#elif ECB_C99
#define ecb_inline static inline
#else
#define ecb_inline static
#endif

// 承诺指针无别名，只有当前智能能访问指定数据，类似于unique_ptr.
#if ECB_GCC_VERSION(3, 3)
#define ecb_restrict __restrict__
#elif ECB_C99
#define ecb_restrict restrict
#else
#define ecb_restrict
#endif

typedef int ecb_bool;

#define ECB_CONCAT_(a, b) a##b
#define ECB_CONCAT(a, b) ECB_CONCAT_(a, b)
#define ECB_STRINGIFY_(a) #a
#define ECB_STRINGIFY(a) ECB_STRINGIFY_(a)
#define ECB_STRINGIFY_EXPR(expr) ((expr), ECB_STRINGIFY_(expr))

#define ecb_function_ ecb_inline

#if ECB_GCC_VERSION(3, 1) || ECB_CLANG_VERSION(2, 8)
#define ecb_attribute(attrlist) __attribute__(attrlist)
#else
#define ecb_attribute(attrlist)
#endif

#if ECB_GCC_VERSION(3, 1) || ECB_CLANG_BUILTIN(__builtin_constant_p)
#define ecb_is_constant(expr) __builtin_constant_p(expr)
#else

/* possible C11 impl for integral types
typedef struct ecb_is_constant_struct ecb_is_constant_struct;
#define ecb_is_constant(expr)          _Generic ((1 ? (struct ecb_is_constant_struct *)0 : (void *)((expr) - (expr)), ecb_is_constant_struct *: 0, default: 1)) */

#define ecb_is_constant(expr) 0

#endif

#if ECB_GCC_VERSION(3, 1) || ECB_CLANG_BUILTIN(__builtin_expect)
#define ecb_expect(expr, value) __builtin_expect((expr), (value))
#else
#define ecb_expect(expr, value) (expr)
#endif

#if ECB_GCC_VERSION(3, 1) || ECB_CLANG_BUILTIN(__builtin_prefetch)
#define ecb_prefetch(addr, rw, locality) __builtin_prefetch(addr, rw, locality)
#else
#define ecb_prefetch(addr, rw, locality)
#endif

/* no emulation for ecb_decltype */
#if ECB_CPP11
// older implementations might have problems with decltype(x)::type, work around it
template <class T>
struct ecb_decltype_t
{
    typedef T type;
};
#define ecb_decltype(x) ecb_decltype_t<decltype(x)>::type
#elif ECB_GCC_VERSION(3, 0) || ECB_CLANG_VERSION(2, 8)
#define ecb_decltype(x) __typeof__(x)
#endif

#if _MSC_VER >= 1300
#define ecb_deprecated __declspec(deprecated)
#else
#define ecb_deprecated ecb_attribute((__deprecated__))
#endif

#if _MSC_VER >= 1500
#define ecb_deprecated_message(msg) __declspec(deprecated(msg))
#elif ECB_GCC_VERSION(4, 5)
#define ecb_deprecated_message(msg) ecb_attribute ((__deprecated__ (msg))
#else
#define ecb_deprecated_message(msg) ecb_deprecated
#endif

#if _MSC_VER >= 1400
#define ecb_noinline __declspec(noinline)
#else
#define ecb_noinline ecb_attribute((__noinline__))
#endif

#define ecb_unused ecb_attribute((__unused__))
#define ecb_const ecb_attribute((__const__))
#define ecb_pure ecb_attribute((__pure__))

#if ECB_C11 || __IBMC_NORETURN
/* http://www-01.ibm.com/support/knowledgecenter/SSGH3R_13.1.0/com.ibm.xlcpp131.aix.doc/language_ref/noreturn.html */
#define ecb_noreturn _Noreturn
#elif ECB_CPP11
#define ecb_noreturn [[noreturn]]
#elif _MSC_VER >= 1200
/* http://msdn.microsoft.com/en-us/library/k6ktzx3s.aspx */
#define ecb_noreturn __declspec(noreturn)
#else
#define ecb_noreturn ecb_attribute((__noreturn__))
#endif

#if ECB_GCC_VERSION(4, 3)
#define ecb_artificial ecb_attribute((__artificial__))
#define ecb_hot ecb_attribute((__hot__))
#define ecb_cold ecb_attribute((__cold__))
#else
#define ecb_artificial
#define ecb_hot
#define ecb_cold
#endif

/* 如果你非常确定条件表达式的结果通常为真或通常为假，请使用这些宏包裹条件表达式 /
/* 注意：这些宏返回布尔值，而非原始表达式的结果 */
#define ecb_expect_false(expr) ecb_expect(!!(expr), 0)
#define ecb_expect_true(expr) ecb_expect(!!(expr), 1)
/* for compatibility to the rest of the world */
#define ecb_likely(expr) ecb_expect_true(expr)
#define ecb_unlikely(expr) ecb_expect_false(expr)

/* count trailing zero bits and count #of one bits */
#if ECB_GCC_VERSION(3, 4) || (ECB_CLANG_BUILTIN(__builtin_clz) && ECB_CLANG_BUILTIN(__builtin_clzll) && ECB_CLANG_BUILTIN(__builtin_ctz) && ECB_CLANG_BUILTIN(__builtin_ctzll) && ECB_CLANG_BUILTIN(__builtin_popcount))
/* we assume int == 32 bit, long == 32 or 64 bit and long long == 64 bit */

// 最高有效位的位数
#define ecb_ld32(x) (__builtin_clz(x) ^ 31)

// 最高有效位的位数
#define ecb_ld64(x) (__builtin_clzll(x) ^ 63)
// 最低有效位的位数
#define ecb_ctz32(x) __builtin_ctz(x)
// 最低有效位的位数
#define ecb_ctz64(x) __builtin_ctzll(x)
#define ecb_popcount32(x) __builtin_popcount(x) // 汉明重量，（1的个数）
/* no popcountll */
#else
constexpr static inline int ecb_ctz32(uint32_t x);
constexpr static inline int ecb_ctz32(uint32_t x)
{
#if 1400 <= _MSC_VER && (_M_IX86 || _M_X64 || _M_IA64 || _M_ARM)
    unsigned long r;
    _BitScanForward(&r, x);
    return (int)r;
#else
    int r = 0;

    x &= ~x + 1; /* this isolates the lowest bit */

#if ECB_branchless_on_i386
    r += !!(x & 0xaaaaaaaa) << 0;
    r += !!(x & 0xcccccccc) << 1;
    r += !!(x & 0xf0f0f0f0) << 2;
    r += !!(x & 0xff00ff00) << 3;
    r += !!(x & 0xffff0000) << 4;
#else
    if (x & 0xaaaaaaaa)
        r += 1;
    if (x & 0xcccccccc)
        r += 2;
    if (x & 0xf0f0f0f0)
        r += 4;
    if (x & 0xff00ff00)
        r += 8;
    if (x & 0xffff0000)
        r += 16;
#endif

    return r;
#endif
}

constexpr static inline int ecb_ctz64(uint64_t x);
constexpr static inline int ecb_ctz64(uint64_t x)
{
#if 1400 <= _MSC_VER && (_M_X64 || _M_IA64 || _M_ARM)
    unsigned long r;
    _BitScanForward64(&r, x);
    return (int)r;
#else
    int shift = x & 0xffffffff ? 0 : 32;
    return ecb_ctz32(x >> shift) + shift;
#endif
}

constexpr static inline int ecb_popcount32(uint32_t x);
constexpr static inline int ecb_popcount32(uint32_t x)
{
    x -= (x >> 1) & 0x55555555;
    x = ((x >> 2) & 0x33333333) + (x & 0x33333333);
    x = ((x >> 4) + x) & 0x0f0f0f0f;
    x *= 0x01010101;

    return x >> 24;
}

constexpr static inline int ecb_ld32(uint32_t x);
constexpr static inline int ecb_ld32(uint32_t x)
{
#if 1400 <= _MSC_VER && (_M_IX86 || _M_X64 || _M_IA64 || _M_ARM)
    unsigned long r;
    _BitScanReverse(&r, x);
    return (int)r;
#else
    int r = 0;

    if (x >> 16)
    {
        x >>= 16;
        r += 16;
    }
    if (x >> 8)
    {
        x >>= 8;
        r += 8;
    }
    if (x >> 4)
    {
        x >>= 4;
        r += 4;
    }
    if (x >> 2)
    {
        x >>= 2;
        r += 2;
    }
    if (x >> 1)
    {
        r += 1;
    }

    return r;
#endif
}

constexpr static inline int ecb_ld64(uint64_t x);
constexpr static inline int ecb_ld64(uint64_t x)
{
#if 1400 <= _MSC_VER && (_M_X64 || _M_IA64 || _M_ARM)
    unsigned long r;
    _BitScanReverse64(&r, x);
    return (int)r;
#else
    int r = 0;

    if (x >> 32)
    {
        x >>= 32;
        r += 32;
    }

    return r + ecb_ld32(x);
#endif
}
#endif

// 是否为2的幂 power of two。 __attribute__((__const__)): 纯函数，无副作用，结果进依赖于输入
constexpr static inline ecb_bool ecb_is_pot32(uint32_t x);
constexpr static inline ecb_bool ecb_is_pot32(uint32_t x) { return !(x & (x - 1)); }
constexpr static inline ecb_bool ecb_is_pot64(uint64_t x);
constexpr static inline ecb_bool ecb_is_pot64(uint64_t x) { return !(x & (x - 1)); }

constexpr static inline uint8_t ecb_bitrev8(uint8_t x);
constexpr static inline uint8_t ecb_bitrev8(uint8_t x)
{
    return ((x * 0x0802U & 0x22110U) | (x * 0x8020U & 0x88440U)) * 0x10101U >> 16;
}

constexpr static inline uint16_t ecb_bitrev16(uint16_t x);
constexpr static inline uint16_t ecb_bitrev16(uint16_t x)
{
    x = ((x >> 1) & 0x5555) | ((x & 0x5555) << 1);
    x = ((x >> 2) & 0x3333) | ((x & 0x3333) << 2);
    x = ((x >> 4) & 0x0f0f) | ((x & 0x0f0f) << 4);
    x = (x >> 8) | (x << 8);

    return x;
}

constexpr static inline uint32_t ecb_bitrev32(uint32_t x);
constexpr static inline uint32_t ecb_bitrev32(uint32_t x)
{
    x = ((x >> 1) & 0x55555555) | ((x & 0x55555555) << 1);
    x = ((x >> 2) & 0x33333333) | ((x & 0x33333333) << 2);
    x = ((x >> 4) & 0x0f0f0f0f) | ((x & 0x0f0f0f0f) << 4);
    x = ((x >> 8) & 0x00ff00ff) | ((x & 0x00ff00ff) << 8);
    x = (x >> 16) | (x << 16);

    return x;
}

/* popcount64 is only available on 64 bit cpus as gcc builtin */
/* so for this version we are lazy */
constexpr static inline int ecb_popcount64(uint64_t x);
constexpr static inline int ecb_popcount64(uint64_t x)
{
    return ecb_popcount32(x) + ecb_popcount32(x >> 32);
}

constexpr static inline uint8_t ecb_rotl8(uint8_t x, unsigned int count);
constexpr static inline uint8_t ecb_rotr8(uint8_t x, unsigned int count);
constexpr static inline uint16_t ecb_rotl16(uint16_t x, unsigned int count);
constexpr static inline uint16_t ecb_rotr16(uint16_t x, unsigned int count);
constexpr static inline uint32_t ecb_rotl32(uint32_t x, unsigned int count);
constexpr static inline uint32_t ecb_rotr32(uint32_t x, unsigned int count);
constexpr static inline uint64_t ecb_rotl64(uint64_t x, unsigned int count);
constexpr static inline uint64_t ecb_rotr64(uint64_t x, unsigned int count);

constexpr static inline uint8_t ecb_rotl8(uint8_t x, unsigned int count) { return (x >> (8 - count)) | (x << count); }
constexpr static inline uint8_t ecb_rotr8(uint8_t x, unsigned int count) { return (x << (8 - count)) | (x >> count); }
constexpr static inline uint16_t ecb_rotl16(uint16_t x, unsigned int count) { return (x >> (16 - count)) | (x << count); }
constexpr static inline uint16_t ecb_rotr16(uint16_t x, unsigned int count) { return (x << (16 - count)) | (x >> count); }
constexpr static inline uint32_t ecb_rotl32(uint32_t x, unsigned int count) { return (x >> (32 - count)) | (x << count); }
constexpr static inline uint32_t ecb_rotr32(uint32_t x, unsigned int count) { return (x << (32 - count)) | (x >> count); }
constexpr static inline uint64_t ecb_rotl64(uint64_t x, unsigned int count) { return (x >> (64 - count)) | (x << count); }
constexpr static inline uint64_t ecb_rotr64(uint64_t x, unsigned int count) { return (x << (64 - count)) | (x >> count); }

#if ECB_GCC_VERSION(4, 3) || (ECB_CLANG_BUILTIN(__builtin_bswap32) && ECB_CLANG_BUILTIN(__builtin_bswap64))
#if ECB_GCC_VERSION(4, 8) || ECB_CLANG_BUILTIN(__builtin_bswap16)
// 交换高8位和低8位
#define ecb_bswap16(x) __builtin_bswap16(x)
#else
#define ecb_bswap16(x) (__builtin_bswap32(x) >> 16)
#endif
// 交换高16位和低16位
#define ecb_bswap32(x) __builtin_bswap32(x)
// 交换高32位和低32位
#define ecb_bswap64(x) __builtin_bswap64(x)

#elif _MSC_VER
#include <stdlib.h>
#define ecb_bswap16(x) ((uint16_t)_byteswap_ushort((uint16_t)(x)))
#define ecb_bswap32(x) ((uint32_t)_byteswap_ulong((uint32_t)(x)))
#define ecb_bswap64(x) ((uint64_t)_byteswap_uint64((uint64_t)(x)))
#else
constexpr static inline uint16_t ecb_bswap16(uint16_t x);
constexpr static inline uint16_t ecb_bswap16(uint16_t x)
{
    return ecb_rotl16(x, 8);
}

constexpr static inline uint32_t ecb_bswap32(uint32_t x);
constexpr static inline uint32_t ecb_bswap32(uint32_t x)
{
    return (((uint32_t)ecb_bswap16(x)) << 16) | ecb_bswap16(x >> 16);
}

constexpr static inline uint64_t ecb_bswap64(uint64_t x);
constexpr static inline uint64_t ecb_bswap64(uint64_t x)
{
    return (((uint64_t)ecb_bswap32(x)) << 32) | ecb_bswap32(x >> 32);
}
#endif

#if ECB_GCC_VERSION(4, 5) || ECB_CLANG_BUILTIN(__builtin_unreachable)
// 显示标记代码不可达，帮助编译器优化
#define ecb_unreachable() __builtin_unreachable()
#else
/* this seems to work fine, but gcc always emits a warning for it :/ */
static inline ecb_noreturn void ecb_unreachable(void);
static inline ecb_noreturn void ecb_unreachable(void) {}
#endif

/* try to tell the compiler that some condition is definitely true */
#define ecb_assume(cond)   \
    if (!(cond))           \
        ecb_unreachable(); \
    else                   \
        0

constexpr static inline uint32_t ecb_byteorder_helper(void);
constexpr static inline uint32_t ecb_byteorder_helper(void)
{
    /* 联合体代码在gcc压力测试下仍会生成指令，/
    / 但比使用指针方式更精简，且似乎总能成功返回常量值。 /
    / 之所以采用这种复杂的预处理器逻辑，/
    / 是为了在所有情况下避免该操作——至少在常见架构上 /
    / 或使用较新gcc版本时(>=4.6) */
#if (defined __BYTE_ORDER__ && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) || ((__i386 || __i386__ || _M_IX86 || ECB_GCC_AMD64 || ECB_MSVC_AMD64) && !__VOS__)
#define ECB_LITTLE_ENDIAN 1
    return 0x44332211;
#elif (defined __BYTE_ORDER__ && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) || ((__AARCH64EB__ || __MIPSEB__ || __ARMEB__) && !__VOS__)
#define ECB_BIG_ENDIAN 1
    return 0x11223344;
#else
    union
    {
        uint8_t c[4];
        uint32_t u;
    } u = {0x11, 0x22, 0x33, 0x44};
    return u.u;
#endif
}

constexpr static inline ecb_bool ecb_big_endian(void);
constexpr static inline ecb_bool ecb_big_endian(void) { return ecb_byteorder_helper() == 0x11223344; }
constexpr static inline ecb_bool ecb_little_endian(void);
constexpr static inline ecb_bool ecb_little_endian(void) { return ecb_byteorder_helper() == 0x44332211; }

#if ECB_GCC_VERSION(3, 0) || ECB_C99
#define ecb_mod(m, n) ((m) % (n) + ((m) % (n) < 0 ? (n) : 0))
#else
// 取模运算
#define ecb_mod(m, n) ((m) < 0 ? ((n) - 1 - ((-1 - (m)) % (n))) : ((m) % (n)))
#endif

#endif

/*

!==============================================================
!
! SAMPLE SOURCE CODE - SUBJECT TO THE TERMS OF SAMPLE CODE LICENSE AGREEMENT,
! http://software.intel.com/en-us/articles/intel-sample-source-code-license-agreement/
!
! Copyright 2016 Intel Corporation
!
! THIS FILE IS PROVIDED "AS IS" WITH NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT
! NOT LIMITED TO ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
! PURPOSE, NON-INFRINGEMENT OF INTELLECTUAL PROPERTY RIGHTS.
!
! =============================================================
*/

#ifndef __TBB_machine_gcc_ia32_common_H
#define __TBB_machine_gcc_ia32_common_H

//TODO: Add a higher-level function, e.g. tbb::interal::log2(), into tbb_stddef.h, which
//uses __TBB_Log2 and contains the assert and remove the assert from here and all other
//platform-specific headers.
//TODO: Check if use of gcc intrinsic gives a better chance for cross call optimizations
static inline intptr_t __TBB_machine_lg( uintptr_t x ) {
    __TBB_ASSERT(x, "__TBB_Log2(0) undefined");
    uintptr_t j;
    __asm__ ("bsr %1,%0" : "=r"(j) : "r"(x));
    return j;
}
#define __TBB_Log2(V)  __TBB_machine_lg(V)

#ifndef __TBB_Pause
//TODO: check if raising a ratio of pause instructions to loop control instructions
//(via e.g. loop unrolling) gives any benefit for HT.  E.g, the current implementation
//does about 2 CPU-consuming instructions for every pause instruction.  Perhaps for
//high pause counts it should use an unrolled loop to raise the ratio, and thus free
//up more integer cycles for the other hyperthread.  On the other hand, if the loop is
//unrolled too far, it won't fit in the core's loop cache, and thus take away
//instruction decode slots from the other hyperthread.

//TODO: check if use of gcc __builtin_ia32_pause intrinsic gives a "some how" better performing code
static inline void __TBB_machine_pause( int32_t delay ) {
    for (int32_t i = 0; i < delay; i++) {
       __asm__ __volatile__("pause;");
    }
    return;
}
#define __TBB_Pause(V) __TBB_machine_pause(V)
#endif /* !__TBB_Pause */

// API to retrieve/update FPU control setting
#ifndef __TBB_CPU_CTL_ENV_PRESENT
#define __TBB_CPU_CTL_ENV_PRESENT 1

struct __TBB_cpu_ctl_env_t {
    int     mxcsr;
    short   x87cw;
};
inline void __TBB_get_cpu_ctl_env ( __TBB_cpu_ctl_env_t* ctl ) {
#if __TBB_ICC_12_0_INL_ASM_FSTCW_BROKEN
    __TBB_cpu_ctl_env_t loc_ctl;
    __asm__ __volatile__ (
            "stmxcsr %0\n\t"
            "fstcw %1"
            : "=m"(loc_ctl.mxcsr), "=m"(loc_ctl.x87cw)
    );
    *ctl = loc_ctl;
#else
    __asm__ __volatile__ (
            "stmxcsr %0\n\t"
            "fstcw %1"
            : "=m"(ctl->mxcsr), "=m"(ctl->x87cw)
    );
#endif
}
inline void __TBB_set_cpu_ctl_env ( const __TBB_cpu_ctl_env_t* ctl ) {
    __asm__ __volatile__ (
            "ldmxcsr %0\n\t"
            "fldcw %1"
            : : "m"(ctl->mxcsr), "m"(ctl->x87cw)
    );
}
#endif /* !__TBB_CPU_CTL_ENV_PRESENT */

#endif /* __TBB_machine_gcc_ia32_common_H */

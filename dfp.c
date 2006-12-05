/* DFP.C        (c) Copyright Roger Bowler, 2006                     */
/*              Decimal Floating Point instructions                  */

/*-------------------------------------------------------------------*/
/* This module implements the Decimal Floating Point instructions    */
/* and the Floating Point Support Enhancement Facility instructions  */
/* described in the z/Architecture Principles of Operation manual.   */
/*-------------------------------------------------------------------*/

#include "hstdinc.h"

#if !defined(_HENGINE_DLL_)
#define _HENGINE_DLL_
#endif

#if !defined(_DFP_C_)
#define _DFP_C_
#endif

#include "hercules.h"
#include "opcode.h"
#include "inline.h"

#if defined(FEATURE_DECIMAL_FLOATING_POINT)
#include "decimal128.h"
#include "decimal64.h"
#include "decimal32.h"
#endif /*defined(FEATURE_DECIMAL_FLOATING_POINT)*/

#if defined(FEATURE_FPS_ENHANCEMENT)
/*===================================================================*/
/* FLOATING POINT SUPPORT INSTRUCTIONS                               */
/*===================================================================*/
/* Note: the Floating Point Support instructions use the HFPREG_CHECK
   and HFPREG2_CHECK macros to enforce an AFP-register data exception
   if an FPS instruction attempts to use one of the 12 additional FPR
   registers when the AFP-register-control bit in CR0 is zero. */

/*-------------------------------------------------------------------*/
/* B370 LPDFR - Load Positive FPR Long Register                [RRE] */
/*-------------------------------------------------------------------*/
DEF_INST(load_positive_fpr_long_reg)
{
int     r1, r2;                         /* Values of R fields        */
int     i1, i2;                         /* FP register subscripts    */

    RRE(inst, regs, r1, r2);
    HFPREG2_CHECK(r1, r2, regs);
    i1 = FPR2I(r1);
    i2 = FPR2I(r2);

    /* Copy register contents, clear the sign bit */
    regs->fpr[i1] = regs->fpr[i2] & 0x7FFFFFFF;
    regs->fpr[i1+1] = regs->fpr[i2+1];

} /* end DEF_INST(load_positive_fpr_long_reg) */


/*-------------------------------------------------------------------*/
/* B371 LNDFR - Load Negative FPR Long Register                [RRE] */
/*-------------------------------------------------------------------*/
DEF_INST(load_negative_fpr_long_reg)
{
int     r1, r2;                         /* Values of R fields        */
int     i1, i2;                         /* FP register subscripts    */

    RRE(inst, regs, r1, r2);
    HFPREG2_CHECK(r1, r2, regs);
    i1 = FPR2I(r1);
    i2 = FPR2I(r2);

    /* Copy register contents, set the sign bit */
    regs->fpr[i1] = regs->fpr[i2] | 0x80000000;
    regs->fpr[i1+1] = regs->fpr[i2+1];

} /* end DEF_INST(load_negative_fpr_long_reg) */


/*-------------------------------------------------------------------*/
/* B372 CPSDR - Copy Sign FPR Long Register                    [RRF] */
/*-------------------------------------------------------------------*/
DEF_INST(copy_sign_fpr_long_reg)
{
int     r1, r2, r3;                     /* Values of R fields        */
int     i1, i2, i3;                     /* FP register subscripts    */

    RRF_M(inst, regs, r1, r2, r3);
    HFPREG2_CHECK(r1, r2, regs);
    HFPREG_CHECK(r3, regs);
    i1 = FPR2I(r1);
    i2 = FPR2I(r2);
    i3 = FPR2I(r3);

    /* Copy register contents */
    regs->fpr[i1] = regs->fpr[i2];
    regs->fpr[i1+1] = regs->fpr[i2+1];

    /* Copy the sign bit from r3 register */
    regs->fpr[i1] &= 0x7FFFFFFF;
    regs->fpr[i1] |= regs->fpr[i3] & 0x80000000;

} /* end DEF_INST(copy_sign_fpr_long_reg) */


/*-------------------------------------------------------------------*/
/* B373 LCDFR - Load Complement FPR Long Register              [RRE] */
/*-------------------------------------------------------------------*/
DEF_INST(load_complement_fpr_long_reg)
{
int     r1, r2;                         /* Values of R fields        */
int     i1, i2;                         /* FP register subscripts    */

    RRE(inst, regs, r1, r2);
    HFPREG2_CHECK(r1, r2, regs);
    i1 = FPR2I(r1);
    i2 = FPR2I(r2);

    /* Copy register contents, invert sign bit */
    regs->fpr[i1] = regs->fpr[i2] ^ 0x80000000;
    regs->fpr[i1+1] = regs->fpr[i2+1];

} /* end DEF_INST(load_complement_fpr_long_reg) */


/*-------------------------------------------------------------------*/
/* B3C1 LDGR  - Load FPR from GR Long Register                 [RRE] */
/*-------------------------------------------------------------------*/
DEF_INST(load_fpr_from_gr_long_reg)
{
int     r1, r2;                         /* Values of R fields        */
int     i1;                             /* FP register subscript     */

    RRE(inst, regs, r1, r2);
    HFPREG_CHECK(r1, regs);
    i1 = FPR2I(r1);

    /* Load FP register contents from general register */
    regs->fpr[i1] = regs->GR_H(r2);
    regs->fpr[i1+1] = regs->GR_L(r2);

} /* end DEF_INST(load_fpr_from_gr_long_reg) */


/*-------------------------------------------------------------------*/
/* B3CD LGDR  - Load GR from FPR Long Register                 [RRE] */
/*-------------------------------------------------------------------*/
DEF_INST(load_gr_from_fpr_long_reg)
{
int     r1, r2;                         /* Values of R fields        */
int     i2;                             /* FP register subscript     */

    RRE(inst, regs, r1, r2);
    HFPREG_CHECK(r2, regs);
    i2 = FPR2I(r2);

    /* Load general register contents from FP register */
    regs->GR_H(r1) = regs->fpr[i2];
    regs->GR_L(r1) = regs->fpr[i2+1];

} /* end DEF_INST(load_gr_from_fpr_long_reg) */


#endif /*defined(FEATURE_FPS_ENHANCEMENT)*/


#if defined(FEATURE_DECIMAL_FLOATING_POINT)
/*===================================================================*/
/* DECIMAL FLOATING POINT INSTRUCTIONS                               */
/*===================================================================*/
/* Note: the DFP instructions use the DFPINST_CHECK macro to check the
   setting of the AFP-register-control bit in CR0. If this bit is zero
   then the macro generates a DFP-instruction data exception. */

#if !defined(_DFP_ARCH_INDEPENDENT_)
/*-------------------------------------------------------------------*/
/* Check if IEEE-interruption-simulation event is to be recognized   */
/*                                                                   */
/* This subroutine is called by the LFAS and SFASR instructions to   */
/* determine whether the instruction should raise a data exception   */
/* at the end of the instruction and, if so, the DXC code to be set. */
/*                                                                   */
/* Input:                                                            */
/*      cur_fpc         Current value of the FPC register            */
/*      src_fpc         Value of instruction source operand          */
/* Output:                                                           */
/*      The return value is the data exception code (DXC), or        */
/*      zero if no IEEE-interruption-simulation event is recognized  */
/*-------------------------------------------------------------------*/
static U32
fpc_signal_check(U32 cur_fpc, U32 src_fpc)
{
U32     cf, sm, enabled_flags;          /* Mask and flag work areas  */
U32     dxc;                            /* Data exception code or 0  */

    /* AND the current FPC flags with the source FPC mask */
    cf = (cur_fpc & FPC_FLAG) >> FPC_FLAG_SHIFT;
    sm = (src_fpc & FPC_MASK) >> FPC_MASK_SHIFT;
    enabled_flags = (cf & sm) << FPC_FLAG_SHIFT;

    /* An IEEE-interruption-simulation event is recognized 
       if any current flag corresponds to the source mask */
    if (enabled_flags & FPC_FLAG_SFI)
    {
        dxc = DXC_IEEE_INV_OP_IISE;
    }
    else if (enabled_flags & FPC_FLAG_SFZ)
    {
        dxc = DXC_IEEE_DIV_ZERO_IISE;
    }
    else if (enabled_flags & FPC_FLAG_SFO)
    {
        dxc = (cur_fpc & FPC_FLAG_SFX) ?
                DXC_IEEE_OF_INEX_IISE :
                DXC_IEEE_OF_EXACT_IISE;
    }
    else if (enabled_flags & FPC_FLAG_SFU)
    {
        dxc = (cur_fpc & FPC_FLAG_SFX) ?
                DXC_IEEE_UF_INEX_IISE :
                DXC_IEEE_UF_EXACT_IISE;
    }
    else if (enabled_flags & FPC_FLAG_SFX)
    {
        dxc = DXC_IEEE_INEXACT_IISE;
    }
    else
    {
        dxc = 0;
    }

    /* Return data exception code or zero */
    return dxc;
} /* end function fpc_signal_check */
#define _DFP_ARCH_INDEPENDENT_
#endif /*!defined(_DFP_ARCH_INDEPENDENT_)*/

/*-------------------------------------------------------------------*/
/* Copy a DFP extended register into a decimal128 structure          */
/*                                                                   */
/* Input:                                                            */
/*      rn      FP register number (left register of pair)           */
/*      xp      Pointer to decimal128 structure                      */
/*      regs    CPU register context                                 */
/*-------------------------------------------------------------------*/
static inline void
ARCH_DEP(dfp_reg_to_decimal128) (int rn, decimal128 *xp, REGS *regs)
{
int     i, j;                           /* FP register subscripts    */
QW      *qp;                            /* Quadword pointer          */

    i = FPR2I(rn);                      /* Left register index       */
    j = i + FPREX;                      /* Right register index      */
    qp = (QW*)xp;                       /* Convert to QW pointer     */
    qp->F.HH.F = regs->fpr[i];          /* Copy FPR bits 0-31        */
    qp->F.HL.F = regs->fpr[i+1];        /* Copy FPR bits 32-63       */
    qp->F.LH.F = regs->fpr[j];          /* Copy FPR bits 64-95       */
    qp->F.LL.F = regs->fpr[j+1];        /* Copy FPR bits 96-127      */

} /* end function dfp_reg_to_decimal128 */

/*-------------------------------------------------------------------*/
/* Load a DFP extended register from a decimal128 structure          */
/*                                                                   */
/* Input:                                                            */
/*      rn      FP register number (left register of pair)           */
/*      xp      Pointer to decimal128 structure                      */
/*      regs    CPU register context                                 */
/*-------------------------------------------------------------------*/
static inline void
ARCH_DEP(dfp_reg_from_decimal128) (int rn, decimal128 *xp, REGS *regs)
{
int     i, j;                           /* FP register subscripts    */
QW      *qp;                            /* Quadword pointer          */

    i = FPR2I(rn);                      /* Left register index       */
    j = i + FPREX;                      /* Right register index      */
    qp = (QW*)xp;                       /* Convert to QW pointer     */
    regs->fpr[i]   = qp->F.HH.F;        /* Load FPR bits 0-31        */
    regs->fpr[i+1] = qp->F.HL.F;        /* Load FPR bits 32-63       */
    regs->fpr[j]   = qp->F.LH.F;        /* Load FPR bits 64-95       */
    regs->fpr[j+1] = qp->F.LL.F;        /* Load FPR bits 96-127      */

} /* end function dfp_reg_from_decimal128 */

/*-------------------------------------------------------------------*/
/* Check for DFP exception conditions and raise data exception       */
/*                                                                   */
/* Input:                                                            */
/*      set     Decimal number context                               */
/*      regs    CPU register context                                 */
/*-------------------------------------------------------------------*/
static inline void
ARCH_DEP(dfp_status_check) (decContext *set, REGS *regs)
{


} /* end function dfp_status_check */


#define UNDEF_INST(_x) \
        DEF_INST(_x) { ARCH_DEP(operation_exception) \
        (inst,regs); }

/*-------------------------------------------------------------------*/
/* B3DA AXTR  - Add DFP Extended Register                      [RRR] */
/*-------------------------------------------------------------------*/
DEF_INST(add_dfp_ext_reg) 
{
int             r1, r2, r3;             /* Values of R fields        */
int             i1, i2, i3;             /* FP register subscripts    */
decimal128      x1, x2, x3;             /* Extended DFP values       */
decNumber       d1, d2, d3;             /* Working decimal numbers   */
decContext      set;                    /* Working context           */

    RRR(inst, regs, r1, r2, r3);
    DFPINST_CHECK(regs);
    DFPREGPAIR3_CHECK(r1, r2, r3, regs);
    i1 = FPR2I(r1);
    i2 = FPR2I(r2);
    i3 = FPR2I(r3);

    /* Initialise the context for extended DFP */
    decContextDefault(&set, DEC_INIT_DECIMAL128);

    /* Add r3 to r2 giving result in r1 */
    ARCH_DEP(dfp_reg_to_decimal128)(r2, &x2, regs);
    ARCH_DEP(dfp_reg_to_decimal128)(r3, &x3, regs);
    decimal128ToNumber(&x2, &d2);
    decimal128ToNumber(&x3, &d3);
    decNumberAdd(&d1, &d2, &d3, &set);
    decimal128FromNumber(&x1, &d1, &set);
    ARCH_DEP(dfp_reg_from_decimal128)(r1, &x1, regs);

    /* Set condition code */
    regs->psw.cc = decNumberIsZero(&d1) ? 0 :
                   decNumberIsNegative(&d1) ? 1 : 2;

    /* Raise data exception if error occurred */
    ARCH_DEP(dfp_status_check)(&set, regs);

} /* end DEF_INST(add_dfp_ext_reg) */ 


/* Additional Decimal Floating Point instructions to be inserted here */
UNDEF_INST(add_dfp_long_reg)
UNDEF_INST(compare_dfp_ext_reg)
UNDEF_INST(compare_dfp_long_reg)
UNDEF_INST(compare_and_signal_dfp_ext_reg)
UNDEF_INST(compare_and_signal_dfp_long_reg)
UNDEF_INST(compare_exponent_dfp_ext_reg)
UNDEF_INST(compare_exponent_dfp_long_reg)
UNDEF_INST(convert_fix64_to_dfp_ext_reg)
UNDEF_INST(convert_fix64_to_dfp_long_reg)
UNDEF_INST(convert_sbcd128_to_dfp_ext_reg)
UNDEF_INST(convert_sbcd64_to_dfp_long_reg)
UNDEF_INST(convert_ubcd128_to_dfp_ext_reg)
UNDEF_INST(convert_ubcd64_to_dfp_long_reg)
UNDEF_INST(convert_dfp_ext_to_fix64_reg)
UNDEF_INST(convert_dfp_long_to_fix64_reg)
UNDEF_INST(convert_dfp_ext_to_sbcd128_reg)
UNDEF_INST(convert_dfp_long_to_sbcd64_reg)
UNDEF_INST(convert_dfp_ext_to_ubcd128_reg)
UNDEF_INST(convert_dfp_long_to_ubcd64_reg)
UNDEF_INST(divide_dfp_ext_reg)
UNDEF_INST(divide_dfp_long_reg)
UNDEF_INST(extract_biased_exponent_dfp_ext_to_fix64_reg)
UNDEF_INST(extract_biased_exponent_dfp_long_to_fix64_reg)
UNDEF_INST(extract_significance_dfp_ext_reg)
UNDEF_INST(extract_significance_dfp_long_reg)
UNDEF_INST(insert_biased_exponent_fix64_to_dfp_ext_reg)
UNDEF_INST(insert_biased_exponent_fix64_to_dfp_long_reg)
UNDEF_INST(load_and_test_dfp_ext_reg)
UNDEF_INST(load_and_test_dfp_long_reg)
UNDEF_INST(load_fp_int_dfp_ext_reg)
UNDEF_INST(load_fp_int_dfp_long_reg)

 
/*-------------------------------------------------------------------*/
/* B2BD LFAS  - Load FPC and Signal                              [S] */
/*-------------------------------------------------------------------*/
DEF_INST(load_fpc_and_signal)
{
int     b2;                             /* Base of effective addr    */
VADR    effective_addr2;                /* Effective address         */
U32     src_fpc, new_fpc;               /* New value for FPC         */
U32     dxc;

    S(inst, regs, b2, effective_addr2);

    DFPINST_CHECK(regs);

    /* Load new FPC register contents from operand location */
    src_fpc = ARCH_DEP(vfetch4) (effective_addr2, b2, regs);

    /* Program check if reserved bits are non-zero */
    FPC_CHECK(src_fpc, regs);

    /* OR the flags from the current FPC register */
    new_fpc = src_fpc | (regs->fpc & FPC_FLAG);

    /* Determine whether an event is to be signaled */
    dxc = fpc_signal_check(regs->fpc, src_fpc);

    /* Update the FPC register */
    regs->fpc = new_fpc;

    /* Signal an IEEE-interruption-simulation event if needed */
    if (dxc != 0)
    {
        regs->dxc = dxc;
        ARCH_DEP(program_interrupt) (regs, PGM_DATA_EXCEPTION);
    }

} /* end DEF_INST(load_fpc_and_signal) */


UNDEF_INST(load_lengthened_dfp_long_to_ext_reg)
UNDEF_INST(load_lengthened_dfp_short_to_long_reg)
UNDEF_INST(load_rounded_dfp_ext_to_long_reg)
UNDEF_INST(load_rounded_dfp_long_to_short_reg)
UNDEF_INST(multiply_dfp_ext_reg)
UNDEF_INST(multiply_dfp_long_reg)
UNDEF_INST(quantize_dfp_ext_reg)
UNDEF_INST(quantize_dfp_long_reg)
UNDEF_INST(reround_dfp_ext_reg)
UNDEF_INST(reround_dfp_long_reg)

 
/*-------------------------------------------------------------------*/
/* B2B9 SRNMT - Set DFP Rounding Mode                            [S] */
/*-------------------------------------------------------------------*/
DEF_INST(set_dfp_rounding_mode)
{
int     b2;                             /* Base of effective addr    */
VADR    effective_addr2;                /* Effective address         */

    S(inst, regs, b2, effective_addr2);

    DFPINST_CHECK(regs);

    /* Set DFP rounding mode in FPC register from address bits 61-63 */
    regs->fpc &= ~(FPC_DRM);
    regs->fpc |= ((effective_addr2 << FPC_DRM_SHIFT) & FPC_DRM);

} /* end DEF_INST(set_dfp_rounding_mode) */


/*-------------------------------------------------------------------*/
/* B385 SFASR - Set FPC and Signal                             [RRE] */
/*-------------------------------------------------------------------*/
DEF_INST(set_fpc_and_signal)
{
int     r1, unused;                     /* Values of R fields        */
U32     src_fpc, new_fpc;               /* New value for FPC         */
U32     dxc;

    RRE(inst, regs, r1, unused);

    DFPINST_CHECK(regs);

    /* Load new FPC register contents from R1 register bits 32-63 */
    src_fpc = regs->GR_L(r1);

    /* Program check if reserved bits are non-zero */
    FPC_CHECK(src_fpc, regs);
     
    /* OR the flags from the current FPC register */
    new_fpc = src_fpc | (regs->fpc & FPC_FLAG);

    /* Determine whether an event is to be signaled */
    dxc = fpc_signal_check(regs->fpc, src_fpc);

    /* Update the FPC register */
    regs->fpc = new_fpc;

    /* Signal an IEEE-interruption-simulation event if needed */
    if (dxc != 0)
    {
        regs->dxc = dxc;
        ARCH_DEP(program_interrupt) (regs, PGM_DATA_EXCEPTION);
    }

} /* end DEF_INST(set_fpc_and_signal) */


UNDEF_INST(shift_coefficient_left_dfp_ext)
UNDEF_INST(shift_coefficient_left_dfp_long)
UNDEF_INST(shift_coefficient_right_dfp_ext)
UNDEF_INST(shift_coefficient_right_dfp_long)
UNDEF_INST(subtract_dfp_ext_reg)
UNDEF_INST(subtract_dfp_long_reg)
UNDEF_INST(test_data_class_dfp_ext)
UNDEF_INST(test_data_class_dfp_long)
UNDEF_INST(test_data_class_dfp_short)
UNDEF_INST(test_data_group_dfp_ext)
UNDEF_INST(test_data_group_dfp_long)
UNDEF_INST(test_data_group_dfp_short)

#endif /*defined(FEATURE_DECIMAL_FLOATING_POINT)*/

#if !defined(_GEN_ARCH)

#if defined(_ARCHMODE2)
 #define  _GEN_ARCH _ARCHMODE2
 #include "dfp.c"
#endif

#if defined(_ARCHMODE3)
 #undef   _GEN_ARCH
 #define  _GEN_ARCH _ARCHMODE3
 #include "dfp.c"
#endif

#endif /*!defined(_GEN_ARCH)*/


/* end of dfp.c */

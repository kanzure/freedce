/*
 * POWERPC/ndr_rep.h 
 *
 * platform dependent (OS + Architecture) file split out from stubbase.h 
 * for DCE 1.1 code cleanup.
 *
 * For POWERPC processors - Little Endian mode
 *
 * This file contains the architecture specific definitions of the 
 * local scalar data representation used 
 *
 * This file is always included as part of stubbase.h 
 */

#ifndef _NDR_REP_H 
#define _NDR_REP_H

#define NDR_LOCAL_INT_REP     ndr_c_big_little_endian
#define NDR_LOCAL_FLOAT_REP   ndr_c_float_ieee
#define NDR_LOCAL_CHAR_REP    ndr_c_char_ascii

/* 
 * macro for IDL alignment 
 */
 
#define IDL_NATURAL_ALIGN_4

#endif /* _NDR_REP_H */

/***********************************************************************
 * Copyright (c) 2020 Peter Dettman                                    *
 * Distributed under the MIT software license, see the accompanying    *
 * file COPYING or https://www.opensource.org/licenses/mit-license.php.*
 **********************************************************************/

#ifndef SECP256K1_MODINV64_IMPL_H
#define SECP256K1_MODINV64_IMPL_H

#include "modinv64.h"

#include "util.h"

/* This file implements modular inversion based on the paper "Fast constant-time gcd computation and
 * modular inversion" by Daniel J. Bernstein and Bo-Yin Yang.
 *
 * For an explanation of the algorithm, see doc/safegcd_implementation.md. This file contains an
 * implementation for N=62, using 62-bit signed limbs represented as int64_t.
 */

#ifdef VERIFY
/* Helper function to compute the absolute value of an int64_t.
 * (we don't use abs/labs/llabs as it depends on the int sizes). */
static int64_t secp256k1_modinv64_abs(int64_t v) {
    VERIFY_CHECK(v > INT64_MIN);
    if (v < 0) return -v;
    return v;
}

static const secp256k1_modinv64_signed62 SECP256K1_SIGNED62_ONE = {{1}};

/* Compute a*factor and put it in r. All but the top limb in r will be in range [0,2^62). */
static void secp256k1_modinv64_mul_62(secp256k1_modinv64_signed62 *r, const secp256k1_modinv64_signed62 *a, int64_t factor) {
    const int64_t M62 = (int64_t)(UINT64_MAX >> 2);
    int128_t c = 0;
    int i;
    for (i = 0; i < 4; ++i) {
        c += (int128_t)a->v[i] * factor;
        r->v[i] = (int64_t)c & M62; c >>= 62;
    }
    c += (int128_t)a->v[4] * factor;
    VERIFY_CHECK(c == (int64_t)c);
    r->v[4] = (int64_t)c;
}

/* Return -1 for a<b*factor, 0 for a==b*factor, 1 for a>b*factor. */
static int secp256k1_modinv64_mul_cmp_62(const secp256k1_modinv64_signed62 *a, const secp256k1_modinv64_signed62 *b, int64_t factor) {
    int i;
    secp256k1_modinv64_signed62 am, bm;
    secp256k1_modinv64_mul_62(&am, a, 1); /* Normalize all but the top limb of a. */
    secp256k1_modinv64_mul_62(&bm, b, factor);
    for (i = 0; i < 4; ++i) {
        /* Verify that all but the top limb of a and b are normalized. */
        VERIFY_CHECK(am.v[i] >> 62 == 0);
        VERIFY_CHECK(bm.v[i] >> 62 == 0);
    }
    for (i = 4; i >= 0; --i) {
        if (am.v[i] < bm.v[i]) return -1;
        if (am.v[i] > bm.v[i]) return 1;
    }
    return 0;
}
#endif

/* Take as input a signed62 number in range (-2*modulus,modulus), and add a multiple of the modulus
 * to it to bring it to range [0,modulus). If sign < 0, the input will also be negated in the
 * process. The input must have limbs in range (-2^62,2^62). The output will have limbs in range
 * [0,2^62). */
static void secp256k1_modinv64_normalize_62(secp256k1_modinv64_signed62 *r, int64_t sign, const secp256k1_modinv64_modinfo *modinfo) {
    const int64_t M62 = (int64_t)(UINT64_MAX >> 2);
    int64_t r0 = r->v[0], r1 = r->v[1], r2 = r->v[2], r3 = r->v[3], r4 = r->v[4];
    int64_t cond_add, cond_negate;

#ifdef VERIFY
    /* Verify that all limbs are in range (-2^62,2^62). */
    int i;
    for (i = 0; i < 5; ++i) {
        VERIFY_CHECK(r->v[i] >= -M62);
        VERIFY_CHECK(r->v[i] <= M62);
    }
    VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(r, &modinfo->modulus, -2) > 0); /* r > -2*modulus */
    VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(r, &modinfo->modulus, 1) < 0); /* r < modulus */
#endif

    /* In a first step, add the modulus if the input is negative, and then negate if requested.
     * This brings r from range (-2*modulus,modulus) to range (-modulus,modulus). As all input
     * limbs are in range (-2^62,2^62), this cannot overflow an int64_t. Note that the right
     * shifts below are signed sign-extending shifts (see assumptions.h for tests that that is
     * indeed the behavior of the right shift operator). */
    cond_add = r4 >> 63;
    r0 += modinfo->modulus.v[0] & cond_add;
    r1 += modinfo->modulus.v[1] & cond_add;
    r2 += modinfo->modulus.v[2] & cond_add;
    r3 += modinfo->modulus.v[3] & cond_add;
    r4 += modinfo->modulus.v[4] & cond_add;
    cond_negate = sign >> 63;
    r0 = (r0 ^ cond_negate) - cond_negate;
    r1 = (r1 ^ cond_negate) - cond_negate;
    r2 = (r2 ^ cond_negate) - cond_negate;
    r3 = (r3 ^ cond_negate) - cond_negate;
    r4 = (r4 ^ cond_negate) - cond_negate;
    /* Propagate the top bits, to bring limbs back to range (-2^62,2^62). */
    r1 += r0 >> 62; r0 &= M62;
    r2 += r1 >> 62; r1 &= M62;
    r3 += r2 >> 62; r2 &= M62;
    r4 += r3 >> 62; r3 &= M62;

    /* In a second step add the modulus again if the result is still negative, bringing
     * r to range [0,modulus). */
    cond_add = r4 >> 63;
    r0 += modinfo->modulus.v[0] & cond_add;
    r1 += modinfo->modulus.v[1] & cond_add;
    r2 += modinfo->modulus.v[2] & cond_add;
    r3 += modinfo->modulus.v[3] & cond_add;
    r4 += modinfo->modulus.v[4] & cond_add;
    /* And propagate again. */
    r1 += r0 >> 62; r0 &= M62;
    r2 += r1 >> 62; r1 &= M62;
    r3 += r2 >> 62; r2 &= M62;
    r4 += r3 >> 62; r3 &= M62;

    r->v[0] = r0;
    r->v[1] = r1;
    r->v[2] = r2;
    r->v[3] = r3;
    r->v[4] = r4;

#ifdef VERIFY
    VERIFY_CHECK(r0 >> 62 == 0);
    VERIFY_CHECK(r1 >> 62 == 0);
    VERIFY_CHECK(r2 >> 62 == 0);
    VERIFY_CHECK(r3 >> 62 == 0);
    VERIFY_CHECK(r4 >> 62 == 0);
    VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(r, &modinfo->modulus, 0) >= 0); /* r >= 0 */
    VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(r, &modinfo->modulus, 1) < 0); /* r < modulus */
#endif
}

/* Data type for transition matrices (see section 3 of explanation).
 *
 * t = [ u  v ]
 *     [ q  r ]
 */
typedef struct {
    int64_t u, v, q, r;
} secp256k1_modinv64_trans2x2;

/* Compute the transition matrix and eta for 62 divsteps.
 *
 * Input:  eta: initial eta
 *         f0:  bottom limb of initial f
 *         g0:  bottom limb of initial g
 * Output: t: transition matrix
 * Return: final eta
 *
 * Implements the divsteps_n_matrix function from the explanation.
 */
static int64_t secp256k1_modinv64_divsteps_62(int64_t eta, uint64_t f0, uint64_t g0, secp256k1_modinv64_trans2x2 *t) {
    /* u,v,q,r are the elements of the transformation matrix being built up,
     * starting with the identity matrix. Semantically they are signed integers
     * in range [-2^62,2^62], but here represented as unsigned mod 2^64. This
     * permits left shifting (which is UB for negative numbers). The range
     * being inside [-2^63,2^63) means that casting to signed works correctly.
     */
    uint64_t u = 1, v = 0, q = 0, r = 1;
    uint64_t c1, c2, f = f0, g = g0, x, y, z;
    int i;

    for (i = 0; i < 62; ++i) {
        VERIFY_CHECK((f & 1) == 1); /* f must always be odd */
        VERIFY_CHECK((u * f0 + v * g0) == f << i);
        VERIFY_CHECK((q * f0 + r * g0) == g << i);
        /* Compute conditional masks for (eta < 0) and for (g & 1). */
        c1 = eta >> 63;
        c2 = -(g & 1);
        /* Compute x,y,z, conditionally negated versions of f,u,v. */
        x = (f ^ c1) - c1;
        y = (u ^ c1) - c1;
        z = (v ^ c1) - c1;
        /* Conditionally add x,y,z to g,q,r. */
        g += x & c2;
        q += y & c2;
        r += z & c2;
        /* In what follows, c1 is a condition mask for (eta < 0) and (g & 1). */
        c1 &= c2;
        /* Conditionally negate eta, and unconditionally subtract 1. */
        eta = (eta ^ c1) - (c1 + 1);
        /* Conditionally add g,q,r to f,u,v. */
        f += g & c1;
        u += q & c1;
        v += r & c1;
        /* Shifts */
        g >>= 1;
        u <<= 1;
        v <<= 1;
        /* Bounds on eta that follow from the bounds on iteration count (max 12*62 divsteps). */
        VERIFY_CHECK(eta >= -745 && eta <= 745);
    }
    /* Return data in t and return value. */
    t->u = (int64_t)u;
    t->v = (int64_t)v;
    t->q = (int64_t)q;
    t->r = (int64_t)r;
    /* The determinant of t must be a power of two. This guarantees that multiplication with t
     * does not change the gcd of f and g, apart from adding a power-of-2 factor to it (which
     * will be divided out again). As each divstep's individual matrix has determinant 2, the
     * aggregate of 62 of them will have determinant 2^62. */
    VERIFY_CHECK((int128_t)t->u * t->r - (int128_t)t->v * t->q == ((int128_t)1) << 62);
    return eta;
}

/* Compute the transition matrix and eta for 62 divsteps (variable time).
 *
 * Input:  eta: initial eta
 *         f0:  bottom limb of initial f
 *         g0:  bottom limb of initial g
 * Output: t: transition matrix
 * Return: final eta
 *
 * Implements the divsteps_n_matrix_var function from the explanation.
 */
static int64_t secp256k1_modinv64_divsteps_62_var(int64_t eta, uint64_t f0, uint64_t g0, secp256k1_modinv64_trans2x2 *t) {
    /* Transformation matrix; see comments in secp256k1_modinv64_divsteps_62. */
    uint64_t u = 1, v = 0, q = 0, r = 1;
    uint64_t f = f0, g = g0, m;
    uint32_t w;
    int i = 62, limit, zeros;

    for (;;) {
        /* Use a sentinel bit to count zeros only up to i. */
        zeros = secp256k1_ctz64_var(g | (UINT64_MAX << i));
        /* Perform zeros divsteps at once; they all just divide g by two. */
        g >>= zeros;
        u <<= zeros;
        v <<= zeros;
        eta -= zeros;
        i -= zeros;
        /* We're done once we've done 62 divsteps. */
        if (i == 0) break;
        VERIFY_CHECK((f & 1) == 1);
        VERIFY_CHECK((g & 1) == 1);
        VERIFY_CHECK((u * f0 + v * g0) == f << (62 - i));
        VERIFY_CHECK((q * f0 + r * g0) == g << (62 - i));
        /* Bounds on eta that follow from the bounds on iteration count (max 12*62 divsteps). */
        VERIFY_CHECK(eta >= -745 && eta <= 745);
        /* If eta is negative, negate it and replace f,g with g,-f. */
        if (eta < 0) {
            uint64_t tmp;
            eta = -eta;
            tmp = f; f = g; g = -tmp;
            tmp = u; u = q; q = -tmp;
            tmp = v; v = r; r = -tmp;
            /* Use a formula to cancel out up to 6 bits of g. Also, no more than i can be cancelled
             * out (as we'd be done before that point), and no more than eta+1 can be done as its
             * will flip again once that happens. */
            limit = ((int)eta + 1) > i ? i : ((int)eta + 1);
            VERIFY_CHECK(limit > 0 && limit <= 62);
            /* m is a mask for the bottom min(limit, 6) bits. */
            m = (UINT64_MAX >> (64 - limit)) & 63U;
            /* Find what multiple of f must be added to g to cancel its bottom min(limit, 6)
             * bits. */
            w = (f * g * (f * f - 2)) & m;
        } else {
            /* In this branch, use a simpler formula that only lets us cancel up to 4 bits of g, as
             * eta tends to be smaller here. */
            limit = ((int)eta + 1) > i ? i : ((int)eta + 1);
            VERIFY_CHECK(limit > 0 && limit <= 62);
            /* m is a mask for the bottom min(limit, 4) bits. */
            m = (UINT64_MAX >> (64 - limit)) & 15U;
            /* Find what multiple of f must be added to g to cancel its bottom min(limit, 4)
             * bits. */
            w = f + (((f + 1) & 4) << 1);
            w = (-w * g) & m;
        }
        g += f * w;
        q += u * w;
        r += v * w;
        VERIFY_CHECK((g & m) == 0);
    }
    /* Return data in t and return value. */
    t->u = (int64_t)u;
    t->v = (int64_t)v;
    t->q = (int64_t)q;
    t->r = (int64_t)r;
    /* The determinant of t must be a power of two. This guarantees that multiplication with t
     * does not change the gcd of f and g, apart from adding a power-of-2 factor to it (which
     * will be divided out again). As each divstep's individual matrix has determinant 2, the
     * aggregate of 62 of them will have determinant 2^62. */
    VERIFY_CHECK((int128_t)t->u * t->r - (int128_t)t->v * t->q == ((int128_t)1) << 62);
    return eta;
}

/* Compute (t/2^62) * [d, e] mod modulus, where t is a transition matrix for 62 divsteps.
 *
 * On input and output, d and e are in range (-2*modulus,modulus). All output limbs will be in range
 * (-2^62,2^62).
 *
 * This implements the update_de function from the explanation.
 */
static void secp256k1_modinv64_update_de_62(secp256k1_modinv64_signed62 *d, secp256k1_modinv64_signed62 *e, const secp256k1_modinv64_trans2x2 *t, const secp256k1_modinv64_modinfo* modinfo) {
    const int64_t M62 = (int64_t)(UINT64_MAX >> 2);
    const int64_t d0 = d->v[0], d1 = d->v[1], d2 = d->v[2], d3 = d->v[3], d4 = d->v[4];
    const int64_t e0 = e->v[0], e1 = e->v[1], e2 = e->v[2], e3 = e->v[3], e4 = e->v[4];
    const int64_t u = t->u, v = t->v, q = t->q, r = t->r;
    int64_t md, me, sd, se;
    int128_t cd, ce;
#ifdef VERIFY
    VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(d, &modinfo->modulus, -2) > 0); /* d > -2*modulus */
    VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(d, &modinfo->modulus, 1) < 0);  /* d <    modulus */
    VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(e, &modinfo->modulus, -2) > 0); /* e > -2*modulus */
    VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(e, &modinfo->modulus, 1) < 0);  /* e <    modulus */
    VERIFY_CHECK((secp256k1_modinv64_abs(u) + secp256k1_modinv64_abs(v)) >= 0); /* |u|+|v| doesn't overflow */
    VERIFY_CHECK((secp256k1_modinv64_abs(q) + secp256k1_modinv64_abs(r)) >= 0); /* |q|+|r| doesn't overflow */
    VERIFY_CHECK((secp256k1_modinv64_abs(u) + secp256k1_modinv64_abs(v)) <= M62 + 1); /* |u|+|v| <= 2^62 */
    VERIFY_CHECK((secp256k1_modinv64_abs(q) + secp256k1_modinv64_abs(r)) <= M62 + 1); /* |q|+|r| <= 2^62 */
#endif
    /* [md,me] start as zero; plus [u,q] if d is negative; plus [v,r] if e is negative. */
    sd = d4 >> 63;
    se = e4 >> 63;
    md = (u & sd) + (v & se);
    me = (q & sd) + (r & se);
    /* Begin computing t*[d,e]. */
    cd = (int128_t)u * d0 + (int128_t)v * e0;
    ce = (int128_t)q * d0 + (int128_t)r * e0;
    /* Correct md,me so that t*[d,e]+modulus*[md,me] has 62 zero bottom bits. */
    md -= (modinfo->modulus_inv62 * (uint64_t)cd + md) & M62;
    me -= (modinfo->modulus_inv62 * (uint64_t)ce + me) & M62;
    /* Update the beginning of computation for t*[d,e]+modulus*[md,me] now md,me are known. */
    cd += (int128_t)modinfo->modulus.v[0] * md;
    ce += (int128_t)modinfo->modulus.v[0] * me;
    /* Verify that the low 62 bits of the computation are indeed zero, and then throw them away. */
    VERIFY_CHECK(((int64_t)cd & M62) == 0); cd >>= 62;
    VERIFY_CHECK(((int64_t)ce & M62) == 0); ce >>= 62;
    /* Compute limb 1 of t*[d,e]+modulus*[md,me], and store it as output limb 0 (= down shift). */
    cd += (int128_t)u * d1 + (int128_t)v * e1;
    ce += (int128_t)q * d1 + (int128_t)r * e1;
    if (modinfo->modulus.v[1]) { /* Optimize for the case where limb of modulus is zero. */
        cd += (int128_t)modinfo->modulus.v[1] * md;
        ce += (int128_t)modinfo->modulus.v[1] * me;
    }
    d->v[0] = (int64_t)cd & M62; cd >>= 62;
    e->v[0] = (int64_t)ce & M62; ce >>= 62;
    /* Compute limb 2 of t*[d,e]+modulus*[md,me], and store it as output limb 1. */
    cd += (int128_t)u * d2 + (int128_t)v * e2;
    ce += (int128_t)q * d2 + (int128_t)r * e2;
    if (modinfo->modulus.v[2]) { /* Optimize for the case where limb of modulus is zero. */
        cd += (int128_t)modinfo->modulus.v[2] * md;
        ce += (int128_t)modinfo->modulus.v[2] * me;
    }
    d->v[1] = (int64_t)cd & M62; cd >>= 62;
    e->v[1] = (int64_t)ce & M62; ce >>= 62;
    /* Compute limb 3 of t*[d,e]+modulus*[md,me], and store it as output limb 2. */
    cd += (int128_t)u * d3 + (int128_t)v * e3;
    ce += (int128_t)q * d3 + (int128_t)r * e3;
    if (modinfo->modulus.v[3]) { /* Optimize for the case where limb of modulus is zero. */
        cd += (int128_t)modinfo->modulus.v[3] * md;
        ce += (int128_t)modinfo->modulus.v[3] * me;
    }
    d->v[2] = (int64_t)cd & M62; cd >>= 62;
    e->v[2] = (int64_t)ce & M62; ce >>= 62;
    /* Compute limb 4 of t*[d,e]+modulus*[md,me], and store it as output limb 3. */
    cd += (int128_t)u * d4 + (int128_t)v * e4;
    ce += (int128_t)q * d4 + (int128_t)r * e4;
    cd += (int128_t)modinfo->modulus.v[4] * md;
    ce += (int128_t)modinfo->modulus.v[4] * me;
    d->v[3] = (int64_t)cd & M62; cd >>= 62;
    e->v[3] = (int64_t)ce & M62; ce >>= 62;
    /* What remains is limb 5 of t*[d,e]+modulus*[md,me]; store it as output limb 4. */
    d->v[4] = (int64_t)cd;
    e->v[4] = (int64_t)ce;
#ifdef VERIFY
    VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(d, &modinfo->modulus, -2) > 0); /* d > -2*modulus */
    VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(d, &modinfo->modulus, 1) < 0);  /* d <    modulus */
    VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(e, &modinfo->modulus, -2) > 0); /* e > -2*modulus */
    VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(e, &modinfo->modulus, 1) < 0);  /* e <    modulus */
#endif
}

/* Compute (t/2^62) * [f, g], where t is a transition matrix for 62 divsteps.
 *
 * This implements the update_fg function from the explanation.
 */
static void secp256k1_modinv64_update_fg_62(secp256k1_modinv64_signed62 *f, secp256k1_modinv64_signed62 *g, const secp256k1_modinv64_trans2x2 *t) {
    const int64_t M62 = (int64_t)(UINT64_MAX >> 2);
    const int64_t f0 = f->v[0], f1 = f->v[1], f2 = f->v[2], f3 = f->v[3], f4 = f->v[4];
    const int64_t g0 = g->v[0], g1 = g->v[1], g2 = g->v[2], g3 = g->v[3], g4 = g->v[4];
    const int64_t u = t->u, v = t->v, q = t->q, r = t->r;
    int128_t cf, cg;
    /* Start computing t*[f,g]. */
    cf = (int128_t)u * f0 + (int128_t)v * g0;
    cg = (int128_t)q * f0 + (int128_t)r * g0;
    /* Verify that the bottom 62 bits of the result are zero, and then throw them away. */
    VERIFY_CHECK(((int64_t)cf & M62) == 0); cf >>= 62;
    VERIFY_CHECK(((int64_t)cg & M62) == 0); cg >>= 62;
    /* Compute limb 1 of t*[f,g], and store it as output limb 0 (= down shift). */
    cf += (int128_t)u * f1 + (int128_t)v * g1;
    cg += (int128_t)q * f1 + (int128_t)r * g1;
    f->v[0] = (int64_t)cf & M62; cf >>= 62;
    g->v[0] = (int64_t)cg & M62; cg >>= 62;
    /* Compute limb 2 of t*[f,g], and store it as output limb 1. */
    cf += (int128_t)u * f2 + (int128_t)v * g2;
    cg += (int128_t)q * f2 + (int128_t)r * g2;
    f->v[1] = (int64_t)cf & M62; cf >>= 62;
    g->v[1] = (int64_t)cg & M62; cg >>= 62;
    /* Compute limb 3 of t*[f,g], and store it as output limb 2. */
    cf += (int128_t)u * f3 + (int128_t)v * g3;
    cg += (int128_t)q * f3 + (int128_t)r * g3;
    f->v[2] = (int64_t)cf & M62; cf >>= 62;
    g->v[2] = (int64_t)cg & M62; cg >>= 62;
    /* Compute limb 4 of t*[f,g], and store it as output limb 3. */
    cf += (int128_t)u * f4 + (int128_t)v * g4;
    cg += (int128_t)q * f4 + (int128_t)r * g4;
    f->v[3] = (int64_t)cf & M62; cf >>= 62;
    g->v[3] = (int64_t)cg & M62; cg >>= 62;
    /* What remains is limb 5 of t*[f,g]; store it as output limb 4. */
    f->v[4] = (int64_t)cf;
    g->v[4] = (int64_t)cg;
}

/* Compute the inverse of x modulo modinfo->modulus, and replace x with it (constant time in x). */
static void secp256k1_modinv64(secp256k1_modinv64_signed62 *x, const secp256k1_modinv64_modinfo *modinfo) {
    /* Start with d=0, e=1, f=modulus, g=x, eta=-1. */
    secp256k1_modinv64_signed62 d = {{0, 0, 0, 0, 0}};
    secp256k1_modinv64_signed62 e = {{1, 0, 0, 0, 0}};
    secp256k1_modinv64_signed62 f = modinfo->modulus;
    secp256k1_modinv64_signed62 g = *x;
    int i;
    int64_t eta = -1;

    /* Do 12 iterations of 62 divsteps each = 744 divsteps. 724 suffices for 256-bit inputs. */
    for (i = 0; i < 12; ++i) {
        /* Compute transition matrix and new eta after 62 divsteps. */
        secp256k1_modinv64_trans2x2 t;
        eta = secp256k1_modinv64_divsteps_62(eta, f.v[0], g.v[0], &t);
        /* Update d,e using that transition matrix. */
        secp256k1_modinv64_update_de_62(&d, &e, &t, modinfo);
        /* Update f,g using that transition matrix. */
#ifdef VERIFY
        VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(&f, &modinfo->modulus, -1) > 0); /* f > -modulus */
        VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(&f, &modinfo->modulus, 1) <= 0); /* f <= modulus */
        VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(&g, &modinfo->modulus, -1) > 0); /* g > -modulus */
        VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(&g, &modinfo->modulus, 1) < 0);  /* g <  modulus */
#endif
        secp256k1_modinv64_update_fg_62(&f, &g, &t);
#ifdef VERIFY
        VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(&f, &modinfo->modulus, -1) > 0); /* f > -modulus */
        VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(&f, &modinfo->modulus, 1) <= 0); /* f <= modulus */
        VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(&g, &modinfo->modulus, -1) > 0); /* g > -modulus */
        VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(&g, &modinfo->modulus, 1) < 0);  /* g <  modulus */
#endif
    }

    /* At this point sufficient iterations have been performed that g must have reached 0
     * and (if g was not originally 0) f must now equal +/- GCD of the initial f, g
     * values i.e. +/- 1, and d now contains +/- the modular inverse. */
#ifdef VERIFY
    /* g == 0 */
    VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(&g, &SECP256K1_SIGNED62_ONE, 0) == 0);
    /* |f| == 1, or (x == 0 and d == 0 and |f|=modulus) */
    VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(&f, &SECP256K1_SIGNED62_ONE, -1) == 0 ||
                 secp256k1_modinv64_mul_cmp_62(&f, &SECP256K1_SIGNED62_ONE, 1) == 0 ||
                 (secp256k1_modinv64_mul_cmp_62(x, &SECP256K1_SIGNED62_ONE, 0) == 0 &&
                  secp256k1_modinv64_mul_cmp_62(&d, &SECP256K1_SIGNED62_ONE, 0) == 0 &&
                  (secp256k1_modinv64_mul_cmp_62(&f, &modinfo->modulus, 1) == 0 ||
                   secp256k1_modinv64_mul_cmp_62(&f, &modinfo->modulus, -1) == 0)));
#endif

    /* Optionally negate d, normalize to [0,modulus), and return it. */
    secp256k1_modinv64_normalize_62(&d, f.v[4], modinfo);
    *x = d;
}

/* Compute the inverse of x modulo modinfo->modulus, and replace x with it (variable time). */
static void secp256k1_modinv64_var(secp256k1_modinv64_signed62 *x, const secp256k1_modinv64_modinfo *modinfo) {
    /* Start with d=0, e=1, f=modulus, g=x, eta=-1. */
    secp256k1_modinv64_signed62 d = {{0, 0, 0, 0, 0}};
    secp256k1_modinv64_signed62 e = {{1, 0, 0, 0, 0}};
    secp256k1_modinv64_signed62 f = modinfo->modulus;
    secp256k1_modinv64_signed62 g = *x;
    int j;
#ifdef VERIFY
    int i = 0;
#endif
    int64_t eta = -1;
    int64_t cond;

    /* Do iterations of 62 divsteps each until g=0. */
    while (1) {
        /* Compute transition matrix and new eta after 62 divsteps. */
        secp256k1_modinv64_trans2x2 t;
        eta = secp256k1_modinv64_divsteps_62_var(eta, f.v[0], g.v[0], &t);
        /* Update d,e using that transition matrix. */
        secp256k1_modinv64_update_de_62(&d, &e, &t, modinfo);
        /* Update f,g using that transition matrix. */
#ifdef VERIFY
        VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(&f, &modinfo->modulus, -1) > 0); /* f > -modulus */
        VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(&f, &modinfo->modulus, 1) <= 0); /* f <= modulus */
        VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(&g, &modinfo->modulus, -1) > 0); /* g > -modulus */
        VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(&g, &modinfo->modulus, 1) < 0);  /* g <  modulus */
#endif
        secp256k1_modinv64_update_fg_62(&f, &g, &t);
        /* If the bottom limb of g is zero, there is a chance that g=0. */
        if (g.v[0] == 0) {
            cond = 0;
            /* Check if the other limbs are also 0. */
            for (j = 1; j < 5; ++j) {
                cond |= g.v[j];
            }
            /* If so, we're done. */
            if (cond == 0) break;
        }
#ifdef VERIFY
        VERIFY_CHECK(++i < 12); /* We should never need more than 12*62 = 744 divsteps */
        VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(&f, &modinfo->modulus, -1) > 0); /* f > -modulus */
        VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(&f, &modinfo->modulus, 1) <= 0); /* f <= modulus */
        VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(&g, &modinfo->modulus, -1) > 0); /* g > -modulus */
        VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(&g, &modinfo->modulus, 1) < 0);  /* g <  modulus */
#endif
    }

    /* At this point g is 0 and (if g was not originally 0) f must now equal +/- GCD of
     * the initial f, g values i.e. +/- 1, and d now contains +/- the modular inverse. */
#ifdef VERIFY
    /* g == 0 */
    VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(&g, &SECP256K1_SIGNED62_ONE, 0) == 0);
    /* |f| == 1, or (x == 0 and d == 0 and |f|=modulus) */
    VERIFY_CHECK(secp256k1_modinv64_mul_cmp_62(&f, &SECP256K1_SIGNED62_ONE, -1) == 0 ||
                 secp256k1_modinv64_mul_cmp_62(&f, &SECP256K1_SIGNED62_ONE, 1) == 0 ||
                 (secp256k1_modinv64_mul_cmp_62(x, &SECP256K1_SIGNED62_ONE, 0) == 0 &&
                  secp256k1_modinv64_mul_cmp_62(&d, &SECP256K1_SIGNED62_ONE, 0) == 0 &&
                  (secp256k1_modinv64_mul_cmp_62(&f, &modinfo->modulus, 1) == 0 ||
                   secp256k1_modinv64_mul_cmp_62(&f, &modinfo->modulus, -1) == 0)));
#endif

    /* Optionally negate d, normalize to [0,modulus), and return it. */
    secp256k1_modinv64_normalize_62(&d, f.v[4], modinfo);
    *x = d;
}

#endif /* SECP256K1_MODINV64_IMPL_H */

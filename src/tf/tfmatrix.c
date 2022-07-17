/* -*- mode: C; c-basic-offset: 4 -*- */
/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*
 * Copyright (c) 2014, Georgia Tech Research Corporation
 * All rights reserved.
 *
 * Author(s): Neil T. Dantam <ntd@gatech.edu>
 * Georgia Tech Humanoid Robotics Lab
 * Under Direction of Prof. Mike Stilman <mstilman@cc.gatech.edu>
 *
 *
 * This file is provided under the following "BSD-style" License:
 *
 *
 *   Redistribution and use in source and binary forms, with or
 *   without modification, are permitted provided that the following
 *   conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 *   CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *   INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 *   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 *   USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 *   AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *   POSSIBILITY OF SUCH DAMAGE.
 *
 */

#define _GNU_SOURCE
#include "amino.h"
#include "amino_internal.h"
#include <float.h>


AA_API void
aa_tf_9( const double R[AA_RESTRICT 9],
         const double p0[AA_RESTRICT 3],
         double p1[AA_RESTRICT 4] )
{
    aa_tf_rotmat_rot( R, p0, p1 );
}

AA_API void
aa_tf_9mul( const double R0[AA_RESTRICT 9],
            const double R1[AA_RESTRICT 9],
            double R[AA_RESTRICT 9] )
{
    aa_tf_rotmat_mul(R0,R1,R);
}

AA_API void
aa_tf_93( const double R[AA_RESTRICT 9],
          const double v[AA_RESTRICT 3],
          const double p0[AA_RESTRICT 3],
          double p1[AA_RESTRICT 4] )
{
    aa_tf_tfmat2_tf(R, v, p0, p1 );
}


AA_API void
aa_tf_93chain( const double R0[AA_RESTRICT 9],
               const double v0[AA_RESTRICT 3],
               const double R1[AA_RESTRICT 9],
               const double v1[AA_RESTRICT 3],
               double R[AA_RESTRICT 9], double v[AA_RESTRICT 3] )
{
    aa_tf_tfmat2_mul( R0, v0,
                      R1, v1,
                      R, v );
}


/*************/
/* ROTATIONS */
/*************/
static void
vnormalize2( const double x[AA_RESTRICT 3],
             double x_norm[AA_RESTRICT 3] )
{
    double n = sqrt( x[0]*x[0] + x[1]*x[1] + x[2]*x[2] );
    for( size_t i = 0; i < 3; i ++ ) x_norm[i] = x[i] / n;
}


AA_API void
aa_tf_rotmat_xy( const double x_axis[AA_RESTRICT 3],
                 const double y_axis[AA_RESTRICT 3],
                 double R[AA_RESTRICT 9] )
{
    vnormalize2( x_axis, R+0 );
    vnormalize2( y_axis, R+3 );
    aa_tf_cross( R+0, R+3, R+6 );
}

AA_API void
aa_tf_rotmat_yz( const double y_axis[AA_RESTRICT 3],
                 const double z_axis[AA_RESTRICT 3],
                 double R[AA_RESTRICT 9] )
{
    vnormalize2( y_axis, R+3 );
    vnormalize2( z_axis, R+6 );
    aa_tf_cross( R+3, R+6, R+0 );

}

AA_API void
aa_tf_rotmat_zx( const double z_axis[AA_RESTRICT 3],
                 const double x_axis[AA_RESTRICT 3],
                 double R[AA_RESTRICT 9] )
{
    vnormalize2( z_axis, R+6 );
    vnormalize2( x_axis, R+0 );
    aa_tf_cross( R+6, R+0, R+3 );

}


#define RREF(R,row,col) ((R)[(col)*3 + (row)])

AA_API void
aa_tf_skew_sym( const double u[AA_RESTRICT 3],
                double R[AA_RESTRICT 9] )
{
    double x=u[0], y=u[1], z=u[2];
    RREF(R,0,0) = 0;
    RREF(R,1,0) = z;
    RREF(R,2,0) = -y;

    RREF(R,0,1) = -z;
    RREF(R,1,1) = 0;
    RREF(R,2,1) = x;

    RREF(R,0,2) = y;
    RREF(R,1,2) = -x;
    RREF(R,2,2) = 0;
}
AA_API void
aa_tf_skew_sym1( double a, const double u[AA_RESTRICT 3],
                 double R[AA_RESTRICT 9] )
{
    double au[3] = { a*u[0], a*u[1], a*u[2] };
    aa_tf_skew_sym( au, R );


}


AA_API void
aa_tf_skewsym_scal_c( const double u[AA_RESTRICT 3],
                      const double a[AA_RESTRICT 3], const double b[AA_RESTRICT 3],
                      double R[AA_RESTRICT 9] )
{
    /* Operations:
     * -----------
     * Mul: 6
     * Add: 12
     */
    double  bu[3] = { b[0]*u[0],
                      b[1]*u[1],
                      b[2]*u[2] };

    double bc[3] = { b[0]*u[1],
                     b[1]*u[2],
                     b[2]*u[0] };

    double cx = 1 - bu[2] - bu[1];
    double cy = 1 - bu[2] - bu[0];
    double cz = 1 - bu[1] - bu[0];

    double dx = a[2] + bc[0];
    double dy = a[0] + bc[1];
    double dz = a[1] + bc[2];

    double ex = bc[2] - a[1];
    double ey = bc[0] - a[2];
    double ez = bc[1] - a[0];

    RREF(R,0,0) = cx;
    RREF(R,1,0) = dx;
    RREF(R,2,0) = ex;

    RREF(R,0,1) = ey;
    RREF(R,1,1) = cy;
    RREF(R,2,1) = dy;

    RREF(R,0,2) = dz;
    RREF(R,1,2) = ez;
    RREF(R,2,2) = cz;
}


AA_API void
aa_tf_skewsym_scal2( double a, double b, const double u[AA_RESTRICT 3],
                     double R[AA_RESTRICT 9] )
{

    /* Operations:
     * -----------
     * Mul: 12
     * Add: 12
     */
    double au[3] = {a*u[0], a*u[1], a*u[2]};
    double bu[3] = {b*u[0], b*u[1], b*u[2]};
    aa_tf_skewsym_scal_c( u, au, bu, R );
}

AA_API void
aa_tf_unskewsym_scal( double c, const double R[AA_RESTRICT 9], double u[AA_RESTRICT 3] )
{
    /* Operations:
     * -----------
     * Mul: 3
     * Add: 3
     */
    double a[3] = {RREF(R,2,1), RREF(R,0,2), RREF(R,1,0)};
    double b[3] = {RREF(R,1,2), RREF(R,2,0), RREF(R,0,1)};

    FOR_VEC(i) u[i] = c * ( a[i] - b[i] );
}

AA_API void
aa_tf_unskewsym( const double R[AA_RESTRICT 9], double u[AA_RESTRICT 3] )
{
    /* Operations:
     * -----------
     * Mul: 4
     * Add: 6
     * Other: sqrt
     */
    double tr = RREF(R,0,0) + RREF(R,1,1) + RREF(R,2,2);
    double c = sqrt( tr + 1 ) / 2;
    aa_tf_unskewsym_scal( c, R, u );
}

AA_API void
aa_tf_rotmat_exp_aa( const double aa[AA_RESTRICT 4], double E[9] )
{
    double s = sin(aa[W]), c = cos(aa[W]);
    aa_tf_skewsym_scal2( s, 1-c, aa, E );
}

AA_API void
aa_tf_rotmat_expv( const double rv[AA_RESTRICT 4], double E[9] )
{
    /* Operations:
     * -----------
     * Mul: 17
     * Add: 15
     * Other: sqrt, sincos
     */
    double theta2 = dot3(rv,rv);
    double theta = sqrt(theta2);
    double sc,cc;
    if( theta2*theta2 < DBL_EPSILON ) {
        sc = aa_tf_sinc_series2(theta2);
        cc = theta * aa_horner3( theta2, 1./2, -1./24, 1./720 );
    } else {
        double s = sin(theta), c = cos(theta);
        sc = s/theta;
        cc = (1-c) / theta2;
    }
    aa_tf_skewsym_scal2( sc, cc, rv, E );
}

AA_API void
aa_tf_rotmat_angle( const double R[AA_RESTRICT 9], double *c, double *s, double *theta )
{
    /* Operations:
     * -----------
     * Mul: 2
     * Add: 4
     * Other: sqrt, atan2
     */

    *c = (RREF(R,0,0) + RREF(R,1,1) + RREF(R,2,2) - 1) / 2;

    /* Less stable */
    /* *s = sqrt( 1 - (*c)*(*c) ); */

    /* More Stable */
    {
        double u[3];
        u[0] = RREF(R, 2, 1) - RREF(R, 1, 2);
        u[1] = RREF(R, 0, 2) - RREF(R, 2, 0);
        u[2] = RREF(R, 1, 0) - RREF(R, 0, 1);
        *s = sqrt(dot3(u, u)) / 2;
    }

    *theta = atan2(*s, *c);
}

AA_API void aa_tf_rotmat_lnv(const double R[AA_RESTRICT 9],
                             double lnv[AA_RESTRICT 3])
{
    double x =  RREF(R,0,0);
    double y =  RREF(R,1,1);
    double z =  RREF(R,2,2);
    double trace = x+y+z;

    double c = (trace-1)/2;

    if (c < -1 + AA_TF_EPSILON) {
        /* Singularity near [+/-] M_PI, when the rotation matrix is nearly
         * symmetric.
         *
         * Quaternion construction avoids this singularity at pi. */

        double q[4];
        double *q_v = q + AA_TF_QUAT_V;
        double *q_w = &q[AA_TF_QUAT_W];
        size_t u = (x < y) ?
            ( (y < z) ? 2 : 1 ) :
            ( (x < z) ? 2 : 0 );
        size_t v = (u + 1) % 3;
        size_t w = (u + 2) % 3;
        double r = sqrt( 1 + RREF(R,u,u) -
                         RREF(R,v,v) -
                         RREF(R,w,w) );
        q_v[u] = r / 2;
        double t = 0.5 / r;
        *q_w = (RREF(R,w,v) - RREF(R,v,w)) * t;
        q_v[v] = (RREF(R,u,v) + RREF(R,v,u)) * t;
        q_v[w] = (RREF(R,w,u) + RREF(R,u,w)) * t;
        aa_tf_qminimize(q);  // q_w might be negative

        double qs = sqrt(dot3(q_v, q_v));
        double qtheta = atan2(qs, *q_w);

        double a;
        if (qtheta < sqrt(sqrt(DBL_EPSILON))) {
            a = 2 * aa_tf_invsinc_series(qtheta);
        } else {
            a = 2 * qtheta / qs;
        }

        FOR_VEC(i) lnv[i] = a * q_v[i];

    } else {
        /* Not robust near theta=pi, but OK elsewhere */

        double u[3];
        u[0] = RREF(R, 2, 1) - RREF(R, 1, 2);
        u[1] = RREF(R, 0, 2) - RREF(R, 2, 0);
        u[2] = RREF(R, 1, 0) - RREF(R, 0, 1);

        double s = sqrt(dot3(u, u)) / 2;
        double theta = atan2(s, c);
        double theta2 = theta*theta;

        double isinc;
        if (theta2 * theta2 < DBL_EPSILON) {
            isinc = aa_tf_invsinc_series2(theta2);
        } else {
            isinc = theta/s;
        }

        FOR_VEC(i) lnv[i] = isinc/2 *  u[i];
    }
}

void aa_tf_rotmat2rotvec(const double R[restrict 9], double v[restrict 3])
{
    aa_tf_rotmat_lnv(R, v);

    /* Numerically Unstable */

    /* rv[0] = 0.5 * (AA_MATREF(R,3,2,1) - AA_MATREF(R,3,1,2)); */
    /* rv[1] = 0.5 * (AA_MATREF(R,3,0,2) - AA_MATREF(R,3,2,0)); */
    /* rv[2] = 0.5 * (AA_MATREF(R,3,1,0) - AA_MATREF(R,3,0,1)); */

    /* double s = aa_la_norm( 3, rv ); */
    /* double c = (aa_la_trace(3,R) - 1) / 2 ; */

    /* aa_la_scal( 3, ( (s > AA_TF_EPSILON) ? atan2(s,c)/s : 1 ), rv ); */
}

void aa_tf_rotmat2axang(const double R[restrict 9], double ra[restrict 4])
{
    double v[3];
    aa_tf_rotmat2rotvec(R,v);
    aa_tf_rotvec2axang(v,ra);
}



AA_API void
aa_tf_rotmat_vel2diff( const double R[AA_RESTRICT 9],
                       const double w[AA_RESTRICT 3],
                       double dR[AA_RESTRICT 9] )
{
    double K[9];
    aa_tf_skew_sym( w, K );
    aa_tf_9mul( K, R, dR );
}

AA_API void
aa_tf_rotmat_diff2vel( const double R[AA_RESTRICT 9],
                       const double dR[AA_RESTRICT 9],
                       double w[AA_RESTRICT 3] )
{
    double K[9], Rt[9];
    aa_la_transpose2( 3, 3, R, Rt );
    aa_tf_9mul( dR, Rt, K );
    aa_tf_unskewsym(K, w);
}

AA_API void
aa_tf_rotmat_svel( const double R0[9], const double w[3], double dt, double R1[9] )
{
    double delta[3], e[9];
    size_t i;
    for( i = 0; i < 3; i ++ ) delta[i] = w[i]*dt;
    aa_tf_rotmat_expv(delta,e);
    aa_tf_9mul(e,R0,R1);
}

AA_API void
aa_tf_tfmat_svel( const double T0[12], const double dx[6], double dt, double T1[12] )
{
    double delta[6];
    size_t i;

    for( i = 0; i < 6; i ++ )
        delta[i] = dx[i] * dt;

    {
        double cross[3];
        aa_tf_cross( delta + AA_TF_DX_W, T0+AA_TF_TFMAT_V, cross );
        for( i = 0; i < 3; i ++ )
            (delta+AA_TF_DX_V)[i] -= cross[i];
    }

    {
        double E[12];
        aa_tf_tfmat_expv( delta, E );
        aa_tf_12chain( E, T0, T1 );
    }
}

/*************/
/* INVERSE   */
/*************/
AA_API void aa_tf_rotmat_inv1( double R[AA_RESTRICT 9] )
{
    aa_la_transpose( 3, R);
}

AA_API void aa_tf_rotmat_inv2( const double R[AA_RESTRICT 9],
                               double Ri[AA_RESTRICT 9] )
{

    aa_la_transpose2( 3, 3, R, Ri);
}

AA_API void aa_tf_tfmat_inv1( double T[AA_RESTRICT 12] )
{
    aa_tf_rotmat_inv1(T);
    double *v = T + AA_TF_TFMAT_V;
    double vv[3] = {-v[0],-v[1],-v[2]};
    aa_tf_9rot( T, vv, v );
}

AA_API void aa_tf_tfmat_inv2( const double T[AA_RESTRICT 12],
                              double Ti[AA_RESTRICT 12] )
{
    aa_tf_rotmat_inv2(T,Ti);
    const double *v = T + AA_TF_TFMAT_V;
    double vv[3] = {-v[0],-v[1],-v[2]};
    aa_tf_9rot( Ti, vv, Ti + AA_TF_TFMAT_V );
}


/* Transform */

AA_API void
aa_tf_tfmat_diff2vel( const double T[12], const double dT[12], double dx[6] )
{
    aa_tf_rotmat_diff2vel( T+AA_TF_TFMAT_R, dT+AA_TF_TFMAT_R, dx+AA_TF_DX_W );
    AA_MEM_CPY( dx+AA_TF_DX_V,  dT+AA_TF_TFMAT_V, 3 );
}

AA_API void
aa_tf_tfmat_vel2diff( const double T[12], const double dx[6], double dT[12] )
{
    aa_tf_rotmat_vel2diff( T+AA_TF_TFMAT_R, dx+AA_TF_DX_W, dT+AA_TF_TFMAT_R );
    AA_MEM_CPY( dT+AA_TF_TFMAT_V, dx+AA_TF_DX_V, 3 );
}


AA_API void aa_tf_rotmat_rot( const double R[AA_RESTRICT 9],
                              const double p0[AA_RESTRICT 3],
                              double p1[AA_RESTRICT 3] )
{
    p1[0] =  R[0]*p0[0] + R[3]*p0[1] + R[6]*p0[2];
    p1[1] =  R[1]*p0[0] + R[4]*p0[1] + R[7]*p0[2];
    p1[2] =  R[2]*p0[0] + R[5]*p0[1] + R[8]*p0[2];
}

AA_API void aa_tf_tfmat_tf( const double T[AA_RESTRICT 12],
                            const double p0[AA_RESTRICT 3],
                            double p1[AA_RESTRICT 3] )
{
    aa_tf_tfmat2_tf( T,T+9, p0, p1 );
}

AA_API void aa_tf_12( const double T[AA_RESTRICT 12],
                      const double p0[AA_RESTRICT 3],
                      double p1[AA_RESTRICT 3] )
{
    aa_tf_tfmat2_tf( T,T+9, p0, p1 );
}

AA_API void aa_tf_tfmat2_tf( const double R[AA_RESTRICT 9],
                             const double v[AA_RESTRICT 3],
                             const double p0[AA_RESTRICT 3],
                             double p1[AA_RESTRICT 4] )
{
    /*
     *  FMA: 6
     */

    p1[0] = v[0] + R[0]*p0[0] + R[3]*p0[1] + R[6]*p0[2];
    p1[1] = v[1] + R[1]*p0[0] + R[4]*p0[1] + R[7]*p0[2];
    p1[2] = v[2] + R[2]*p0[0] + R[5]*p0[1] + R[8]*p0[2];


    /* dgemv is slower than direct arithmetic */
    /* p1[0] = v[0]; */
    /* p1[1] = v[1]; */
    /* p1[2] = v[2]; */
    /* cblas_dgemv( CblasColMajor, CblasNoTrans, */
    /*              3, 3, */
    /*              1.0, R, 3, p0, 1, */
    /*              1, p1, 1 ); */
}


/* Multiply */
AA_API void aa_tf_rotmat_mul( const double R1[AA_RESTRICT 9],
                              const double R2[AA_RESTRICT 9],
                              double R3[AA_RESTRICT 9] )
{
    /* aa_tf_rotmat_rot( R1, R2, R3 ); */
    /* aa_tf_rotmat_rot( R1, R2+3, R3+3 ); */
    /* aa_tf_rotmat_rot( R1, R2+6, R3+6 ); */

    /*  Mul: 9
     *  FMA: 18
     */

    const double *p0 = R2;

    R3[0] =  R1[0]*p0[0] + R1[3]*p0[1] + R1[6]*p0[2];
    R3[1] =  R1[1]*p0[0] + R1[4]*p0[1] + R1[7]*p0[2];
    R3[2] =  R1[2]*p0[0] + R1[5]*p0[1] + R1[8]*p0[2];

    p0 = R2+3;
    R3[3] =  R1[0]*p0[0] + R1[3]*p0[1] + R1[6]*p0[2];
    R3[4] =  R1[1]*p0[0] + R1[4]*p0[1] + R1[7]*p0[2];
    R3[5] =  R1[2]*p0[0] + R1[5]*p0[1] + R1[8]*p0[2];

    p0 = R2+6;
    R3[6] =  R1[0]*p0[0] + R1[3]*p0[1] + R1[6]*p0[2];
    R3[7] =  R1[1]*p0[0] + R1[4]*p0[1] + R1[7]*p0[2];
    R3[8] =  R1[2]*p0[0] + R1[5]*p0[1] + R1[8]*p0[2];


    /* dgemm is slower than direct arithmetic */

    /* cblas_dgemm( CblasColMajor, CblasNoTrans, CblasNoTrans, */
    /*              3, 3, 3, */
    /*              1.0, R1, 3, R2, 3, */
    /*              0, R3, 3 ); */

}

AA_API void aa_tf_tfmat_mul( const double T0[AA_RESTRICT 12],
                             const double T1[AA_RESTRICT 12],
                             double T[AA_RESTRICT 12] )
{
    aa_tf_tfmat2_mul( T0, T0+9,
                      T1, T1+9,
                      T,  T+9 );
}

AA_API void
aa_tf_12chain( const double T0[AA_RESTRICT 12],
               const double T1[AA_RESTRICT 12],
               double T2[AA_RESTRICT 12] )
{
    aa_tf_tfmat_mul( T0, T1, T2 );
}

AA_API void aa_tf_tfmat2_mul( const double R0[AA_RESTRICT 9],
                              const double v0[AA_RESTRICT 3],
                              const double R1[AA_RESTRICT 9],
                              const double v1[AA_RESTRICT 3],
                              double R[AA_RESTRICT 9], double v[AA_RESTRICT 3] )
{
    aa_tf_rotmat_mul(R0,R1,R);
    aa_tf_tfmat2_tf( R0, v0, v1, v );
}


/* Inverting Multiply */

/* AA_API void aa_tf_rotmat_imul( const double R0[AA_RESTRICT 9], */
/*                                const double R1[AA_RESTRICT 9], */
/*                                double R[AA_RESTRICT 9] ); */

/* AA_API void aa_tf_rotmat_muli( const double R0[AA_RESTRICT 9], */
/*                                const double R1[AA_RESTRICT 9], */
/*                                double R[AA_RESTRICT 9] ); */

/* AA_API void aa_tf_tfmat_imul( const double T0[AA_RESTRICT 12], */
/*                               const double T1[AA_RESTRICT 12], */
/*                               double T[AA_RESTRICT 12] ); */

/* AA_API void aa_tf_tfmat_muli( const double T0[AA_RESTRICT 12], */
/*                               const double T1[AA_RESTRICT 12], */
/*                               double T[AA_RESTRICT 12] ); */

/* AA_API void aa_tf_tfmat2_imul( const double R0[AA_RESTRICT 9], */
/*                                const double v0[AA_RESTRICT 3], */
/*                                const double R1[AA_RESTRICT 9], */
/*                                const double v1[AA_RESTRICT 3], */
/*                                double R[AA_RESTRICT 9], double v[AA_RESTRICT 3] ); */

/* AA_API void aa_tf_tfmat2_muli( const double R0[AA_RESTRICT 9], */
/*                                const double v0[AA_RESTRICT 3], */
/*                                const double R1[AA_RESTRICT 9], */
/*                                const double v1[AA_RESTRICT 3], */
/*                                double R[AA_RESTRICT 9], double v[AA_RESTRICT 3] ); */

AA_API void
aa_tf_tfmat_expv( const double v[AA_RESTRICT 6],
                  double T[AA_RESTRICT 12] )
{
    /* Operations:
     * -----------
     * (self) Mul: 6
     * (self) Add: 4
     *
     * (other) Mul: 2*12 + 9
     * (other) Add: 2*12 + 6
     *
     * (total) Mul: 39
     * (total) Add: 34
     * (total) other: sqrt, sincos
     */

    double theta2 = dot3(v+3, v+3);
    double theta = sqrt(theta2);
    double sc, cc, ssc;
    if( theta2*theta2 < DBL_EPSILON ) {
        sc = aa_tf_sinc_series2(theta2);
        cc = theta * aa_horner3( theta2, 1./2, -1./24, 1./720 );
        ssc = aa_horner3( theta2, 1./6, -1./120, 1./5040 );
    } else {
        double s = sin(theta), c = cos(theta);
        sc = s/theta;
        cc = (1-c)/theta2;
        ssc = (1-sc)/theta2;
    }

    double K[9];
    aa_tf_skewsym_scal2( sc, cc, v+3, T );
    aa_tf_skewsym_scal2( cc, ssc, v+3, K );
    aa_tf_9(K,v, T+9);
}

AA_API void
aa_tf_tfmat_lnv( const double T[AA_RESTRICT 12],
                 double v[AA_RESTRICT 6] )
{
    /* Operations:
     * -----------
     * (self) Mul: 5
     * (self) Add: 4
     *
     * (other) Mul: 2 + 3 + 12 + 9
     * (other) Add: 4 + 3 + 12 + 6
     * (other) other: sqrt, atan2
     *
     * (total) Mul: 31
     * (total) Add: 32
     * (total) other: sqrt, atant2
     */
    double c,s,theta, a,b;
    aa_tf_rotmat_angle( T, &c, &s, &theta );
    double theta2 = theta*theta;
    if( theta2 < DBL_EPSILON ) {
        a = aa_tf_invsinc_series2(theta2);
        b = aa_horner3( theta2, 1./12, 1./720, 1./30240 );
    } else {
        a = theta/s;
        b = (2*s - theta*(1+c)) / (2*theta2*s);
    }
    double K[9];
    aa_tf_unskewsym_scal( a/2, T, v+3 );
    aa_tf_skewsym_scal2( -.5, b, v+3, K );
    aa_tf_9( K, T+9, v );
}


AA_API void
aa_tf_rotmat_normalize( double R[AA_RESTRICT 9] )
{
    double *a = R;
    double *b = a + 3;
    double *c = b + 3;
    /* Operations:
     * -----------
     * Mul: 3*(6+3) = 27
     * Add: 3*(3+2)
     * Other:
     */

    aa_tf_cross( a, b, c );
    aa_tf_vnormalize(c);

    aa_tf_cross( c, a, b );
    aa_tf_vnormalize(b);

    aa_tf_cross( b, c, a );
    aa_tf_vnormalize(a);
}

AA_API void
aa_tf_tfmat_normalize( double T[AA_RESTRICT 12] )
{
    aa_tf_rotmat_normalize(T);
}

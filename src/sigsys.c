/* -*- mode: C; c-basic-offset: 4 -*- */
/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*
 * Copyright (c) 2011, Georgia Tech Research Corporation
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



#include "amino.h"
#include "amino/ct/traj.h"


struct path_cx {
    const struct aa_rx_ik_parm *opts;
    const struct aa_rx_sg_sub *ssg;
    const struct aa_rx_sg *sg;
    size_t n_q_all;
    size_t n_q_sub;
    size_t n_f_all;


    /* initial state */
    const double *q_start_all;
    const double *TF_rel0;
    const double *TF_abs0;

    /* work area */
    struct aa_ct_state *state;
    double *J;
    double *TF_rel;
    double *TF_abs;
    double *q_all;

    struct aa_ct_seg_list *segs;
    struct aa_mem_region *region;


};


void aa_lsim_dstep( size_t m, size_t n,
                    const double *restrict A,
                    const double *restrict B,
                    const double *restrict x0, const double *restrict u,
                    double *restrict x1 ) {
    /*-- x1 := A*x0 + B*u -- */
    // 0:  x1 := A*x0
    cblas_dgemv( CblasColMajor, CblasNoTrans,
                 (int)m, (int)m,
                 1.0, A, (int)m,
                 x0, 1,
                 0, x1, 1 );

    // 1:  x1 += B*u
    cblas_dgemv( CblasColMajor, CblasNoTrans,
                 (int)m, (int)n,
                 1.0, B, (int)m,
                 u, 1,
                 1, x1, 1 );
}

void aa_lsim_estep( size_t m, size_t n,
                    double dt,
                    const double *restrict A,
                    const double *restrict B,
                    const double *restrict x0,
                    const double *restrict u,
                    double *restrict x1 ) {
    // dx := A*x_0 + B*u
    aa_lsim_dstep( m, n, A, B, x0, u, x1 );

    // \delta x := dx*dt
    cblas_dscal( (int)m, dt, x1, 1 );

    // x_1 := \delta x + x_0
    cblas_daxpy( (int)m, 1.0, x0, 1, x1, 1 );

    // The following method is ~20% slower on an Intel Core2Duo
    /* // x1 := x0 */
    /* cblas_dcopy( m, x0, 1, x1, 1 ); */

    /* // 0:  x1 += dt*A*x0 */
    /* cblas_dgemv( CblasColMajor, CblasNoTrans, */
    /*              (int)m, (int)m, */
    /*              dt, A, (int)m, */
    /*              x0, 1, */
    /*              1, x1, 1 ); */

    /* // 1:  x1 += dt*B*u */
    /* cblas_dgemv( CblasColMajor, CblasNoTrans, */
    /*              (int)m, (int)n, */
    /*              dt, B, (int)m, */
    /*              u, 1, */
    /*              1, x1, 1 ); */

}

struct lsim_cx {
    const size_t m;
    const size_t n;
    const double *A;
    const double *B;
    const double *u;
};

static void lsim_sys( const void *cx_, double t,
                      const double *restrict x,
                      double *dx ) {

    struct lsim_cx *cx = (struct lsim_cx*)cx_;
    (void)t;
    aa_lsim_dstep( cx->m, cx->n, cx->A, cx->B, x, cx->u, dx );
}

void aa_lsim_rk4step( size_t m, size_t n,
                      double dt,
                      const double *restrict A,
                      const double *restrict B,
                      const double *restrict x0,
                      const double *restrict u,
                      double *restrict x1 ) {
    struct lsim_cx cx = {.m=m, .n=n, .A=A, .B=B, .u=u};
    aa_odestep_rk4( m, lsim_sys, (void*)&cx,
                    0, dt, x0, x1 );
}


void aa_odestep_euler( size_t n, double dt,
                       const double *restrict dx,
                       const double *restrict x0,
                       double *restrict x1 ) {
    // x1 = x0 + dt*dx

    // Method A
    memcpy(x1,x0,sizeof(double)*n);
    cblas_daxpy( (int)n, dt, dx, 1, x1, 1 );

    /* // Method B */
    /* for( size_t i = 0; i < n; i ++ ) */
    /*     x1[i] = x0[i] + dt*dx[i]; */
}


void aa_odestep_rk1( size_t n, aa_sys_fun sys, const void *cx,
                     double t0, double dt,
                     const double *restrict x0, double *restrict x1 )
{
    double dx[n];
    sys(cx, t0, x0, dx);
    aa_odestep_euler(n,dt,dx,x0,x1);
}

/* Butcher Tableau for Heun's Method
 *
 * 0  |
 * 1  | 1
 * ---+---------
 *    | 1/2  1/2
 */
void aa_odestep_rk2( size_t n, aa_sys_fun sys, const void *cx,
                     double t0, double dt,
                     const double *restrict x0, double *restrict x1 ) {
    double k[2][n];

    // k1
    sys( cx, t0, x0, k[0] );

    // k2
    aa_odestep_euler( n, dt, k[0], x0, x1 );
    sys( cx, t0+dt, x1, k[1] );

    // output
    // Method A
    static const double v[] =  {1.0/2, 1.0/2};
    memcpy(x1,x0,sizeof(double)*n);
    cblas_dgemv( CblasColMajor, CblasNoTrans,
                 (int)n, (int)2,
                 dt, k[0], (int)n,
                 v , 1,
                 1.0, x1, 1 );

    /* // Method B */
    /* for( size_t i = 0; i < n; i ++ ) { */
    /*     x1[i] = x0[i] + dt/2 * (k[0][i] + k[1][i]); */
    /* } */
}

/* Butcher Tableau for Runge-Kutta 4
 *
 * 0   |
 * 1/2 | 1/2
 * 1/2 | 0    1/2
 * 1   | 0    0     1
 * ----+---------------------
 *     | 1/6  1/3   1/3  1/6
*/
void aa_odestep_rk4( size_t n, aa_sys_fun sys, const void *cx,
                     double t0, double dt,
                     const double *restrict x0, double *restrict x1 ) {
    // Method A
    double k[4][n];

    // k0
    sys( cx, t0, x0, k[0] );

    // k1
    aa_odestep_euler( n, dt/2, k[0], x0, x1 );
    sys( cx, t0+dt/2, x1, k[1] );

    // k2
    aa_odestep_euler( n, dt/2, k[1], x0, x1 );
    sys( cx, t0+dt/2, x1, k[2] );

    // k3
    aa_odestep_euler( n, dt, k[2], x0, x1 );
    sys( cx, t0+dt, x1, k[3] );

    // output
    static const double v[] = {1.0/6, 1.0/3, 1.0/3, 1.0/6};
    memcpy(x1,x0,sizeof(double)*n);
    cblas_dgemv( CblasColMajor, CblasNoTrans,
                 (int)n, (int)4,
                 dt, k[0], (int)n,
                 v, 1,
                 1.0, x1, 1 );

    /* // Method A.1 */
    /* for( size_t i = 0; i < n; i ++ ) { */
    /*     x1[i] = x0[i] + (dt/6) * (k[0][i] + k[3][i] + 2 * (k[1][i] + k[2][i])); */
    /* } */


    /* // Method B */
    /* double k[n], xx[n]; */

    /* // x1 := x0 */
    /* memcpy(x1,x0,sizeof(xx)); */

    /* // k1 */
    /* sys( cx, t0, x0, k ); */
    /* cblas_daxpy( (int)n, dt/6, k, 1, x1, 1 ); */

    /* // k2 */
    /* aa_odestep_rk1( n, dt/2, k, x0, xx ); */
    /* sys( cx, t0+dt/2, xx, k ); */
    /* cblas_daxpy( (int)n, dt/3, k, 1, x1, 1 ); */

    /* // k3 */
    /* aa_odestep_rk1( n, dt/2, k, x0, xx ); */
    /* sys( cx, t0+dt/2, xx, k ); */
    /* cblas_daxpy( (int)n, dt/3, k, 1, x1, 1 ); */

    /* // k4 */
    /* aa_odestep_rk1( n, dt, k, x0, xx ); */
    /* sys( cx, t0+dt, xx, k ); */
    /* cblas_daxpy( (int)n, dt/6, k, 1, x1, 1 ); */

}



// compute row of the butcher tableau
static void butcher( size_t n, aa_sys_fun sys, const void *cx,
                     double t0, double dt,
                     const double *AA_RESTRICT x0,
                     double c, size_t p,  const double *a,
                     double *k, double *dx, double *x1 ) {

    // x1 := K*a + x0
    memcpy(x1, x0, sizeof(double)*n);
    cblas_dgemv( CblasColMajor, CblasNoTrans,
                 (int)n, (int)p,
                 dt, k, (int)n,
                 a, 1,
                 1, x1, 1 );
    // dx := f(t+dt*c, x1)
    sys( cx, t0+dt*c, x1, dx );
}


/* Butcher Tableau for Runge-Kutta-Fehlberg
 *
 *        | 0
 * 1/4    | 1/4
 * 3/8    | 3/32        9/32
 * 12/13  | 1932/2197   -7200/2197  7296/2197
 * 1      | 439/216     -8          3680/513    -845/4104
 * 1/2    | -8/27       2          -3544/2565    1859/4104   -11/40
 * -------+---------------------------------------------------------------
 *        | 25/216      0           1408/2565    2197/4104    -1/5     0
 *        | 16/135      0           6656/12825  28561/56430   -9/50    2/55
 *
 */
void aa_odestep_rkf45( size_t n, aa_sys_fun sys, const void *cx,
                       double t0, double dt,
                       const double *restrict x0,
                       double *restrict k,
                       double *restrict x4,
                       double *restrict x5 ) {


    static const double a[5][5] = {
        { 1.0/4 },
        { 3.0/32,       9.0/32},
        { 1932.0/2197,  -7200.0/2197, 7296.0/2197},
        { 439.0/216,    -8,         3680.0/513,   -845.0/4104},
        { -8.0/27,      2,         -3544.0/2565,   1859.0/4104,  -11.0/40}};

    static const double c[] = {1.0/4, 3.0/8, 12.0/13, 1.0, 1.0/2};

    // k0 already has dx at x0

    // k1
    memcpy(x4, x0, sizeof(double)*n);
    cblas_daxpy( (int)n, a[0][0]*dt, k, 1, x4, 1 );
    sys( cx, t0+dt*c[0], x4, k+n );

    // k2-k4
    for( size_t i = 1; i < 4; i ++ ) {
        butcher( n, sys, cx,
                 t0, dt, x0,
                 c[i], i+1, a[i],
                 k, k+n*(i+1), x4);
    }

    // k5, exploit the zeros in the butcher tableau
    butcher( n, sys, cx,
             t0, dt, x0,
             c[4], 4+1, a[4],
             k, k+n, x4 );

    // output
    memcpy(x4,x0,sizeof(double)*n);
    memcpy(x5,x0,sizeof(double)*n);
    static const double v1[] = {25.0/216, 0, 1408.0/2565, 2197.0/4104, -1.0/5};
    cblas_dgemv( CblasColMajor, CblasNoTrans,
                 (int)n, (int)5,
                 dt, k, (int)n,
                 v1, 1,
                 1, x4, 1 );
    static const double v2[] = {16.0/135, 2.0/55, 6656.0/12825, 28561.0/56430, -9.0/50};
    cblas_dgemv( CblasColMajor, CblasNoTrans,
                 (int)n, (int)5,
                 dt, k, (int)n,
                 v2, 1,
                 1, x5, 1 );

}


/* Butcher Tableau for Cash-Karp
 *
 * 0     |
 * 1/5   | 1/5
 * 3/10  | 3/40        9/40
 * 3/5   | 3/10        -9/10    6/5
 * 1     |-11/54       5/2      -70/27      35/27
 * 7/8   | 1631/55296  175/512  575/13824   44275/110592    253/4096
 * ------+-----------------------------------------------------------------
 *       | 37/378      0        250/621     125/594         0          512/1771
 *       | 2825/27648  0        18575/48384 13525/55296     277/14336  1/4
*/
void aa_odestep_rkck45( size_t n, aa_sys_fun sys, const void *cx,
                        double t0, double dt,
                        const double *restrict x0,
                        double *restrict k,
                        double *restrict x4,
                        double *restrict x5 ) {
    static const double a[5][5] = {
        {1.0/5},
        {3.0/40,        9.0/40},
        {3.0/10,        -9.0/10,    6.0/5},
        {-11.0/54,       5.0/2,     -70.0/27,     35.0/27},
        {1631.0/55296,  175.0/512,  575.0/13824,  44275.0/110592,    253.0/4096}};

    static const double c[] = {1.0/5, 3.0/10, 3.0/5, 1.0, 7.0/8};

    // k0 already has dx at x0

    // k1
    memcpy(x4, x0, sizeof(double)*n);
    cblas_daxpy( (int)n, a[0][0]*dt, k, 1, x4, 1 );
    sys( cx, t0+dt*c[0], x4, k+n );

    // k2-k4
    for( size_t i = 1; i < 4; i ++ ) {
        butcher( n, sys, cx,
                 t0, dt, x0,
                 c[i], i+1, a[i],
                 k, k + n*(i+1), x4);
    }

    // k5, exploit the zeros in the butcher tableau
    butcher( n, sys, cx,
             t0, dt, x0,
             c[4], 4+1, a[4],
             k, k+n, x4 );

    // output
    memcpy(x4,x0,sizeof(double)*n);
    memcpy(x5,x0,sizeof(double)*n);
    static const double v1[] = {37.0/378, 512.0/1771, 250.0/621, 125.0/594};
    cblas_dgemv( CblasColMajor, CblasNoTrans,
                 (int)n, (int)4,
                 dt, k, (int)n,
                 v1, 1,
                 1, x4, 1 );
    static const double v2[] = { 2825.0/27648, 277.0/14336, 18575.0/48384, 13525.0/55296, 1.0/4};
    cblas_dgemv( CblasColMajor, CblasNoTrans,
                 (int)n, (int)5,
                 dt, k, (int)n,
                 v2, 1,
                 1, x5, 1 );

}

/* Butcher Tableau for Dormand-Prince
 *
 * 0          |
 * 1/5        | 1/5
 * 3/10       | 3/40        9/40
 * 4/5        | 44/45       -56/15      32/9
 * 8/9        | 19372/6561  -25360/2187 64448/6561    -212/729
 * 1          | 9017/3168   -355/33     46732/5247    49/176    -5103/18656
 * 1          | 35/384      0           500/1113      125/192   -2187/6784      11/84
 * -----------+--------------------------------------------------------------------------
 *            | 35/384      0           500/1113      125/192   -2187/6784      11/84      0
 *            | 5179/57600  0           7571/16695    393/640   -92097/339200   187/2100   1/40
 *
 */
void aa_odestep_dorpri45( size_t n, aa_sys_fun sys, const void *cx,
                          double t0, double dt,
                          const double *restrict x0,
                          double *restrict k,
                          double *restrict x4,
                          double *restrict x5 ) {

    // a is slightly reordered from actual butcher tableau to exploit the zeros
    static const double a[6][5] = {
        {1.0/5},
        {3.0/40,        9.0/40},
        {44.0/45,       -56.0/15,       32.0/9},
        {19372.0/6561,  -25360.0/2187,  64448.0/6561,  -212.0/729},
        {9017.0/3168,   -355.0/33,      46732.0/5247,  49.0/176,   -5103.0/18656},
        {35.0/384,       11.0/84.0,     500.0/1113,    125.0/192,  -2187.0/6784}
    };

    static const double c[] = {1.0/5, 3.0/10, 4.0/5, 8/9, 1, 1};

    // k0 already has dx at x0

    // k1
    memcpy(x4, x0, sizeof(double)*n);
    cblas_daxpy( (int)n, a[0][0]*dt, k, 1, x4, 1 );
    sys( cx, t0+dt*c[0], x4, k + n );

    // k2-k4
    for( size_t i = 1; i < 4; i ++ ) {
        butcher( n, sys, cx,
                 t0, dt, x0,
                 c[i], i+1, a[i],
                 k, k + n*(i+1), x4);
    }

    // k5, exploit the zeros in the butcher tableau
    // second column of k gets k4
    butcher( n, sys, cx,
             t0, dt, x0,
             c[4], 4+1, a[4],
             k, k + n, x4 );

    // k6
    // sixth column of k gets k6
    butcher( n, sys, cx,
             t0, dt, x0,
             c[5], 5+1-1, a[5],
             k, k + 5*n, x4 );

    // output (already have x4 from last butcher step)
    memcpy(x5,x0,sizeof(double)*n);
    static const double v[] = {5179.0/57600, 187.0/2100, 7571.0/16695, 393.0/640,
                               -92097.0/339200, 1.0/40};
    cblas_dgemv( CblasColMajor, CblasNoTrans,
                 (int)n, (int)6,
                 dt, k, (int)n,
                 v, 1,
                 1, x5, 1 );

}


/* Butcher Tableau for Bogacki-Shampine
 *
 * 0    |
 * 1/2  | 1/2
 * 3/4  | 0     3/4
 * 1    | 2/9   1/3  4/9
 * -----+--------------------
 *      | 2/9   1/3  4/9  0
 *      | 7/24  1/4  1/3  1/8
 *
 */
void aa_odestep_rkbs23( size_t n, aa_sys_fun sys, const void *cx,
                        double t0, double dt,
                        const double *restrict x0,
                        double *restrict k,
                        double *restrict x2,
                        double *restrict x3 ) {
    // k0 already has dx at x0

    // k1
    memcpy(x2, x0, sizeof(double)*n);
    cblas_daxpy( (int)n, dt/2, k, 1, x2, 1 );
    sys( cx, t0+dt/2, x2, k + n );

    // k2
    memcpy(x2, x0, sizeof(double)*n);
    cblas_daxpy( (int)n, 3*dt/4, k+n, 1, x2, 1 );
    sys( cx, t0+3*dt/4, x2, k + 2*n );

    // k3
    static const double v1[] = {2.0/9, 1.0/3, 4.0/9};
    butcher( n, sys, cx,
             t0, dt, x0,
             1, 3, v1,
             k, k + 3*n, x2 );

    // output (already have x2 from last butcher step)
    memcpy(x3,x0,sizeof(double)*n);
    static const double v2[] = {7.0/24, 1.0/4, 1.0/3, 1.0/8};
    cblas_dgemv( CblasColMajor, CblasNoTrans,
                 (int)n, (int)4,
                 dt, k, (int)n,
                 v2, 1,
                 1.0, x3, 1 );
}



void aa_sys_affine( const aa_sys_affine_t *cx,
                    double t, const double *restrict x,
                    double *restrict dx ) {
    (void)t;
    // x1 = Ax + D
    memcpy( dx, cx->D, sizeof(double)*cx->n );
    cblas_dgemv( CblasColMajor, CblasNoTrans,
                 (int)cx->n, (int)cx->n,
                 1, cx->A, (int)cx->n,
                 x, 1,
                 1, dx, 1 );

}

AA_API int aa_ode_sol( enum aa_ode_integrator integrator,
                       const struct aa_ode_sol_opts * AA_RESTRICT opts,
                       size_t n,
                       aa_sys_fun sys, const void *sys_cx,
                       aa_ode_check check, void *check_cx,
                       double t0, double dt0,
                       const double *AA_RESTRICT x0,
                       double *AA_RESTRICT x1 )
{

    /* default: RK4, fixed step */
    aa_odestep_fixed *fixed_integrator = aa_odestep_rk4;
    aa_odestep_adaptive *adaptive_integrator = NULL;
    size_t n_k = 0;

    /* Select Integration Function */
    switch( integrator ) {
    case AA_ODE_RK1:
        fixed_integrator = aa_odestep_rk1;
        break;
    case AA_ODE_RK2:
        fixed_integrator = aa_odestep_rk2;
        break;
    case AA_ODE_RK4:
        fixed_integrator = aa_odestep_rk4;
        break;
    case AA_ODE_RK45_F:
        adaptive_integrator = aa_odestep_rkf45;
        n_k = 5;
        break;
    case AA_ODE_RK45_CK:
        adaptive_integrator = aa_odestep_rkck45;
        n_k = 5;
        break;
    case AA_ODE_RK45_DP:
        n_k = 6;
        adaptive_integrator = aa_odestep_dorpri45;
        break;
    case AA_ODE_RK23_BS:
        n_k = 4;
        adaptive_integrator = aa_odestep_rkbs23;
        break;
    }

    double t = t0;
    double dt = dt0;

    int check_result;

    if( adaptive_integrator ) {
        /* Adaptive Step Integration */
        double k[n_k*n]; /* intermediate derivative matrix */
        double *kn = k + (n_k-1)*n; /* Last column of k */
        double X[3*n];
        double *px0 = X;
        double *px1 = px0 + n;
        double *px2 = px1 + n;
        AA_MEM_CPY(px0, x0, n);

        /* Initialize derivative at first timestep */
        sys( sys_cx, t0, x0, k );

        for(;;) {
            for(;;) { /* adaptive loop */
                adaptive_integrator( n, sys, sys_cx,
                                     t, dt,
                                     px0, k, px1, px2 );
                double err = aa_la_ssd( n, px1, px2 );
                // adapt the step size
                if( err > opts->adapt_tol_dec ) {
                    dt *= opts->adapt_factor_dec;
                    continue;
                } else if ( err < opts->adapt_tol_inc ) {
                    dt *= opts->adapt_factor_inc;
                }
                break;
            }

            /* Check if complete */
            if( (check_result = check( check_cx, t, px2, kn)) ) {
                AA_MEM_CPY(x1, px2, n);
                break;
            } else {
                double *tmp = px0; px0 = px2; px2 = tmp;
                AA_MEM_CPY(k, kn, n); // extract dx to k0
            }
        }
    } else {
        /* Fixed Step Integration */
        double X[2*n];
        double y[n];
        double *px0 = X;
        double *px1 = X + n;
        assert( px0 + n == px1 );
        AA_MEM_CPY(px0, x0, n);
        for(;;) {
            fixed_integrator( n, sys, sys_cx,
                              t, dt,
                              px0, px1 );
            t += dt;
            /* TODO: avoid the duplicate sys() evaluation here */
            sys( sys_cx, t, px1, y );
            /* Check if complete */
            if( (check_result = check( check_cx, t, px1, y)) ) {
                AA_MEM_CPY( x1, px1, n );
                break;
            } else {
                double *tmp = px0; px0 = px1; px1 = tmp;
            }

        }
    }

    if( check_result > 0 ) {
        return 0;
    } else {
        return check_result;
    }
}


struct ode_path_cx {
    struct aa_mem_region *region;
    aa_ode_check *check;
    void *check_cx;
    size_t n;
    size_t cnt;

    struct aa_mem_rlist *list;
};


int ode_path_check( void *cx_, double t, double * AA_RESTRICT x, double *AA_RESTRICT y )
{
    struct ode_path_cx *cx = (struct ode_path_cx*)cx_;
    aa_mem_rlist_enqueue_cpy( cx->list, x, cx->n*sizeof(x[0]) );
    cx->cnt++;

    return cx->check( cx->check_cx, t, x, y );
}

AA_API int
aa_ode_path( enum aa_ode_integrator integrator,
             const struct aa_ode_sol_opts * AA_RESTRICT opts,
             size_t n,
             aa_sys_fun sys, const void *sys_cx,
             aa_ode_check check, void *check_cx,
             double t0, double dt0,
             const double *AA_RESTRICT x0,
             struct aa_mem_region *region, size_t *n_points, double **path )
{
    /* Setup context */
    struct aa_mem_region *local_region = aa_mem_region_local_get();
    struct ode_path_cx cx;
    cx.check = check;
    cx.check_cx = check_cx;
    cx.region = ( local_region == region ) ? aa_mem_region_create(4096) : local_region;
    cx.list = aa_mem_rlist_alloc( cx.region );
    cx.n = n;
    cx.cnt = 1;

    aa_mem_rlist_enqueue_cpy( cx.list, x0, n*sizeof(*x0) );

    /* Call ode solve function */
    double x1[n];
    int r = aa_ode_sol( integrator, opts, n,
                        sys, sys_cx,
                        ode_path_check, &cx,
                        t0, dt0, x0, x1 );

    /* Copy result to array */
    *n_points = cx.cnt;
    *path = AA_MEM_REGION_NEW_N(region, double, n*(*n_points));

    int i = 0;
    for( struct aa_mem_cons *c = cx.list->head; c; c = c->next ) {
        assert( i < cx.cnt);
        double *p = (*path) + i*n;
        AA_MEM_CPY(p, c->data, n);
        i++;
    }

    /* Free temp list */
    if( local_region == region ) {
        aa_mem_region_destroy(cx.region);
    } else {
        aa_mem_region_pop(cx.region, cx.list);
    }

    return r;
}

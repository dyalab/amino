/* -*- mode: C; c-basic-offset: 4 -*- */
/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*
 * Copyright (c) 2018-2019, Colorado School of Mines
 * All rights reserved.
 *
 * Author(s): Neil T. Dantam <ndantam@miens.edu>
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

// uncomment to check that local allocs actually get freed
// #define AA_ALLOC_STACK_MAX 0


#include <stdlib.h>
#include <cblas.h>

#include "amino.h"

//#define AA_MATPP_TRACE
#include "amino/mat.hpp"

#include "amino/test.h"

using namespace amino;


static inline void
admeq( const char *name, const DMat &A, const DMat &B, double tol)
{
    aafeq( name, DMat::ssd(A,B), 0, tol );
}

static inline void
adveq( const char *name, const DVec &A, const DVec &B, double tol)
{
    aafeq( name, DVec::ssd(A,B), 0, tol );
}


static void s_scal()
{
    RegionScope rs;

    const DVec A(rs, {1,2});
    DVec B(rs,2);

    {
        B = A;
        assert(2 == B.len);
        assert(2 == A.len);
        aveq( "DVec =", B.len, B.data, A.data, 0);


    }



    {
        double A2[2] = {2, 4};
        double A4[2] = {4, 8};

        B = A;
        aveq( "Dec = ", B.len, B.data, A.data, 0);
        B *= 2;
        aveq( "Dec *= double", B.len, B.data, A2, 0);

        B = 2*A;
        aveq( "Dec = dscal 0", B.len, B.data, A2, 0);

        B = (2*A)*2;
        aveq( "Dec = dscal 1", B.len, B.data, A4, 0);

        B = 2*(A*2);
        aveq( "DVec = dscal 2", B.len, B.data, A4, 0);

        B = 2*(A+A);
        aveq( "Dec = dscal 2", B.len, B.data, A4, 0);

        B = (A+A)*2;
        aveq( "Dec = dscal 2", B.len, B.data, A4, 0);

        B = 2*(A+A);
        aveq( "Dec = dscal 2", B.len, B.data, A4, 0);

        B = 2*A+(A+A);
        aveq( "Dec = dscal 2", B.len, B.data, A4, 0);

        B = (A+A)+2*A;
        aveq( "Dec = dscal 2", B.len, B.data, A4, 0);

        B = A+2*A+A;
        aveq( "Dec = dscal 2", B.len, B.data, A4, 0);

        B = A*2 + 2*A;
        aveq( "Dec = dscal 2", B.len, B.data, A4, 0);

        {
            struct aa_dvec Avv;
            aa_dvec_view(&Avv, 2,A.data,1);

            B = 2*Avv;
            aveq( "Dec *= double", B.len, B.data, A2, 0);

            //*Bv += 2*Avv;
            //aveq( "DVec = dscal 2", Bv->len, Bv->data, A4, 0);
        }
    }

}


static void s_axpy()
{
    RegionScope rs;
    DVec A(rs, {1,2});

    DVec B(rs, 2);

    {
        double R[] = {2,4};
        B = A + A;
        aveq( "DVec  axpy", B.len, B.data, R, 0);
    }

    {
        double R[] = {3,6};
        B = (A + A) + A;
        aveq( "DVec  axpy", B.len, B.data, R, 0);

        B = A + (A + A);
        aveq( "DVec  axpy", B.len, B.data, R, 0);


        B = 2*A + A;
        aveq( "DVec  axpy", B.len, B.data, R, 0);

        B = A + 2*A;
        aveq( "DVec  axpy", B.len, B.data, R, 0);
    }

}


static void s_mscal()
{
    {
        RegionScope rs;
        DMat &A = *DMat::col_mat(rs, {{1,2}, {3,4}});
        DMat &Ap = *DMat::col_mat(rs, {{1,2}, {3,4}});
        DMat &A2 = *DMat::col_mat(rs, {{2,4}, {6,8}});
        DMat &A4 = *DMat::col_mat(rs, {{4,8}, {12,16}});

        A *= 2;
        admeq( "dmat_scal-0", A, A2, 1e-6 );

        A = Ap*2;
        admeq( "dmat_scal-0", A, A2, 1e-6 );

        A = 2*Ap;
        admeq( "dmat_scal-0", A, A2, 1e-6 );


        A = 2*(Ap*2);
        admeq( "dmat_scal-0", A, A4, 1e-6 );

        A = (2*Ap)*2;
        admeq( "dmat_scal-0", A, A4, 1e-6 );

    }

    {
        double d[] = {1,2,3,0, 4,5,6,0};
        double dp[] = {1,2,3,0, 4,5,6,0};
        double d2[] = {2,4,6, 8,10,12 };
        DMat A(3,2,d,4);
        DMat Ap(3,2,dp,4);
        DMat A2(3,2,d2,3);
        A *= 2;
        admeq( "dmat_scal-1", A, A2, 1e-6 );

        A = Ap*2;
        admeq( "dmat_scal-1", A, A2, 1e-6 );

        A = 2*Ap;
        admeq( "dmat_scal-1", A, A2, 1e-6 );
    }
    {
        double d[] =  {1,2,0, 3,4,0, 5,6,0};
        double dp[] =  {1,2,0, 3,4,0, 5,6,0};
        double d2[] = {2,4, 6,8, 10,12};
        DMat A(2,3,d,3);
        DMat Ap(2,3,dp,3);
        DMat A2(2,3,d2,2);
        A *= 2;
        admeq( "dmat_scal-2", A, A2, 1e-6 );

        A = Ap*2;
        admeq( "dmat_scal-1", A, A2, 1e-6 );

        A = 2*Ap;
        admeq( "dmat_scal-1", A, A2, 1e-6 );
    }
}

static void s_transpose()
{
    double Xd[3*2 + 1] = { 1,2,3, 4,5,6, 7};

    {
        RegionScope rs;
        double Rd[] = {1,4, 2,5, 3,6};
        DMat X(3,2,Xd,3);
        DMat Y(rs,2,3);
        DMat R(2,3,Rd,2);
        Y = X.transpose();
        admeq( "dmat_trans-0", Y, R, 1e-6 );
    }

    {
        double Yd[3*3] = {0};
        double Rd[] = {1,4,0, 2,5,0, 3,6,0};
        DMat X(3,2,Xd,3);
        DMat Y(2,3,Yd,3);
        DMat R(2,3,Rd,3);
        Y = X.transpose();
        admeq( "dmat_trans-1", Y, R, 1e-6 );
    }

    {
        RegionScope rs;
        DMat X(2,2,Xd,3);
        DMat Y(rs,2,2);
        DMat &R = *DMat::col_mat(rs, {{1,4}, {2,5}});
        Y = X.transpose();
        admeq( "dmat_trans-1", Y, R, 1e-6 );
    }

}

static void s_inv()
{
    RegionScope rs;

    double Ad[] = {1,3,0, 2,4,0 };
    DMat A(2,2,Ad,3);
    DMat Ab(rs,2,2);
    DMat &Ai = *DMat::col_mat(rs, {{-2,1.5}, {1,-.5}});

    Ab = A.inverse();
    admeq( "dmat_inv", Ab, Ai, 1e-6 );

    A.invert();
    admeq( "dmat_inv", A, Ai, 1e-6 );
}

static void s_gemv()
{
    RegionScope rs;


    DMat &A = *DMat::col_mat(rs, {{1,2}, {3,4}});
    DVec x(rs, {1,2});
    DVec y1(rs, {7,10});
    DVec y2(rs, {14,20});
    DVec y4(rs, {28,40});
    DVec yt1(rs, {5,11} );
    DVec yt2(rs, {10,22});
    DVec ys(rs, {0,0});

    ys = A*x;
    adveq( "gemv 0", ys, y1, 1e-6 );

    ys = (2*A)*x;
    adveq( "gemv 1", ys, y2, 1e-6 );

    ys = A*(2*x);
    adveq( "gemv 2", ys, y2, 1e-6 );

    ys = (2*A)*(2*x);
    adveq( "gemv 3", ys, y4, 1e-6 );

    ys = (2*A*2)*(x);
    adveq( "gemv 4", ys, y4, 1e-6 );

    ys = A.t()*x;
    adveq( "gemv 5", ys, yt1, 1e-6 );

    ys = 2*(A.t()*x);
    adveq( "gemv 6", ys, yt2, 1e-6 );

    ys = (A.t()*x)*2;
    adveq( "gemv 7", ys, yt2, 1e-6 );

    ys = A.t()*(x*2);
    adveq( "gemv 8", ys, yt2, 1e-6 );

}

static void s_gemv2() {
    RegionScope rs;

    DMat &A = *DMat::col_mat(rs, {{1,2}, {3,4}});

    DVec x(rs, {1,2});
    DVec y(rs, {3,9});
    DVec yy(rs, 2);
    DVec r(rs, {10,19});

    DVec r2(rs, {13,28});
    DVec r3(rs, {17,29});
    DVec r4(rs, {20,38});

    yy = A*x + y;
    adveq( "gemv2 0", yy, r, 1e-6 );

    yy = y + A*x;
    adveq( "gemv2 1", yy, r, 1e-6 );

    yy =  A*x + (y+y);
    adveq( "gemv2 2", yy, r2, 1e-6 );

    yy = (y+y) + A*x;
    adveq( "gemv2 3", yy, r2, 1e-6 );

    yy =  A*x + 2*y;
    adveq( "gemv2 4", yy, r2, 1e-6 );

    yy = 2*y + A*x;
    adveq( "gemv2 5", yy, r2, 1e-6 );

    yy =  2*A*x + y;
    adveq( "gemv2 6", yy, r3, 1e-6 );

    yy = y + 2*A*x;
    adveq( "gemv2 7", yy, r3, 1e-6 );

    yy =  2*A*x + 2*y;
    adveq( "gemv2 8", yy, r4, 1e-6 );

    yy = 2*y + 2*A*x;
    adveq( "gemv2 9", yy, r4, 1e-6 );

    yy =  2*(A*x + y);
    adveq( "gemv2 10", yy, r4, 1e-6 );

    yy = (y + A*x)*2;
    adveq( "gemv2 11", yy, r4, 1e-6 );
}

static void s_gemm() {
    double Ad[] = {1,2, 3,4};
    double Bd[] = {5,6, 7,8};
    double ABd[] = {23,34, 31,46};
    double AtBd[] = {17,39, 23,53};
    double ABtd[] = {26,38, 30,44};
    double AtBtd[] = {19,43, 22,50};

    DMat A(2,2,Ad);
    DMat B(2,2,Bd);
    DMat AB(2,2,ABd);
    DMat AtB(2,2,AtBd);
    DMat ABt(2,2,ABtd);
    DMat AtBt(2,2,AtBtd);
    DMat &C = *DMat::alloc_local(2,2);

    C = A*B;
    admeq("gemm 0", C, AB, 1e-6);

    C = A.t()*B;
    admeq("gemm 1", C, AtB, 1e-6);

    C = A*B.t();
    admeq("gemm 2", C, ABt, 1e-6);

    C = A.t()*B.t();
    admeq("gemm 3", C, AtBt, 1e-6);

    {
        DMat &AtB2 = * DMat::alloc_local(2,2);
        DMat &AtB4 = * DMat::alloc_local(2,2);
        AtB2 = 2*AtB;
        AtB4 = 4*AtB;

        C = 2*(A.t()*B);
        admeq("gemm atb2 0", C, AtB2, 1e-6);

        C = (A.t()*B)*2;
        admeq("gemm atb2 1", C, AtB2, 1e-6);

        C = (2*A.t())*B;
        admeq("gemm atb2 2", C, AtB2, 1e-6);

        C = (A.t()*2)*B;
        admeq("gemm atb2 3", C, AtB2, 1e-6);

        C = (A.t())*(2*B);
        admeq("gemm atb2 4", C, AtB2, 1e-6);

        C = (A.t())*(B*2);
        admeq("gemm atb2 5", C, AtB2, 1e-6);

        C = (2*A.t())*(B*2);
        admeq("gemm atb4 ", C, AtB4, 1e-6);
    }

    {
        RegionScope rs;
        DMat ABt2(rs, 2, 2);
        DMat ABt4(rs, 2, 2);
        ABt2 = 2*ABt;
        ABt4 = 4*ABt;

        C = 2*(A*B.t());
        admeq("gemm abt2 0", C, ABt2, 1e-6);

        C = (A*B.t())*2;
        admeq("gemm abt2 1", C, ABt2, 1e-6);

        C = (2*A)*B.t();
        admeq("gemm abt2 2", C, ABt2, 1e-6);

        C = (A*2)*B.t();
        admeq("gemm abt2 3", C, ABt2, 1e-6);

        C = A*(2*B.t());
        admeq("gemm abt2 4", C, ABt2, 1e-6);

        C = A*(B.t()*2);
        admeq("gemm abt2 5", C, ABt2, 1e-6);

        C = (2*A)*(B.t()*2);
        admeq("gemm abt4 0", C, ABt4, 1e-6);
    }

    {
        RegionScope rs;
        DMat AtBt2(rs, 2, 2);
        DMat AtBt4(rs, 2, 2);
        AtBt2 = 2*AtBt;
        AtBt4 = 4*AtBt;

        C = 2*(A.t()*B.t());
        admeq("gemm abt2 0", C, AtBt2, 1e-6);

        C = (A.t()*B.t())*2;
        admeq("gemm abt2 1", C, AtBt2, 1e-6);

        C = (2*A.t())*B.t();
        admeq("gemm abt2 2", C, AtBt2, 1e-6);

        C = (A.t()*2)*B.t();
        admeq("gemm abt2 3", C, AtBt2, 1e-6);

        C = A.t()*(2*B.t());
        admeq("gemm abt2 4", C, AtBt2, 1e-6);

        C = A.t()*(B.t()*2);
        admeq("gemm abt2 5", C, AtBt2, 1e-6);

        C = (2*A.t())*(B.t()*2);
        admeq("gemm abt4 0", C, AtBt4, 1e-6);
    }
}


static void s_gemm2()
{
    RegionScope rs;
    DMat &A = *DMat::col_mat(rs, {{1,2}, {3,4}});
    DMat &B = *DMat::col_mat(rs, {{11,13}, {17,23}});
    DMat &C = *DMat::col_mat(rs, {{5,6}, {7,8}});
    DMat &D = *DMat::col_mat(rs, {{50,60}, {70,80}});

    DMat &ABpC = *DMat::col_mat(rs, {{55, 80}, {93, 134}});

    DMat R(rs, 2, 2);

    R = A*B + C;
    admeq("gemm 2 0", R, ABpC, 1e-6);

    R = C + A*B ;
    admeq("gemm 2 0", R, ABpC, 1e-6);

    {
        double ABpCtd[] = {55,81, 92, 134};
        DMat ABpCt(2,2,ABpCtd);


        R = A*B + C.t();
        admeq("gemm 2e 0", R, ABpCt, 1e-6);

        R = C.t() + A*B;
        admeq("gemm 2e 1", R, ABpCt, 1e-6);
    }

    {
        DMat &ABp2Ct = *DMat::col_mat(rs, {{60,88}, {98, 142}});
        R = A*B + 2*C.t();
        admeq("gemm 2e 2", R, ABp2Ct, 1e-6);

        R = C.t()*2 + A*B;
        admeq("gemm 2e 3", R, ABp2Ct, 1e-6);
    }

    {
        DMat E(rs, 2, 2);
        E = 2*ABpC;

        R = 2*A*B + 2*C; //+ C*2;
        admeq("gemm 22 0", R, E, 1e-6);

        R = 2*(A*B + C);
        admeq("gemm 22 1", R, E, 1e-6);

        R = (A*B + C)*2;
        admeq("gemm 22 2", R, E, 1e-6);
    }
}

static void s_slice()
{
    RegionScope rs;
    DMat &A = *DMat::col_mat(rs, {{1,2}, {3,4}});
    DVec vr0(rs, {1,3});
    DVec vr1(rs, {2,4});
    DVec vc0(rs, {1,2});
    DVec vc1(rs, {3,4});

    DVec rc0 = A.col_vec(0);
    adveq("slice", rc0, vc0, 0);

    DVec rc1 = A.col_vec(1);
    adveq("slice", rc1, vc1, 0);

    DVec rr0 = A.row_vec(0);
    adveq("slice", rr0, vr0, 0);

    DVec rr1 = A.row_vec(1);
    adveq("slice", rr1, vr1, 0);

}

static void s_block()
{

    {
        RegionScope rs;
        DMat &A = *DMat::col_mat(rs, {{1,2,3}, {4,5,6}});
        assert(3 == A.rows);
        assert(2 == A.cols);
        afeq( 1, A(0,0), 0);
        afeq( 2, A(1,0), 0);
        afeq( 3, A(2,0), 0);

        afeq( 4, A(0,1), 0);
        afeq( 5, A(1,1), 0);
        afeq( 6, A(2,1), 0);

        DMat &Ar = *DMat::col_mat(rs, {{1,4}, {2,5}, {3,6}});
        DMat At(rs, 2, 3);
        At = A.transpose();
        admeq("block", At, Ar, 0);

        DMat &B = *DMat::row_mat(rs, {{1,4}, {2,5}, {3,6}});
        assert(3 == B.rows);
        assert(2 == B.cols);

        afeq( 1, B(0,0), 0);
        afeq( 2, B(1,0), 0);
        afeq( 3, B(2,0), 0);

        afeq( 4, B(0,1), 0);
        afeq( 5, B(1,1), 0);
        afeq( 6, B(2,1), 0);

        DMat &Br = *DMat::row_mat(rs, {{1,2,3}, {4,5,6}});
        DMat Bt(rs, 2, 3);
        Bt = B.transpose();
        admeq("block", Bt, Br, 0);

        admeq("block", A, B, 0);
        admeq("block", At, Bt, 0);

    }

    {
        RegionScope rs;
        DMat &A = *DMat::col_mat(rs, {{1,2,3}, {4,5,6}});
        DMat &B = *DMat::col_mat(rs, {{1,2}, {4,5}});
        admeq("block", A.block(0,0, 2, 2), B, 0);
    }

    {
        RegionScope rs;
        DMat &A = *DMat::row_mat(rs,
                                 {{1,2,3},
                                  {4,5,6},
                                  {7,8,9}});
        DMat &B = *DMat::row_mat(rs, {{2,3}, {5,6}});
        admeq("block", A.block(0,1, 2, 3), B, 0);

        DMat &C = *DMat::col_mat(rs, {{4,7}, {5,8}});
        admeq("block", A.block(1,0, 3, 2), C, 0);
    }
}

void s_init() {
    RegionScope rs;
    DVec &A = *DVec::init(rs, {1,2});
    DVec &B = *DVec::init(rs, A);

    afeq(A[0], 1, 0);
    afeq(A[1], 2, 0);

    afeq(A[0], B[0], 0);
    afeq(A[1], B[1], 0);
}

int main(void)
{

    s_init();

    s_block();

    s_scal();
    s_axpy();
    s_mscal();
    s_transpose();
    s_inv();

    s_gemv();
    s_gemv2();

    s_gemm();
    s_gemm2();

    s_slice();


    printf("MATPP: OK\n");
    return 0;
}

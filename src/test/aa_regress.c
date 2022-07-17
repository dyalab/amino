/* -*- mode: C; c-basic-offset: 4 -*- */
/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*
 * Copyright (c) 2010-2013, Georgia Tech Research Corporation
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
#include "amino/test.h"

static void rotmat_axang() {
    /* Had some numerical stability issues */
    double q[4] =  {0.538444,0.141454,0.510387,0.655419};
    double R[9];
    aa_tf_quat2rotmat(q, R); // get rotation matrix

    double qr[4];
    aa_tf_rotmat2quat(R, qr); // convert to quaternion
    aveq("quat", 4, q, qr, .001);

    double vr[3], vq[3];      // get rotation vectors
    aa_tf_quat2rotvec(q, vq);
    aa_tf_rotmat2rotvec(R, vr);
    aveq("vec", 3, vq, vr, .001);

    double qvr[4], qvq[4];    // convert back to quaternions
    aa_tf_rotvec2quat( vq, qvq );
    aa_tf_rotvec2quat( vr, qvr );
    aveq("quat2", 4, qvq, qvr, .001);
}

static void slerp() {
    double q[4], q1[4], u;
    aa_tf_qurand(q);
    u = aa_frand();
    aa_tf_qslerp(u, q, q, q1);
    aveq( "slerp-equiv", 4, q1, q, 1e-8 );
    aa_tf_qslerpalg(u, q, q, q1);
    aveq( "slerpalg-equiv", 4, q1, q, 1e-8 );
}



static void rotmat( void ) {
    double R0[9] = { 0, 1, 0,
                     0, 0, 1,
                     1, 0, 0 };
    assert(aa_tf_isrotmat(R0));
}


static void rotmat_lnv_test(const double e[3]) {
    // Rotation
    {
        double ee[9], rln[3], qe[4], eln[3];
        aa_tf_rotmat_expv(e, ee);

        aa_tf_rotmat2quat(ee, qe);
        aa_tf_quat2rotvec(qe, eln);
        aa_tf_rotmat_lnv(ee, rln);

        /* fprintf(stderr, "R:\n"); */
        /* aa_dump_mat(stderr, ee, 3, 3); */
        /* fprintf(stderr, "\n"); */

        /* fprintf(stderr, "e: "); */
        /* aa_dump_vec(stderr, e, 3); */
        /* fprintf(stderr, "eln: "); */
        /* aa_dump_vec(stderr, eln, 3); */
        /* fprintf(stderr, "rln: "); */
        /* aa_dump_vec(stderr, rln, 3); */

        /* assert(aa_tf_isrotmat(ee)); */
        arveq("rotmat_expv", e, eln, 1e-4);
        arveq("rotmat_lnv", e, rln, 1e-4);
    }
}

static void rotmat_lnv()
{
    {
        /* The rotation angle is very close to pi */
        double e[3] = {1.808519650250, 0.065124237394, 2.567988211640};
        rotmat_lnv_test(e);
    }

    {
        /* The rotation angle is pi */
        double u[3] = {0.575671, 0.020730, 0.817418};
        aa_tf_vnormalize(u);
        double e[3] = {M_PI * u[0], M_PI * u[1], M_PI * u[2]};
        rotmat_lnv_test(e);
    }
    {
        /* The rotation angle is -pi */
        double u[3] = {0.575671, 0.020730, 0.817418};
        aa_tf_vnormalize(u);
        double  e[3] = {-M_PI * u[0], -M_PI * u[1], -M_PI * u[2]};
        rotmat_lnv_test(e);
    }
}

int main() {
    srand((unsigned int)time(NULL)); // might break in 2038
    aa_test_ulimit();

    rotmat_axang();
    slerp();
    rotmat();
    rotmat_lnv();
}

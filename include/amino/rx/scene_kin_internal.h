/* -*- mode: C; c-basic-offset: 4; -*- */
/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*
 * Copyright (c) 2015, Rice University
 * All rights reserved.
 *
 * Author(s): Neil T. Dantam <ntd@rice.edu>
 *
 *   Redistribution and use in source and binary forms, with or
 *   without modification, are permitted provided that the following
 *   conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of copyright holder the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
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

#ifndef AMINO_RX_SCENE_KIN_INTERNAL_H
#define AMINO_RX_SCENE_KIN_INTERNAL_H

#include "scene_wk_internal.h"
#include "scene_kin.h"

struct aa_rx_sg_sub
{
    const struct aa_rx_sg *scenegraph;
    size_t frame_count;
    aa_rx_frame_id *frames;

    size_t config_count;
    aa_rx_frame_id *configs;

};


/**
 * Return the number of joint frames for the chain from root to tip.
 */
AA_API size_t
aa_rx_sg_chain_frame_count( const struct aa_rx_sg *sg,
                            aa_rx_frame_id root, aa_rx_frame_id tip );

/**
 * Fill in the joint frame ids for the chain from root to tip.
 */
AA_API void
aa_rx_sg_chain_frames( const struct aa_rx_sg *sg,
                       aa_rx_frame_id root, aa_rx_frame_id tip,
                       size_t n_frames, aa_rx_frame_id *chain_frames );

/**
 * Return the number of configuration variables in the chain.
 */
AA_API size_t
aa_rx_sg_chain_config_count( const struct aa_rx_sg *sg,
                             size_t n_frames, const aa_rx_frame_id  *chain_frames );

/**
 * Fill in the config ids for the chain over frames
 */
AA_API void
aa_rx_sg_chain_configs( const struct aa_rx_sg *sg,
                        size_t n_frames, const aa_rx_frame_id *chain_frames,
                        size_t n_configs, aa_rx_config_id *chain_configs );
/**
 * Compute the Jacobian for a chain
 */
AA_API void
aa_rx_sg_chain_jacobian( const struct aa_rx_sg *sg,
                         size_t n_tf, const double *TF_abs, size_t ld_TF,
                         size_t n_ids, aa_rx_frame_id *chain_frames,
                         size_t n_configs, aa_rx_frame_id *chain_configs ,
                         double *J, size_t ld_J );


struct aa_rx_ksol_opts {
    struct aa_rx_wk_opts wk_opts; ///< workspace solver options

    double dt;            ///< initial timestep

    double tol_angle;    ///< angle error tolerate
    double tol_trans;    ///< translation error tolerance

    double tol_angle_svd;    ///< angle error tolerate
    double tol_trans_svd;    ///< translation error tolerance

    double tol_dq;       ///< translation error tolerance

    //double dx_dt;        ///< scaling for cartesian error
    /* double gain_angle;     ///< scaling for cartesian error */
    /* double gain_trans;     ///< scaling for cartesian error */

    const double *dq_dt;       ///< scaling for joint error
    size_t n_dq_dt;
    double *dq_dt_data;

    const double *q_ref;
    size_t n_q_ref;
    double *q_ref_data;

    const double *q_all_seed;
    size_t n_all_seed;
    double *q_all_seed_data;

    size_t max_iterations;

    aa_rx_frame_id frame;

    enum aa_rx_ik_algo ik_algo;
};

struct aa_rx_ik_cx
{
    const struct aa_rx_sg_sub *ssg;
    const struct aa_rx_ksol_opts *opts;

    struct aa_dvec *q_start;
    struct aa_dvec *q_seed;

    aa_rx_frame_id frame;
};

typedef int (*rfx_kin_duqu_fun) ( const void *cx, const double *q, double S[8],  double *J);

struct kin_solve_cx {
    struct aa_mem_region *reg;

    const struct aa_rx_ik_cx *ik_cx;

    const struct aa_rx_ksol_opts *opts;
    const struct aa_rx_sg_sub *ssg;

    const struct aa_dmat *TF_ref;

    size_t iteration;

    struct aa_dvec *q_all;
    struct aa_dvec *q_sub;

    double *TF_rel0;
    double *TF_abs0;


    aa_rx_frame_id frame;
};



#endif /*AMINO_RX_SCENE_KIN_H*/

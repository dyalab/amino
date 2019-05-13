/* -*- mode: C; c-basic-offset: 4; -*- */
/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*
 * Copyright (c) 2018, Colorado School of Mines
 * All rights reserved.
 *
 * Author(s): Neil T. Dantam <ndantam@mines.edu>
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

#ifndef AMINO_RX_SCENE_WK_H
#define AMINO_RX_SCENE_WK_H

/**
 * @file scene_wk.h
 * @brief Workspace control for scene graphs
 */

/**
 * @struct aa_rx_wk_opts
 * Opaque structure for workspace control options.
 */
struct aa_rx_wk_opts;

/**
 * Create workspce control options.
 */
AA_API struct aa_rx_wk_opts *
aa_rx_wk_opts_create(void);

/**
 * Destroy workspce control options.
 */
AA_API void
aa_rx_wk_opts_destroy( struct aa_rx_wk_opts * );


/**
 * Proportional control on pose error.
 *
 * The computed reference term is ADDED to dx.
 *
 * @param[in] opts Workspace control options
 * @param[in] E_act Actual pose (quaternion-translation)
 * @param[in] E_ref Reference pose (quaternion-translation)
 * @param[inout] dx Reference workspace velocity (size 6)
 */
AA_API void
aa_rx_wk_dx_pos( const struct aa_rx_wk_opts * opts,
                 const double *E_act, const double *E_ref,
                 struct aa_dvec *dx );

/**
 * Convert workspace (Cartesian) velocity to joint velocity.
 *
 * @param[in] ssg the subscenegraph to control
 * @param[in] opts workspace control options
 * @param[in] fk updated forward kinematics
 * @param[in] n_x size of dx
 * @param[in] dx reference workspace velocity
 * @param[in] n_q size of dq
 * @param[out] dq reference joint velocity for subscenegraph
 */
AA_API int
aa_rx_wk_dx2dq( const struct aa_rx_sg_sub *ssg,
                const struct aa_rx_wk_opts * opts,
                const struct aa_rx_fk *fk,
                const struct aa_dvec *dx,
                struct aa_dvec *dq );

/**
 * Convert workspace (Cartesian) velocity to joint velocity, with
 * nullspace projection
 *
 * @see aa_rx_wk_dqcenter()
 *
 * @param[in] ssg the subscenegraph to control
 * @param[in] opts workspace control options
 * @param[in] fk updated forward kinematics
 * @param[in] n_x size of dx
 * @param[in] dx reference workspace velocity
 * @param[in] n_q size of dq
 * @param[in] dq_r reference joint velocity (nullspace projected) for subscenegraph
 * @param[out] dq computed reference joint velocity for subscenegraph
 */
AA_API int
aa_rx_wk_dx2dq_np( const struct aa_rx_sg_sub *ssg,
                   const struct aa_rx_wk_opts * opts,
                   const struct aa_rx_fk *fk,
                   const struct aa_dvec *dx, const struct aa_dvec *dq_r,
                   struct aa_dvec *dq );


/**
 * @struct aa_rx_wk_lc3_cx;
 *
 * Opaque context struct for LC3.
 *
 *  Z. Kingston, N. Dantam, and L. Kavraki.
 * [Kinematically Constrained  Workspace Control via Linear  Optimization]
 * (http://dx.doi.org/10.1109/HUMANOIDS.2015.7363455). International
 *  Conference on Humanoid Robots (Humanoids), IEEE. 2015.
 *
 */
struct aa_rx_wk_lc3_cx;

AA_API  struct aa_rx_wk_lc3_cx *
aa_rx_wk_lc3_create ( const struct aa_rx_sg_sub *ssg,
                      const struct aa_rx_wk_opts * opts );


AA_API int
aa_rx_wk_dx2dq_lc3( const struct aa_rx_wk_lc3_cx *lc3,
                    double dt,
                    const struct aa_rx_fk *fk,
                    const struct aa_dvec *dx_r,
                    const struct aa_dvec *q_a, const struct aa_dvec *dq_a,
                    const struct aa_dvec *dq_r, struct aa_dvec *dq );

/**
 * Find joint-centering reference velocity.
 *
 * @param[in] ssg the subscenegraph to control
 * @param[in] opts workspace control options
 * @param[in] n_q size of q
 * @param[in] q actual joint positions for the subscenegraph
 * @param[out] dq_r centering velocities for the subscenegraph
 */
AA_API void
aa_rx_wk_dqcenter( const struct aa_rx_sg_sub *ssg,
                   const struct aa_rx_wk_opts * opts,
                   const struct aa_dvec *q,
                   struct aa_dvec *dq_r );


#endif /*AMINO_RX_SCENE_WK_H*/

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

#include "amino.h"
#include "amino/rx/rxtype.h"

static const char *errstrs[] = {"no_solution",
                                "no_inverse_kinematics",
                                "no_motion_plan",
                                "invalid_frame",
                                "invalid_parameter" };


char *aa_rx_errstr( struct aa_mem_region *reg,
                          int e )
{
    size_t n = 0;
    const char *strs[sizeof(e) * 8 + 1];
    strs[0] = "OK";
    strs[1] = NULL;

    for( size_t i = 0; i < sizeof(errstrs) / sizeof(*errstrs); i ++ ) {
        if( e & (1<<i) ) {
            strs[n] = errstrs[i];
            n++;
            strs[n] = NULL;
        }
    }

    if( 0 == n ) { n++; }

    /* TODO: compute exact size */

    char *buf = (char*)aa_mem_region_alloc(reg, 256);
    *buf = '\0';
    for( size_t i = 0; i < n; i ++ ) {
        strcat(buf, strs[i]);
        if(strs[i+1]) {
            strcat(buf, ":");
        }
    }

    return buf;
}

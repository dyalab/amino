/* -*- mode: C; c-basic-offset: 4 -*- */
/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*
 * Copyright (c) 2011-2012, Georgia Tech Research Corporation
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


#include "amino/def.h"

/** Transpose A into B.
 *
 * \param m rows of A, cols of B
 * \param n cols of A, rows of B
 * \param[in] A source matrix, m*n
 * \param[in] B destination matrix, n*m
 * \param lda leading dimension of A
 * \param ldb leading dimension of B
 */
AA_API void AA_NAME(la,transpose)
( size_t m, size_t n,
  const AA_TYPE *A, size_t lda,
  AA_TYPE *B, size_t ldb );

/** Sum-square differences of x and y. */
AA_FDEC(AA_TYPE, la, ssd,
        size_t n,
        const AA_TYPE *x, size_t incx,
        const AA_TYPE *y, size_t incy )

/** Mean of columns of A.
 * \param m rows of A
 * \param n cols of A
 * \param lda leading dimension of A
 * \param[in] A source matrix, m*n
 * \param[out] x destination vector for mean, length m
 */
AA_FDEC(AA_TYPE, la, colmean,
        size_t m, size_t n,
        const AA_TYPE *A, size_t lda,
        AA_TYPE *x);
/* AA_API void AA_NAME(la,_cmean) */
/* ( size_t m, size_t n, */
/*   const AA_TYPE *A, size_t lda, */
/*   AA_TYPE *x ); */

/** Covariance of columns of A.
 * \param m rows of A
 * \param n cols of A
 * \param lda leading dimension of A
 * \param[in] A source matrix, m*n
 * \param[in] x mean of columns of A, length m
 * \param[out] E covariance of columns of A, m*m
 * \param lde leading dimension of E
 */

AA_FDEC(AA_TYPE, la, colcov,
        size_t m, size_t n,
        const AA_TYPE *A, size_t lda,
        const AA_TYPE *x,
        AA_TYPE *E, size_t lde);

/* AA_API void AA_NAME(la,_ccov) */
/* ( size_t m, size_t n, */
/*   const AA_TYPE *A, size_t lda, */
/*   const AA_TYPE *x, */
/*   AA_TYPE *E, size_t lde ); */



/** Hungarian algorithm to solve min assignment problem
 * \param n rows and cols of A
 * \param A cost matrix, column major, destroyed on exit
 * \param lda leading dimension of A
 * \param row_assign array of column assigned to row at index i
 * \param iwork array of size 3*n*n +2*n
 */
AA_API void AA_NAME(la,opt_hungarian)
( size_t n, AA_TYPE *A, size_t lda,
  ssize_t *row_assign,
  ssize_t *col_assign,
  ssize_t *iwork);

static inline size_t AA_NAME(la,opt_hungarian_iworksize)
( size_t n ) {
    return 3*n*n + 2*n;
}

/** Hungarian algorithm for rectangular distance matrix by padding with zeros.
 *
 * \param m rows of A
 * \param n cols of A
 * \param A distance matrix for minimization problem
 * \param lda leading dimension of A
 * \param[out] row_assign array of column assigned to row at index i,
 *             length m. Unmatched elements have the value of -1.
 * \param work work array of length max(m,n)**2
 * \param iwork integer work array of length (3*max(m,n)**2 + 4*max(m,n))
 */
AA_API void AA_NAME(la,opt_hungarian_pad)
( size_t m, size_t n, const AA_TYPE *A, size_t lda,
  ssize_t *row_assign,
  ssize_t *col_assign,
  AA_TYPE *work, ssize_t *iwork);

static inline size_t AA_NAME(la,opt_hungarian_pad_iworksize)
(size_t m,size_t n) {
    size_t p = AA_MAX(m,n);
    return 3*p*p + 4*p;
}
static inline size_t AA_NAME(la,opt_hungarian_pad_worksize)
(size_t m,size_t n) {
    size_t p = AA_MAX(m,n);
    return p*p;
}


/** Converts max assignment to min assignment for Hungarian algorithm.
 * \param n rows and cols of A
 * \param A cost matrix for max problem,
 *        converted to matrix for min proble on exit
 * \param lda leading dimension of A
 */
AA_API void AA_NAME(la,opt_hungarian_max2min)
( size_t m, size_t n, AA_TYPE *A, size_t lda );

/** Minimum location of vector x */
static inline size_t AA_NAME(la,minloc)
( size_t n, AA_TYPE *x, size_t incx ) {
    size_t imin = 0;
    AA_TYPE xmin = *x;
    for( size_t i = 1; i < n; i ++ ) {
        if( x[i*incx] < xmin ) {
            imin = i;
            xmin = x[i*incx];
        }
    }
    return imin;
}

/** Minimum location of vector x */
static inline AA_TYPE AA_NAME(la,mat_max)
( size_t m, size_t n, AA_TYPE *A, size_t lda,
  size_t *pi, size_t *pj ) {
    size_t im = 0;
    size_t jm = 0;
    AA_TYPE xm = *A;
    for( size_t i = 0; i < m; i++) {
        for( size_t j = 0; j < n; j++) {
            if( AA_MATREF(A, lda, i, j ) > xm ) {
                im = i;
                jm = j;
                xm =  AA_MATREF(A, lda, i, j );
            }
        }
    }
    if( pi ) *pi = im;
    if( pj ) *pj = jm;
    return xm;
}


/** Maximum location of vector x */
static inline size_t AA_NAME(la,maxloc)
( size_t n, AA_TYPE *x, size_t incx ) {
    size_t imax = 0;
    AA_TYPE xmax = *x;
    for( size_t i = 1; i < n; i ++ ) {
        if( x[i*incx] > xmax ) {
            imax = i;
            xmax = x[i*incx];
        }
    }
    return imax;
}

AA_FDEC(AA_TYPE, la, angle,
        size_t n, const AA_TYPE *p, const AA_TYPE *q)



/** Linear Least Squares.
 * \f[ b = Ax \f]
 * Solves for x.
 * \param m rows in A
 * \param n cols in A
 * \param p cols in b and x
 * \param A matrix
 * \param lda leading dimension of A
 * \param b offset matrix
 * \param ldb leading dimension of b
 * \param x solution matrix
 * \param ldx leading dimension of x
 */
AA_API void AA_NAME(la,lls)
( size_t m, size_t n, size_t p,
  const AA_TYPE *A, size_t lda,
  const AA_TYPE *b, size_t ldb,
  AA_TYPE *x, size_t ldx );


/** Singular Value Decomposition of A.
 *
 * \f[ A =  U \Sigma V^T \f]
 *
 * \param m rows
 * \param n columns
 * \param A \f$A \in \Re^m \times \Re^n\f$, column major
 * \param lda leading dimension of A
 * \param U \f$U \in \Re^m \times \Re^m\f$, column major.
 *    If null or ldu==0, U is returned in A.  U and Vt cannot both be null.
 * \param ldu leading dimension of U
 * \param S \f$S \in \Re^m min(m,n)\f$, singular values
 * \param Vt \f$V^T \in \Re^n \times \Re^n\f$, singular vectors. If
 *   null or ldvt==0, Vt is returned in A.  Vt and U cannot both be null.
 * \param ldvt leading dimension of Vt
 */
AA_API int AA_NAME(la,svd)
( size_t m, size_t n, const AA_TYPE *A, size_t lda,
  AA_TYPE *U, size_t ldu,
  AA_TYPE *S,
  AA_TYPE *Vt, size_t ldvt );


/** Fit a least-squares hyperplane to columns of A.
 *
 * Normalizes data first.
 *
 * \param m rows of A, size of space
 * \param n cols of A, number of points
 * \param A data points
 * \param lda leading dimension of A
 * \param x output hyperplance in hessian normal form
 *
 */
AA_FDEC( void, la, colfit,
         size_t m, size_t n,
         const AA_TYPE *A, size_t lda, AA_TYPE *x );

#include "amino/undef.h"

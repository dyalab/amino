;;;; -*- mode: lisp -*-
;;;;
;;;; Copyright (c) 2013, Georgia Tech Research Corporation
;;;; All rights reserved.
;;;;
;;;; Author(s): Neil T. Dantam <ntd@gatech.edu>
;;;; Georgia Tech Humanoid Robotics Lab
;;;; Under Direction of Prof. Mike Stilman <mstilman@cc.gatech.edu>
;;;;
;;;;
;;;; This file is provided under the following "BSD-style" License:
;;;;
;;;;
;;;;   Redistribution and use in source and binary forms, with or
;;;;   without modification, are permitted provided that the following
;;;;   conditions are met:
;;;;
;;;;   * Redistributions of source code must retain the above copyright
;;;;     notice, this list of conditions and the following disclaimer.
;;;;
;;;;   * Redistributions in binary form must reproduce the above
;;;;     copyright notice, this list of conditions and the following
;;;;     disclaimer in the documentation and/or other materials provided
;;;;     with the distribution.
;;;;
;;;;   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
;;;;   CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
;;;;   INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
;;;;   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
;;;;   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
;;;;   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
;;;;   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
;;;;   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
;;;;   USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
;;;;   AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
;;;;   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
;;;;   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
;;;;   POSSIBILITY OF SUCH DAMAGE.

(in-package :amino)

;; (def-la ("aa_la_d_angle" d-angle) :double
;;   (n size-t (:length x) (:length y))
;;   (x :vector)
;;   (y :vector))


(def-la-cfun ("aa_la_d_lls" aa-la-d-lls :pointer-type :pointer) :void
  (m size-t)
  (n size-t)
  (p size-t)
  (a :matrix)
  (b :matrix)
  (x :matrix))

(defun linear-least-squares (a b &optional x)
  (let ((x x))
    (with-foreign-matrix (a ld-a m n) a :input
      (with-foreign-matrix (b ld-b b-rows p) b :input
        (unless x
          (setq x (if (= 1 p)
                      (make-vec m)
                      (make-matrix m p))))
        (with-foreign-matrix (x ld-x x-rows x-cols) x :output
          (assert (= m x-rows))
          (assert (= n b-rows))
          (assert (= p x-cols))
          (sb-int:with-float-traps-masked (:divide-by-zero)
            (aa-la-d-lls m n p
                         a ld-a
                         b ld-b
                         x ld-x)))))
    x))

(def-la-cfun ("aa_la_d_ssd" aa-la-d-ssd :pointer-type :pointer) :double
  (n size-t)
  (x :vector)
  (y :vector))

(defun vec-ssd (a b)
  "Return the sum of squared differences between vectors A and B."
  (let ((r 0d0))
    (with-foreign-vector (a inc-a length-a) a :input
      (with-foreign-vector (b inc-b length-b) b :input
        (assert (= length-a length-b))
        (setq r (aa-la-d-ssd length-a
                             a inc-a
                             b inc-b))))
    r))

(defun vec-dist (a b)
  "Return the Euclidean distance between two vectors."
  (sqrt (vec-ssd a b)))

(defun vec-norm (x)
  "Return the Euclidean normal of the vectors."
  (dnrm2 x))

(defun vec-normalize (x &optional y)
  "Return the Euclidean-normalized vector."
  (let ((y (dcopy x y)))
    (with-foreign-vector (y inc-y len-y) y :inout
      (blas-dscal len-y
                  (/ 1d0 (blas-dnrm2 len-y y inc-y))
                  y inc-y))
    y))

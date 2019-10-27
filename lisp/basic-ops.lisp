;;;; -*- mode: lisp -*-
;;;;
;;;; Copyright (c) 2012, Georgia Tech Research Corporation
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

;;; Matrix type
;;; - Default
;;;   - Column Major
;;;   - Double Float
;;;   - Vectors are column
;;; - Fields
;;;   - Data Vector
;;;   - Offset
;;;   - Stride
;;;   - Rows
;;;   - Cols
(in-package :amino)

(defconstant π pi)
(defconstant π/2 (/ pi 2))
(defconstant π/4 (/ pi 4))
(defconstant -π (- pi))
(defconstant -π/2 (- (/ pi 2)))

(defconstant +x+ 0)
(defconstant +y+ 1)
(defconstant +z+ 2)
(defconstant +w+ 3)


(defun make-matrix (m n)
  "Make a new matrix with M rows and N cols."
  (if (= 1 n)
      (make-array m :element-type 'double-float)
      (%make-matrix :data (make-array (* m n)
                                      :element-type 'double-float)
                    :offset 0
                    :stride m
                    :rows m
                    :cols n)))

(defun matrix-data (m)
  (etypecase m
    ((simple-array * (*))  m)
    (matrix (%matrix-data m))
    (real-array (real-array-data m))
    (list (map-into (make-array (length m) :element-type 'double-float)
                    (lambda (k) (coerce k 'double-float))
                    m))))

(defun matrix-offset (m)
  (etypecase m
    ((simple-array * (*))  0)
    (list 0)
    (matrix (%matrix-offset m))
    (real-array 0)))

(defun matrix-stride (m)
  (etypecase m
    ((simple-array * (*))  (length m))
    (matrix (%matrix-stride m))
    (real-array (length (real-array-data m)))
    (list (length m))))

(defun matrix-rows (m)
  (etypecase m
    ((simple-array * (*)) (length m))
    (list (length m))
    (matrix (%matrix-rows m))))

(defun matrix-cols (m)
  (etypecase m
    ((simple-array * (*)) 1)
    (list 1)
    (matrix (%matrix-cols m))))

(defun matrix-type (matrix)
  "Return the element type of MATRIX."
  (array-element-type (matrix-data matrix)))

(defun matrix-size (matrix)
  "Returns number of elements in matrix."
  (* (matrix-rows matrix)
     (matrix-cols matrix)))

(defun matrix-index (matrix i j)
  "Compute the array index for element at row i, col j."
  (let ((rows (matrix-rows matrix))
        (cols (matrix-cols matrix))
        (stride (matrix-stride matrix))
        (offset (matrix-offset matrix)))
    (assert (< i rows))
    (assert (< j cols))
    (+ offset
       (* j stride)
        i)))

;; (defun matrix-view-array (array)
;;   (ecase (array-rank array)
;;     (1
;;      (%make-matrix :data array
;;                    :offset 0
;;                    :stride (length array)
;;                    :rows (length array)
;;                    :cols 1))
;;     (2)))

(defun matref (matrix i j)
  "Return element at row i, col j."
  (aref (matrix-data matrix)
        (matrix-index matrix i j)))

(defun vecref (vector i)
  "Return I'th element of column vector VECTOR"
  (etypecase vector
    (simple-vector (svref vector i))
    (matrix
     (cond ((= 1 (matrix-cols vector))
            (matref vector i 0))
           ((= 1 (matrix-rows vector))
            (matref vector 0 1))
           (t (matrix-storage-error "Matrix is not a vector\n"))))
    (real-array (aref (real-array-data vector) i))
    (array (aref vector i))
    (cons (nth i vector))))

(defun (setf vecref) (value vec i)
  "Set the I'th value of VEC to VALUE."
  (etypecase vec
    (simple-vector (setf (svref vec i) value))
    (array (setf (aref vec i) value))
    (matrix
     (cond ((= 1 (matrix-cols vec))
            (setf (matref vec i 0) value))
           ((= 1 (matrix-rows vec))
            (setf (matref vec 0 i) value))
           (t (matrix-storage-error "Matrix is not a vector\n"))))))

(defun (setf matref) (value matrix i j)
  "Set element at row i, col j."
  (setf (aref (matrix-data matrix)
              (matrix-index matrix i j))
        value))

(defun vec-x (vector)
  "Return the X element of the vector."
  (vecref vector +x+))

(defun vec-y (vector)
  "Return the Y element of the vector."
  (vecref vector +y+))

(defun vec-z (vector)
  "Return the Z element of the vector."
  (vecref vector +z+))

(defun vec-w (vector)
  "Return the W element of the vector."
  (vecref vector +w+))


(declaim (inline matrix-block))
(defun matrix-block (matrix i j m n)
  "Make a new descriptor for a sub-block of MATRIX.
MATRIX: original matrix.
I: first row in MATRIX of the block.
J: first col in MATRIX of the block.
M: rows in the block.
N: cols in the block."
  ;; TODO: Check bounds
  (%make-matrix :data (matrix-data matrix)
                :offset (matrix-index matrix i j)
                :stride (matrix-stride matrix)
                :rows m
                :cols n))

(defun matrix-row (matrix i)
  "Block for row I"
  (matrix-block matrix i 0 1 (matrix-cols matrix)))

(defun matrix-col (matrix j)
  "Block for col J"
  (matrix-block matrix 0 j (matrix-rows matrix) 1))


(defun matrix-contiguous-p (matrix)
  "Is the matrix stored contiguously?"
  (or (= (matrix-stride matrix)
         (matrix-rows matrix))
      (= 1 (matrix-cols matrix))))

(defmethod print-object ((object matrix) stream)
  "Print a matrix."
  (format stream "~&#S(MATRIX: ")
  (dotimes (i (matrix-rows object))
    (unless (= i 0)
      (dotimes (j 11)
        (write-char #\  stream)))

    (write-char #\[ stream)
    (write-char #\  stream)
    (dotimes (j (matrix-cols object))
      (format stream "~A " (let ((x (matref object i j)))
                             (if (and (< x 1d-15) (> x -1d-15 ))
                                 0d0 x))))
    (write-char #\] stream)
    (unless (= (1+ i) (matrix-rows object))
      (terpri stream)))
  (format stream ")"))


;;;;;;;;;;;;;;;
;;; VECTORS ;;;
;;;;;;;;;;;;;;;

;; Lisp array vectors
(declaim (inline make-vec n))
(defun make-vec (n &key (initial-element 0d0))
  "Make a floating point (double) vector."
  (make-array n
              :element-type 'double-float
              :initial-element initial-element))

(declaim (inline make-fnvec n))
(defun make-fnvec (n)
  "Make a floating point (double) vector."
  (make-array n :element-type 'fixnum))

(declaim (inline %vec))
(defun %vec (type args)
  (let ((vec (make-array (length args) :element-type type)))
    (loop
       for i from 0
       for x in args
       do (setf (aref vec i)
                (coerce x type)))
    vec))

(defun list-vec (args)
  (%vec 'double-float args))

(defun vec (&rest args)
  "Create a floating-point vector"
  (list-vec args))

(defun ensure-vec (thing)
  "Coerce THING to a vec."
  (etypecase thing
    ((vec *) thing)
    (list (list-vec thing))
    (real-array (real-array-data thing))))

(defun fnvec (&rest args)
  "Create a fixnum vector"
  (%vec 'fixnum args))

(defun vec-length (vec)
  (etypecase vec
    (list (length vec))
    (simple-array (length vec))
    (array (length vec))
    (matrix (error "Can't find vector length of a matrix"))
    (real-array (length (real-array-data vec)))))

(defun vec-cat (&rest args)
  "Concatenate vectors."
  (let* ((n (loop for x in args summing (vec-length x)))
         (y (make-vec n))
         (i -1))
    (dolist (x args)
      (dotimes (j (vec-length x))
        (setf (vecref y (incf i))
              (coerce (vecref x j) 'double-float))))
    y))

(defun vec-copy (vec &key (start 0) (end (length vec)))
  ;; TODO: matrix version
  (let* ((n (- end start))
         (new (make-vec n)))
    (dotimes (i n)
      (setf (aref new i)
            (aref vec (+ start i))))
    new))

(defun vec-list (vec)
  "Convert VEC to a list."
  (etypecase vec
    (list vec)
    (array
     (loop for x across vec
        collect x))
     ;; TODO: block matrix version
     (real-array
      (vec-list (real-array-data vec)))))

(defun vec-sequence (vec)
  "Ensure that VEC is a sequence."
  (etypecase vec
    (sequence vec)
     ;; TODO: block matrix version
     (real-array
      (real-array-data vec))))

;(defun vec-flatten (sequence)

(declaim (inline %vec-flatten))
(defun %vec-flatten (type sequence)
  (let* ((n (length sequence))
         (m (vec-length (elt sequence 0)))
         (v (make-array (* m n) :element-type type))
         (j 0))
    (map 'nil (lambda (x)
                (assert (= m (vec-length x)))
                (dotimes (i m)
                  (setf (aref v (+ (* 3 j) i))
                        (vecref x i)))
                (incf j))
         sequence)
         v))

(defun vec-flatten (sequence)
  "Flatten positionly-nested sequences into a single numeric vector."
  (%vec-flatten 'double-float sequence))

(defun fnvec-flatten (sequence)
  "Flatten positionly-nested sequences into a single numeric vector."
  (%vec-flatten 'fixnum sequence))


(defun matrix-vector-store-p (matrix)
  "Is the matrix stored in a way that looks like a vector?"
  (or (= (matrix-stride matrix)
         (matrix-rows matrix))
      (= 1 (matrix-rows matrix))
      (= 1 (matrix-cols matrix))))

(defun matrix-vector-n-p (matrix n inc)
  (let ((rows (matrix-rows matrix))
        (cols (matrix-cols matrix))
        (stride (matrix-stride matrix)))
  (or
   ;; column vector storage
   (and (= n rows stride)
        (= inc 1 cols))
   ;; row vector storage
   (and (= n cols)
        (= 1 rows)
        (= inc stride)))))

(defmacro check-vector-storage (place)
  `(progn
     (check-type ,place matrix)
     (unless (matrix-vector-store-p ,place)
       (matrix-storage-error "~A is not stored as a vector" ,place))))

(defun matrix-counts-vector-increment (rows cols stride)
  (cond
    ((or (= 1 cols)
         (= rows stride))
     1)
    ((= 1 rows) stride)
    (t (matrix-storage-error "stored as a vector"))))

(defun matrix-vector-increment (matrix)
  (declare (type matrix matrix))
  (matrix-counts-vector-increment (matrix-rows matrix)
                                  (matrix-cols matrix)
                                  (matrix-stride matrix)))

(defun make-row-vector (n)
  (make-matrix 1 n))

(defun make-col-vector (n)
  (make-matrix n 1))


(defun wrap-col-vector (column)
  (declare (type (simple-array double-float (*)) column))
  (%make-matrix :data column
                :offset 0
                :stride (length column)
                :cols 1
                :rows (length column)))

(defmacro with-matrix ((var value) &body body)
  "Ensure value is a matrix descriptor.
Will wrap simple-vectors in a descriptor struct."
  (with-gensyms (body-fun desc value-sym n)
    `(flet ((,body-fun (,var) ,@body))
       (let ((,value-sym ,value))
         (etypecase ,value-sym
           (matrix (,body-fun ,value-sym))
           ((simple-array double-float (*))
            (let* ((,n (length ,value-sym))
                   (,desc (%make-matrix :data ,value-sym
                                        :stride ,n
                                        :rows ,n
                                        :cols 1)))
              ;; (declare (dynamic-extent ,desc))
              (,body-fun ,desc))))))))

(defun check-vec-length (a b)
  (check-matrix-dimensions (vec-length a)
                           (vec-length b)))

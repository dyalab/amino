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


(in-package :amino-ffi)

;;;;;;;;;;;;;;;;;;;;;;;;
;; FOREIGN CONTAINERS ;;
;;;;;;;;;;;;;;;;;;;;;;;;

(defstruct foreign-container
  pointer)

(defmacro def-foreign-container (lisp-type cffi-type
                                 &key
                                   slots
                                   struct-type
                                   destructor)
  "Define a new container for a foreign type along with CFFI type convertors.

If destructor is provided, pointer instances returned from foreign
code are automatically bound with a finalizer to call the destructor.
Note that destructor must operate on the raw pointer type.
"
  (labels ((sym (&rest args)
             (intern (apply #'concatenate 'string args)
                     (symbol-package lisp-type))))
    (let ((%make-it (sym "%MAKE-" (string lisp-type)))
          (null-fun (sym "NULL-" (string lisp-type)))
          ;;(%destroy-it (sym "%DESTROY-" (string lisp-type)))
          (%finalize-it (sym "%FINALIZE-" (string lisp-type))))
      `(progn
         (defstruct (,lisp-type (:include foreign-container)
                                (:constructor ,%make-it (pointer)))
           ,@slots)
         (cffi:define-foreign-type ,cffi-type ()
           ()
           (:simple-parser ,cffi-type)
           (:actual-type :pointer))
         (defun ,null-fun ()
           (,%make-it (cffi:null-pointer)))
         (defmethod cffi:expand-to-foreign (value (type ,cffi-type))
           (list 'progn
                 (list 'check-type value ',lisp-type)
                 (list 'foreign-container-pointer value)))
         ,@(when struct-type
                 `((declaim (inline ,(sym (string lisp-type) "-SLOT-VALUE")))
                   (defun ,(sym (string lisp-type) "-SLOT-VALUE") (object slot)
                     (cffi:foreign-slot-value (,(sym (string lisp-type) "-POINTER") object)
                                              '(:struct ,struct-type) slot))))

         ,@(when destructor
             (let ((destructor-lisp (if (listp destructor)
                                        (second destructor)
                                        destructor)))
               `((cffi:defcfun ,destructor :void
                   (object :pointer))
                 (defun ,%finalize-it (pointer)
                   (let ((object (,%make-it pointer)))
                     (sb-ext:finalize object (lambda () (,destructor-lisp pointer)))
                     object)))))
         (defmethod cffi:expand-from-foreign (form (type ,cffi-type))
           ,(if destructor
                `(list ',%finalize-it form)
                `(list ',%make-it form)))))))

(defmacro def-foreign-container-accessor (lisp-type slot)
  (let ((accessor-name (intern (concatenate 'string (string lisp-type) "-" (string slot))
                               (symbol-package lisp-type)))
        (slot-accessor (intern (concatenate 'string (string lisp-type) "-SLOT-VALUE")
                               (symbol-package lisp-type)))
        (object (gensym "OBJECT")))
    `(progn (declaim (inline ,accessor-name))
            (defun ,accessor-name (,object)
              (,slot-accessor ,object ',slot)))))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Friendly Double Floats ;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(cffi:define-foreign-type coercible-double ()
  ()
  (:simple-parser coercible-double)
  (:actual-type :double))

(defmethod expand-to-foreign-dyn (value var body (type coercible-double))
  `(let ((,var (coerce ,value 'double-float)))
     ,@body))

;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; CALL BY REFERENCING ;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;

(defgeneric ref-type (value-type))

(defun ref-true-type (value-type by-reference)
  (if by-reference
      (ref-type value-type)
      value-type))

(defmacro with-reference ((var value type) &body body)
  (with-gensyms (ptr)
    `(let ((,ptr ,value))
       (cffi:with-foreign-object (,var ',type)
         (setf (cffi:mem-ref ,var ',type) ,ptr)
         ,@body))))

(defmacro def-ref-type (name value-type)
  `(progn
     (defmethod ref-type ((value-type (eql ',value-type))) ',name)
     (define-foreign-type ,name ()
       ()
       (:simple-parser ,name)
       (:actual-type :pointer))
     (defmethod expand-to-foreign-dyn (value var body (type ,name))
       (append (list 'with-reference (list var value ',value-type))
                     body))))

(def-ref-type int-ref-t :int)
(def-ref-type double-ref-t :double)
(def-ref-type float-ref-t :float)
(def-ref-type size-ref-t size-t)
(def-ref-type char-ref-t :uint8)


;;;;;;;;;;;;;;;;;;;;;;
;;; FOREIGN ARRAYS ;;;
;;;;;;;;;;;;;;;;;;;;;;

(defun fill-pointer-from-list (pointer list)
  "Fill memory at pointer with double-float values in list"
  (loop
     for i from 0
     for x in list
     do (setf (mem-aref pointer :double i)
              (coerce x 'double-float))))

(defun fill-list-from-pointer (list pointer)
  "Fill list with double-float values at pointer"
  (loop for i from 0
     for cons on list
     do (rplaca cons (mem-aref pointer :double i))))

(defmacro with-foreign-list ((pointer length) list intent &body body)
  (with-gensyms (value)
    `(let* ((,value ,list)
            (,length (length ,value)))
       (with-foreign-object (,pointer :double ,length)
         ;; Maybe copy in
         ,@(ecase intent
             ((:input :inout)
              `((fill-pointer-from-list ,pointer ,list)))
             (:output nil))
         ;; Body
         (prog1 (progn ,@body)
           ;; Maybe copy out
           ,@(ecase intent
               (:input nil)
               ((:output :inout)
                `((fill-list-from-pointer ,value ,pointer)))))))))

(defmacro with-foreign-vector ((pointer increment length) vector intent &body body)
  "Bind POINTER and LENGTH to corresponding values of VECTOR, then evaluate BODY.

INTENT: One of (or :input :output :inout). When value
        is (or :output :inout), return the vector after evaluating
        BODY.
"
  (with-gensyms (ptr-fun array-offset-fun array-fun value data offset stride rows columns)
    `(labels ((,ptr-fun (,pointer ,increment ,length) ,@body)
              (,array-offset-fun (,data ,offset ,increment ,length)
                (with-pointer-to-vector-data (,pointer ,data)
                  (,ptr-fun (mem-aptr ,pointer :double ,offset)
                            ,increment ,length)))
              (,array-fun (,data)
                (with-pointer-to-vector-data (,pointer ,data)
                  (,ptr-fun ,pointer 1 (length ,data)))))
       (let ((,value ,vector))
         (etypecase ,value
           (matrix
            (let ((,data (%matrix-data ,value))
                  (,offset (%matrix-offset ,value))
                  (,stride (%matrix-stride ,value))
                  (,rows (%matrix-rows ,value))
                  (,columns (%matrix-cols ,value)))
              (check-matrix-bounds ,data ,offset ,stride ,rows ,columns)
              (cond ((= 1 ,columns) ; row vector
                     (,array-offset-fun ,data ,offset 1 ,rows))
                    ((= 1 ,rows)    ; column vector
                     (,array-offset-fun ,data ,offset ,stride ,columns))
                    ((= ,stride ,rows) ; non-blocked matrix
                     (,array-offset-fun ,data ,offset 1 (* ,rows ,columns)))
                    (t (matrix-storage-error "Matrix ~A is not a vector" ',vector)))))
           ((simple-array double-float (*))
            (,array-fun ,value))
           (real-array
            (,array-fun (real-array-data ,value)))
           (list
            (with-foreign-list (,pointer ,length) ,value ,intent
              (,ptr-fun ,pointer 1 ,length))))))))

(defmacro with-foreign-simple-vector ((pointer length) vector intent &body body)
  "Bind POINTER and LENGTH to corresponding values of VECTOR, then evaluate BODY.

VECTOR must have an increment of 1.

INTENT: One of (or :input :output :inout). When value
        is (or :output :inout), return the vector after evaluating
        BODY.
"
  (with-gensyms (increment simple-vector-fun)
    `(flet ((,simple-vector-fun (,pointer ,length)
              ,@body))
       (with-foreign-vector (,pointer ,increment ,length) ,vector ,intent
         (unless (= 1 ,increment) (matrix-storage-error "Not a simple vector"))
         (,simple-vector-fun ,pointer ,length)))))

(defmacro with-foreign-fixed-vector (pointer vector fixed-length intent &body body)
  "Bind POINTER corresponding values of VECTOR, then evaluate BODY.

VECTOR must have an increment of 1 and be of the given fixed length.

INTENT: One of (or :input :output :inout). When value
        is (or :output :inout), return the vector after evaluating
        BODY.
"
  (with-gensyms (actual-length)
    `(with-foreign-simple-vector (,pointer ,actual-length) ,vector ,intent
       (unless (= ,actual-length ,fixed-length)
         (matrix-storage-error "Wrong vector size"))
       ,@body)))

(defmacro with-foreign-matrix ((pointer stride rows columns) matrix intent
                               &body body)
  "Evaluate BODY with matrix pointer and counts bound to POINTER, STRIDE, ROWS, and COLUMNS.

INTENT: One of (or :input :output :inout). When value
        is (or :output :inout), return the vector after evaluating
        BODY.
"
  (declare (ignore intent))
  (with-gensyms (ptr-fun vector-fun value data offset)
    `(labels ((,ptr-fun (,pointer ,stride ,rows ,columns) ,@body)
              (,vector-fun (,data)
                (with-pointer-to-vector-data (,pointer ,data)
                  (,ptr-fun ,pointer (length ,data) (length ,data) 1))))
       (let ((,value ,matrix))
         (etypecase ,value
           (matrix
            (let ((,data (%matrix-data ,value))
                  (,offset (%matrix-offset ,value))
                  (,stride (%matrix-stride ,value))
                  (,rows (%matrix-rows ,value))
                  (,columns (%matrix-cols ,value)))
              (check-matrix-bounds ,data ,offset ,stride ,rows ,columns)
              (with-pointer-to-vector-data (,pointer ,data)
                (,ptr-fun (mem-aptr ,pointer :double ,offset)
                          ,stride ,rows ,columns))))
           ((simple-array double-float (*))
            (,vector-fun ,value))
           (real-array
            (,vector-fun (real-array-data ,value))))))))

;;; Foreign Binding ;;;

(defun prefix-gensym (prefix var)
  (gensym (concatenate 'string prefix "-" (string var) "-")))

(defun extra-arg-symbol (var type)
  (case type
    (:vector
     (prefix-gensym "INC" var))
    (:matrix
     (prefix-gensym "LD" var))))

;; Bind the C function
(defmacro def-la-cfun ((c-name lisp-name &key
                               by-reference
                               (size-type 'size-t)
                               (pointer-type 'foreign-array-t))
                        result-type &rest args)
  `(defcfun (,c-name ,lisp-name) ,result-type
     ,@(loop for (name type &rest checks) in args
          append (case type
                   ((:vector :matrix)
                    `((,name ,pointer-type)
                      (,(extra-arg-symbol name type) ,(ref-true-type size-type by-reference))))
                   (otherwise (list (list name (ref-true-type type by-reference))))))))

;;;;;;;;;;;;;;;;;;
;;; BLAS TYPES ;;;
;;;;;;;;;;;;;;;;;;

(defctype blas-size-t :int)
(def-ref-type blas-size-ref-t blas-size-t)

(defconstant +transpose+ (char-code #\T))
(defconstant +no-transpose+ (char-code #\N))

(define-foreign-type transpose-ref-t ()
  ()
  (:simple-parser transpose-ref-t)
  (:actual-type :pointer))

(defmethod ref-type ((value-type (eql 'transpose-t))) 'transpose-ref-t)

(defmethod expand-to-foreign-dyn (value var body (type transpose-ref-t))
  `(with-reference (,var (if ,value +transpose+ +no-transpose+) :uint8)
     ,@body))

(defmacro def-blas-cfun ((name lisp-name) result-type &rest args)
  `(def-la-cfun (,name ,lisp-name
                       :by-reference t :size-type blas-size-t :pointer-type :pointer)
       ,result-type
     ,@args))

;;;;;;;;;;;;;;;;;;
;;; BLAS TYPES ;;;
;;;;;;;;;;;;;;;;;;

(defun foreign-vector-dup (foreign-pointer count foreign-type lisp-type)
  (let ((v (make-array count :element-type lisp-type)))
    (with-pointer-to-vector-data (lisp-ptr v)
      (libc-memcpy lisp-ptr foreign-pointer (* count (foreign-type-size foreign-type))))
    v))

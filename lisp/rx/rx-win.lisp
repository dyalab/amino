;;;; -*- mode: lisp -*-
;;;;
;;;; Copyright (c) 2015, Rice University
;;;; All rights reserved.
;;;;
;;;; Author(s): Neil T. Dantam <ntd@gatech.edu>
;;;;
;;;;   Redistribution and use in source and binary forms, with or
;;;;   without modification, are permitted provided that the following
;;;;   conditions are met:
;;;;
;;;;   * Redistributions of source code must retain the above
;;;;     copyright notice, this list of conditions and the following
;;;;     disclaimer.
;;;;   * Redistributions in binary form must reproduce the above
;;;;     copyright notice, this list of conditions and the following
;;;;     disclaimer in the documentation and/or other materials
;;;;     provided with the distribution.
;;;;   * Neither the name of copyright holder the names of its
;;;;     contributors may be used to endorse or promote products
;;;;     derived from this software without specific prior written
;;;;     permission.
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

(in-package :robray)

;;;;;;;;;;;;;;;;;;;;;;;;
;;; Foreign Bindings ;;;
;;;;;;;;;;;;;;;;;;;;;;;;

(cffi:defcfun aa-rx-win-default-create rx-win-t
  (title :string)
  (width :int)
  (height :int))

(cffi:defcfun aa-rx-win-set-sg :void
  (window rx-win-t)
  (sg rx-sg-t))

(cffi:defcfun aa-rx-win-set-sg-sub :void
  (window rx-win-t)
  (sg-sub rx-sg-sub-t))

(cffi:defcfun aa-rx-win-run :void)

(cffi:defcfun aa-rx-win-run-async :void)

(cffi:defcfun aa-rx-win-lock :void
  (window rx-win-t))

(cffi:defcfun aa-rx-win-unlock :void
  (window rx-win-t))

(cffi:defcfun aa-rx-win-set-config :void
  (window rx-win-t)
  (n size-t)
  (q :pointer))

(defun rx-win-set-config (win config)
  (with-foreign-simple-vector (pointer length) config :input
    (aa-rx-win-set-config win length pointer)))


(cffi:defcfun aa-rx-win-get-config :void
  (window rx-win-t)
  (n size-t)
  (q :pointer))

(defun rx-win-get-config (win)
  (let ((vec (make-vec (mutable-scene-graph-config-count (rx-win-mutable-scene-graph win)))))
    (with-foreign-simple-vector (pointer length) vec :output
      (aa-rx-win-get-config win length pointer))
    vec))

(cffi:defcfun aa-rx-win-sg-gl-init :void
  (win rx-win-t)
  (sg rx-sg-t))

(cffi:defcfun aa-rx-win-stop-on-quit :void
  (win rx-win-t)
  (value :boolean))

(cffi:defcfun aa-rx-win-stop :void
  (win rx-win-t))

(cffi:defcfun aa-rx-win-set-display-plan :void
  (win rx-win-t)
  (sg rx-sg-t)
  (n-plan amino-ffi:size-t)
  (plan :pointer))

(cffi:defcfun aa-rx-win-gl-globals :pointer
  (win rx-win-t))

(cffi:defcfun aa-rx-win-set-display-seq :void
  (win rx-win-t)
  (mp-seq (rx-mp-seq-t)))

(cffi:defcfun aa-rx-win-get-tf-cam :void
  (win rx-win-t)
  (e :pointer))


(cffi:defcfun aa-rx-win-set-tf-cam :void
  (win rx-win-t)
  (e :pointer))

(cffi:defcfun aa-sdl-bind-key :void
  (keycode sdl-keycode)
  (handler :pointer)
  (cx :pointer))

;;;;;;;;;;;;;;;;;;;
;;; Convenience ;;;
;;;;;;;;;;;;;;;;;;;

(defvar *%window%* nil)
(defvar *window-thread* nil)


(defun win-create (&key
                     (title "AminoGL")
                     (width 800)
                     (height 600)
                     stop-on-quit)
  (let ((window
         (with-main-thread
           (unless *%window%*
             (aa-sdl-bind-key :p (cffi:callback render-callback) (cffi:null-pointer))
             (aa-sdl-bind-key :r (cffi:callback reload-callback) (cffi:null-pointer))

             (setq *%window%*
                   (sb-int:with-float-traps-masked (:divide-by-zero :overflow :invalid :inexact)
                     (aa-rx-win-default-create title width height)))
             (aa-rx-win-stop-on-quit *%window%* stop-on-quit))
           *%window%*)))
    (setq *%window%* window)
    window))


(defun window ()
  (or *%window%* (win-create)))

(defun (setf window) (value)
  (with-main-thread
    (setf *%window%* value)))


(defmacro with-win-lock (win &body body)
  (with-gensyms (v-win)
    `(let ((,v-win ,win))
       (aa-rx-win-lock ,v-win)
       (unwind-protect (progn ,@body)
         (aa-rx-win-unlock ,v-win)))))



(defun win-stop (&optional (window (window)))
  (aa-rx-win-stop window))

(cffi:defcfun aa-rx-win-destroy :void
  (obj :pointer))

(defun win-run (&key synchronous)
  (sb-int:with-float-traps-masked (:divide-by-zero :overflow :underflow :invalid :inexact)
    (if synchronous
        (aa-rx-win-run)
        (aa-rx-win-run-async))))

(defun win-destroy (&optional (window (window)))
  (assert window)
  (aa-rx-win-destroy (rx-win-pointer window))
  (setq window nil)
  (values))


(defun win-set-scene-graph (scene-graph &key
                                          (window (window))
                                          configuration-map
                                          end-effector
                                          )
  (let* ((m-sg (mutable-scene-graph scene-graph))
         (sg-sub (when end-effector
                   (let ((id (aa-rx-sg-frame-id m-sg end-effector)))
                     (assert (not (= id +frame-id-none+)))
                     (aa-rx-sg-chain-create m-sg +frame-id-root+ id))))
         (q (make-vec (aa-rx-sg-config-count m-sg))))
    (with-win-lock window
      (aa-rx-win-sg-gl-init window m-sg)
      (if sg-sub
          (aa-rx-win-set-sg-sub window sg-sub)
          (aa-rx-win-set-sg window m-sg))
      ;; Preserve the window display object.  The active display
      ;; function may be using it.
      (setf (rx-win-mutable-scene-graph window) m-sg
            (rx-win-sub-scene-graph window) sg-sub
            (rx-win-end-effector window ) end-effector
            (rx-win-config-vector window) q)
    (when configuration-map
      (win-set-config configuration-map window))))
  (values))

(defun win-scene-graph (&optional (window (window)))
  (mutable-scene-graph-scene-graph (rx-win-mutable-scene-graph window)))

(defun win-set-config (configs &optional (win (window)))
  (let ((sg (rx-win-mutable-scene-graph win))
        (q (rx-win-config-vector win)))
    (mutable-scene-graph-config-vector sg configs q)
    (rx-win-set-config win q)))


(defmacro with-win-gl-globals ((var &optional (window '(window))) &body body)
  `(with-win-lock ,window
     (let ((,var (%make-rx-gl-globals (aa-rx-win-gl-globals ,window))))
       ,@body)))

(defun win-view-visual ()
  (with-win-gl-globals (gl-globals)
    (gl-globals-show-visual gl-globals t)
    (gl-globals-show-collision gl-globals nil)))

(defun win-view-collision ()
  (with-win-gl-globals (gl-globals)
    (gl-globals-show-visual gl-globals nil)
    (gl-globals-show-collision gl-globals t)))

(defun win-mask-frames (frames &key
                                 (hide t)
                                 (window (window)))
  (with-win-gl-globals (gl-globals window)
    (let ((m-sg (rx-win-mutable-scene-graph window)))
      (map nil (lambda (frame)
                 (let ((id (mutable-scene-graph-frame-id m-sg frame)))
                   (check-type id frame-id)
                   (aa-gl-globals-mask gl-globals id hide)))
           (ensure-list frames)))))


(defun win-mask-all (&key (window (window))
                       (hide t))
  (with-win-gl-globals (gl-globals window)
    (if hide
        (let* ((m-sg (rx-win-mutable-scene-graph window))
               (sg (mutable-scene-graph-scene-graph m-sg)))
          (win-mask-frames (scene-graph-frame-names sg)
                           :window window
                           :hide hide))
        (gl-globals-unmask-all gl-globals))))

(defun win-display-motion-plan (motion-plan)
  (let ((window (win-create))
        (m-sg (motion-plan-mutable-scene-graph motion-plan))
        (path (motion-plan-path motion-plan))
        (n-path (motion-plan-length motion-plan)))
    (setf (rx-win-display-object window) motion-plan)
    (when (zerop (length path))
      (error "Cannot view empty plan"))
    (with-win-lock window
      (with-foreign-simple-vector (pointer length) path :input
        (declare (ignore length))
        (aa-rx-win-set-display-plan window m-sg n-path pointer)))))

(defun win-display-mp-seq (mp-seq)
  (let ((window (win-create)))
    (setf (rx-win-display-object window) mp-seq)
    (with-win-lock window
      (aa-rx-win-set-display-seq window mp-seq))))

(defun win-display-motion-plan-sequence (motion-plans)
  (let ((window (win-create)))
    (win-display-mp-seq (fold (lambda (mp-seq motion-plan)
                                (aa-rx-win-sg-gl-init window (motion-plan-mutable-scene-graph motion-plan))
                                (mp-seq-append-mp mp-seq motion-plan))
                              (make-mp-seq)
                              motion-plans))))

(defun win-tf-camera (&optional (window (window)))
  (let ((vec (make-vec 7)))
    (declare (dynamic-extent vec))
    (cffi:with-pointer-to-vector-data (vec vec)
      (aa-rx-win-get-tf-cam window vec))
    (tf vec)))

(defun (setf win-tf-camera) (value &optional (win (window)))
  (let ((vec (make-vec 7)))
    (declare (dynamic-extent vec))
    (tf-array (tf value) vec)
    (cffi:with-pointer-to-vector-data (vec vec)
      (aa-rx-win-set-tf-cam win vec))
    (tf vec)))

(defun win-config-map (&optional (window (window)))
  (mutable-scene-graph-config-map (rx-win-mutable-scene-graph window)
                                  (rx-win-get-config window)))





;;;;;;;;;;;;;;;;;;
;;; Raytracing ;;;
;;;;;;;;;;;;;;;;;;

(defvar *win-frame* 0)
(defvar *win-render* nil)

(defun actual-render-callback ()
  (let* ((frame *win-frame*)
         (render *win-render*)
         (filename (format nil "frame-~D.pov" frame)))
    (incf *win-frame*)
    (sb-thread:make-thread
     (lambda ()
       (format t "~&writing frame-~D.pov~%" frame)
       (render-win :directory nil
                   :render render
                   :output filename)
       (format t "~&finished frame-~D.pov~%" frame)
       (finish-output *standard-output*))))
  0)

(defun actual-reload-callback ()
  (let* ((win (window))
         (sg0 (win-scene-graph win))
         (e (rx-win-end-effector win))
         (q (win-config-map win)))
    (flet ((helper ()
             (win-set-scene-graph (reload-scene-graph sg0)
                                  :configuration-map q
                                  :end-effector e
                                  :window win)))
      (if (uiop/os:getenv "AARXC")
          (handler-case (progn
                          (helper)
                          (format t "~&Reloaded ~{'~A'~^, ~}~%" (tree-set-list (scene-graph-files sg0))))
            (condition (e)
              (format *error-output* "~&~A~%" e)))
          (helper)))))

(cffi:defcallback render-callback :void
    ((context :pointer) (params :pointer))
  (declare (ignore context params))
  (actual-render-callback)
  0)


(cffi:defcallback reload-callback :void
    ((context :pointer) (params :pointer))
  (declare (ignore context params))
  (actual-reload-callback)
  0)

(defun render-win (&key
                     (window (window))
                     (camera-tf (win-tf-camera window))
                     (render t)
                     (options (render-options-default))
                     (configuration-map (win-config-map window))
                     output
                     (directory *robray-tmp-directory*)
                     include
                     (default-configuration 0d0))
  "Render the scene graph currently displayed in WINDOW"
  ;; TODO: configurations
  (render-scene-graph (mutable-scene-graph-scene-graph (rx-win-mutable-scene-graph window))
                      :camera-tf camera-tf
                      :render render
                      :options options
                      :configuration-map configuration-map
                      :output output
                      :directory directory
                      :include include
                      :default-configuration default-configuration))

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

;;; GEOMETRY DRAWING ;;;

(defun octet-color (r g b)
  (list (/ r 255)
        (/ g 255)
        (/ b 255)))

(defun draw-tf-axis (axis &optional (translation (identity-vec3)))
  "Construct a TF given the axis for Z in the parent frame."
  (tf* (quaternion-from-vectors (vec 0d0 0d0 1d0)
                                axis)
                      translation))

(defun draw-subframe (parent name)
  (format nil "~A/~A" parent name))

(defun item-cylinder-axis (parent name &key height radius axis (translation (identity-vec3))
                                  options)
  "Draw a cylinder in a new frame given an axis."
  (scene-frame-fixed parent name
                     :tf (draw-tf-axis axis translation)
                     :geometry (scene-geometry-cylinder options  :height height :radius radius)))


(defun item-cone-axis (parent name
                       &key height start-radius (end-radius 0d0) axis (translation (identity-vec3))
                         options)
  "Draw a cone in a new frame given an axis."
  (scene-frame-fixed parent name
                     :tf (draw-tf-axis axis translation)
                     :geometry (scene-geometry-cone options :height height
                                                    :start-radius start-radius
                                                    :end-radius end-radius)))




(defun item-arrow-axis (parent name
                        &key
                          axis
                          length
                          width
                          end-arrow
                          start-arrow
                          (end-arrow-start-width (* 2 width))
                          (end-arrow-end-width 0d0)
                          (end-arrow-length width)
                          (start-arrow-start-width (* 2 width))
                          (start-arrow-end-width 0d0)
                          (start-arrow-length width)
                          (translation (identity-vec3))
                          options)

  "Draw an arrow in a new frame given an axis."
  (let ((body-length (- length
                        (if start-arrow start-arrow-length 0)
                        (if end-arrow end-arrow-length 0))))

    (nconc (list (item-cylinder-axis parent name
                                     :options options
                                     :height body-length :radius (/ width 2)
                                     :axis axis
                                     :translation (if start-arrow
                                                      (g+ (g* start-arrow-length axis)
                                                          translation)
                                                      translation)))
           (when start-arrow
             (list (item-cone-axis parent (draw-subframe name "start-arrow")
                                   :options options
                                   :height start-arrow-length
                                   :start-radius (/ start-arrow-start-width 2)
                                   :end-radius (/ start-arrow-end-width 2)
                                   :axis (g* -1 axis)
                                   :translation  (g+ (g* axis start-arrow-length)
                                                             translation))))

           (when end-arrow
             (list (item-cone-axis parent (draw-subframe name "end-arrow")
                                   :options options
                                   :height end-arrow-length
                                   :start-radius (/ end-arrow-start-width 2)
                                   :end-radius (/ end-arrow-end-width 2)
                                   :axis axis
                                   :translation  (g+ (g* axis (- length end-arrow-length))
                                                     translation)))))))


(defun draw-arrow-points (scene-graph tail tip name &key
                                                      options
                                                      width
                                                      end-arrow
                                                      start-arrow
                                                      (start-translation (identity-vec3))
                                                      (end-translation (identity-vec3))
                                                      (end-arrow-start-width (* 2 width))
                                                      (end-arrow-end-width 0d0)
                                                      (end-arrow-length width)
                                                      (start-arrow-start-width (* 2 width))
                                                      (start-arrow-end-width 0d0)
                                                      (start-arrow-length width)
                                                      configuration-map)
  "Draw an arrow given a start and end point."
  (let* ((tf-start (scene-graph-tf-absolute scene-graph tail
                                            :configuration-map configuration-map))
         (tf-end  (scene-graph-tf-absolute scene-graph tip
                                           :configuration-map configuration-map))
         (tf-rel (tf-mul (tf-inverse (tf-mul tf-start
                                             (tf* nil start-translation)))
                         (tf-mul tf-end
                                 (tf* nil end-translation))))
         (v (tf-translation tf-rel)))
    (item-arrow-axis tail name
                     :options options
                     :axis (vec3-normalize v)
                     :length (vec-norm v)
                     :width width
                     :translation  start-translation
                     :end-arrow end-arrow
                     :start-arrow start-arrow
                     :end-arrow-start-width end-arrow-start-width
                     :end-arrow-end-width end-arrow-end-width
                     :end-arrow-length end-arrow-length
                     :start-arrow-start-width start-arrow-start-width
                     :start-arrow-end-width start-arrow-end-width
                     :start-arrow-length start-arrow-length)))


(defun draw-dimension-linear (scene-graph tail tip name
                              &key
                                options
                                end-arrow
                                start-arrow
                                (width .01)
                                (arrow-width (* 2 width))
                                (arrow-length width)
                                (end-arrow-start-width arrow-width)
                                (end-arrow-end-width 0d0)
                                (end-arrow-length arrow-length)
                                (start-arrow-start-width arrow-width)
                                (start-arrow-end-width 0d0)
                                (start-arrow-length arrow-length)
                                (offset '(0 0 .33))
                                extension-width
                                (extension-length 0)
                                (extension-options (draw-options-default))
                                configuration-map)
  "Draw arrow orthogonal to OFFSET"
  (let* ((v (tf-translation (scene-graph-tf-relative scene-graph tail tip
                                                     :configuration-map configuration-map)))
         (y (amino::tf-orth v offset)))
    (nconc
     (when extension-width
       (list
        (item-cylinder-axis tail (draw-subframe name "extension-0")
                            :options extension-options
                            :height (+ (vec-norm offset) extension-length)
                            :radius (/ extension-width 2)
                            :axis offset)
        (item-cylinder-axis tail (draw-subframe name "extension-1")
                            :translation v
                            :options extension-options
                            :height (+ (vec-norm offset) extension-length)
                            :radius (/ extension-width 2)
                            :axis offset)))
     (item-arrow-axis tail name
                      :options options
                      :translation offset
                      :axis (vec3-normalize y)
                      :length (vec-norm y)
                      :width width
                      :end-arrow end-arrow
                      :start-arrow start-arrow
                      :end-arrow-start-width end-arrow-start-width
                      :end-arrow-end-width end-arrow-end-width
                      :end-arrow-length end-arrow-length
                      :start-arrow-start-width start-arrow-start-width
                      :start-arrow-end-width start-arrow-end-width
                      :start-arrow-length start-arrow-length))))


(defun draw-dimension-angular (scene-graph parent name
                               &key
                                 rotation
                                 options
                                 radius
                                 width
                                 offset
                                 angle
                                 end-arrow
                                 (arrow-width (* 2 width))
                                 (arrow-length (* 2 width))
                                 (end-arrow-start-width arrow-width)
                                 (end-arrow-length arrow-length)
                                 start-arrow
                                 (start-arrow-start-width arrow-width)
                                 (start-arrow-length arrow-length)
                                 )
  (declare (ignore scene-graph))
  (let* ((start-angle (if start-arrow
                          (/ start-arrow-length radius)
                          0))
         (angle (- angle
                   (if end-arrow
                       (/ end-arrow-length radius)
                       0)
                   start-angle)))

    (nconc
     (list
      (scene-frame-fixed parent name
                         ;; TODO: offset rotation
                         :tf (tf* rotation offset))

      (scene-frame-fixed name (draw-subframe name "body")
                         :tf (tf* (z-angle start-angle)
                                  nil)
                         :geometry
                         (scene-geometry-torus options
                                               :angle angle
                                               :major-radius radius
                                               :minor-radius (/ width 2))))
     (when start-arrow
       (list (item-cone-axis (draw-subframe name "body") (draw-subframe name "start-arrow")
                             :options options
                             :height start-arrow-length
                             :start-radius (/ start-arrow-start-width 2)
                             :end-radius 0
                             :axis (vec3* 0 -1 0)
                             :translation (vec3* radius 0 0)
                             )))
     (when end-arrow
       (let ((R (rotation-matrix (z-angle angle))))
         (list (item-cone-axis (draw-subframe name "body") (draw-subframe name "end-arrow")
                               :options options
                               :height end-arrow-length
                               :start-radius (/ end-arrow-start-width 2)
                               :end-radius 0
                               :axis (transform R (vec3* 0 1 0))
                               :translation (transform R (vec3* radius 0 0))
                               ))))
     )))




(defun item-frame-marker (parent name
                          &key
                            length
                            (width (/ length 10))
                            (arrow-width (* 2 width))
                            (arrow-length (* 1 arrow-width))
                            (x-color '(1 0 0))
                            (y-color '(0 1 0))
                            (z-color '(0 0 1))
                            options)
  "Draw principal axis markers for frame PARENT."
  (flet ((helper (subname axis color)
           (item-arrow-axis parent (draw-subframe name subname)
                            :options (merge-draw-options (draw-options :color color)
                                                         options)
                            :axis axis
                            :length length
                            :width width
                            :end-arrow t
                            :end-arrow-start-width arrow-width
                            :end-arrow-length arrow-length)))

  (append (helper "x" (vec3* 1 0 0) x-color )
          (helper "y" (vec3* 0 1 0) y-color )
          (helper "z" (vec3* 0 0 1) z-color ))))


;; (defun draw-geometry (scene-graph parent name
;;                       &key
;;                         geometry
;;                         tf
;;                         (actual-parent parent)
;;                         (options *draw-options*))
;;   (scene-graph-add-tf scene-graph (tf-tag parent tf name)
;;                       :actual-parent actual-parent
;;                       :geometry (scene-geometry geometry options)))

;; (defun draw-items (scene-graph parent items
;;                    &key
;;                      (options *draw-options*))
;;   (fold (lambda (scene-graph x)
;;           (let ((frame (copy-structure x)))
;;             (unless (scene-frame-parent frame)
;;               (setf (scene-frame-parent frame)
;;                     parent))
;;             (setf (scene-frame-geometry frame)
;;                   (map 'list (lambda (g)
;;                                (scene-geometry (scene-geometry-shape g)
;;                                                (merge-draw-options (merge-draw-options (scene-geometry-options g)
;;                                                                                        options))))
;;                        (scene-frame-geometry frame)))
;;             (scene-graph scene-graph frame)))
;;         scene-graph
;;         (ensure-list items)))

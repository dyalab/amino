(in-package :robray)

;; Define hosts for rendering
(setq *render-host-alist*
      '(("localhost"  ; 4 core (HT), 3.6GHz
         :jobs 8 :threads 1 :nice 0)
        ("dione"      ; 12 core, 1.4GHz
         :jobs 6 :threads 2 :nice 0)
        ("zeus"       ; 16 core, 2.4GHz
         :jobs 7 :threads 2 :nice 1 :povray "/home/ndantam/local/bin/povray")
        ))
;; Define search path for URDF
(setq *urdf-package-alist*
      `(("baxter_description" . ,(concatenate 'string (namestring (user-homedir-pathname))
                                              "ros_ws/src/baxter_common/baxter_description"))))

;; Load robot scene graph from URDF
(defvar *scene-graph-baxter*)
(setq *scene-graph-baxter*
      (urdf-parse "/home/ntd/ros_ws/src/baxter_common/baxter_description/urdf/baxter.urdf"))

(setq *scene-graph*
      (scene-graph-add-frame *scene-graph-baxter*
                             (scene-frame-fixed nil "table"
                                                :tf (tf* nil (vec3* 1 0 0)))))

(setq *scene-graph*
      (scene-graph-add-visual *scene-graph* "table"
                              (make-scene-visual :geometry (make-scene-box :dimension (list 1 1 .01))
                                                 :color (list 1 0 0)
                                                 :alpha 0.1d0)))


;; (setq *scene-graph*
;;       (scene-graph-

;; Produce a simple animation
;; (time (scene-graph-time-animate
;;  (keyframe-configuration-function (list
;;                                    (joint-keyframe 0d0 nil)
;;                                    (joint-keyframe 2d0 `(("right_s0" ,(* .25 pi))
;;                                                          ("right_s1" ,(* -0.25 pi))
;;                                                          ("right_e0" ,(* 0.0 pi))
;;                                                          ("right_e1" ,(* 0.25 pi))
;;                                                          ("right_w0" ,(* 0.0 pi))
;;                                                          ("right_w1" ,(* 0.5 pi))
;;                                                          ("right_w2" ,(* 0.0 pi))))))
;;  :frames-per-second 15
;;  :time-end 2d0
;;  :encode-video t
;;  :include "baxter.inc" ))





(pov-render (scene-graph-pov-frame  *scene-graph*
                                    :configuration-map
                                    (alist-tree-map `(("right_s0" . ,(* .25 pi))
                                                      ("right_s1" . ,(* -0.25 pi))
                                                      ("right_e0" . ,(* 0.0 pi))
                                                      ("right_e1" . ,(* 0.25 pi))
                                                      ("right_w0" . ,(* 0.0 pi))
                                                      ("right_w1" . ,(* 0.5 pi))
                                                      ("right_w2" . ,(* 0.0 pi)))
                                                    #'string-compare)
                                    :include "/tmp/robray/baxter.inc" )
            :options (render-options-default :height 1080
                                             :width 1920)
            :file "robray.pov")

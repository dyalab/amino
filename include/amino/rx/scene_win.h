/* -*- mode: C; c-basic-offset: 4; -*- */
/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*
 * Copyright (c) 2015, Rice University
 * Copyright (c) 2019, Colorado School of Mines
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

#ifndef AMINO_RX_SCENE_WIN_H
#define AMINO_RX_SCENE_WIN_H

#include <SDL.h>



struct aa_rx_sg_sub;

/**
 * @file scene_win.h
 * @brief SDL/OpenGL window management
 */

/**
 * Create a new SDL / OpenGL window.
 *
 * @param title  The window title
 * @param x_pos  Initial X position of the window
 * @param y_pos  Initial Y position of the window
 * @param width  Initial width of the window
 * @param height Initial height of the window
 * @param flags  Flags for SDL_CreateWindow()
 */
AA_API struct aa_rx_win *
aa_rx_win_create(
    const char* title,
    int x_pos,
    int y_pos,
    int width,
    int height,
    Uint32 flags );

/**
 * Create a new SDL / OpenGL window with default parameters.
 *
 * On Mac OS X, this function may only be called from the main thread.
 *
 * @param title   The window title
 * @param width   Initial width of the window
 * @param height  Initial height of the window
 */
AA_API struct aa_rx_win *
aa_rx_win_default_create(
    const char* title, int width, int height );


/**
 * Destroy SDL / OpenGL window.
 */
AA_API void
aa_rx_win_destroy( struct aa_rx_win *  win);

/**
 * Return a pointer to the window's GL globals struct.
 *
 * Note that the window's rendering thread may asynchronously access
 * the GL globals.  If you need to modify any parameters, synchronize
 * access by locking the window.
 *
 * \sa aa_rx_win_lock
 * \sa aa_rx_win_unlock
 */
AA_API struct aa_gl_globals *
aa_rx_win_gl_globals( struct aa_rx_win * win);


/**
 * Set a scenegraph for the window.
 *
 * The scenegraph is used by the default rendering function.  Custom
 * rendering functions may not need to set the window scene graph
 * first.
 */
AA_API void
aa_rx_win_set_sg( struct aa_rx_win * win,
                  const struct aa_rx_sg *sg );

/**
 * Set a sub-scenegraph for the window.
 *
 * The scenegraph is used by the default rendering function.  Custom
 * rendering functions may not need to set the window scene graph
 * first.
 *
 * The end-effector of the sub-scenegraph can be interactively
 * controlled in workspace.
 */
AA_API void
aa_rx_win_set_sg_sub( struct aa_rx_win * win,
                      const struct aa_rx_sg_sub *sg_sub );

/**
 * Retreive the scene graph for the window.
 */
AA_API const struct aa_rx_sg *
aa_rx_win_get_sg( struct aa_rx_win * win );

/**
 * Render a scenegraph at the given configuration in the window.
 */
AA_API void
aa_rx_win_display_sg_config( struct aa_rx_win *win, struct aa_sdl_display_params *params,
                             const struct aa_rx_sg *scenegraph,
                             size_t n_q, const double *q );


/**
 * Display a scene graph in the window with the given transforms.
 */
AA_API void
aa_rx_win_display_sg_tf( struct aa_rx_win *win, struct aa_sdl_display_params *params,
                         const struct aa_rx_sg *scenegraph,
                         size_t n_tf, const double *tf_abs, size_t ld_tf );


/**
 * Display a scene graph in the window with the forward kinematics.
 */
AA_API void
aa_rx_win_display_fk( struct aa_rx_win *win, struct aa_sdl_display_params *params,
                      const struct aa_rx_fk *fk );

/**
 * Set the configuration vector for the window.
 *
 * This configuration is used by the default rendering function.
 * Custom rendering functions may not need to set the configuration with this function.
 */
AA_API void
aa_rx_win_set_config( struct aa_rx_win * win,
                      size_t n,
                      const double *q );

/**
 * Set the configuration vector (block-vector version) for the window.
 *
 * This configuration is used by the default rendering function.
 * Custom rendering functions may not need to set the configuration with this function.
 */
AA_API void
aa_rx_win_set_bconfig( struct aa_rx_win * win,
                       const struct aa_dvec *q );

/**
 * Set the window's display function
 *
 * @param win The window
 *
 * @param display The display function
 *
 * @param context The context argument for the display function
 *
 * @param destructor A cleanup function that is called on context when
 *                   either another display function is set for the
 *                   window or the window is destroyed.
 */
AA_API void
aa_rx_win_set_display( struct aa_rx_win * win,
                       aa_sdl_win_display_fun display,
                       void *context,
                       void (*destructor)(void*) );

/**
 * Display a motion plan in the window.
 */
AA_API void
aa_rx_win_set_display_plan( struct aa_rx_win * win,
                            struct aa_rx_sg *sg,
                            size_t n_plan_elements,
                            const double *plan );

struct aa_rx_mp_seq;

/**
 * Display a sequence of motion plans.
 */
AA_API void
aa_rx_win_set_display_seq( struct aa_rx_win * win, struct aa_rx_mp_seq *mp_seq);

/**
 * Synchronous display using current thread
 *
 * This function is thread-safe.  The window will be locked while each
 * window's display() function is called.
 *
 * On Mac OS X under the Cocoa/Quartz GUI, this function may only be
 * called from the main thread.
 *
 */
AA_API void aa_rx_win_run( void );


/**
 * Asynchronous run the display in a new current thread.
 *
 * This function is thread-safe.  The window will be locked while each
 * window's display() function is called.
 *
 * This function will not work on Mac OS X under the Cocoa GUI because
 * Cocoa limits GUI interaction to the main program thread.  Instead,
 * you must call aa_rx_win_run() from your program's main thread.
 *
 */
AA_API void aa_rx_win_run_async( void );

/**
 * Instruct the rendering thread to stop.
 *
 * The rendering thread will gracefully terminate, possibly after it
 * finishes displaying another frame.
 */
AA_API void
aa_rx_win_stop( struct aa_rx_win * win );

/**
 * Return true if the window is still running.
 */
AA_API int
aa_rx_win_is_running( struct aa_rx_win * win );


/**
 * Instruct the rendering thread to stop when the user closes the window.
 */
AA_API void
aa_rx_win_stop_on_quit( struct aa_rx_win * win, int value );

/**
 * Lock the window.
 *
 * The rendering thread also takes this lock before calling the
 * display function.
 */
AA_API void
aa_rx_win_lock( struct aa_rx_win * win );

/**
 * Unlock the window
 */
AA_API void
aa_rx_win_unlock( struct aa_rx_win * win );

/**
 * Initialize scenegraph GL values for the given window.
 *
 * This function is threadsafe.
 */
AA_API void
aa_rx_win_sg_gl_init( struct aa_rx_win * win,
                      struct aa_rx_sg *sg );


/**
 * Return the current camera pose.
 *
 * @param win The window
 * @param E   The camera pose in quaternion-vector format
 */
AA_API void
aa_rx_win_get_tf_cam( struct aa_rx_win * win, double *E );

/**
 * Set the camera pose.
 *
 * @param win The window
 * @param E   The camera pose in quaternion-vector format
 */
AA_API void
aa_rx_win_set_tf_cam( struct aa_rx_win * win, const double *E );


#endif /*AMINO_RX_SCENE_WIN_H*/

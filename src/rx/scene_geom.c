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


#include "config.h"
#include "amino.h"

#include "amino/rx/rxtype.h"
#include "amino/rx/rxtype_internal.h"

#include "amino/rx/scenegraph.h"
#include "amino/rx/scenegraph_internal.h"
#include "amino/rx/scene_geom.h"
#include "amino/rx/scene_geom_internal.h"

#include "sg_convenience.h"

void
(*aa_gl_buffers_destroy_fun)( struct aa_gl_buffers *bufs ) = NULL;

void
(*aa_rx_cl_geom_destroy_fun)( struct aa_rx_cl_geom *bufs ) = NULL;

#define ALLOC_GEOM(TYPE, var, type_value, geom_opt )            \
    TYPE *var = AA_NEW0(TYPE);                                  \
    AA_MEM_CPY(&g->base.opt, geom_opt, 1);                      \
    var->base.type = type_value;                                \
    var->base.gl_buffers = NULL;                                \
    var->base.refcount = 1;

//aa_rx_sg_add_geom(sg, frame, &var->base);

struct aa_rx_geom *
aa_rx_geom_box (
    struct aa_rx_geom_opt *opt,
    const double dimension[3] )
{
    ALLOC_GEOM( struct aa_rx_geom_box, g,
                AA_RX_BOX, opt );
    AA_MEM_CPY(g->shape.dimension, dimension, 3);
    return &g->base;
}

struct aa_rx_geom *
aa_rx_geom_sphere (
    struct aa_rx_geom_opt *opt,
    double radius )
{
    ALLOC_GEOM( struct aa_rx_geom_sphere, g,
                AA_RX_SPHERE, opt );
    g->shape.radius = radius;
    return &g->base;
}

struct aa_rx_geom *
aa_rx_geom_cylinder (
    struct aa_rx_geom_opt *opt,
    double height,
    double radius )
{
    ALLOC_GEOM( struct aa_rx_geom_cylinder, g,
                AA_RX_CYLINDER, opt );
    g->shape.radius = radius;
    g->shape.height = height;
    return &g->base;
}

struct aa_rx_geom *
aa_rx_geom_cone (
    struct aa_rx_geom_opt *opt,
    double height,
    double start_radius,
    double end_radius )
{
    ALLOC_GEOM( struct aa_rx_geom_cone, g,
                AA_RX_CONE, opt );
    g->shape.start_radius = start_radius;
    g->shape.end_radius = end_radius;
    g->shape.height = height;
    return &g->base;
}


struct aa_rx_geom *
aa_rx_geom_grid (
    struct aa_rx_geom_opt *opt,
    const double dimension[2],
    const double delta[2],
    double width )
{
    ALLOC_GEOM( struct aa_rx_geom_grid, g,
                AA_RX_GRID, opt );
    AA_MEM_CPY(g->shape.dimension, dimension, 2);
    AA_MEM_CPY(g->shape.delta, delta, 2);
    g->shape.width = width;
    return &g->base;
}

struct aa_rx_geom *
aa_rx_geom_torus (
    struct aa_rx_geom_opt *opt,
    double angle,
    double major_radius,
    double minor_radius
    )
{
    ALLOC_GEOM( struct aa_rx_geom_torus, g,
                AA_RX_TORUS, opt );
    g->shape.angle = angle;
    g->shape.major_radius = major_radius;
    g->shape.minor_radius = minor_radius;

    return &g->base;
}

struct aa_rx_geom *
aa_rx_geom_mesh (
    struct aa_rx_geom_opt *opt,
    struct aa_rx_mesh *mesh )
{
    aa_mem_ref_inc( &mesh->refcount );
    ALLOC_GEOM( struct aa_rx_geom_mesh, g,
                AA_RX_MESH, opt );
    g->shape = mesh;
    return &g->base;
}

void
aa_rx_geom_attach (
    struct aa_rx_sg *sg,
    const char *frame,
    struct aa_rx_geom *geom )
{
    aa_rx_sg_add_geom(sg, frame, geom);
}

struct aa_rx_geom *
aa_rx_geom_copy( struct aa_rx_geom *src )
{
    unsigned oldcount = aa_mem_ref_inc(&src->refcount);
    if( 0 == oldcount ) {
        fprintf(stderr, "Error, copied geom with 0 refcount\n");
        abort();
    }
    return src;
}

struct aa_rx_geom *
aa_rx_geom_modify_opt( struct aa_rx_geom *src, struct aa_rx_geom_opt *opt )
{
    struct aa_rx_geom *result = NULL;

    enum aa_rx_geom_shape shape_type;
    void *vshape = aa_rx_geom_shape(src,&shape_type);

    switch(src->type) {
    case AA_RX_NOSHAPE:
        fprintf(stderr, "Error, geom has no shape type\n");
        break;
    case AA_RX_MESH: {
        struct aa_rx_mesh *shape = (struct aa_rx_mesh *)vshape;
        result = aa_rx_geom_mesh( opt, shape );
    }
        break;
    case AA_RX_BOX: {
        struct aa_rx_shape_box *shape = (struct aa_rx_shape_box *)vshape;
        result = aa_rx_geom_box( opt, shape->dimension );
    }
        break;
    case AA_RX_SPHERE: {
        struct aa_rx_shape_sphere *shape = (struct aa_rx_shape_sphere *)vshape;
        result = aa_rx_geom_sphere( opt, shape->radius );
    }
        break;
    case AA_RX_CYLINDER: {
        struct aa_rx_shape_cylinder *shape = (struct aa_rx_shape_cylinder *)vshape;
        result = aa_rx_geom_cylinder( opt, shape->height, shape->radius );
    }
        break;
    case AA_RX_CONE: {
        struct aa_rx_shape_cone *shape = (struct aa_rx_shape_cone *)vshape;
        result = aa_rx_geom_cone( opt, shape->height,
                                  shape->start_radius, shape->end_radius );
    }
        break;
    case AA_RX_GRID: {
        struct aa_rx_shape_grid *shape = (struct aa_rx_shape_grid *)vshape;
        result = aa_rx_geom_grid( opt, shape->dimension,
                                  shape->delta, shape->width );
    }
        break;
    case AA_RX_TORUS: {
        struct aa_rx_shape_torus *shape = (struct aa_rx_shape_torus *)vshape;
        result = aa_rx_geom_torus( opt, shape->angle,
                                   shape->major_radius, shape->minor_radius );
    }
        break;
    }

    if( NULL == result ) {
        fprintf(stderr, "Error, unhandled shape type\n");
        abort();
    }

    return result;
}

void
aa_rx_geom_destroy( struct aa_rx_geom *geom )
{
    unsigned oldcount = aa_mem_ref_dec(&geom->refcount);

    if( 0 == oldcount) {
        fprintf(stderr, "Error, destroying geom with 0 refcount\n");
        abort();
    }

    if( 1 == oldcount ) {
        /* Free Mesh */
        if( AA_RX_MESH == geom->type ) {
            struct aa_rx_geom_mesh *mesh_geom = (struct aa_rx_geom_mesh *)geom;
            aa_rx_mesh_destroy(mesh_geom->shape);
        }
        /* Free collision */
        if( geom->cl_geom ) {
            aa_rx_cl_geom_destroy_fun( geom->cl_geom );
        }
        /* Free gl */
        if( geom->gl_buffers ) {
            aa_gl_buffers_destroy_fun(geom->gl_buffers);
        }

        free(geom);
    }
}

void *
aa_rx_geom_shape ( const struct aa_rx_geom *g,
                   enum aa_rx_geom_shape *shape_type )
{
    void *shape = NULL;
    switch(g->type ) {
    case AA_RX_NOSHAPE: break;
    case AA_RX_MESH:
        shape = ((struct aa_rx_geom_mesh*)g)->shape;
        break;
    case AA_RX_BOX:
        shape = &((struct aa_rx_geom_box*)g)->shape;
        break;
    case AA_RX_SPHERE:
        shape = &((struct aa_rx_geom_sphere*)g)->shape;
        break;
    case AA_RX_CYLINDER:
        shape = &((struct aa_rx_geom_cylinder*)g)->shape;
        break;
    case AA_RX_CONE:
        shape = &((struct aa_rx_geom_cone*)g)->shape;
        break;
    case AA_RX_GRID:
        shape = &((struct aa_rx_geom_grid*)g)->shape;
        break;
    case AA_RX_TORUS:
        shape = &((struct aa_rx_geom_torus*)g)->shape;
        break;
    }
    if( shape_type ) *shape_type = g->type;
    return shape;
}


AA_API const struct aa_rx_geom_opt*
aa_rx_geom_get_opt ( const struct aa_rx_geom *geom )
{
    return &geom->opt;
}

AA_API struct aa_rx_cl_geom *
aa_rx_geom_get_collision ( const struct aa_rx_geom *geom )
{
    return geom->cl_geom;
}


AA_API void
aa_rx_geom_set_collision ( struct aa_rx_geom *geom,  struct aa_rx_cl_geom * obj)
{
    geom->cl_geom = obj;
}


struct aa_rx_mesh* aa_rx_mesh_create()
{
    struct aa_rx_mesh *mesh = AA_NEW0( struct aa_rx_mesh );
    mesh->refcount = 1;
    return mesh;
}

void aa_rx_mesh_destroy( struct aa_rx_mesh * mesh )
{
    unsigned oldcount = aa_mem_ref_dec(&mesh->refcount);
    if( 1 == oldcount ) {
        if( mesh->destructor ) {
            mesh->destructor( mesh->destructor_context );
        }

        aa_checked_free(mesh->vertices_data);
        aa_checked_free(mesh->normals_data);
        aa_checked_free(mesh->indices_data);
        aa_checked_free(mesh->rgba_data);
        aa_checked_free(mesh->uv_data);
        free(mesh);
    }
}



#define MESH_SET_THINGN( TYPE, THING, LD, N )                   \
    void aa_rx_mesh_set_ ## THING (                             \
        struct aa_rx_mesh *mesh, size_t n,                      \
        const TYPE * THING, int copy )                          \
    {                                                           \
        if(mesh->THING##_data) {                                \
            free(mesh->THING##_data);}                          \
        mesh->N = n;                                            \
        if( copy ) {                                            \
            mesh->THING##_data = AA_NEW_AR(TYPE, LD*n);         \
            AA_MEM_CPY(mesh->THING##_data, THING, LD*n);        \
            mesh->THING = mesh->THING##_data;                   \
        } else {                                                \
            mesh->THING ## _data = NULL;                        \
            mesh->THING = THING;                                \
        }                                                       \
    }


#define MESH_SET_THING( TYPE, THING, LD )                       \
    void aa_rx_mesh_set_ ## THING (                             \
        struct aa_rx_mesh *mesh, size_t n,                      \
        const TYPE * THING, int copy )                          \
    {                                                           \
        if(mesh->THING##_data) {                                \
            free(mesh->THING##_data);}                          \
        if( copy ) {                                            \
            mesh->THING##_data = AA_NEW_AR(TYPE, LD*n);         \
            AA_MEM_CPY(mesh->THING##_data, THING, LD*n);        \
            mesh->THING = mesh->THING##_data;                   \
        } else {                                                \
            mesh->THING ## _data = NULL;                        \
            mesh->THING = THING;                                \
        }                                                       \
    }

MESH_SET_THINGN( float, vertices, 3, n_vertices)
MESH_SET_THING( float, normals, 3 )
MESH_SET_THING( float, uv, 2 )
MESH_SET_THINGN( unsigned, indices, 3, n_indices )

AA_API const float *aa_rx_mesh_get_vertices (
    const struct aa_rx_mesh *mesh, size_t *size )
{
    if (size) *size = mesh->n_vertices;
    return mesh->vertices;
}

AA_API const unsigned *aa_rx_mesh_get_indices (
    const struct aa_rx_mesh *mesh, size_t *size )
{
    if (size) *size = mesh->n_indices;
    return mesh->indices;
}

void aa_rx_mesh_set_rgba (
    struct aa_rx_mesh *mesh,
    size_t width, size_t height,
    const uint8_t *rgba, int copy )
{
    if(mesh->rgba_data) free(mesh->rgba_data);
    if( copy ) {
        mesh->rgba_data = AA_NEW_AR(uint8_t, 4*width*height);
        AA_MEM_CPY(mesh->rgba_data, rgba, 4*width*height);
        mesh->rgba = mesh->rgba_data;
    } else {
        mesh->rgba_data = NULL;
        mesh->rgba = rgba;
    }
    mesh->width_rgba = width;
    mesh->height_rgba = height;
}

void aa_rx_mesh_set_texture (
    struct aa_rx_mesh *mesh,
    const struct aa_rx_geom_opt *opt )
{
    // color
    uint8_t rgba[4];
    for( size_t i = 0; i < 4; i ++ ) rgba[i] = (uint8_t)(opt->color[i] * 255);
    aa_rx_mesh_set_rgba( mesh, 1, 1, rgba, 1 );

    // indices
    /* if( mesh->uv_data ) free(mesh->uv_data); */
    mesh->uv = mesh->uv_data = AA_NEW0_AR(float, 2*mesh->n_vertices);
}

const char *
aa_rx_geom_shape_str( enum aa_rx_geom_shape shape )
{
    switch( shape ) {
    case AA_RX_NOSHAPE: return "none";
    case AA_RX_MESH: return "mesh";
    case AA_RX_BOX: return "box";
    case AA_RX_SPHERE: return "sphere";
    case AA_RX_CYLINDER: return "cylinder";
    case AA_RX_CONE: return "cone";
    case AA_RX_GRID: return "grid";
    case AA_RX_TORUS: return "torus";
    }
    return "?";
}


AA_API unsigned
aa_rx_geom_refcount ( const struct aa_rx_geom *g ) {
    return g->refcount;
}

AA_API unsigned
aa_rx_mesh_refcount ( const struct aa_rx_mesh *m )
{
    return  m->refcount;
}

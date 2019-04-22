/*
 * types.h
 *
 *  Created on: 18 апр. 2019 г.
 *      Author: sadko
 */

#ifndef RENDERING_TYPES_H_
#define RENDERING_TYPES_H_

#include <core/types.h>

enum r3d_window_handle_t
{
    R3D_WND_HANDLE_X11,
    R3D_WND_HANDLE_WINNT
};

enum r3d_matrix_type_t
{
    R3D_MATRIX_PROJECTION,  /* Projection matrix */
    R3D_MATRIX_VIEW,        /* View matrix */
    R3D_MATRIX_WORLD        /* World matrix for additional transformations if view matrix is not enough */
};

enum r3d_light_type_t {
    R3D_LIGHT_NONE,
    R3D_LIGHT_POINT,
    R3D_LIGHT_DIRECTIONAL,
    R3D_LIGHT_SPOT
};

enum r3d_primitive_type_t {
    R3D_PRIMITIVE_TRIANGLES,
    R3D_PRIMITIVE_WIREFRAME_TRIANGLES,
    R3D_PRIMITIVE_LINES,
    R3D_PRIMITIVE_POINTS,
};

// Basic type: backend
struct r3d_backend_t;

// Light parameters
typedef struct r3d_light_t
{
    r3d_light_type_t    type;           /* Light type */
    vector3d_t          position;       /* Light position */
    vector3d_t          direction;      /* Light direction */
    color3d_t           ambient;        /* Ambient color */
    color3d_t           diffuse;        /* Diffuse color */
    color3d_t           specular;       /* Specular color */
    float               constant;       /* Constant attenuation parameter */
    float               linear;         /* Linear attenuation parameter */
    float               quadratic;      /* Quadratic attenuation parameter */
    float               cutoff;         /* Spot cutoff angle */
} r3d_light_t;

typedef struct r3d_buffer_t
{
    /* Properties */
    r3d_primitive_type_t    type;       // Type of primitive
    float                   size;       // Point size or line width
    size_t                  count;      // Number of elements in buffer

    /* Vertices */
    struct {
        point3d_t          *data;
        size_t              stride;
    } vertex;

    /* Normals  */
    struct {
        vector3d_t         *data;
        size_t              stride;
    } normal;

    /* Colors */
    struct {
        color3d_t          *data;
        size_t              stride;
    } color;

    /* Vertex indices (always packed) */
    struct {
        uint32_t           *data;
    } index;
} r3d_buffer_t;

/**
 * Backend instantiation function
 * @param version required backend version
 * @return backend pointer or NULL
 */
typedef r3d_backend_t * (* lsp_r3d_instantiate_t)(const char *version);

#ifdef __cplusplus
template <typename D, typename S>
    inline void export_func(D &dst, const S &src)
    {
        union { D xdst; S xsrc; } uni;
        uni.xsrc    = src;
        dst         = uni.xdst;
    }
#endif /* __cplusplus */

#endif /* RENDERING_TYPES_H_ */

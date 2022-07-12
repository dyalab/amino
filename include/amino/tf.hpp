/* -*- mode: C++; c-basic-offset: 4 -*- */
/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*
 * Copyright (c) 2013, Georgia Tech Research Corporation
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

#ifndef AA_TF_HPP
#define AA_TF_HPP

/**
 * @file tf.hpp
 *
 * Object types for SE(3) representations.
 *
 * The class definitions in this file provide convenience methods to
 * simplify type conversions between various orientation and
 * transformation representations.  The underlying operatations (and
 * many others) are declared in tf.h.
 *
 * \sa tf.h for low level definitions of SE(3) operations.
 */

namespace amino {

/**
 * A vector of length 3.
 */
struct Vec3 : aa_tf_vec3 {
    Vec3() {}

    /**
     * Extract the translation part of a unit dual quaternion.
     */
    Vec3(const struct aa_tf_duqu *S) : aa_tf_vec3(from_duqu(S->data)) {}

    /**
     * Extract the translation part of a unit dual quaternion.
     */
    Vec3(const struct aa_tf_duqu &S) : aa_tf_vec3(from_duqu(S.data)) {}

    /**
     * Extract the translation part of a transformation matrix.
     */
    Vec3(const struct aa_tf_tfmat *T) : aa_tf_vec3(from_tfmat(T->data)) {}

    /**
     * Extract the translation part of a transformation matrix.
     */
    Vec3(const struct aa_tf_tfmat &T) : aa_tf_vec3(from_tfmat(T.data)) {}

    /**
     * Construct from individual components.
     */
    Vec3(double _x, double _y, double _z) : aa_tf_vec3(from_xyz(_x, _y, _z)) {}

    /**
     * Construct from array.
     */
    Vec3(const double *_xyz) : aa_tf_vec3(from_vec3(_xyz)) {}

    /**
     * Construct from vec3.
     */
    Vec3(const aa_tf_vec3 &src) : aa_tf_vec3(src) {}

    /**
     * Create a Vec3 from components.
     */
    static aa_tf_vec3 from_xyz(double x, double y, double z)
    {
        aa_tf_vec3 V;
        V.x = x;
        V.y = y;
        V.z = z;
        return V;
    }

    /**
     * Create a Vec3 from array.
     */
    static aa_tf_vec3 from_vec3(const double a_x[3])
    {
        aa_tf_vec3 V;
        memcpy(V.data, a_x, 3 * sizeof(V.data[0]));
        return V;
    }

    /**
     * Create a Vec3 from translation part of unit dual quation.
     */
    static aa_tf_vec3 from_duqu(const double S[8])
    {
        aa_tf_vec3 V;
        aa_tf_duqu_trans(S, V.data);
        return V;
    }

    /**
     * Create a Vec3 from translation part of transformation matrix.
     */
    static aa_tf_vec3 from_tfmat(const double T[12])
    {
        return from_vec3(T + 9);
    }
};

/*------------------------------*/
/*-------- ORIENTATIONS --------*/
/*------------------------------*/

/**
 * A rotation about the X axis.
 */
struct XAngle {
    double value;  ///< The angle value in radians

    /** Construct with the given angle */
    XAngle(double v) : value(v) {}
};

/**
 * A rotation about the Y axis.
 */
struct YAngle {
    double value;  ///< The angle value in radians

    /** Construct with the given angle */
    YAngle(double v) : value(v) {}
};

/**
 * A rotation about the Z axis.
 */
struct ZAngle {
    double value;  ///< The angle value in radians

    /** Construct with the given angle */
    ZAngle(double v) : value(v) {}
};

/**
 * A quaternion object.
 */
struct Quat : aa_tf_quat {
    /**
     * Construct an identity  quaternion
     */
    Quat() : aa_tf_quat(from_xyzw(0, 0, 0, 1)) {}

    /**
     * Construct from another quaternion.
     */
    Quat(const aa_tf_quat *p) : aa_tf_quat(from_quat(p->data)) {}

    /**
     * Construct from another quaternion.
     */
    Quat(const aa_tf_quat &p) : aa_tf_quat(from_quat(p.data)) {}

    /**
     * Construct from a rotation matrix.
     */
    Quat(const aa_tf_rotmat *p) : aa_tf_quat(from_rotmat(p->data)) {}

    /**
     * Construct from a rotation matrix.
     */
    Quat(const aa_tf_rotmat &p) : aa_tf_quat(from_rotmat(p.data)) {}

    /**
     * Construct from an axix angle rotation.
     */
    Quat(const aa_tf_axang *p) : aa_tf_quat(from_axang(p->data)) {}

    /**
     * Construct from an axix angle rotation.
     */
    Quat(const aa_tf_axang &p) : aa_tf_quat(from_axang(p.data)) {}

    /**
     * Construct from a rotation about the x axis.
     */
    Quat(const XAngle &p) : aa_tf_quat(from_xangle(p.value)) {}

    /**
     * Construct from a rotation about the Y axis.
     */
    Quat(const YAngle &p) : aa_tf_quat(from_yangle(p.value)) {}

    /**
     * Construct from a rotation about the Z axis.
     */
    Quat(const ZAngle &p) : aa_tf_quat(from_zangle(p.value)) {}

    void rotate(const aa_tf_vec3 *p, aa_tf_vec3 *q) const
    {
        aa_tf_qrot(this->data, p->data, q->data);
    }

    aa_tf_vec3 rotate(const aa_tf_vec3 &p) const
    {
        aa_tf_vec3 q;
        this->rotate(&p, &q);
        return q;
    }

    /**
     * Create a quaternion object from a quaternion array.
     */
    static aa_tf_quat from_quat(const double x[4])
    {
        aa_tf_quat y;
        memcpy(y.data, x, 4 * sizeof(y.data[0]));
        return y;
    }

    /**
     * Create a quaternion object from components.
     */
    static aa_tf_quat from_xyzw(double x, double y, double z, double w)
    {
        aa_tf_quat q;
        q.x = x;
        q.y = y;
        q.z = z;
        q.w = w;
        return q;
    }

    /**
     * Create a quaternion object a rotation matrix.
     */
    static aa_tf_quat from_rotmat(const double x[9])
    {
        aa_tf_quat y;
        aa_tf_rotmat2quat(x, y.data);
        return y;
    }

    /**
     * Create a quaternion object an axis-angle.
     */
    static aa_tf_quat from_axang(const double x[4])
    {
        aa_tf_quat y;
        aa_tf_axang2quat(x, y.data);
        return y;
    }

    /**
     * Create a quaternion object an axis-angle.
     */
    static aa_tf_quat from_axang(const double a[3], double angle)
    {
        double x[4] = {a[0], a[1], a[2], angle};
        return from_axang(x);
    }

    /**
     * Create a quaternion object a rotation vector.
     */
    static aa_tf_quat from_rotvec(const double x[3])
    {
        aa_tf_quat y;
        aa_tf_rotvec2quat(x, y.data);
        return y;
    }

    /**
     * Create a quaternion object rotation about x.
     */
    static aa_tf_quat from_xangle(const double v)
    {
        aa_tf_quat y;
        aa_tf_xangle2quat(v, y.data);
        return y;
    }

    /**
     * Create a quaternion object rotation about y.
     */
    static aa_tf_quat from_yangle(const double v)
    {
        aa_tf_quat y;
        aa_tf_yangle2quat(v, y.data);
        return y;
    }

    /**
     * Create a quaternion object rotation about z.
     */
    static aa_tf_quat from_zangle(const double v)
    {
        aa_tf_quat y;
        aa_tf_zangle2quat(v, y.data);
        return y;
    }
};

/**
 * A rotation matrix object.
 */
struct RotMat : aa_tf_rotmat {
    /**
     * Construct an identity rotation matrix.
     */
    RotMat() : aa_tf_rotmat(from_rotmat(1, 0, 0, 0, 1, 0, 0, 0, 1)) {}

    /**
     * Construct a rotation matrix from a unit quaternion.
     */
    RotMat(const aa_tf_quat *p) : aa_tf_rotmat(from_quat(p->data)) {}

    /**
     * Construct a rotation matrix from a unit quaternion.
     */
    RotMat(const aa_tf_quat &p) : aa_tf_rotmat(from_quat(p.data)) {}

    /**
     * Construct a rotation matrix from another rotation matrix.
     */
    RotMat(const aa_tf_rotmat *p) : aa_tf_rotmat(from_rotmat(p->data)) {}

    /**
     * Construct a rotation matrix from another rotation matrix.
     */
    RotMat(const aa_tf_rotmat &p) : aa_tf_rotmat(from_rotmat(p.data)) {}

    /**
     * Construct a rotation matrix from an axis-angle.
     */
    RotMat(const aa_tf_axang *p) : aa_tf_rotmat(from_axang(p->data)) {}

    /**
     * Construct a rotation matrix from an axis-angle.
     */
    RotMat(const aa_tf_axang &p) : aa_tf_rotmat(from_axang(p.data)) {}

    /**
     * Construct a rotation matrix from a rotation about X.
     */
    RotMat(const XAngle &p) : aa_tf_rotmat(from_xangle(p.value)) {}

    /**
     * Construct a rotation matrix from a rotation about Y.
     */
    RotMat(const YAngle &p) : aa_tf_rotmat(from_yangle(p.value)) {}

    /**
     * Construct a rotation matrix from a rotation about Z.
     */
    RotMat(const ZAngle &p) : aa_tf_rotmat(from_zangle(p.value)) {}

    /**
     * Construct a rotation matrix from individual components.
     */
    RotMat(double r11, double r12, double r13, double r21, double r22,
           double r23, double r31, double r32, double r33)
        : aa_tf_rotmat(from_rotmat(r11, r12, r13, r21, r22, r23, r31, r32, r33))
    {
    }

    /**
     * Create a rotation matrix from individual components.
     */
    static aa_tf_rotmat from_rotmat(double r11, double r12, double r13,
                                    double r21, double r22, double r23,
                                    double r31, double r32, double r33)
    {
        aa_tf_rotmat R;
        R.data[0] = r11;
        R.data[1] = r21;
        R.data[2] = r31;

        R.data[3] = r12;
        R.data[4] = r22;
        R.data[5] = r32;

        R.data[6] = r13;
        R.data[7] = r23;
        R.data[8] = r33;
        return R;
    }

    /**
     * Create a rotation matrix from a unit quaternion.
     */
    static aa_tf_rotmat from_quat(const double x[4])
    {
        aa_tf_rotmat y;
        aa_tf_quat2rotmat(x, y.data);
        return y;
    }

    /**
     * Create a rotation matrix object from a rotation matrix array.
     */
    static aa_tf_rotmat from_rotmat(const double x[9])
    {
        aa_tf_rotmat y;
        memcpy(y.data, x, 9 * sizeof(y.data[0]));
        return y;
    }

    /**
     * Create a rotation matrix from an axis-angle.
     */
    static aa_tf_rotmat from_axang(const double x[4])
    {
        aa_tf_rotmat y;
        aa_tf_axang2rotmat(x, y.data);
        return y;
    }

    /**
     * Create a rotation matrix from a rotation vector.
     */
    static aa_tf_rotmat from_rotvec(const double x[3])
    {
        aa_tf_rotmat y;
        aa_tf_rotvec2rotmat(x, y.data);
        return y;
    }

    /**
     * Create a rotation matrix from a rotation about X.
     */
    static aa_tf_rotmat from_xangle(const double v)
    {
        aa_tf_rotmat y;
        aa_tf_xangle2rotmat(v, y.data);
        return y;
    }

    /**
     * Create a rotation matrix from a rotation about Y.
     */
    static aa_tf_rotmat from_yangle(const double v)
    {
        aa_tf_rotmat y;
        aa_tf_yangle2rotmat(v, y.data);
        return y;
    }

    /**
     * Create a rotation matrix from a rotation about Z.
     */
    static aa_tf_rotmat from_zangle(const double v)
    {
        aa_tf_rotmat y;
        aa_tf_zangle2rotmat(v, y.data);
        return y;
    }
};

/**
 * An axis-angle object.
 */
struct AxisAngle : aa_tf_axang {
    /**
     * Construct an idenity axis-angle.
     */
    AxisAngle() : aa_tf_axang(from_axang(0, 0, 1, 0)) {}

    /**
     * Construct an axis-angle from a unit quaternion.
     */
    AxisAngle(const aa_tf_quat *p) : aa_tf_axang(from_quat(p->data)) {}

    /**
     * Construct an axis-angle from a unit quaternion.
     */
    AxisAngle(const aa_tf_quat &p) : aa_tf_axang(from_quat(p.data)) {}

    /**
     * Construct an axis-angle from a rotation matrix.
     */
    AxisAngle(const aa_tf_rotmat *p) : aa_tf_axang(from_rotmat(p->data)) {}

    /**
     * Construct an axis-angle from a rotation matrix.
     */
    AxisAngle(const aa_tf_rotmat &p) : aa_tf_axang(from_rotmat(p.data)) {}

    /**
     * Construct an axis-angle from another axis-angle.
     */
    AxisAngle(const aa_tf_axang *p) : aa_tf_axang(from_axang(p->data)) {}

    /**
     * Construct an axis-angle from another axis-angle.
     */
    AxisAngle(const aa_tf_axang &p) : aa_tf_axang(from_axang(p.data)) {}

    /**
     * Construct an axis-angle from individual components.
     */
    AxisAngle(double x, double y, double z, double theta)
        : aa_tf_axang(from_axang(x, y, z, theta))
    {
    }

    /**
     * Construct an axis-angle from an axis array and an angle.
     */
    AxisAngle(const double *_axis, double _angle)
        : aa_tf_axang(from_axang(_axis, _angle))
    {
    }

    /**
     * Create an axis angle from a unit quaternion.
     */
    static aa_tf_axang from_quat(const double x[4])
    {
        aa_tf_axang y;
        aa_tf_quat2axang(x, y.data);
        return y;
    }

    /**
     * Create an axis angle from a rotation matrix.
     */
    static aa_tf_axang from_rotmat(const double x[9])
    {
        aa_tf_axang y;
        aa_tf_rotmat2axang(x, y.data);
        return y;
    }

    /**
     * Create an axis angle from individual components.
     */
    static aa_tf_axang from_axang(double x, double y, double z, double theta)
    {
        double n = sqrt(x * x + y * y + z * z);
        double a[4] = {x / n, y / n, z / n, theta};
        return from_axang(a);
    }

    /**
     * Create an axis angle from an axis angle array.
     */
    static aa_tf_axang from_axang(const double x[4])
    {
        aa_tf_axang y;
        memcpy(y.data, x, 4 * sizeof(y.data[0]));
        return y;
    }

    /**
     * Create an axis angle from an axis array and an angle.
     */
    static aa_tf_axang from_axang(const double axis[3], double angle)
    {
        aa_tf_axang y;
        y.axis.x = axis[0];
        y.axis.y = axis[1];
        y.axis.z = axis[2];
        y.angle = angle;
        return y;
    }

    /**
     * Create an axis angle from a rotation vector.
     */
    static aa_tf_axang from_rotvec(const double x[3])
    {
        aa_tf_axang y;
        aa_tf_rotvec2axang(x, y.data);
        return y;
    }
};

/*-----------------------------*/
/*------ TRANSFORMATIONS ------*/
/*-----------------------------*/

/**
 * A dual quaternion object
 */
struct DualQuat : aa_tf_duqu {
    /**
     * Construct an identity dual quaternion.
     */
    DualQuat() : aa_tf_duqu(from_xyzw(0, 0, 0, 1, 0, 0, 0, 0)) {}

    /**
     * Construct from another dual quaternion.
     */
    DualQuat(const double *S) : aa_tf_duqu(from_duqu(S)) {}

    /**
     * Construct from another dual quaternion.
     */
    DualQuat(const struct aa_tf_duqu *S) : aa_tf_duqu(from_duqu(S->data)) {}

    /**
     * Construct from another dual quaternion.
     */
    DualQuat(const struct aa_tf_duqu &S) : aa_tf_duqu(from_duqu(S.data)) {}

    /**
     * Construct from a quaternion-vector.
     */
    DualQuat(const struct aa_tf_qv *S)
        : aa_tf_duqu(from_qv(S->r.data, S->v.data))
    {
    }

    /**
     * Construct from a quaternion-vector.
     */
    DualQuat(const struct aa_tf_qv &S) : aa_tf_duqu(from_qv(S.r.data, S.v.data))
    {
    }

    /**
     * Construct from a quaternion-vector.
     */
    DualQuat(const struct aa_tf_quat *r, const struct aa_tf_vec3 *v)
        : aa_tf_duqu(from_qv(r->data, v->data))
    {
    }

    /**
     * Construct from a quaternion-vector.
     */
    DualQuat(const struct aa_tf_quat &r, const struct aa_tf_vec3 &v)
        : aa_tf_duqu(from_qv(r.data, v.data))
    {
    }

    /**
     * Construct from a transformation matrix.
     */
    DualQuat(const struct aa_tf_tfmat *T) : aa_tf_duqu(from_tfmat(T->data)) {}

    /**
     * Construct from a transformation matrix.
     */
    DualQuat(const struct aa_tf_tfmat &T) : aa_tf_duqu(from_tfmat(T.data)) {}

    /**
     * Construct from a rotation about X and a translation vector.
     */
    DualQuat(const XAngle &r, const struct aa_tf_vec3 &v)
        : aa_tf_duqu(from_xxyz(r.value, v.x, v.y, v.z))
    {
    }

    /**
     * Construct from a rotation about Y and a translation vector.
     */
    DualQuat(const YAngle &r, const struct aa_tf_vec3 &v)
        : aa_tf_duqu(from_yxyz(r.value, v.x, v.y, v.z))
    {
    }

    /**
     * Construct from a rotation about Z and a translation vector.
     */
    DualQuat(const ZAngle &r, const struct aa_tf_vec3 &v)
        : aa_tf_duqu(from_zxyz(r.value, v.x, v.y, v.z))
    {
    }

    /**
     * Construct from an identity rotation and a translation vector.
     */
    DualQuat(const struct aa_tf_vec3 &v) : aa_tf_duqu(from_xyz(v.x, v.y, v.z))
    {
    }

    /**
     * Construct from an axis-angle rotation and a translation vector.
     */
    DualQuat(const aa_tf_axang &r, const struct aa_tf_vec3 &v)
        : aa_tf_duqu(from_qv(Quat::from_axang(r.data).data, v.data))
    {
    }

    /**
     * Return the conjugate of this.
     */
    DualQuat conj()
    {
        DualQuat S;
        aa_tf_duqu_conj(this->data, S.data);
        return S;
    }

    /**
     * Return the translation part of a unit dual quaternion.
     */
    aa_tf_vec3 translation()
    {
        aa_tf_vec3 V;
        aa_tf_duqu_trans(this->data, V.data);
        return V;
    }

    /**
     * Create a dual quaternion from components.
     */
    static aa_tf_duqu from_xyzw(double x_real, double y_real, double z_real,
                                double w_real, double x_dual, double y_dual,
                                double z_dual, double w_dual)
    {
        DualQuat S;
        S.real = Quat::from_xyzw(x_real, y_real, z_real, w_real);
        S.dual = Quat::from_xyzw(x_dual, y_dual, z_dual, w_dual);
        return S;
    }

    /**
     * Create a dual quaternion from another dual quaternion.
     */
    static aa_tf_duqu from_duqu(const double s[8])
    {
        DualQuat S;
        memcpy(S.data, s, 8 * sizeof(s[0]));
        return S;
    }

    /**
     * Create a dual quaternion from a rotation quaternion and a translation
     * vector.
     */
    static aa_tf_duqu from_qv(const double q[4], const double v[3])
    {
        DualQuat S;
        aa_tf_qv2duqu(q, v, S.data);
        return S;
    }

    /**
     * Create a dual quaternion from a transformation matrix.
     */
    static aa_tf_duqu from_tfmat(const double T[12])
    {
        DualQuat S;
        aa_tf_tfmat2duqu(T, S.data);
        return S;
    }

    /**
     * Create a dual quaternion an X axis rotation and translation components.
     */
    static aa_tf_duqu from_xxyz(double theta, double x, double y, double z)
    {
        DualQuat S;
        aa_tf_xxyz2duqu(theta, x, y, z, S.data);
        return S;
    }

    /**
     * Create a dual quaternion an Y axis rotation and translation components.
     */
    static aa_tf_duqu from_yxyz(double theta, double x, double y, double z)
    {
        DualQuat S;
        aa_tf_yxyz2duqu(theta, x, y, z, S.data);
        return S;
    }

    /**
     * Create a dual quaternion an Z axis rotation and translation components.
     */
    static aa_tf_duqu from_zxyz(double theta, double x, double y, double z)
    {
        DualQuat S;
        aa_tf_zxyz2duqu(theta, x, y, z, S.data);
        return S;
    }

    /**
     * Create a dual quaternion from and identity rotation and translation
     * components.
     */
    static aa_tf_duqu from_xyz(double x, double y, double z)
    {
        DualQuat S;
        aa_tf_xyz2duqu(x, y, z, S.data);
        return S;
    }

    /**
     * Create a dual quaternion derivative from a dual quaternion and the
     * spatial velocity.
     */
    static aa_tf_duqu from_dx(double *s, double *dx)
    {
        DualQuat S;
        aa_tf_duqu_vel2diff(s, dx, S.data);
        return S;
    }
};

/**
 * A rotation quaternion and translation vector object.
 */
struct QuatTran : aa_tf_qv {
    /**
     * Construct an idenity object
     */
    QuatTran() : aa_tf_qv(from_xyzw(0, 0, 0, 1, 0, 0, 0)) {}

    /**
     * Construct from another quaternion-translation.
     */
    QuatTran(const struct aa_tf_qv *S) : aa_tf_qv(from_qv(S->r.data, S->v.data))
    {
    }

    /**
     * Construct from another quaternion-translation.
     */
    QuatTran(const struct aa_tf_qv &S) : aa_tf_qv(from_qv(S.r.data, S.v.data))
    {
    }

    /**
     * Construct from another quaternion-translation.
     */
    QuatTran(const struct aa_tf_quat *_r, const struct aa_tf_vec3 *_v)
        : aa_tf_qv(from_qv(_r->data, _v->data))
    {
    }

    /**
     * Construct from another quaternion-translation.
     */
    QuatTran(const struct aa_tf_quat &_r, const struct aa_tf_vec3 &_v)
        : aa_tf_qv(from_qv(_r.data, _v.data))
    {
    }

    /**
     * Construct from a principal angle and a vector.
     */
    QuatTran(const struct XAngle &_r, const struct aa_tf_vec3 &_v)
        : aa_tf_qv(from_qv(Quat(_r).data, _v.data))
    {
    }

    /**
     * Construct from a principal angle and a vector.
     */
    QuatTran(const struct YAngle &_r, const struct aa_tf_vec3 &_v)
        : aa_tf_qv(from_qv(Quat(_r).data, _v.data))
    {
    }

    /**
     * Construct from a principal angle and a vector.
     */
    QuatTran(const struct ZAngle &_r, const struct aa_tf_vec3 &_v)
        : aa_tf_qv(from_qv(Quat(_r).data, _v.data))
    {
    }

    /**
     * Construct from a dual quaternion.
     */
    QuatTran(const struct aa_tf_duqu *S) : aa_tf_qv(from_duqu(S->data)) {}

    /**
     * Construct from a dual quaternion.
     */
    QuatTran(const struct aa_tf_duqu &S) : aa_tf_qv(from_duqu(S.data)) {}

    /**
     * Construct from a transformation matrix.
     */
    QuatTran(const struct aa_tf_tfmat *T) : aa_tf_qv(from_tfmat(T->data)) {}

    /**
     * Construct from a transformation matrix.
     */
    QuatTran(const struct aa_tf_tfmat &T) : aa_tf_qv(from_tfmat(T.data)) {}

    /**
     * Return the conjugate (inverse).
     */
    aa_tf_qv conj()
    {
        aa_tf_qv qv;
        aa_tf_qutr_conj(this->data, qv.data);
        return qv;
    }

    /**
     * Create a quaternion-translation from components.
     */
    static aa_tf_qv from_xyzw(double q_x, double q_y, double q_z, double q_w,
                              double t_x, double t_y, double t_z)
    {
        aa_tf_qv qv;
        qv.r = Quat::from_xyzw(q_x, q_y, q_z, q_w);
        qv.v = Vec3::from_xyz(t_x, t_y, t_z);
        return qv;
    }

    /**
     * Create a quaternion-translation from a rotation quaternion and a
     * translation vector.
     */
    static aa_tf_qv from_qv(const double a_r[4], const double a_v[3])
    {
        aa_tf_qv qv;
        memcpy(qv.r.data, a_r, 4 * sizeof(qv.r.data[0]));
        memcpy(qv.v.data, a_v, 3 * sizeof(qv.v.data[0]));
        return qv;
    }

    /**
     * Create a quaternion-translation from a rotation quaternion and a
     * translation vector.
     */
    static aa_tf_qv from_qv(const double e[7]) { return from_qv(e, e + 4); }

    /**
     * Create a quaternion-translation from a dual quaternion.
     */
    static aa_tf_qv from_duqu(const double s[8])
    {
        aa_tf_qv qv;
        aa_tf_duqu2qv(s, qv.r.data, qv.v.data);
        return qv;
    }

    /**
     * Create a quaternion-translation from a transformation matrix.
     */
    static aa_tf_qv from_tfmat(const double t[12])
    {
        aa_tf_qv qv;
        aa_tf_tfmat2duqu(t, qv.data);
        return qv;
    }

    void transform(const double a[3], double b[3]) const
    {
        aa_tf_qutr_tf(this->data, a, b);
    }

    void transform(const aa_tf_vec3 *a, aa_tf_vec3 *b) const
    {
        this->transform(a->data, b->data);
    }

    aa_tf_vec3 transform(const aa_tf_vec3 &a) const
    {
        aa_tf_vec3 b;
        this->transform(&a, &b);
        return b;
    }
};

/**
 * A transformation matrix object.
 */
struct TfMat : aa_tf_tfmat {
    /**
     * Construct an identity transformation matrix.
     */
    TfMat() : aa_tf_tfmat(from_tfmat(aa_tf_ident)) {}

    /**
     * Construct a transformation matrix from another transformation matrix.
     */
    TfMat(const struct aa_tf_tfmat *T) : aa_tf_tfmat(from_tfmat(T->data)) {}

    /**
     * Construct a transformation matrix from another transformation matrix.
     */
    TfMat(const struct aa_tf_tfmat &T) : aa_tf_tfmat(from_tfmat(T.data)) {}

    /**
     * Construct a transformation matrix from a dual quaternion.
     */
    TfMat(const struct aa_tf_duqu *S) : aa_tf_tfmat(from_duqu(S->data)) {}

    /**
     * Construct a transformation matrix from a dual quaternion.
     */
    TfMat(const struct aa_tf_duqu &S) : aa_tf_tfmat(from_duqu(S.data)) {}

    /**
     * Construct a transformation matrix from a quaternion-translation.
     */
    TfMat(const struct aa_tf_qv *S) : aa_tf_tfmat(from_qv(S->r.data, S->v.data))
    {
    }

    /**
     * Construct a transformation matrix from a quaternion-translation.
     */
    TfMat(const struct aa_tf_qv &S) : aa_tf_tfmat(from_qv(S.r.data, S.v.data))
    {
    }

    /**
     * Construct a transformation matrix from a quaternion-translation.
     */
    TfMat(const struct aa_tf_quat *_r, const struct aa_tf_vec3 *_v)
        : aa_tf_tfmat(from_qv(_r->data, _v->data))
    {
    }

    /**
     * Construct a transformation matrix from a quaternion-translation.
     */
    TfMat(const struct aa_tf_quat &_r, const struct aa_tf_vec3 &_v)
        : aa_tf_tfmat(from_qv(_r.data, _v.data))
    {
    }

    /**
     * Construct a transformation matrix from a dual quaternion.
     */
    static aa_tf_tfmat from_duqu(const double s[8])
    {
        aa_tf_tfmat T;
        aa_tf_duqu2tfmat(s, T.data);
        return T;
    }

    /**
     * Construct a transformation matrix from a rotation quaternion and
     * translation vector.
     */
    static aa_tf_tfmat from_qv(const double q[4], const double v[3])
    {
        aa_tf_tfmat T;
        aa_tf_qv2tfmat(q, v, T.data);
        return T;
    }

    /**
     * Construct a transformation matrix from another transformation matrix.
     */
    static aa_tf_tfmat from_tfmat(const double t[12])
    {
        aa_tf_tfmat T;
        memcpy(T.data, t, 12 * sizeof(T.data[0]));
        return T;
    }

    void transform(const double a[3], double b[3]) const
    {
        aa_tf_tfmat_tf(this->data, a, b);
    }

    void transform(const aa_tf_vec3 *a, aa_tf_vec3 *b) const
    {
        this->transform(a->data, b->data);
    }

    aa_tf_vec3 transform(const aa_tf_vec3 &a) const
    {
        aa_tf_vec3 b;
        this->transform(&a, &b);
        return b;
    }
};

/*---- OPERATORS -----*/

/**
 * Add two Vec3 objects.
 */
static inline struct aa_tf_vec3 operator+(const struct aa_tf_vec3 &a,
                                          const struct aa_tf_vec3 &b)
{
    struct aa_tf_vec3 c;
    c.x = a.x + b.x;
    c.y = a.y + b.y;
    c.z = a.z + b.z;
    return c;
}

/**
 * Divide a Vec3 by a scalar.
 */
static inline struct aa_tf_vec3 operator/(const struct aa_tf_vec3 &a, double b)
{
    struct aa_tf_vec3 c;
    c.x = a.x / b;
    c.y = a.y / b;
    c.z = a.z / b;
    return c;
}

/**
 * Multiply two rotation matrices.
 */
static inline struct aa_tf_rotmat operator*(const struct aa_tf_rotmat &a,
                                            const struct aa_tf_rotmat &b)
{
    struct aa_tf_rotmat c;
    aa_tf_9mul(a.data, b.data, c.data);
    return c;
}

/**
 * Multiply two ordinary quaternions.
 */
static inline struct aa_tf_quat operator*(const struct aa_tf_quat &a,
                                          const struct aa_tf_quat &b)
{
    struct aa_tf_quat c;
    aa_tf_qmul(a.data, b.data, c.data);
    return c;
}

/**
 * Multiply two transformation matrices.
 */
static inline struct aa_tf_tfmat operator*(const struct aa_tf_tfmat &a,
                                           const struct aa_tf_tfmat &b)
{
    struct aa_tf_tfmat c;
    aa_tf_12chain(a.data, b.data, c.data);
    return c;
}

/**
 * Multiply two dual quaternions.
 */
static inline struct aa_tf_duqu operator*(const struct aa_tf_duqu &a,
                                          const struct aa_tf_duqu &b)
{
    struct aa_tf_duqu c;
    aa_tf_duqu_mul(a.data, b.data, c.data);
    return c;
}

/**
 * Chain two poses stored in quaternion-vector form.
 *
 * It is a slight abuse of notation to use the \f$*\f$ symbol for this
 * chaining as chaining quaternion-vector poses is not linear.
 *
 */
static inline struct aa_tf_qv operator*(const struct aa_tf_qv &a,
                                        const struct aa_tf_qv &b)
{
    struct aa_tf_qv c;
    aa_tf_qv_chain(a.r.data, a.v.data, b.r.data, b.v.data, c.r.data, c.v.data);
    return c;
}

/**
 * Rotate point by rotation matrix.
 */
static inline struct aa_tf_vec3 operator*(const struct aa_tf_rotmat &r,
                                          const struct aa_tf_vec3 &p)
{
    struct aa_tf_vec3 q;
    aa_tf_rotmat_rot(r.data, p.data, q.data);
    return q;
}

}  // namespace amino

#endif  // AA_MEM_HPP

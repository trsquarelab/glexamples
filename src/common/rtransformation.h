
#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#include <cstring>
#include <cmath>
#include <stack>

#include "rshared_object.h"
#include "rvector.h"
#include "rmatrix.h"

namespace chikku
{

class RTransformationData
{
public:
    typedef std::stack<RMatrix> MatrixStack;

    MatrixStack mMatrixStack;
};

class RTransformation: public RDetachableRSharedObject<RTransformationData>
{
private:
    typedef RTransformationData::MatrixStack MatrixStack;

public:
    RTransformation()
    {
        push();
    }

    RTransformation(RMatrix const & m)
    {
        push();
        load(m);
    }

    const RMatrix & matrix() const
    {
        return data_p().mMatrixStack.top();
    }

    RMatrix & matrix()
    {
        return data_p().mMatrixStack.top();
    }

    void load(RMatrix const & m)
    {
        matrix() = m;
    }

    void push()
    {
        if (data_p().mMatrixStack.empty()) {
            data_p().mMatrixStack.push(RMatrix());
        } else {
            data_p().mMatrixStack.push(data_p().mMatrixStack.top());
        }
    }

    void pop()
    {
        data_p().mMatrixStack.pop();
    }

    void reset()
    {
        data_p().mMatrixStack.top().reset();
    }

    RTransformation multiply(RTransformation const & t) const
    {
        RTransformation result;
        RMatrix::multimatrix(matrix(), t.matrix(), result.matrix());
        return result;
    }

    RTransformation multiply(RMatrix const & m) const
    {
        RTransformation result;
        RMatrix::multimatrix(matrix(), m, result.matrix());
        return result;
    }

    RVector multiply(float x, float y, float z, float w=1.0f) const
    {
        return multiply(RVector(x, y, z, w));
    }

    RVector multiply(RVector const & p) const
    {
        return matrix().multiply(p);
    }

    RTransformation inverse() const
    {
        RTransformation r;
        RMatrix m = matrix().inverse();
        r.load(m);
        return r;
    }

    RTransformation transpose() const
    {
        RTransformation r;
        
        RMatrix m = matrix().transpose();
        r.load(m);

        return r;
    }

    void perspective(float fovy, float aspect, float near, float far)
    {
        float f = std::tan((90.0f - fovy/2.0f) * M_PI / 180.0f);

        matrix().at(0, 0)  = f / aspect;
        matrix().at(0, 1)  = 0.0f;
        matrix().at(0, 2)  = 0.0f;
        matrix().at(0, 3)  = 0.0f;
        matrix().at(1, 0)  = 0.0f;
        matrix().at(1, 1)  = f;
        matrix().at(1, 2)  = 0.0f;
        matrix().at(1, 3)  = 0.0f;
        matrix().at(2, 0)  = 0.0f;
        matrix().at(2, 1)  = 0.0f;
        matrix().at(2, 2) = (far + near) / (near - far);
        matrix().at(2, 3) = -1.0f;
        matrix().at(3, 0) = 0.0f;
        matrix().at(3, 1) = 0.0f;
        matrix().at(3, 2) = (2*far*near)/(near-far);
        matrix().at(3, 3) = 0.0f;
    }

    void parallel(float left, float right, float bottom, float top, float near, float far)
    {
        matrix().at(0, 0)  = 2.0f / (right - left);
        matrix().at(0, 1)  = 0.0f;
        matrix().at(0, 2)  = 0.0f;
        matrix().at(0, 3)  = 0.0f;
        matrix().at(1, 0)  = 0.0f;
        matrix().at(1, 1)  = 2.0f / (top - bottom);
        matrix().at(1, 2)  = 0.0f;
        matrix().at(1, 3)  = 0.0f;
        matrix().at(2, 0)  = 0.0f;
        matrix().at(2, 1)  = 0.0f;
        matrix().at(2, 2) = -2.0f / (far - near);
        matrix().at(2, 3) = 0.0f;
        matrix().at(3, 0) = -(right + left) / (right - left);
        matrix().at(3, 1) = -(top + bottom) / (top - bottom);
        matrix().at(3, 2) = -(far + near) / (far - near);
        matrix().at(3, 3) = 1.0f;
    }

    void translate(float tx, float ty, float tz)
    {
        matrix().at(3, 0) += (matrix().at(0, 0) * tx + matrix().at(1, 0) * ty + matrix().at(2, 0) * tz);
        matrix().at(3, 1) += (matrix().at(0, 1) * tx + matrix().at(1, 1) * ty + matrix().at(2, 1) * tz);
        matrix().at(3, 2) += (matrix().at(0, 2) * tx + matrix().at(1, 2) * ty + matrix().at(2, 2) * tz);
        matrix().at(3, 3) += (matrix().at(0, 3) * tx + matrix().at(1, 3) * ty + matrix().at(2, 3) * tz);
    }

    void rotate(float angle, float x, float y, float z)
    {
        float mag = std::sqrt(x * x + y * y + z * z);

        if (mag > 0.0f) {
            float sinAngle = std::sin(angle * (float)M_PI / 180.0f);
            float cosAngle = std::cos(angle * (float)M_PI / 180.0f);

            float xx, yy, zz, xy, yz, zx, xs, ys, zs;
            float oneMinusCos;
            RMatrix rotMat;

            x /= mag;
            y /= mag;
            z /= mag;

            xx = x * x;
            yy = y * y;
            zz = z * z;
            xy = x * y;
            yz = y * z;
            zx = z * x;
            xs = x * sinAngle;
            ys = y * sinAngle;
            zs = z * sinAngle;
            oneMinusCos = 1.0f - cosAngle;

            rotMat.at(0, 0) = (oneMinusCos * xx) + cosAngle;
            rotMat.at(0, 1) = (oneMinusCos * xy) - zs;
            rotMat.at(0, 2) = (oneMinusCos * zx) + ys;
            rotMat.at(0, 3) = 0.0f;

            rotMat.at(1, 0) = (oneMinusCos * xy) + zs;
            rotMat.at(1, 1) = (oneMinusCos * yy) + cosAngle;
            rotMat.at(1, 2) = (oneMinusCos * yz) - xs;
            rotMat.at(1, 3) = 0.0f;

            rotMat.at(2, 0) = (oneMinusCos * zx) - ys;
            rotMat.at(2, 1) = (oneMinusCos * yz) + xs;
            rotMat.at(2, 2) = (oneMinusCos * zz) + cosAngle;
            rotMat.at(2, 3) = 0.0f;

            rotMat.at(3, 0) = 0.0f;
            rotMat.at(3, 1) = 0.0f;
            rotMat.at(3, 2) = 0.0f;
            rotMat.at(3, 3) = 1.0f;

            RMatrix resultMat;

            RMatrix::multimatrix(rotMat, matrix(), resultMat);

            load(resultMat);
        }
    }

    void scale(float sx, float sy, float sz)
    {
        matrix().at(0, 0) *= sx;
        matrix().at(0, 1) *= sx;
        matrix().at(0, 2) *= sx;
        matrix().at(0, 3) *= sx;

        matrix().at(1, 0) *= sy;
        matrix().at(1, 1) *= sy;
        matrix().at(1, 2) *= sy;
        matrix().at(1, 3) *= sy;

        matrix().at(2, 0) *= sz;
        matrix().at(2, 1) *= sz;
        matrix().at(2, 2) *= sz;
        matrix().at(2, 3) *= sz;
    }
};

}

static std::ostream & operator<<(std::ostream & o, chikku::RTransformation const & t)
{
    o << t.matrix();
    return o;
}

#endif


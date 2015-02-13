
#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>
#include <iomanip>

#include "rshared_object.h"
#include "rvector.h"

namespace chikku
{

class RMatrixData
{
public:
    float mData[4*4];
};

class RMatrix: public RDetachableRSharedObject<RMatrixData>
{
public:
    RMatrix()
    {
        reset();
    }

    void reset()
    {
        at(0, 0) = 1.0f;
        at(0, 1) = 0.0f;
        at(0, 2) = 0.0f;
        at(0, 3) = 0.0f;
        at(1, 0) = 0.0f;
        at(1, 1) = 1.0f;
        at(1, 2) = 0.0f;
        at(1, 3) = 0.0f;
        at(2, 0) = 0.0f;
        at(2, 1) = 0.0f;
        at(2, 2) = 1.0f;
        at(2, 3) = 0.0f;
        at(3, 0) = 0.0f;
        at(3, 1) = 0.0f;
        at(3, 2) = 0.0f;
        at(3, 3) = 1.0f;
    }

    void load(float d[4*4])
    {
    	for (int i=0; i<4*4; ++i) {
    			data()[i] = d[i];
    	}
    }
    
    float & at(int i, int j)
    {
        return data_p().mData[i * 4 + j];
    }

    const float & at(int i, int j) const
    {
        return data_p().mData[i * 4 + j];
    }

    void set(int i, int j, float v)
    {
        at(i, j) = v;
    }

    float * data()
    {
        return data_p().mData;
    }

    const float * data() const
    {
        return data_p().mData;
    }

    RMatrix multimatrix(const RMatrix & matrix2) const
    {
        RMatrix result;
        return multimatrix(*this, matrix2, result);
    }

    float det3x3(float a, float b, float c,
                 float d, float e, float f,
                 float g, float h, float i) const
    {
        return a*e*i + d*h*c + g*b*f - a*h*f - g*e*c - d*b*i;
    }

    float determinant() const
    {
        float v = at(3, 3) * det3x3(at(0, 0), at(1, 0), at(2, 0),
                         at(0, 1), at(1, 1), at(2, 1),
                         at(0, 2), at(1, 2), at(2, 2));

        if (at(0, 3) == 0.0 && at(1, 3) == 0.0 && at(2, 3) == 0.0) {
            return v;
        }

        v = -at(3, 0) * det3x3(at(0, 1), at(1, 1), at(2, 1),
                      at(0, 2), at(1, 2), at(2, 2),
                      at(0, 3), at(1, 3), at(2, 3))


            +at(3, 1) * det3x3(at(0, 0), at(1, 0), at(2, 0),
                      at(0, 2), at(1, 2), at(2, 2),
                      at(0, 3), at(1, 3), at(2, 3))

            -at(3, 2) * det3x3(at(0, 0), at(1, 0), at(2, 0),
                      at(0, 1), at(1, 1), at(2, 1),
                      at(0, 3), at(1, 3), at(2, 3))

            +v;

        return v;
    }

    RMatrix inverse() const
    {
        RMatrix r;

        float det = determinant();

        r.set(0, 0, det3x3(at(1, 1), at(2, 1), at(3, 1),
                at(1, 2), at(2, 2), at(3, 2),
                at(1, 3), at(2, 3), at(3, 3)) / det);

        r.set(1, 0, -det3x3(at(1, 0), at(2, 0), at(3, 0),
                at(1, 2), at(2, 2), at(3, 2),
                at(1, 3), at(2, 3), at(3, 3)) / det);

        r.set(2, 0, det3x3(at(1, 0), at(2, 0), at(3, 0),
                at(1, 1), at(2, 1), at(3, 1),
                at(1, 3), at(2, 3), at(3, 3)) / det);

        r.set(3, 0, -det3x3(at(1, 0), at(2, 0), at(3, 0),
                at(1, 1), at(2, 1), at(3, 1),
                at(1, 2), at(2, 2), at(3, 2)) / det);

        r.set(0, 1, -det3x3(at(0, 1), at(2, 1), at(3, 1),
                at(0, 2), at(2, 2), at(3, 2),
                at(0, 3), at(2, 3), at(3, 3)) / det);

        r.set(1, 1, det3x3(at(0, 0), at(2, 0), at(3, 0),
                at(0, 2), at(2, 2), at(3, 2),
                at(0, 3), at(2, 3), at(3, 3)) / det);

        r.set(2, 1, -det3x3(at(0, 0), at(2, 0), at(3, 0),
                at(0, 1), at(2, 1), at(3, 1),
                at(0, 3), at(2, 3), at(3, 3)) / det);

        r.set(3, 1, det3x3(at(0, 0), at(2, 0), at(3, 0),
                at(0, 1), at(2, 1), at(3, 1),
                at(0, 2), at(2, 2), at(3, 2)) / det);

        r.set(0, 2, det3x3(at(0, 1), at(1, 1), at(3, 1),
                at(0, 2), at(1, 2), at(3, 2),
                at(0, 3), at(1, 3), at(3, 3)) / det);

        r.set(1, 2, -det3x3(at(0, 0), at(1, 0), at(3, 0),
                at(0, 2), at(1, 2), at(3, 2),
                at(0, 3), at(1, 3), at(3, 3)) / det);

        r.set(2, 2, det3x3(at(0, 0), at(1, 0), at(3, 0),
                at(0, 1), at(1, 1), at(3, 1),
                at(0, 3), at(1, 3), at(3, 3)) / det);

        r.set(3, 2, -det3x3(at(0, 0), at(1, 0), at(3, 0),
                at(0, 1), at(1, 1), at(3, 1),
                at(0, 2), at(1, 2), at(3, 2)) / det);

        r.set(0, 3, -det3x3(at(0, 1), at(1, 1), at(2, 1),
                at(0, 2), at(1, 2), at(2, 2),
                at(0, 3), at(1, 3), at(2, 3)) / det);

        r.set(1, 3, det3x3(at(0, 0), at(1, 0), at(2, 0),
                at(0, 2), at(1, 2), at(2, 2),
                at(0, 3), at(1, 3), at(2, 3)) / det);

        r.set(2, 3, -det3x3(at(0, 0), at(1, 0), at(2, 0),
                at(0, 1), at(1, 1), at(2, 1),
                at(0, 3), at(1, 3), at(2, 3)) / det);

        r.set(3, 3, det3x3(at(0, 0), at(1, 0), at(2, 0),
                at(1, 1), at(1, 1), at(2, 1),
                at(0, 2), at(1, 2), at(2, 2)) / det);

        return r;
    }

    RMatrix transpose() const
    {
        RMatrix r;

        for (int i=0; i<4; ++i) {
            for (int j=0; j<4; ++j) {
                r.set(i, j, at(j, i));
            }
        }

        return r;
    }

    RVector multiply(RVector const & p) const
    {
        RVector r;

        r.x() = at(0, 0) * p.x() + at(1, 0) * p.y() +
            at(2, 0) * p.z() + at(3, 0) * p.w();
        r.y() = at(0, 1) * p.x() + at(1, 1) * p.y() +
            at(2, 1) * p.z() + at(3, 1) * p.w();
        r.z() = at(0, 2) * p.x() + at(1, 2) * p.y() +
            at(2, 2) * p.z() + at(3, 2) * p.w();
        r.w() = at(0, 3) * p.x() + at(1, 3) * p.y() +
            at(2, 3) * p.z() + at(3, 3) * p.w();

        return r;
    }

public:
    static RMatrix & multimatrix(RMatrix const & matrix1, const RMatrix & matrix2, RMatrix & matrix3)
    {
        for (int i = 0; i < 4; i++) {
            matrix3.at(i, 0) = (matrix1.at(i, 0) * matrix2.at(0, 0)) +
                                 (matrix1.at(i, 1) * matrix2.at(1, 0)) +
                                 (matrix1.at(i, 2) * matrix2.at(2, 0)) +
                                 (matrix1.at(i, 3) * matrix2.at(3, 0));

            matrix3.at(i, 1) = (matrix1.at(i, 0) * matrix2.at(0, 1)) +
                                 (matrix1.at(i, 1) * matrix2.at(1, 1)) +
                                 (matrix1.at(i, 2) * matrix2.at(2, 1)) +
                                 (matrix1.at(i, 3) * matrix2.at(3, 1));

            matrix3.at(i, 2) = (matrix1.at(i, 0) * matrix2.at(0, 2)) +
                                 (matrix1.at(i, 1) * matrix2.at(1, 2)) +
                                 (matrix1.at(i, 2) * matrix2.at(2, 2)) +
                                 (matrix1.at(i, 3) * matrix2.at(3, 2));

            matrix3.at(i, 3) = (matrix1.at(i, 0) * matrix2.at(0, 3)) +
                                 (matrix1.at(i, 1) * matrix2.at(1, 3)) +
                                 (matrix1.at(i, 2) * matrix2.at(2, 3)) +
                                 (matrix1.at(i, 3) * matrix2.at(3, 3));
        }

        return matrix3;
    }
};

}

static std::ostream & operator<<(std::ostream & o, chikku::RMatrix const & m)
{
    const float *mat = m.data();
    o << std::setprecision(3) << "\n[";
    for (int i=0; i<16; ++i) {
        o << mat[i] << " ";
        if ((i % 4) == 3)
            o << "\n";
    }
    o << "]";
    return o;
}

#endif


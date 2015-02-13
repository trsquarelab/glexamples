
#ifndef VECTOR_H
#define VECTOR_H

#include <cmath>

namespace chikku
{

class RVector
{
public:
    RVector(float v1=0.0f, float v2=0.0f, float v3=0.0f, float v4=1.0f)
    {
        set(v1, v2, v3, v4);
    }

    void set(const float *v)
    {
        set(v[0], v[1], v[2], v[3]);
    }

    void set(float v1=0.0f, float v2=0.0f, float v3=0.0f, float v4=1.0f)
    {
        x() = v1;
        y() = v2;
        z() = v3;
        w() = v4;
    }

    bool operator==(RVector const &o) const
    {
        return x() == o.x() &&
               y() == o.y() &&
               z() == o.z() &&
               w() == o.z();
    }

    RVector operator*(RVector const &o)
    {
        RVector v;
        v.x() = x() * o.x();
        v.y() = y() * o.y();
        v.z() = z() * o.z();
        v.w() = w() * o.w();
        return v;
    }

    RVector operator*(float s)
    {
        RVector v;
        v.x() = x() * s;
        v.y() = y() * s;
        v.z() = z() * s;
        v.w() = w() * s;
        return v;
    }

    float &x()
    {
        return mData[0];
    }

    float &y()
    {
        return mData[1];
    }

    float &z()
    {
        return mData[2];
    }

    float &w()
    {
        return mData[3];
    }

    float &r()
    {
        return x();
    }

    float &g()
    {
        return y();
    }

    float &b()
    {
        return z();
    }

    float &a()
    {
        return w();
    }

    float * data()
    {
        return mData;
    }

    const float &x() const
    {
        return mData[0];
    }

    const float &y() const
    {
        return mData[1];
    }

    const float &z() const
    {
        return mData[2];
    }

    const float &w() const
    {
        return mData[3];
    }

    const float &r() const
    {
        return x();
    }

    const float &g() const
    {
        return y();
    }

    const float &b() const
    {
        return z();
    }

    const float &a() const
    {
        return w();
    }

    const float * data() const
    {
        return mData;
    }

    RVector operator-(RVector const & o) const
    {
    	RVector v;
    	
        v.x() = x() - o.x();
        v.y() = y() - o.y();
        v.z() = z() - o.z();
        
        return v;
    }

    RVector operator+(RVector const & o) const
    {
    	RVector v;
    	
        v.x() = x() + o.x();
        v.y() = y() + o.y();
        v.z() = z() + o.z();
        
        return v;
    }
    
    float length() const
    {
        return std::sqrt(x()*x() + y()*y() + z()*z());
    }

    RVector toUnit() const
    {
        float l = length();
        if (l == 0.0f) {
            l = 1.0f;
        }

        return RVector(x()/l, y()/l, z()/l);
    }

    static RVector normal(RVector const &v1, RVector const &v2, RVector const &v3)
    {
        RVector vect1 = v1 - v2;
        RVector vect2 = v3 - v2;
        RVector n;

        n.x() = vect1.y()*vect2.z() - vect1.z()*vect2.y();
        n.y() = vect1.z()*vect2.x() - vect1.x()*vect2.z();
        n.z() = vect1.x()*vect2.y() - vect1.y()*vect2.x();
        
        return n.toUnit();
    }

private:
    float mData[4];
};

}

#endif


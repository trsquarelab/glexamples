
#ifndef RMATERIAL_H
#define RMATERIAL_H

#include "rcolor.h"

namespace chikku
{

class RMaterial
{
public:
    RMaterial()
    {
        reset();
    }

    bool operator==(RMaterial const & oth) const
    {
        return 
            mAmbient   == oth.mAmbient &&
            mDiffuse   == oth.mDiffuse &&
            mSpecular  == oth.mSpecular &&
            mEmissive  == oth.mEmissive &&
            mShininess == oth.mShininess;
    }
    bool operator!=(RMaterial const & oth) const
    {
        return !((*this) == oth);
    }

    RColor ambient() const
    {
        return mAmbient;
    }
    void setAmbient(RColor const & v)
    {
        mAmbient = v;
    }

    RColor diffuse() const
    {
        return mDiffuse;
    }
    void setDiffuse(RColor const & v)
    {
        mDiffuse = v;
    }

    RColor specular() const
    {
        return mSpecular;
    }
    void setSpecular(RColor const & v)
    {
        mSpecular = v;
    }

    RColor emissive() const
    {
        return mEmissive;
    }
    void setEmissive(RColor const & v)
    {
        mEmissive = v;
    }

    float shininess() const
    {
        return mShininess;
    }
    void setShininess(float v)
    {
        mShininess = v;
    }

    void reset()
    {
        mAmbient.set(0.2f, 0.2f, 0.2f, 1.0f);
        mDiffuse.set(0.8f, 0.8f, 0.8f, 1.0f);
        mSpecular.set(0.0f, 0.0f, 0.0f, 1.0f);
        mEmissive.set(0.0f, 0.0f, 0.0f, 1.0f);
        mShininess = 0.0f;
    }

private:
    RColor mAmbient;
    RColor mDiffuse;
    RColor mSpecular;
    RColor mEmissive;
    float mShininess;
};

}

#endif


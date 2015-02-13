
#ifndef CUBE_H
#define CUBE_H

#include <vector>

#include "rshape.h"

namespace chikku
{

class RSolidCube: public RShape
{
public:
    RSolidCube(float w, float h, float l)
    {
        create(w, h, l);
    }

    void set(float w, float h, float l)
    {
        create(w, h, l);
    }

    float width() const
    {
        return mWidth;
    }
    float height() const
    {
        return mHeight;
    }
    float length() const
    {
        return mLength;
    }

private:
    void create(float w, float h, float l)
    {
        mWidth = w;
        mHeight = h;
        mLength = l;

        float x2 = w/2.0;
        float y2 = h/2.0;
        float z2 = l/2.0;

        float v[] = {-x2, y2, z2, -x2, -y2, z2, x2, -y2, z2, x2, -y2, z2,
                         x2, y2, z2, -x2, y2, z2, x2, y2, z2, x2, -y2, z2, x2, -y2, -z2, x2, -y2, -z2,
                         x2, y2, -z2, x2, y2, z2, x2, y2, -z2, x2, -y2, -z2, -x2, -y2, -z2, -x2, -y2, -
                         z2, -x2, y2, -z2, x2, y2, -z2, -x2, y2, -z2, -x2, -y2, -z2, -x2, -y2, z2, -x2,
                        -y2, z2, -x2, y2, z2, -x2, y2, -z2, -x2, y2, -z2, -x2, y2, z2, x2, y2, z2, x2,
                         y2, z2, x2, y2, -z2, -x2, y2, -z2, -x2, -y2, z2, -x2, -y2, -z2, x2, -y2, -z2,
                         x2, -y2, -z2, x2, -y2, z2, -x2, -y2, z2};

        float n[] = {0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0,
                        1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0,
                        1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, -1.0,
                        0.0, 0.0, -1.0, 0.0, 0.0, -1.0, 0.0, 0.0, -1.0, 0.0, 0.0, -1.0, -1.0, 0.0, 0.0,
                       -1.0, 0.0, 0.0, -1.0, 0.0, 0.0, -1.0, 0.0, 0.0, -1.0, 0.0, 0.0, -1.0, 0.0, 0.0,
                        0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0,
                        1.0, 0.0, 0.0, -1.0, 0.0, 0.0, -1.0, 0.0, 0.0, -1.0, 0.0, 0.0, -1.0, 0.0, 0.0,
                       -1.0, 0.0, 0.0, -1.0, 0.0};

        float t[] = {1.0-1.0/4.0, 3.0/4.0, 1.0-1.0/4.0, 1.0/4.0, 1.0, 1.0/4.0, 1.0, 1.0/4.0, 1.0,
                        3.0/4.0, 1.0-1.0/4.0, 3.0/4.0, 0.0, 3.0/4.0, 0.0, 1.0/4.0, 1.0/4.0, 1.0/4.0,
                        1.0/4.0, 1.0/4.0, 1.0/4.0, 3.0/4.0, 0.0, 3.0/4.0, 1.0/4.0, 3.0/4.0, 1.0/4.0,
                        1.0/4.0, 2.0/4.0, 1.0/4.0, 2.0/4.0, 1.0/4.0, 2.0/4.0, 3.0/4.0, 1.0/4.0,
                        3.0/4.0, 2.0/4.0, 3.0/4.0, 2.0/4.0, 1.0/4.0, 3.0/4.0, 1.0/4.0, 3.0/4.0,
                        1.0/4.0, 3.0/4.0, 3.0/4.0, 2.0/4.0, 3.0/4.0, 1.0, 1.0, 0.0, 1.0, 0.0, 0.0,
                        0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0,
                        0.0, 1.0, 1.0};

        data_p().mVertices.clear();
        data_p().mNormals.clear();
        data_p().mTexCoords.clear();

        data_p().mVertices.insert(data_p().mVertices.begin(), v, v + sizeof(v)/sizeof(v[0]));
        data_p().mNormals.insert(data_p().mNormals.begin(), n, n + sizeof(n)/sizeof(n[0]));
        data_p().mTexCoords.insert(data_p().mTexCoords.begin(), t, t + sizeof(t)/sizeof(t[0]));
    }
private:
    float mWidth;
    float mHeight;
    float mLength;
};

}

#endif


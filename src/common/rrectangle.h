 
#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <vector>

#include "rshape.h"

namespace chikku
{

class RRectangle: public RShape
{
public:
    enum Mode
    {
        FILL,
        STROKE
    };

    RRectangle(float x1, float y1, float x2, float y2, Mode mode = FILL)
     : mX1(x1),
       mY1(y1),
       mX2(x2),
       mY2(y2),
       mMode(mode)
    {
        create();
    }

    void set(float x1, float y1, float x2, float y2, Mode mode = FILL)
    {
        mX1 = x1;
        mY1 = y1;
        mX2 = x2;
        mY2 = y2;
        mMode = mode;
        create();
    }


    GLenum primitives() const
    {
        if (mMode == FILL) {
            return GL_TRIANGLES;
        } else {
            return GL_LINE_LOOP;
        }
    }


private:
    void create()
    {
        if (mMode == FILL) {
            addVertex(mX1, mY1, 0.0f);
            addVertex(mX2, mY1, 0.0f);
            addVertex(mX2, mY2, 0.0f);
            addVertex(mX1, mY1, 0.0f);
            addVertex(mX2, mY2, 0.0f);
            addVertex(mX1, mY2, 0.0f);

            addTexCoord(0.0f, 0.0f);
            addTexCoord(1.0f, 0.0f);
            addTexCoord(1.0f, 1.0f);
            addTexCoord(0.0f, 0.0f);
            addTexCoord(1.0f, 1.0f);
            addTexCoord(0.0f, 1.0f);

            addNormal(0.0f, 0.0f, 1.0f);
            addNormal(0.0f, 0.0f, 1.0f);
            addNormal(0.0f, 0.0f, 1.0f);
            addNormal(0.0f, 0.0f, 1.0f);
            addNormal(0.0f, 0.0f, 1.0f);
            addNormal(0.0f, 0.0f, 1.0f);

        } else {
            addVertex(mX1, mY1, 0.0f);
            addVertex(mX2, mY1, 0.0f);
            addVertex(mX2, mY2, 0.0f);
            addVertex(mX1, mY2, 0.0f);
        }
    }

private:
    float mX1;
    float mY1;
    float mX2;
    float mY2;

    Mode mMode;
};

}

#endif



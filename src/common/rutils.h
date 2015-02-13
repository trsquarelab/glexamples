
#ifndef UITLS_H
#define UITLS_H

namespace chikku
{

template <typename T>
class RSize
{
public:
    RSize()
     : mWidth(0),
       mHeight(0)
    {}

    RSize(const T & width, const T & height)
     : mWidth(width),
       mHeight(height)
    {}
    
    void setWidth(const T & width)
    {
        mWidth = width;
    }

    T width() const
    {
        return mWidth;
    }

    void setHeight(const T & height)
    {
        mHeight = height;
    }

    T height() const
    {
        return mHeight;
    }

    bool isNull()
    {
        return mWidth == 0 || mHeight = 0;
    }

private:
    T mWidth;
    T mHeight;
};

typedef RSize<int> RSizeI;
typedef RSize<unsigned int> RSizeUI;
typedef RSize<float> RSizeF;
typedef RSize<double> RSizeD;

template <typename T>
class RRectType
{
public:
    RRectType()
    {}

    RRectType(T x1, T y1, T x2, T y2)
     : mX1(x1),
       mY1(y1),
       mX2(x2),
       mY2(y2)
    {
    }

    bool isEmpty() const {return ((width() == 0) | (height() == 0));}

    T x1() const {return mX1;}
    T y1() const {return mY1;}
    T x2() const {return mX2;}
    T y2() const {return mY2;}

    void setX1(T v) {mX1 = v;}
    void setY1(T v) {mY1 = v;}
    void setX2(T v) {mX2 = v;}
    void setY2(T v) {mY2 = v;}

    T height() const {return mY2-mY1;}
    T width() const {return mX2-mX1;}

private:
    T mX1;
    T mY1;
    T mX2;
    T mY2;
};

typedef RRectType<int> RRectTypeI;
typedef RRectType<unsigned int> RRectTypeUI;
typedef RRectType<float> RRectTypeF;
typedef RRectType<double> RRectTypeD;

}

#endif


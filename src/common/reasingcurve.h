#ifndef EASINGCURVE_H
#define EASINGCURVE_H

namespace chikku
{

class REasingCurve
{
public:
    enum Type {
        Linear,
        QuadIn,
        CubicIn
    };

    REasingCurve(Type type = Linear)
        : mType(type)
    {}

    Type type() const {
        return mType;
    }

    void setType(Type type) {
        mType = type;
    }

    float eval(float t)
    {
        switch (mType) {
			case Linear: {
				break;
			}
	
			case QuadIn: {
				t = t * t;
				break;
			}
	
			case CubicIn: {
				t = t * t * t;
				break;
			}
        }

        return t;
    }

private:
    Type mType;
};

}

#endif


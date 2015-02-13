
#ifndef TRANS_ANIMATION_H
#define TRANS_ANIMATION_H

#include "ranimation.h"

namespace chikku
{

class RTransAnimation: public RAnimation
{
public:
	enum Type {
		Scaling,
		Translation,
		Rotation
	};
	
public:
	RTransAnimation()
	 : mType(Translation),
	   mSA(0),
	   mEA(0)
	{}

	void setType(Type t)
	{
		mType = t;
	}
	
	void setAngles(float s, float e)
	{
		mSA = s;
		mEA = e;
	}
	
	void setPoints(RVector const & s, RVector const & e)
	{
		mSP = s;
		mEP = e;
	}
	
	RTransformation transformation() const
	{
		return mTrans;
	}

protected:
	void update( float elapsed)
	{
	  float t = elapsed / mDuration;

	  float dx = interpolate(mSP.x(), mEP.x(), t);
	  float dy = interpolate(mSP.y(), mEP.y(), t);
	  float dz = interpolate(mSP.z(), mEP.z(), t);

	  mTrans.reset();

	  switch (mType) {
	    case Scaling: {
	      mTrans.scale(dx, dy, dz);
	      break;
	    }

	    case Translation: {
	      mTrans.translate(dx, dy, dz);
	      break;
	    }

	    case Rotation: {
	      float da = interpolate(mSA, mEA, t);
	      mTrans.rotate(da, dx, dy, dz);
	      break;
	    }
	    
	  }
	}	

protected:
	RTransformation mTrans;
	Type mType;
	float mSA;
	float mEA;
	RVector mSP;
	RVector mEP;
};

}

#endif

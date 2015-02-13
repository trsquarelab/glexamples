
#ifndef ANIMATION_H
#define ANIMATION_H

#include <sys/timeb.h>

#include "reasingcurve.h"

namespace chikku
{

class RAnimationListner
{
public:
    virtual ~RAnimationListner()
    {}

    virtual void started()
    {}

    virtual void stopped()
    {}

    virtual void updated()
    {}
};

class RAnimation
{
public:
    enum State {
        None,
        Running,
        Paused,
        Stopped
    };
    
public:
    RAnimation()
     : mDuration(2.0f),
       mInterval(0.02),
       mState(None),
       mListner(0),
       mRepeat(false)
    {}

    float duration() const {
        return mDuration;
    }

    void setDuration(float d) {
        mDuration = d;
    }

    float interval() const {
        return mInterval;
    }

    void setInterval(float i) {
        mInterval = i;
    }
    
    void setRepeat(bool v)
    {
    	mRepeat = v;
    }
    
    void setListner(RAnimationListner * l)
    {
    	mListner = l;
    }
    
    void step() {
    	if (mState == Running) {
			struct timeb currentTime;
			ftime(&currentTime);

			float diffSec = currentTime.time - mPreviousTime.time;
			float diffMill = currentTime.millitm - mPreviousTime.millitm;
			float mselap = diffSec + (diffMill / 1000.0f);
			
			if (mselap >= interval()) {
				
				mPreviousTime = currentTime;
				
				diffSec = currentTime.time - mStartTime.time;
				diffMill = currentTime.millitm - mStartTime.millitm;
				  
				mselap = diffSec + (diffMill / 1000.0f);
	
				update(mselap);
				
				if (mListner) {
					mListner->updated();
				}
				
				if (mselap > mDuration) {
					stop();
				}
			}
    	}
    }
    
    void start() {
    	if (mState != Running) {
            mState = Running;
            ftime(&mStartTime);
            ftime(&mPreviousTime);
            if (mListner) {
            	mListner->started();
            }
            update(0.0f);
    	}
    }
    
    void stop() {
        if (mState != Stopped) {
            mState = Stopped;
            if (mListner) {
            	mListner->stopped();
            }
            if (mRepeat) {
            	start();
            }
        }
    }
    
protected:
    virtual void update(float t) = 0;

    float interpolate(float s, float e, float t) {
        if (t < 0.0) {
            t = 0.0;
        }
        if (t > 1.0) {
            t = 1.0;
        }
        float et = mEC.eval(t);
        return e * et - s * et + s;
    }
    
protected:
	REasingCurve mEC;
	float mDuration;
	float mInterval;
	State mState;
	RAnimationListner * mListner;
	bool mRepeat;
	struct timeb mStartTime;
	struct timeb mPreviousTime;
};

}

#endif


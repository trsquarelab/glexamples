
#ifndef SHAREDOBJECT_H
#define SHAREDOBJECT_H

namespace chikku
{

template <typename T>
class RSharedObject
{
protected:
    class Data
    {
    public:
        int mRefCount;
        T mData;
    };

public:
    RSharedObject()
     : mData(0)
    {}

    RSharedObject(RSharedObject const & o)
    {
        mData = 0;
        copy(o);
    }

    RSharedObject& operator=(RSharedObject const & o)
    {
        copy(o);
        return *this;
    }

    virtual ~RSharedObject()
    {
        dispose();
    }

protected:
    virtual T & data_p()
    {
    	if (!mData) {
    		create();
    	}
        return mData->mData;
    }

    virtual const T & data_p() const
    {
    	if (!mData) {
    		const_cast<RSharedObject<T> *>(this)->create();
    	}
        return mData->mData;
    }

    void copy(RSharedObject const & oth)
    {
		dispose();
        mData = oth.mData;
        ref();
    }

   void detach()
    {
        if (mData && mData->mRefCount > 1) {
            deref();
            mData = new Data(*mData);
            mData->mRefCount = 1;
        }
    }

private:
    void create()
    {
		mData = new Data();
		ref();
    }

    void dispose()
    {
    	if (mData) {
			deref();
			if (mData->mRefCount == 0) {
				delete mData;
				mData = 0;
			}
    	}
    }

    void ref()
    {
    	if (mData) {
    		++mData->mRefCount;
    	}
    }

    void deref()
    {
    	if (mData) {
    		--mData->mRefCount;
    	}
    }

private:
    Data * mData;
};

template <typename T>
class RDetachableRSharedObject: RSharedObject<T>
{
protected:
    T & data_p()
    {
        this->detach();
        return RSharedObject<T>::data_p();
    }

    const T & data_p() const
    {
        return RSharedObject<T>::data_p();
    }


};


}

#endif



#ifndef SMARTPOINTER_H
#define SMARTPOINTER_H

#include "rshared_object.h"

namespace chikku
{

template <typename A, typename D>
class RSmartPointerBase: public RSharedObject<A>
{
public:
    RSmartPointerBase(D * p)
    {
        this->data_p().mPtr = p;
        this->data_p().mOwn = true;
    }

    bool isNull() const
    {
        return data() == 0;
    }

    D * data()
    {
        return this->data_p().mPtr;
    }

    const D * data() const
    {
        return this->data_p().mPtr;
    }

    D * take()
    {
        this->data_p().mOwn = false;
        return this->data_p().mPtr;
    }

    D * operator->()
    {
        return data();
    }

    const D * operator->() const
    {
        return data();
    }

    D & operator*()
    {
        return *(data());
    }

    const D * operator*() const
    {
        return *(data());
    }
};


template <typename D>
class RSmartPointerData
{
public:
    RSmartPointerData()
     : mOwn(true),
       mPtr(0)
    {}

    ~RSmartPointerData()
    {
        if (mOwn) {
            delete mPtr;
        }
    }

private:
    RSmartPointerData(RSmartPointerData const &);
    RSmartPointerData & operator=(RSmartPointerData const &);

public:
    bool mOwn;
    D * mPtr;
};


template <typename D>
class SmartArrayPointerData
{
public:
    SmartArrayPointerData()
     : mOwn(true),
       mPtr(0)
    {}

    ~SmartArrayPointerData()
    {
        if (mOwn) {
            delete [] mPtr;
        }
    }

private:
    SmartArrayPointerData(SmartArrayPointerData const &);
    SmartArrayPointerData & operator=(SmartArrayPointerData const &);

public:
    bool mOwn;
    D * mPtr;
};

template <typename D>
class SmartArrayPointer: public RSmartPointerBase<SmartArrayPointerData<D>, D>
{
public:
    SmartArrayPointer(D * p=0)
     : RSmartPointerBase<SmartArrayPointerData<D>, D>(p)
    {}

    operator D*()
    {
        return this->data();
    }

    operator const D*() const
    {
        return this->data();
    }
};

template <typename D>
class RSmartPointer: public RSmartPointerBase<RSmartPointerData<D>, D>
{
public:
    RSmartPointer(D * p=0)
     : RSmartPointerBase<RSmartPointerData<D>, D>(p)
    {}

    operator D*()
    {
        return this->data();
    }

    operator const D*() const
    {
        return this->data();
    }
};

}

#endif


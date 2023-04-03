#pragma once

template< class ObjectType>
class TSharedPtr
{
public:
	ObjectType* Object;

    int32 SharedReferenceCount;
    int32 WeakReferenceCount;

    FORCEINLINE ObjectType* Get()
    {
        return Object;
    }
    FORCEINLINE ObjectType* Get() const
    {
        return Object;
    }
    FORCEINLINE ObjectType& operator*()
    {
        return *Object;
    }
    FORCEINLINE const ObjectType& operator*() const
    {
        return *Object;
    }
    FORCEINLINE ObjectType* operator->()
    {
        return Object;
    }
    FORCEINLINE ObjectType* operator->() const
    {
        return Object;
    }
};
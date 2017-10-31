#pragma once
#include <utility>
#define MixinReferenceCounted \
private:\
    uint32_t _refcnt = 1;   \
public:\
    uint32_t getRefCount() const {return _refcnt;}\
    void addRef(){++_refcnt;}\
    void release(){--_refcnt; if (_refcnt == 0) delete this;}\
private:
//! Smart reference counted reference holder
template <typename T>
struct ReferenceHolder
{
    T* ptr;
public:
    
    //! Move constructor 
    ReferenceHolder(ReferenceHolder&& other)
    {
        ptr = other.ptr;
        other.ptr = nullptr;
    }
    
    
    //! Move assignment operator
    ReferenceHolder& operator = (ReferenceHolder&& other)
    {
        reset();
        ptr = other.ptr;
        other.ptr = nullptr;
        return *this;
    }
    
    //! Copy constructor
    ReferenceHolder(const ReferenceHolder& other)
    {
        ptr = other.ptr;
        if (ptr)
            ptr->addRef();
    }
    
    
    //! Copy assignment operator
    ReferenceHolder& operator = (const ReferenceHolder& other)
    {
        reset();
        ptr = other.ptr;
        if (ptr)
            ptr->addRef();
        return *this;
    }
    
    
    //! Default constructor
    ReferenceHolder()
    {
        ptr = nullptr;
    }
    
    //! Construct reference counted object with arguments
    template <typename ... Args>
    static ReferenceHolder construct(Args && ... args)
    {
        ReferenceHolder rh;
        rh.ptr = new T(std::forward(args...));
        return rh;
    }
    
    //! Construct reference counted object 
    static ReferenceHolder construct()
    {
        ReferenceHolder rh;
        rh.ptr = new T();
        return rh;
    }
    
    
    
    
    //! Construct reference holder from plain pointer
    static ReferenceHolder create(T* t)
    {
        ReferenceHolder rh;
        rh.ptr = t;
        if (t)
            t->addRef();
        return rh;
    }
    
    //! Destructor
    ~ReferenceHolder()
    {
        reset();
    }
    
    //! Releases the reference to the object
    void reset()
    {
        if (ptr)
            ptr->release();
        ptr = nullptr;
    }
    
    //! Cast to bool
    operator bool() const
    {
        return bool(ptr);
    }
    
    //! Member access operator
    T* operator ->() const
    {
        return ptr;
    }
    
    //! Dereference operator
    T& operator *() const
    {
        return *ptr;
    }
    
    //! Return the internal pointer
    T* get() const
    {
        return ptr;
    }
};

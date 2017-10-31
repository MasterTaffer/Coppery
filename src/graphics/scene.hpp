#pragma once
#include <stdint.h>
#include <vector>
#include <map>
#include "color.hpp"
#include "vector2.hpp"
#include "shades.hpp"

class Graphics;

//! Key used to identify sub-nodes in the graph
typedef intptr_t SceneNodeKey;

//! Base class for all scene nodes
class SceneNodeBase
{
protected:
    //! Pointer to the Graphics instance containing the graph
    Graphics* graphics;
public:
    
    //! Constructor
    SceneNodeBase(Graphics* g) : graphics(g) {}
    
    //! Destructor
    virtual ~SceneNodeBase() {};

    //! Get the unique key for this node
    virtual intptr_t getValue() const = 0;
    
    //! Get a branch with key 
    virtual SceneNodeBase* getBranch(intptr_t) = 0;

    /*! \brief Traverse this node
     * 
     * \return True if the node should be removed, false otherwise.
     */
    virtual bool traverse() = 0;

    /*! \brief Insert a new node
     * 
     * \return True if node is successfully inserted and ownership was acquired
     */
    virtual bool insertNode(SceneNodeBase*) = 0;

    //! Remove all the children of this node
    virtual void clear() = 0;
};

//! A scene node for leaf nodes
class SceneLeaf : public SceneNodeBase
{
protected:
public:
    SceneLeaf(Graphics* g) : SceneNodeBase(g) {}
    virtual intptr_t getValue() const override { return intptr_t(this); }
    SceneNodeBase* getBranch(intptr_t) override { return nullptr; }

    virtual bool traverse() override = 0;
    bool insertNode(SceneNodeBase*) override { return false; }
    virtual void clear() override {};
};

class Drawable;
//! A scene node for Drawable objects
class SceneDrawableLeaf : public SceneLeaf
{
protected:
    Drawable* drawable = nullptr;
    friend class Drawable;
    friend class Graphics;
    bool remove = false;
public:
    SceneDrawableLeaf(Graphics* g, Drawable* d) : SceneLeaf(g), drawable(d) {}
    virtual intptr_t getValue() const override { return intptr_t(drawable); }
    
    //! Set removal of this node
    void setRemoval() {remove = true;}
    virtual bool traverse() override;
};


//! Generic base for scene node branches
class SceneBranch : public SceneNodeBase
{
protected:
    std::multimap<int, SceneNodeBase*> nodes;

    virtual SceneNodeBase* createNode(int) = 0;
public:
    SceneBranch(Graphics* g) : SceneNodeBase(g) {}
    virtual intptr_t getValue() const override = 0;
    virtual bool insertNode(SceneNodeBase*) override;
    virtual bool traverse() override;
    void clear() override;
    SceneNodeBase* getBranch(intptr_t) override;
};

//! Templated generic base for scene node branches
template <intptr_t I, typename T>
class SceneGenericContainer : public SceneBranch
{
protected:

    virtual SceneNodeBase* createNode(int i)
    {
        T* t = new T(graphics, i);
        insertNode(t);
        return t;
    }
public:
    SceneGenericContainer(Graphics* g) : SceneBranch(g) {}
    virtual intptr_t getValue() const override  {return I;}
};




//! Templated generic base for scene node branches containing leaves
template <intptr_t I>
class SceneGenericLeafContainer : public SceneBranch
{
protected:

    virtual SceneNodeBase* createNode(int) override
    {
        return nullptr;
    }
public:
    SceneGenericLeafContainer(Graphics* g) : SceneBranch(g) {}
    virtual intptr_t getValue() const override  {return I;}
};

class SceneMasterBranch : public SceneBranch
{
protected:
    std::vector<unsigned int> perfQuery;
    std::vector<intptr_t> perfQueryKeys;
    virtual SceneNodeBase* createNode(int) override {return nullptr;}
public:
    /*! Return OpenGL query objects and their associated scene node keys
     * 
     * Call with incrementing indexes, starting from 0. When return value's
     * first member is zero, one should stop the iteration.
     * 
     * Pair's first value is the OpenGL query object name. Second value
     * is the SceneNode value of the node.
     */
    std::pair<unsigned int, intptr_t> getPerformanceQuery(size_t i);
    SceneMasterBranch(Graphics* g);
    ~SceneMasterBranch();
    virtual bool traverse() override;
    intptr_t getValue() const override {return 0;}
    virtual bool insertNode(SceneNodeBase*) override;
};

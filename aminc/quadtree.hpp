#pragma once
#include <cassert>
#include <vector>
#include <array>
#include <functional>
#include <algorithm>
#include <unordered_set>

#include "vector2.hpp"

//! A single quadtree entity
template <typename Entity, typename Vector>
class QuadTreeEntity
{
    Vector aa;
    Vector bb;

public:

    Entity entity;

    QuadTreeEntity(const Vector& aa, const Vector& bb) : aa(aa), bb(bb)
    {

    }

    void move(const Vector& a, const Vector& b)
    {
    	aa = a;
    	bb = b;
    }

    void getBounds(Vector& ina, Vector& inb) const
    {
        ina = aa;
        inb = bb;
    }
};

//! Node of the quadtree
template <typename E, typename Vector>
class QuadTreeNode
{
    void subdivide();
    void concentrate();
    void insertToChild(QuadTreeEntity<E, Vector>* c);
    void removeFromChild(QuadTreeEntity<E, Vector>* c);
public:
    QuadTreeNode(Vector tl, Vector br, unsigned int, unsigned int maxLevel, unsigned int maxBucketSize);
    const unsigned int maxLevel;
    const unsigned int maxBucketSize;

    enum Quadrant
    {
        NW = 0,
        NE = 1,
        SW = 2,
        SE = 3
    };

    enum QuadrantBits
    {
        bitNW = 1,
        bitNE = 2,
        bitSW = 4,
        bitSE = 8
    };

    std::vector<QuadTreeEntity<E, Vector>*> contents;

    QuadTreeNode* parent = nullptr;
    std::array<QuadTreeNode*, 4> children;

    bool isLeaf = true;
    unsigned int level = 0;

    const Vector topLeft;
    const Vector bottomRight;

    Vector getCenter()
    {
        return (topLeft + bottomRight) / 2;
    }
    void getQuadrantBounds(unsigned int i, Vector& intl, Vector& inbr);
    void insert(QuadTreeEntity<E, Vector>*);
    bool remove(QuadTreeEntity<E, Vector>*);
    void balance();
    void clear();

    unsigned int getInQuadrants(Vector a, Vector b);
};

/*! The main quadtree organizer
 * 
 * 
 * \tparam E type of entity stored in quad tree
 * \tparam Vector a 2d vector type used
 */

template <typename E, typename Vector>
class QuadTreeHolder
{
    const typename Vector::dimensionType baseTreeSize;
    const unsigned int maxLevel;
    const unsigned int maxBucketSize;
    unsigned int columns = 0;
    unsigned int rows = 0;
    bool initialized = false;
    Vector topLeft;
    Vector bottomRight;

    //Vector containing all quadtrees in row major order
    //size is always columns*rows
    std::vector<QuadTreeNode<E, Vector>*> trees;
    QuadTreeNode<E, Vector>* getTree(unsigned int x, unsigned int y);

public:
    
    //! Returns true if create has been called for this instance
    bool isInitialized()
    {
        return initialized;
    }

	//! Calls a function on all possibly colliding pairs in quadtree
    void operatePairs(std::function<void(QuadTreeEntity<E, Vector>*, QuadTreeEntity<E, Vector>*)>);

    //! Gets all entities within defined AABB
    void areaFind(Vector tl, Vector br, std::function<void(QuadTreeEntity<E, Vector>*)>);

    /*! \brief Initializes the quadtree collision world

    	The arguments define the size of the collision world.
    	The x and y components of parameter tl should be smaller
    	than corresponding components in the parameter br.

		\param tl the smallest coordinate point of the collision world
		\param br the largest coordinate point of the collision world

	*/
    void create(Vector tl, Vector br);

    //! Deletes the quadtree array
    void clear();

    //! Inserts an entity
    void insert(QuadTreeEntity<E, Vector>*);

    //! Removes an entity
    void remove(QuadTreeEntity<E, Vector>*);

	/*! \brief Constructs QuadTreeHolder object

		The constructor requires knowledge of the overall
		scale of the collision world. The parameter baseTreeSize
		should usually encompass the whole collision world.
        If baseTreeSize is smaller than the collision world
        multiple quad trees are used, which might or might not be 
        inefficient.

        One should fine tune the maximum subdivision level and
        the bucket size. 

		\param baseTreeSize size of a single root quadtree
        \param maxLevel maximum subdivision of a quad tree
        \param maxBucketSize maximum quad tree bucket size

	*/
    QuadTreeHolder(typename Vector::dimensionType baseTreeSize, unsigned int maxLevel, unsigned int maxBucketSize) : baseTreeSize(baseTreeSize), maxLevel(maxLevel), maxBucketSize(maxBucketSize)
    {

    }
    ~QuadTreeHolder();
};

template <typename E, typename Vector>
QuadTreeNode<E, Vector>::QuadTreeNode(Vector tl, Vector br, unsigned int level, unsigned int maxLevel, unsigned int maxBucketSize)
: topLeft(tl), bottomRight(br), level(level), maxLevel(maxLevel), maxBucketSize(maxBucketSize)
{
    for (QuadTreeNode*& qt : children)
        qt = nullptr;
}

template <typename E, typename Vector>
void QuadTreeNode<E, Vector>::clear()
{
    if (isLeaf)
        return;
    for (QuadTreeNode*& qt : children)
    {
        qt->clear();
        delete qt;
    }
}

template <typename E, typename Vector>
void QuadTreeNode<E, Vector>::getQuadrantBounds(unsigned int i, Vector &intl, Vector &inbr)
{
    Vector center = getCenter();
    switch (i)
    {
        case NW:
            intl = topLeft;
            inbr = center;
            return;
        case NE:
            intl = Vector(center.x,topLeft.y);
            inbr = Vector(bottomRight.x,center.y);
            return;
        case SW:
            intl = Vector(topLeft.x,center.y);
            inbr = Vector(center.x,bottomRight.y);
            return;
        default:
        case SE:
            intl = center;
            inbr = bottomRight;
            return;
    }
}

template <typename E, typename Vector>
unsigned int QuadTreeNode<E, Vector>::getInQuadrants(Vector tl, Vector br)
{
    unsigned int res = 0;
    Vector center = getCenter();
    if (tl.allLessOrEqual(center))
        res |= bitNW;
    if (br.allGreaterOrEqual(center))
        res |= bitSE;
    if (br.x >= center.x && tl.y <= center.y)
        res |= bitNE;
    if (tl.x <= center.x && br.y >= center.y)
        res |= bitSW;
    return res;
}

template <typename E, typename Vector>
void QuadTreeNode<E, Vector>::insertToChild(QuadTreeEntity<E, Vector> *c)
{
    if (isLeaf)
        return;
    Vector tl,br;
    c->getBounds(tl,br);

    unsigned int r = getInQuadrants(tl,br);
    if (r & bitNE)
        children[NE]->insert(c);
    if (r & bitNW)
        children[NW]->insert(c);
    if (r & bitSE)
        children[SE]->insert(c);
    if (r & bitSW)
        children[SW]->insert(c);
}

template <typename E, typename Vector>
void QuadTreeNode<E, Vector>::removeFromChild(QuadTreeEntity<E, Vector> *c)
{
    if (isLeaf)
        return;
    Vector tl,br;
    c->getBounds(tl,br);

    bool alltrue = true;
    unsigned int r = getInQuadrants(tl,br);
    if (r & bitNE)
        alltrue &= children[NE]->remove(c);
    if (r & bitNW)
        alltrue &= children[NW]->remove(c);
    if (r & bitSE)
        alltrue &= children[SE]->remove(c);
    if (r & bitSW)
        alltrue &= children[SW]->remove(c);
    if (alltrue)
        balance();
}

template <typename E, typename Vector>
void QuadTreeNode<E, Vector>::subdivide()
{
    if (!isLeaf)
        return;
    unsigned int q = 0;

    //Log << "Subdividin'  (" << topLeft.x << ", " << topLeft.y << ") - (" << bottomRight.x << ", " << bottomRight.y << ")" << Message();
    for (QuadTreeNode*& qt : children)
    {


        Vector tl,br;
        getQuadrantBounds(q,tl,br);
        //Log << "To'  (" << tl.x << ", " << tl.y << ") - (" << br.x << ", " << br.y << ")" << Message();
        qt = new QuadTreeNode(tl,br,level+1,maxLevel,maxBucketSize);
        qt->parent = this;
        q++;
    }
    isLeaf = false;
    for (QuadTreeEntity<E, Vector>* col : contents)
    {
        insertToChild(col);
    }
    contents.clear();
}

template <typename E, typename Vector>
void QuadTreeNode<E, Vector>::concentrate()
{
    if (isLeaf)
        return;
    std::unordered_set<QuadTreeEntity<E, Vector>*> colls;
    for (QuadTreeNode*& qt : children)
    {
        for (QuadTreeEntity<E, Vector>* c : qt->contents)
        {
            colls.insert(c);
        }
        delete qt;
        qt = nullptr;
    }
    isLeaf = true;
    for (QuadTreeEntity<E, Vector>* c : colls)
    {
        insert(c);
    }

}

template <typename E, typename Vector>
bool QuadTreeNode<E, Vector>::remove(QuadTreeEntity<E, Vector>* c)
{
    if (isLeaf)
    {
        auto it = std::find(contents.begin(), contents.end(), c);
        if (it != contents.end())
        {
            #ifdef DEBUGS
                (*it)->qtreeCount --;
            #endif
            contents.erase(it);
        }
        else
        {
            assert(0);
        }
        if (contents.size() < 2)
            return true;
    }
    else
    {
        removeFromChild(c);
        if (isLeaf) //jos poiston yhteydessä puu tasapainotettiin, se voi muuttua lehdeksi
            return true;
    }
    return false;
}

template <typename E, typename Vector>
void QuadTreeNode<E, Vector>::insert(QuadTreeEntity<E, Vector>* c)
{
    if (isLeaf)
    {
        contents.push_back(c);
        if ((contents.size() > maxBucketSize) && (level < maxLevel))
        {
            subdivide();
        }

    }
    else
    {
        insertToChild(c);
    }
}

template <typename E, typename Vector>
void QuadTreeNode<E, Vector>::balance()
{
    if (isLeaf)
    {
        parent->balance();
    }
    else
    {
        bool canCon = true;
        QuadTreeEntity<E, Vector>* only = nullptr;
        for (QuadTreeNode* qt : children)
        {
            if (qt->isLeaf)
            {
                if (qt->contents.size() > 1)
                {
                    canCon = false;
                    break;
                }
                else
                {
                    if (qt->contents.size() == 0)
                        continue;
                    if (only == nullptr)
                        only = qt->contents[0];
                    else
                    {
                        if (only != qt->contents[0])
                        {
                            canCon = false;
                            break;
                        }
                    }
                }

            }
            else
            {
                canCon = false;
                break;
            }
        }
        if (canCon)
        {
            concentrate();
        }
    }
}

template <typename E, typename Vector>
void QuadTreeHolder<E, Vector>::operatePairs(std::function<void(QuadTreeEntity<E, Vector>*, QuadTreeEntity<E, Vector>*)> func)
{
    unsigned int maxLev = 0;
    std::function<void(QuadTreeNode<E, Vector>*)> recursion = [&](QuadTreeNode<E, Vector>* qn)
    {
        if (qn->isLeaf)
        {
            for (auto it = qn->contents.begin(); it != qn->contents.end(); it++)
                for (auto it2 = it+1; it2 != qn->contents.end(); it2++)
                {
                    func(*it,*it2);
                }
            return;
        }
        recursion(qn->children[QuadTreeNode<E, Vector>::NE]);
        recursion(qn->children[QuadTreeNode<E, Vector>::NW]);
        recursion(qn->children[QuadTreeNode<E, Vector>::SE]);
        recursion(qn->children[QuadTreeNode<E, Vector>::SW]);
        maxLev = std::max(maxLev, qn->level);
    };
    for (auto t : trees)
        recursion(t);

}

template <typename E, typename Vector>
void QuadTreeHolder<E, Vector>::areaFind(Vector tl, Vector br, std::function<void(QuadTreeEntity<E, Vector>*)> func)
{
    std::function<void(QuadTreeNode<E, Vector>*)> recursion = [&](QuadTreeNode<E, Vector>* qn)
    {
        if (qn->isLeaf)
        {
            for (QuadTreeEntity<E, Vector>* c : qn->contents)
                func(c);
            return;
        }
        unsigned int r = qn->getInQuadrants(tl, br);
        if (r & QuadTreeNode<E, Vector>::bitNE)
            recursion(qn->children[QuadTreeNode<E, Vector>::NE]);
        if (r & QuadTreeNode<E, Vector>::bitNW)
            recursion(qn->children[QuadTreeNode<E, Vector>::NW]);
        if (r & QuadTreeNode<E, Vector>::bitSE)
            recursion(qn->children[QuadTreeNode<E, Vector>::SE]);
        if (r & QuadTreeNode<E, Vector>::bitSW)
            recursion(qn->children[QuadTreeNode<E, Vector>::SW]);
    };

    Vector2i min = tl/baseTreeSize;
    Vector2i max = br/baseTreeSize + Vector2i(1,1);
    Vector2i a;
    for (a.x = min.x; a.x <= max.x; a.x++)
    for (a.y = min.y; a.y <= max.y; a.y++)
    {
        QuadTreeNode<E, Vector>* qn = getTree(a.x,a.y);
        if (qn != nullptr)
            recursion(qn);
    }


}

template <typename E, typename Vector>
void QuadTreeHolder<E, Vector>::insert(QuadTreeEntity<E, Vector>* t)
{
    Vector tl, br;
    t->getBounds(tl,br);
    Vector2i min = {static_cast<int>(tl.x/baseTreeSize),static_cast<int>(tl.y/baseTreeSize)};
    Vector2i max = {static_cast<int>(br.x/baseTreeSize),static_cast<int>(br.y/baseTreeSize)};
    Vector2i a;
    for (a.x = min.x; a.x <= max.x; a.x++)
    for (a.y = min.y; a.y <= max.y; a.y++)
    {
        QuadTreeNode<E, Vector>* qn = getTree(a.x,a.y);
        if (qn != nullptr)
            qn->insert(t);
    }
}

template <typename E, typename Vector>
void QuadTreeHolder<E, Vector>::remove(QuadTreeEntity<E, Vector>* t)
{
    Vector tl, br;
    t->getBounds(tl,br);
    Vector2i min = {static_cast<int>(tl.x/baseTreeSize),static_cast<int>(tl.y/baseTreeSize)};
    Vector2i max = {static_cast<int>(br.x/baseTreeSize),static_cast<int>(br.y/baseTreeSize)};
    Vector2i a;
    for (a.x = min.x; a.x <= max.x; a.x++)
    for (a.y = min.y; a.y <= max.y; a.y++)
    {
        QuadTreeNode<E, Vector>* qn = getTree(a.x,a.y);
        if (qn != nullptr)
            qn->remove(t);
    }
}

template <typename E, typename Vector>
QuadTreeNode<E, Vector>* QuadTreeHolder<E, Vector>::getTree(unsigned int x, unsigned int y)
{
    unsigned int index = x + y * columns;
    if (index < 0 || index >= trees.size())
        return nullptr;
    return trees[index];
}

template <typename E, typename Vector>
void QuadTreeHolder<E, Vector>::create(Vector tl, Vector br)
{
    if (initialized)
        clear();
    topLeft = tl;
    bottomRight = br;

    Vector size = br-tl;

    columns = ceil(size.x/baseTreeSize);
    rows = ceil(size.y/baseTreeSize);

    for (unsigned int y = 0; y < rows; y++)
    for (unsigned int x = 0; x < columns; x++)
    {
        Vector tl = {baseTreeSize*x, baseTreeSize*y};
        Vector br = {(baseTreeSize)*(x+1), (baseTreeSize)*(y+1)};
        QuadTreeNode<E, Vector>* q = new QuadTreeNode<E, Vector>(tl,br,0,maxLevel, maxBucketSize);
        trees.push_back(q);
    }
    initialized = true;
}

template <typename E, typename Vector>
void QuadTreeHolder<E, Vector>::clear()
{
    for (QuadTreeNode<E, Vector>* q : trees)
    {
        q->clear();
        delete q;
    }
    trees.clear();
    topLeft = {0,0};
    bottomRight = {0,0};
    columns = 0;
    rows = 0;
    initialized = false;
}

template <typename E, typename Vector>
QuadTreeHolder<E, Vector>::~QuadTreeHolder()
{
    if (initialized)
        clear();
}

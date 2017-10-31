#pragma once
#include <type_traits>

#include <cmath>

#ifndef M_PI
#define M_PI 3.1415926535899
#endif

//! \brief A structure representing a mathematical 2 dimensional Vector
template <typename T>
class Vector2
{
public:
    typedef T dimensionType;
    //! The x dimension of the vector
    union {
        T x;
        T u;
        T width;
        };

    //! The y dimension of the vector
    union {
        T y;
        T v;
        T height;
        };

    //! Constructor from dimensions
    Vector2(T x, T y)
        : x{x}, y{y}
    {}

    //! Default constructor
    Vector2()
    {}

    //! Copy constructor from different typed Vector2 instances
    template <typename Y>
    Vector2(const Vector2<Y>& b)
        : x{T(b.x)}, y{T(b.y)}
    {}

    //! Assignment operator from different typed Vector2 instances
    template <typename Y>
    void operator = (const Vector2<Y>& b)
    {
        x = T(b.x);
        y = T(b.y);
    }

    //! Operator overload for addition assignment between vectors
    void operator += (const Vector2& b)
    {
        x += b.x;
        y += b.y;
    }

    //! Operator overload for subtraction assignemnt between vectors
    void operator -= (const Vector2& b)
    {
        x -= b.x;
        y -= b.y;
    }

    //! Operator overload for multiplication assignment between vectors
    void operator *= (const Vector2& b)
    {
        x *= b.x;
        y *= b.y;
    }

    //! Operator overload for division assignment between vectors
    void operator /= (const Vector2& b)
    {
        x /= b.x;
        y /= b.y;        
    };

    //! Operator equality overload
    bool operator == (const Vector2& b) const
    {
        return (x == b.x) && (y == b.y);
    }

    //! Operator inequality overload
    bool operator != (const Vector2& b) const
    {
        //NOT equals
        return (!(*this == b));
    };

    //Operator overload for unary minus
    Vector2 operator - () const
    {
        return {-x, -y};
    }

    //! Operator overload for addition of vectors
    template <typename U>
    Vector2 operator + (const Vector2<U>& b) const
    {
        Vector2 c;
        c.x = x + b.x;
        c.y = y + b.y;
        return c;
    }


    //! Operator overload for subtraction of vectors
    template <typename U>
    Vector2 operator - (const Vector2<U>& b) const
    {
        Vector2 c;
        c.x = x-b.x;
        c.y = y-b.y;
        return c;
    }

    //! Operator overload for elementwise division of vectors
    template <typename U>
    Vector2 operator / (const Vector2<U>& b) const
    {
        Vector2 c;
        c.x = x/b.x;
        c.y = y/b.y;
        return c;
    }

    //! Operator overload for elementwise multiplication of vectors
    template <typename U>
    Vector2 operator * (const Vector2<U>& b) const
    {
        Vector2 c;
        c.x = x*b.x;
        c.y = y*b.y;
        return c;
    }

    //! Operator overload for multiplication with a scalar
    template <typename U>
    Vector2 operator * (U b) const
    {
        Vector2 c;
        c.x = x*b;
        c.y = y*b;
        return c;
    }

    //! Operator overload for division with a scalar
    template <typename U>
    Vector2 operator / (U b) const
    {
        Vector2 c;
        c.x = x/b;
        c.y = y/b;
        return c;
    }

    //! Operator overload for addition with a scalar
    template <typename U>
    Vector2 operator + (U b) const
    {
        Vector2 c;
        c.x = x+b;
        c.y = y+b;
        return c;
    }

    //! Operator overload for subtraction with a scalar
    template <typename U>
    Vector2 operator - (U b) const
    {
        Vector2 c;
        c.x = x-b;
        c.y = y-b;
        return c;
    }

    //! Operator overload for multiplication assignment with a scalar
    template <typename U>
    void operator *= (U b)
    {
        x = x*b;
        y = y*b;
    }

    //! Operator overload for division assignment with a scalar
    template <typename U>
    void operator /= (U b)
    {
        x = x/b;
        y = y/b;
    }

    //! Operator overload for assignment from a scalar
    template <typename U>
    void operator = (U b)
    {
        x = b;
        y = b;
    }

    /*! \brief Returns if the vector is within an AABB defined by two other
     * vectors
     * 
     * Returns true if both elements of the vector are greater than the
     * corresponding elements in parameter a, and less than the elements
     * in parameter b.
     * 
     * \param a the lower boundary of AABB
     * \param b the greater boundary of AABB
     * \return true if the vector is within the AABB, false otherwise
    */
    bool inside(Vector2 a, Vector2 b) const
    {
        return (x>a.x && y>a.y && x<b.x && y<b.y);
    }

	#ifndef AM_VECTOR2_DONT_INCLUDE_MATH

    //! Returns the length of the vector
    double length() const
    {
        return hypot(x,y);
    }

	/*
    //! retuns the manhattan length of the vector
    T manhattanLength() const
    {
        return std::abs(x) + std::abs(y);
    }


    //! retuns the manhattan length of the vector
    T minkowskiLength() const
    {
        return std::abs(x) + std::abs(y);
    }

    //! returns the Chebyshev length of the vector
    T chebyshevLength() const
    {
        return std::abs(double(x)) > std::abs(double(y)) ? std::abs(double(x)) : std::abs(double(y)) ;
    }
	*/

    /*! \brief Returns the normalized version of the vector, defined "v / length(v)"
     * 
     * If the vector is of zero length a {1, 0} vector is returned.
     */
    
    Vector2 normalize() const
    {
    	T len = length();
    	if (len == 0)
    		return Vector2(1,0);
        return Vector2(x,y)/len;
    }

    /*! \brief Constructs a normalized vector from radian angle

        The angle is defined as follows: 0 returns {1, 0},
        pi/2 returns {0,-1}, pi returns {-1, 0} and pi*3/2 returns {0, 1}
    */
    static Vector2<T> generateFromRadians(double a)
    {
        Vector2<T> t;
        t.x = T(cos(a));
        t.y = T(-sin(a));
        return t;
    }

    /*! \brief Constructs a normalized vector from an angle in degrees

        The angle is defined as follows: 0 returns {1, 0},
        90 returns {0,-1}, 180 returns {-1, 0} and 270 returns {0, 1}
    */
    static Vector2<T> generateFromDegrees(double a)
    {
        Vector2<T> t;
        a*=M_PI/180.0;
        t.x = T(cos(a));
        t.y = T(-sin(a));
        return t;
    }

    /*! \brief Returns the angle of the vector in degrees

        The angle is returned in format that
        x = generateFromDegrees(x.normalized().getAngleDegrees()).
    */
    double getAngleDegrees() const
    {
        return atan2(-double(y), double(x))*180.0/M_PI;
    }

    #endif

    //! Returns the dot product of the vectors
    T dot(const Vector2& b) const
    {
        return x*b.x+y*b.y;
    }

    /*! \brief Returns the Z-component of the cross product between the
     * vectors
     * 
     * The 3rd dimensional component required by an cross product is set to 0.
     */
    T cross(const Vector2& b) const
    {
        return x*b.y-y*b.x;
    }

    //! Test if the dimensional components are all greater or equal
    bool allGreaterOrEqual(const Vector2& b) const
    {
        return (x >= b.x && y >= b.y);
    }

    //! Test if the dimensional components are all less or equal
    bool allLessOrEqual(const Vector2& b) const
    {
        return (x <= b.x && y <= b.y);
    }

};

//! Specialization of the Vector2 class for float
typedef Vector2<float> Vector2f;

//! Specialization of the Vector2 class for double
typedef Vector2<double> Vector2d;

//! Specialization of the Vector2 class for int
typedef Vector2<int> Vector2i;



#pragma once
#include "vector2.hpp"

/*! \file
 * \brief Common Game Engine settings
 * 
 * One should define common engine flags and settings here.
 * 
 * ## ENGINE_INTEGER_COLLISION_DETECTION
 * 
 * If this is defined, the collision detection engine will use integer
 * components instead the DefVector2 defined components in the collision
 * detection.
 */

/*! \def DefVector2Component
 *  \brief The component type of Vector2 type DefVector2
 */

#define DefVector2Component double


/*! \def DefVector2ComponentStringify
 *  \brief The stringified version of DefVector2Component
 */

#define DefVector2ComponentStringify "double"

//#define ENGINE_INTEGER_COLLISION_DETECTION

/*! \typedef DefVector2
 *  \brief The Vector2 type used in internal engine operations
 * 
 * This Vector2 type is used in most of internal engine operations, such as
 * collision detection, tile map and script engine. This differs from Vector2
 * used in Graphics which is usually Vector2f.
 */
typedef Vector2<DefVector2Component> DefVector2;

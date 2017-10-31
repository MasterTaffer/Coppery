#pragma once
#include "vector2.hpp"


class Graphics;

//! Abstract base for Particle systems
class ParticleSystem
{
public:
    //! Update the system
    virtual bool update(Graphics* g) = 0;

    //! Draw the system
    virtual void draw(Graphics* g) = 0;

    //! Virtual destructor
    virtual ~ParticleSystem() {};

    //! Remove all particles from system
    virtual void clear() = 0;

    //! Add a new particle with position and velocity
    virtual void addParticle(Vector2f pos, Vector2f vel) = 0;
};

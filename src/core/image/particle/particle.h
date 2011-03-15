/*
 *  Aethyra
 *  Copyright (C) 2006  The Mana World Development Team
 *
 *  This file is part of Aethyra based on original code
 *  from The Mana World.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _PARTICLE_H
#define _PARTICLE_H

#include <list>
#include <string>

#include "../../map/sprite/sprite.h"

#include "../../utils/vector.h"

#include "../../../bindings/guichan/guichanfwd.h"

class Map;
class Particle;
class ParticleEmitter;

typedef std::list<Particle *> Particles;
typedef Particles::iterator ParticleIterator;
typedef std::list<ParticleEmitter *> Emitters;
typedef Emitters::iterator EmitterIterator;

/**
 * A particle spawned by a ParticleEmitter.
 */
class Particle : public Sprite
{
    public:
        enum AliveStatus
        {
            ALIVE = 0,
            DEAD_TIMEOUT = 1,
            DEAD_FLOOR = 2,
            DEAD_SKY = 4,
            DEAD_IMPACT = 8,
            DEAD_OTHER = 16,
            DEAD_LONG_AGO = 128
        };

        static const float PARTICLE_SKY; /**< Maximum Z position of particles */
        static int fastPhysics;          /**< Mode of squareroot calculation */
        static int particleCount;        /**< Current number of particles */
        static int maxCount;             /**< Maximum number of particles */
        static int emitterSkip;          /**< Duration of pause between two
                                              emitter updates in ticks */
        static bool enabled;             /**< Whether unnecessary particle
                                              effects are enabled or not */

        /**
         * Constructor.
         *
         * @param map the map this particle will add itself to, may be NULL
         */
        Particle(Map *map);

        /**
         * Destructor.
         */
        ~Particle();

        /**
         * Deletes all child particles and emitters.
         */
        void clear();

        /**
         * Gives a particle the properties of an engine root particle and loads
         * the particle-related config settings.
         */
        void setupEngine();

        /**
         * Updates particle position, returns false when the particle should
         * be deleted.
         */
        virtual bool update();

        /**
         * Draws the particle image.
         */
        virtual void draw(Graphics *graphics, const int offsetX,
                          const int offsetY) const {}

        /**
         * Necessary for sorting with the other sprites.
         *
         * TODO: Get rid of the hardcoded offset carefully.
         */
        virtual const int getPixelY() const { return (int) (mPos.y + mPos.z) - 64; }

        /**
         * Sets the map the particle is on.
         */
        void setMap(Map *map);

        /**
         * Creates a child particle that hosts some emitters described in the
         * particleEffectFile.
         */
        Particle *addEffect(const std::string &particleEffectFile,
                            const int pixelX, const int pixelY,
                            const int rotation = 0);

        /**
         * Creates a standalone text particle.
         */
        Particle *addTextSplashEffect(const std::string &text, const int x,
                                      const int y, const gcn::Color *color,
                                      gcn::Font *font,
                                      const bool outline = false);

        /**
         * Creates a standalone text particle.
         */
        Particle *addTextRiseFadeOutEffect(const std::string &text,
                                           const int x, const int y,
                                           const gcn::Color *color,
                                           gcn::Font *font,
                                           const bool outline = false);

        /**
         * Adds an emitter to the particle.
         */
        void addEmitter (ParticleEmitter* emitter)
        { mChildEmitters.push_back(emitter); }

        /**
         * Sets the position in 3 dimensional space in pixels relative to map.
         */
        void moveTo(const Vector &pos) { moveBy (pos - mPos);}

        /**
         * Sets the position in 2 dimensional space in pixels relative to map.
         */
        void moveTo(const float x, const float y);

        /**
         * Returns the particle position.
         */
        const Vector& getPosition() const { return mPos; }

        /**
         * Changes the particle position relative
         */
        void moveBy (const Vector &change);

        /**
         * Sets the time in game ticks until the particle is destroyed.
         */
        void setLifetime(const int lifetime)
        { mLifetimeLeft = lifetime; mLifetimePast = 0; }

        /**
         * Sets the age of the pixel in game ticks where the particle has
         * faded in completely.
         */
        void setFadeOut(const int fadeOut) { mFadeOut = fadeOut; }

        /**
         * Sets the remaining particle lifetime where the particle starts to
         * fade out.
         */
        void setFadeIn(const int fadeIn) { mFadeIn = fadeIn; }

        /**
         * Calculates the current alpha transparency taking current fade status
         * into account
         */
        float getCurrentAlpha() const;

        /**
         * Sets the alpha value of the particle
         */
        void setAlpha(const float alpha) { mAlpha = alpha; }

        virtual void setDeathEffect(const std::string &effectFile, char conditions)
        { mDeathEffect = effectFile; mDeathEffectConditions = conditions; }

        /**
         * Sets the sprite iterator of the particle on the current map to make
         * it easier to remove the particle from the map when it is destroyed.
         */
        void setSpriteIterator(std::list<Sprite*>::iterator spriteIterator)
        { mSpriteIterator = spriteIterator; }

        /**
         * Gets the sprite iterator of the particle on the current map.
         */
        const std::list<Sprite*>::iterator &getSpriteIterator() const
        { return mSpriteIterator; }

        /**
         * Sets the current velocity in 3 dimensional space.
         */
        void setVelocity(const float x, const float y, const float z)
        { mVelocity.x = x; mVelocity.y = y; mVelocity.z = z; }

        /**
         * Sets the downward acceleration.
         */
        void setGravity(const float gravity) { mGravity = gravity; }

        /**
         * Sets the amount of random vector changes
         */
        void setRandomness(const int r) { mRandomness = r; }

        /**
         * Sets the amount of velocity particles retain after
         * hitting the ground.
         */
        void setBounce(const float bouncieness) { mBounce = bouncieness; }

        /**
         * Sets the flag if the particle is supposed to be moved by its parent
         */
        void setFollow(const bool follow) { mFollow = follow; }

        /**
         * Gets the flag if the particle is supposed to be moved by its parent
         */
        const bool doesFollow() { return mFollow; }

        /**
         * Makes the particle move toward another particle with a
         * given acceleration and momentum
         */
        void setDestination(Particle *target, const float accel,
                            const float moment)
        { mTarget = target; mAcceleration = accel; mMomentum = moment; }

        /**
         * Sets the distance in pixel the particle can come near the target
         * particle before it is destroyed. Does only make sense after a target
         * particle has been set using setDestination.
         */
        void setDieDistance(const float dist) { mInvDieDistance = 1.0f / dist; }

        bool isAlive() const { return mAlive == ALIVE; }

        /**
         * Determines whether the particle and its children are all dead
         */
        bool isExtinct() const { return !isAlive() && mChildParticles.empty(); }

        /**
         * Manually marks the particle for deletion.
         */
        void kill() { mAlive = DEAD_OTHER; mAutoDelete = true; }

        /**
         * After calling this function the particle will only request
         * deletion when kill() is called
         */
        void disableAutoDelete() { mAutoDelete = false; }

        /**
         * Adjusts the emitter detail level
         */
        void changeParticleDetailLevel(const int value);

    protected:
        float mAlpha;               /**< Opacity of the graphical representation
                                         of the particle */
        Vector mPos;                /**< Position in pixels relative to map. */
        int mLifetimeLeft;          /**< Lifetime left in game ticks*/
        int mLifetimePast;          /**< Age of the particle in game ticks*/
        int mFadeOut;               /**< Lifetime in game ticks left where
                                         fading out begins*/
        int mFadeIn;                /**< Age in game ticks where fading in is
                                         finished*/
        Vector mVelocity;           /**< Speed in pixels per game-tick. */

    private:
        AliveStatus mAlive;         /**< Is the particle supposed to be drawn
                                         and updated?*/
        // generic properties
        bool mAutoDelete;           /**< May the particle request its deletion
                                         by the parent particle? */
        Map *mMap;                  /**< Map the particle is on. */
        std::list<Sprite*>::iterator mSpriteIterator;
                                    /**< iterator of the particle on the current
                                         map */
        Emitters mChildEmitters;    /**< List of child emitters. */
        Particles mChildParticles;  /**< List of particles controlled by this
                                         particle */
        std::string mDeathEffect;   /**< Particle effect file to be spawned when
                                         the particle dies */
        char mDeathEffectConditions;/**< Bitfield of death conditions which
                                         trigger spawning of the death particle */

        // dynamic particle
        float mGravity;             /**< Downward acceleration in pixels per
                                         game-tick. */
        int mRandomness;            /**< amount of random vector change */
        float mBounce;              /**< How much the particle bounces off when
                                         hitting the ground */
        bool mFollow;               /**< is this particle moved when its parent
                                         particle moves? */

        // follow-point particles
        Particle *mTarget;          /**< The particle that attracts this
                                         particle */
        float mAcceleration;        /**< Acceleration towards the target
                                         particle in pixels per game-tick� */
        float mInvDieDistance;      /**< Distance in pixels from the target
                                         particle that causes the destruction of
                                         the particle*/
        float mMomentum;            /**< How much speed the particle retains
                                         after each game tick*/
};

extern Particle *particleEngine;

#endif

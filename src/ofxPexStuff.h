//
//  ofxPexStuff.h
//
//  Created by nausea on 1/22/15.
//
//

#pragma once
#include "ofMain.h"

// ------------------------------------------------------------------------
// Structures
// ------------------------------------------------------------------------

// Structure that defines the elements which make up a color
typedef struct {
	GLfloat red;
	GLfloat green;
	GLfloat blue;
	GLfloat alpha;
} Color4f;

// Structure that defines a vector using x and y
struct Vector2f{
    Vector2f operator+(const Vector2f v){
        this->x += v.x;
        this->y += v.y;
        return *this;
    }
    Vector2f operator+(const ofVec2f v){
        this->x += v.x;
        this->y += v.y;
        return *this;
    }
    Vector2f& operator+=(const Vector2f v){
        this->x += v.x;
        this->y += v.y;
        return *this;
    }
    Vector2f& operator+=(const ofVec2f v){
        this->x += v.x;
        this->y += v.y;
        return *this;
    }
    
    Vector2f operator*(float f){
        this->x *= f;
        this->y *= f;
        return *this;
    }
    Vector2f& operator*=(float f){
        this->x *= f;
        this->y *= f;
        return *this;
    }
    
	GLfloat x;
	GLfloat y;
};

// Particle type
enum kParticleTypes
{
	kParticleTypeGravity = 0,
	kParticleTypeRadial = 1
};

// Structure that holds the location and size for each point sprite
typedef struct
{
	GLfloat x;
	GLfloat y;
	GLfloat size;
	Color4f color;
} PointSprite;

// Structure used to hold particle specific information
typedef struct
 {
 Vector2f	position;
 Vector2f	direction;
 Vector2f	startPos;
 Color4f		color;
 Color4f		deltaColor;
 GLfloat		radialAcceleration;
 GLfloat		tangentialAcceleration;
 GLfloat		radius;
 GLfloat		radiusDelta;
 GLfloat		angle;
 GLfloat		degreesPerSecond;
 GLfloat		particleSize;
 GLfloat		particleSizeDelta;
 GLfloat		timeToLive;
 
 GLfloat     sIdx;
 GLfloat     z; //used for interaction
 } Particle;

// ------------------------------------------------------------------------
// Macros
// ------------------------------------------------------------------------

// Macro which returns a random value between -1 and 1
#define RANDOM_MINUS_1_TO_1() (ofRandom( -1.0f, 1.0f ))

// Macro which returns a random number between 0 and 1
#define RANDOM_0_TO_1() (ofRandom( 0.0f, 1.0f ))

// Macro which converts degrees into radians
#define DEGREES_TO_RADIANS(__ANGLE__) ((__ANGLE__) / 180.0 * PI)

// ------------------------------------------------------------------------
// Inline functions
// ------------------------------------------------------------------------

// Return a Color4f structure populated with 1.0's
static const Color4f Color4fOnes = {1.0f, 1.0f, 1.0f, 1.0f};

// Return a zero populated Vector2f
static const Vector2f Vector2fZero = {0.0f, 0.0f};

// Return a populated Vector2d structure from the floats passed in
static inline Vector2f Vector2fMake(GLfloat x, GLfloat y) {
	Vector2f r; r.x = x; r.y = y;
	return r;
}

// Return a Color4f structure populated with the color values passed in
static inline Color4f Color4fMake(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
	Color4f c; c.red = red; c.green = green; c.blue = blue; c.alpha = alpha;
	return c;
}

// Return a Vector2f containing v multiplied by s
static inline Vector2f Vector2fMultiply(Vector2f v, GLfloat s) {
	Vector2f r;
	r.x = v.x * s;
	r.y = v.y * s;
	return r;
}

// Return a Vector2f containing v1 + v2
static inline Vector2f Vector2fAdd(Vector2f v1, Vector2f v2) {
	Vector2f r;
	r.x = v1.x + v2.x;
	r.y = v1.y + v2.y;
	return r;
}

// Return a Vector2f containing v1 - v2
static inline Vector2f Vector2fSub(Vector2f v1, Vector2f v2) {
	Vector2f r;
	r.x = v1.x - v2.x;
	r.y = v1.y - v2.y;
	return r;
}

// Return the dot product of v1 and v2
static inline GLfloat Vector2fDot(Vector2f v1, Vector2f v2) {
	return (GLfloat) v1.x * v2.x + v1.y * v2.y;
}

// Return the length of the vector v
static inline GLfloat Vector2fLength(Vector2f v) {
	return (GLfloat) sqrtf(Vector2fDot(v, v));
}

// Return a Vector2f containing a normalized vector v
static inline Vector2f Vector2fNormalize(Vector2f v) {
	return Vector2fMultiply(v, 1.0f/Vector2fLength(v));
}

#define MAXIMUM_UPDATE_RATE 90.0f	// The maximum number of updates that occur per frame
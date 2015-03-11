//
// ofxParticleEmitter.h
//
// Copyright (c) 2010 71Squared, ported to Openframeworks by Shawn Roske
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef _OFX_PARTICLE_EMITTER
#define _OFX_PARTICLE_EMITTER

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "base64.h"
#include "ofxPexStuff.h"
#include "ofxBasicVectorfield.h"
#include "ofxParticleMask.h"

// toggles the use of vbo rendering
#ifndef RENDER_FAST
#define RENDER_FAST
#endif

// emits N particles per second, instead of bursting until maximum number is reached
#ifndef EMIT_PPS
#define EMIT_PPS
#endif



// ------------------------------------------------------------------------
// ofxParticleEmitter
// ------------------------------------------------------------------------

class ofxParticleEmitter 
{
	
public:
	
	ofxParticleEmitter();
	~ofxParticleEmitter();
	
	bool	loadFromXml( const std::string& filename );
	void    savePositionToFile(string loadFile, string saveFile);
    void    saveToOldFile(string saveFile);
    
    template<typename T>
    void initArray(T * a);
    
    //mask
    void linkMask(ofxParticleMask * _pMask, bool bActivate = true);
    void activateMask();
    void updateMask();
    
    
    //emitter controls
    void clearParticles();
    void start();
    void stop();
    bool isRunning();
    bool isReady();
    void setLoopType(ofLoopType _loop);
    bool isLooping(){return (loopType==OF_LOOP_NORMAL);}
    void setPosition(ofVec2f np);
    ofVec2f getPosition();
    bool hasParticlesLeft();
    void setViewport(ofRectangle r, float blockX=10, float blockY=10);
#ifdef EMIT_PPS
    void setEmissionRate(int _pps);
    int getEmissionRate();
    int getNaturalEmissionRate();
#endif
    void setMaxParticles(int _maxParts);
    
    //pushing
    void applyEmissionPush(ofVec2f psh);
    void resetPushers();
    void clearVectorField();
    void resetVectorField();
    //---
    
    void	update();
	void	draw( int x = 0, int y = 0 );
	void	exit();
    string  getTextureName();
    ofImage * getSprite();
    void    changeTexture(string filename);
    
    void flipY(bool b);
    void flipSprite(bool bX, bool bY);
    void applySpriteFlipping();

	int				emitterType;
	Vector2f		sourcePosition, sourcePositionVariance;			
	GLfloat			angle, angleVariance;								
	GLfloat			speed, speedVariance;	
	GLfloat			radialAcceleration, tangentialAcceleration;
	GLfloat			radialAccelVariance, tangentialAccelVariance;
	Vector2f		gravity;	
	GLfloat			particleLifespan, particleLifespanVariance;			
	Color4f			startColor, startColorVariance;						
	Color4f			finishColor, finishColorVariance;
	GLfloat			startParticleSize, startParticleSizeVariance;
	GLfloat			finishParticleSize, finishParticleSizeVariance;
	GLint			maxParticles;
	GLint			particleCount;
	GLfloat			duration;
	int				blendFuncSource, blendFuncDestination;

	// Particle ivars only used when a maxRadius value is provided.  These values are used for
	// the special purpose of creating the spinning portal emitter
	GLfloat			maxRadius;						// Max radius at which particles are drawn when rotating
	GLfloat			maxRadiusVariance;				// Variance of the maxRadius
	GLfloat			radiusSpeed;					// The speed at which a particle moves from maxRadius to minRadius
	GLfloat			minRadius;						// Radius from source below which a particle dies
	GLfloat			rotatePerSecond;				// Number of degrees to rotate a particle around the source position per second
	GLfloat			rotatePerSecondVariance;		// Variance in degrees for rotatePerSecond
	
    ofxBasicVectorfield vField;
    ofxParticleMask * pMask;
    float pctInteract;
    float timeInteract;
    float pushMultiplier;
    float delay;
    bool bInteractive;
    bool bStarted;
protected:
    
	void	parseParticleConfig(string _pth="");
	void	setupArrays();
	
	void	stopParticleEmitter();
	bool	addParticle();
	void	initParticle( Particle* particle );
	
	void	drawTextures();

#ifdef EMIT_PPS
    void checkRunningState();
#else
    void checkRunningState(float & aDelta);
#endif
    //void updatePartMask(Particle * currentParticle, int & idx);
    void updateParticle(Particle * currentParticle);
    void updatePartRadial(Particle * currentParticle);
    void updatePartLinear(Particle * currentParticle);
    void applyField2Part(Particle * currentParticle);
    void updatePartMesh(Particle * currentParticle);
    
    void emitParticles(int n);
    void emitParticlesFromDelta(float & aDelta);
    string getImagePathFromName(string imgName, string pexPth="");
    bool hasEmbeddedSprite();
    bool isOverwritingFile(string src, string dst);
#ifdef RENDER_FAST
    string makeVertShdSource(bool bMulti);
    string makeFragShdSource(bool bMulti);
    void    drawVBO();
#else
	void	drawPoints();
#endif
	//void	drawPointsOES();
	
	ofxXmlSettings*	settings;

	ofImage*		texture;												
	ofTextureData	textureData;
    string          textureName;
	
	GLfloat			emissionRate;
	GLfloat			emitCounter;
	GLfloat         startTime;
    GLfloat         endTime;
	GLfloat			elapsedTime;
	int				lastUpdateMillis;

	bool			active, useTexture;
	GLint			particleIndex;	// Stores the number of particles that are going to be rendered

	GLuint			verticesID;		// Holds the buffer name of the VBO that stores the color and vertices info for the particles
	Particle*		particles;		// Array of particles that hold the particle emitters particle details
#ifdef RENDER_FAST
    ofShader shd;
    ofVbo vbo;
    ofVec3f * vtx;
    ofVec3f * nlx;
    ofFloatColor * clx;
#else
	PointSprite*	vertices;		// Array of vertices and color information for each particle to be rendered
#endif
    
#ifdef EMIT_PPS
    void updateFrameTime();
    float frameTime;
    int pps;
    int ppf;
#endif
    
    //multisprites
    bool bMultiSprite;
    int nSprites;
    ofVec2f szSprite;
    //----
    
    //flipping
    float yFlipper;
    bool flipSpriteX;
    bool flipSpriteY;
    //---
    
    //push
    //ofRectangle rVp;
    ofVec2f emissionPush;
    //ofVec2f vpBlock;
    //vector< vector<ofVec2f> > vField;
    //float pctInteract;
    //----
    
    ofLoopType loopType;
    bool bInited;
};

#endif
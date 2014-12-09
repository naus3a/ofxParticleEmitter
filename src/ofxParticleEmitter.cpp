//
// ofxParticleEmitter.cpp
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

#include "ofxParticleEmitter.h"

// ------------------------------------------------------------------------
// Lifecycle
// ------------------------------------------------------------------------

ofxParticleEmitter::ofxParticleEmitter()
{
	bMultiSprite = false;
    nSprites = 0;
    szSprite.set(10,10);
    
    settings = NULL;
	
	emitterType = kParticleTypeGravity;
	texture = NULL;
    textureName = "";
	sourcePosition.x = sourcePosition.y = 0.0f;
	sourcePositionVariance.x = sourcePositionVariance.y = 0.0f;
	angle = angleVariance = 0.0f;								
	speed = speedVariance = 0.0f;	
	radialAcceleration = tangentialAcceleration = 0.0f;
	radialAccelVariance = tangentialAccelVariance = 0.0f;
	gravity.x = gravity.y = 0.0f;
	particleLifespan = particleLifespanVariance = 0.0f;			
	startColor.red = startColor.green = startColor.blue = startColor.alpha = 1.0f;
	startColorVariance.red = startColorVariance.green = startColorVariance.blue = startColorVariance.alpha = 1.0f;
	finishColor.red = finishColor.green = finishColor.blue = finishColor.alpha = 1.0f;
	finishColorVariance.red = finishColorVariance.green = finishColorVariance.blue = finishColorVariance.alpha = 1.0f;
	startParticleSize = startParticleSizeVariance = 0.0f;
	finishParticleSize = finishParticleSizeVariance = 0.0f;
	maxParticles = 0.0f;
	particleCount = 0;
	emissionRate = 0.0f;
	emitCounter = 0.0f;	
	elapsedTime = 0.0f;
	duration = -1;
	lastUpdateMillis = 0;
    
	blendFuncSource = blendFuncDestination = 0;

	maxRadius = maxRadiusVariance = radiusSpeed = minRadius = 0.0f;
	rotatePerSecond = rotatePerSecondVariance = 0.0f;
	
	active = useTexture = false;
	particleIndex = 0;

	verticesID = 0;
	particles = NULL;
#ifdef RENDER_FAST
    vtx = NULL;
    nlx = NULL;
    clx = NULL;
    
    //vbo.setUsage(GL_DYNAMIC_DRAW);
    //vbo.setMode(OF_PRIMITIVE_POINTS);
    vbo.enableNormals();
    vbo.enableColors();
    
    //shd.setupShaderFromSource(GL_VERTEX_SHADER, makeVertShdSource());
    //shd.setupShaderFromSource(GL_FRAGMENT_SHADER, makeFragShdSource());
    //shd.linkProgram();
#else
	vertices = NULL;
#endif
    
#ifdef EMIT_PPS
    frameTime = 1.0/60.0;
    pps = 100;
    ppf = int(pps*frameTime);
#endif
    
    setLoopType(OF_LOOP_NONE);
    bInited = false;
    
    //flipping
    yFlipper = 1;
    flipSpriteX = false;
    flipSpriteY = false;
    //---
    
    //push
    resetPushers();
    pctInteract = 0;
    timeInteract =0;
    pushMultiplier=1.0;
    delay=0;
    bInteractive=false;
    //---
    
    delay = 0;
    bStarted = false;
    
    setViewport(ofRectangle(0,0,ofGetWidth(),ofGetHeight()));
}

#ifdef RENDER_FAST
string ofxParticleEmitter::makeVertShdSource(bool bMulti){
    stringstream src;
    if(bMulti){
        src << "varying float sIdx; \n";
        src << "varying float sTot; \n";
        src << "void main() {\n";
        src << "    gl_Position   = gl_ModelViewProjectionMatrix * gl_Vertex;\n";
        src << "    float size    = gl_Normal.x;\n";
        src << "    sIdx          = gl_Normal.y;\n";
        src << "    sTot          = gl_Normal.z;\n";
        src << "    gl_PointSize  = size;\n";
        src << "    gl_FrontColor = gl_Color;\n";
        src << "}\n";
    }else{
        src << "void main() {\n";
        
        src << "    gl_Position   = gl_ModelViewProjectionMatrix * gl_Vertex;\n";
        src << "    float size    = gl_Normal.x;\n";
        src << "    gl_PointSize  = size;\n";
        src << "    gl_FrontColor = gl_Color;\n";
        src << "}\n";
    }
    return src.str();
}

string ofxParticleEmitter::makeFragShdSource(bool bMulti){
    stringstream src;
    if(bMulti){
        src << "varying float sIdx; \n";
        src << "varying float sTot; \n";
        src << "uniform sampler2D tex;\n";
        
        src << "void main (void) {\n";
        
        src << "    vec2 tc = gl_TexCoord[0].st; \n";
        src << "    float sw = 1.0/sTot; \n";
        src << "    float xx = (sIdx * sw) + (tc.x * sw); \n";
        src << "    tc.x = xx; \n";
        
        src << "    gl_FragColor = texture2D(tex, tc) * gl_Color;\n";
        
        src << "}\n";
    }else{
        src << "uniform sampler2D tex;\n";
    
        src << "void main (void) {\n";
        
        src << "    gl_FragColor = texture2D(tex, gl_TexCoord[0].st) * gl_Color;\n";
        
        src << "}\n";
    }
    return src.str();
}
#endif

ofxParticleEmitter::~ofxParticleEmitter()
{
	exit();
}

void ofxParticleEmitter::exit()
{	
	if ( texture != NULL )
		delete texture;
	texture = NULL;
    
    //initArray<Particle>(particles);
    if(particles!=NULL){
        delete particles;
    }
    particles=NULL;
#ifdef RENDER_FAST
    if(vtx!=NULL){
        delete vtx;
    }
    vtx=NULL;
    
    if(nlx!=NULL){
        delete nlx;
    }
    nlx=NULL;
    
    if(clx!=NULL){
        delete clx;
    }
    clx=NULL;
    
    //initArray<ofVec3f>(vtx);
    //initArray<ofVec3f>(nlx);
    //initArray<ofFloatColor>(clx);
#else
	if ( vertices != NULL )
		delete vertices;
	vertices = NULL;
#endif
	glDeleteBuffers( 1, &verticesID );
    
    resetPushers();
    bInited = false;
}

bool ofxParticleEmitter::hasEmbeddedSprite(){
    return false;//(settings->getAttribute( "texture", "data", "" )!="");
}

string ofxParticleEmitter::getImagePathFromName(string imgName, string pexPth){
    string imgPth = "";
    if(pexPth.size()>0)imgPth += pexPth;
    imgPth += imgName;
    return imgPth;
}

bool ofxParticleEmitter::isOverwritingFile(string src, string dst){
    return (src==dst);
}

void ofxParticleEmitter::savePositionToFile(string loadFile, string saveFile){
    settings = new ofxXmlSettings();
    if(settings->load(loadFile)){
        settings->pushTag( "particleEmitterConfig" );
        /*if(settings->attributeExists("sourcePosition", "x")){
            settings->removeAttribute("sourcePosition", "x");
        }
        if(settings->attributeExists("sourcePosition", "y")){
            settings->removeAttribute("sourcePosition", "y");
        }*/
        settings->setAttribute("sourcePosition", "x", sourcePosition.x, 0);
        settings->setAttribute("sourcePosition", "y", sourcePosition.y, 0);
        
        if(!isOverwritingFile(loadFile, saveFile)){
            if(!hasEmbeddedSprite()){
                string imgName = settings->getAttribute( "texture", "name", "" );
                string oldImgPth = getImagePathFromName(imgName, ofFilePath::getEnclosingDirectory(loadFile));
                imgName = ofFilePath::getBaseName(saveFile) + ".png";
                string newImgPth = ofFilePath::getEnclosingDirectory(saveFile) + imgName;
                settings->setAttribute("texture", "name", imgName, 0);
                ofFile::copyFromTo(oldImgPth, newImgPth);
                cout<<oldImgPth.c_str()<<" "<<newImgPth.c_str()<<endl;
            }
        }
        settings->popTag();
        
        settings->save(saveFile);
        
    }
    delete settings;
    settings = NULL;
}

void ofxParticleEmitter::saveToOldFile(string saveFile){
    if(settings!=NULL){
        delete settings;
        settings = NULL;
    }
    
    settings = new ofxXmlSettings();
    
    if(settings->load(saveFile)){
        settings->pushTag( "particleEmitterConfig" );
        
        //emitter settings
        settings->setAttribute("emitterType", "value", emitterType, 0);
        
        settings->setAttribute("sourcePosition", "x", sourcePosition.x, 0);
        settings->setAttribute("sourcePosition", "y", sourcePosition.y, 0);
        
        settings->setAttribute("sourcePositionVariance", "x", sourcePositionVariance.x, 0);
        settings->setAttribute("sourcePositionVariance", "y", sourcePositionVariance.y, 0);
        
        settings->setAttribute("duration", "value", duration, 0);
        
        settings->setAttribute( "maxParticles", "value", maxParticles, 0);
        
        settings->setAttribute("angle", "value", angle , 0);
        settings->setAttribute("angleVariance", "value", angleVariance, 0);
        
        //emission rate
        if(getEmissionRate()!=getNaturalEmissionRate()){
            if(!settings->tagExists("emissionRate")){
                settings->addTag("emissionRate");
            }
            settings->setAttribute("emissionRate", "value", getEmissionRate(), 0);
        }
        
        //push
        if(!settings->tagExists("pctInteract")){
            settings->addTag("pctInteract");
        }
        settings->setAttribute("pctInteract", "value", pctInteract, 0);
        
        if(!settings->tagExists("timeInteract")){
            settings->addTag("timeInteract");
        }
        settings->setAttribute("timeInteract", "value", timeInteract, 0);
        
        if(!settings->tagExists("pushMultiplier")){
            settings->addTag("pushMultiplier");
        }
        settings->setAttribute("pushMultiplier", "value", pushMultiplier, 0);
        
        if(!settings->tagExists("delay")){
            settings->addTag("delay");
        }
        settings->setAttribute("delay", "value", delay, 0);
        
        if(emitterType==0){
            //gravity settings
            settings->setAttribute("gravity", "x", gravity.x, 0 );
            settings->setAttribute("gravity", "y", gravity.y, 0 );
            
            settings->setAttribute("speed", "value", speed, 0);
            settings->setAttribute("speedVariance", "value", speedVariance, 0);
            
            settings->setAttribute( "radialAcceleration", "value", radialAcceleration,0);
            settings->setAttribute("radialAccelVariance", "value", radialAccelVariance, 0);
            
            settings->setAttribute("tangentialAcceleration", "value", tangentialAcceleration, 0);
            settings->setAttribute("tangentialAccelVariance", "value", tangentialAccelVariance, 0);
        }else if(emitterType==1){
            //radial settings
            settings->setAttribute("maxRadius", "value", maxRadius, 0);
            settings->setAttribute("maxRadiusVariance", "value", maxRadiusVariance, 0);
            
            settings->setAttribute("minRadius", "value", minRadius, 0);
            
            settings->setAttribute("rotatePerSecond", "value", rotatePerSecond, 0);
            settings->setAttribute("rotatePerSecondVariance", "value", rotatePerSecondVariance, 0);
        }
        
        //particle settings
        settings->setAttribute("particleLifeSpan", "value", particleLifespan, 0);
        settings->setAttribute("particleLifespanVariance", "value", particleLifespanVariance, 0);
        
        settings->setAttribute("startParticleSize", "value", startParticleSize, 0);
        settings->setAttribute("startParticleSizeVariance", "value", startParticleSizeVariance, 0);
        
        settings->setAttribute("finishParticleSize", "value", finishParticleSize, 0);
        settings->setAttribute("finishParticleSizeVariance", "value", finishParticleSizeVariance, 0);
        
        //color settings
        settings->setAttribute("startColor", "red", startColor.red, 0);
        settings->setAttribute("startColor", "green", startColor.green, 0);
        settings->setAttribute("startColor", "blue", startColor.blue, 0);
        settings->setAttribute("startColor", "alpha", startColor.alpha, 0);
        
        settings->setAttribute("startColorVariance", "red", startColorVariance.red, 0);
        settings->setAttribute("startColorVariance", "green", startColorVariance.green, 0);
        settings->setAttribute("startColorVariance", "blue", startColorVariance.blue, 0);
        settings->setAttribute("startColorVariance", "alpha", startColorVariance.alpha, 0);
        
        settings->setAttribute("finishColor", "red", finishColor.red, 0);
        settings->setAttribute("finishColor", "green", finishColor.green, 0);
        settings->setAttribute("finishColor", "blue", finishColor.blue, 0);
        settings->setAttribute("finishColor", "alpha", finishColor.alpha, 0);
        
        settings->setAttribute("finishColorVariance", "red", finishColorVariance.red, 0);
        settings->setAttribute("finishColorVariance", "green", finishColorVariance.green, 0);
        settings->setAttribute("finishColorVariance", "blue", finishColorVariance.blue, 0);
        settings->setAttribute("finishColorVariance", "alpha", finishColorVariance.alpha, 0);
        
        settings->setAttribute("blendFuncSource", "value", blendFuncSource, 0);
        settings->setAttribute("blendFuncDestination", "value", blendFuncDestination, 0);
        
        settings->popTag();
        
        settings->save(saveFile);
    }
    
    delete settings;
    settings = NULL;
}

bool ofxParticleEmitter::loadFromXml( const std::string& filename )
{
    // Clean up things
    exit();
    
	bool ok = false;
	
	settings = new ofxXmlSettings();
	
	ok = settings->loadFile( filename );
	if ( ok )
	{
        
        parseParticleConfig(ofFilePath::getEnclosingDirectory(filename,false));
		setupArrays();
		
		//ok = active = true;
        start();
        ok = active;
        bInited = ok;
        //emissionRate =  maxParticles/particleLifespan;

#ifdef RENDER_FAST
        shd.setupShaderFromSource(GL_VERTEX_SHADER, makeVertShdSource(bMultiSprite));
        shd.setupShaderFromSource(GL_FRAGMENT_SHADER, makeFragShdSource(bMultiSprite));
        shd.linkProgram();
#endif
	}

	delete settings;
	settings = NULL;
    
	return ok;
}

void ofxParticleEmitter::parseParticleConfig(string _pth)
{
	if ( settings == NULL )
	{
		ofLog( OF_LOG_ERROR, "ofxParticleEmitter::parseParticleConfig() - XML settings is invalid!" );
		return;
	}
	
    settings->pushTag( "particleEmitterConfig" );
    
    bMultiSprite = false;
    if(settings->tagExists("multi_sprite")){
        settings->pushTag("multi_sprite");
        nSprites = settings->getValue("n", 0);
        if(nSprites>0){
            bMultiSprite = true;
        }
        settings->popTag();
    }

	std::string imageFilename	= settings->getAttribute( "texture", "name", "" );
	std::string imageData		= settings->getAttribute( "texture", "data", "" );
    
    /*if(imageData!=""){
        ofLog( OF_LOG_WARNING, "ofxParticleEmitter::parseParticleConfig() - loading image data" );
        string sBin = base64_decode(imageData);
        ofBuffer buf;
        buf.set(sBin.c_str(), sBin.size());
        ofBufferToFile("cippa.png", buf);
        
        ofDisableArbTex();
        texture =  new ofImage();
        texture->loadImage(buf);
        texture->setUseTexture(true);
        texture->setAnchorPercent( 0.5f, 0.5f );
        textureName = imageFilename;
        textureData = texture->getTextureReference().getTextureData();
        ofEnableArbTex();
    }else if(imageFilename!=""){
        ofLog( OF_LOG_WARNING, "ofxParticleEmitter::parseParticleConfig() - loading image file" );
        string imgPth;
        if(_pth.size()>0)imgPth += _pth;
        imgPth += imageFilename;
        ofDisableArbTex();
        texture = new ofImage();
        ofFile ff;
        ff.open(imgPth);
        ff.getFileBuffer();
        if(texture->loadImage(imgPth)){
            texture->setUseTexture(true);
            texture->setAnchorPercent( 0.5f, 0.5f );
            textureName = imageFilename;
            textureData = texture->getTextureReference().getTextureData();
        }else{
            ofEnableArbTex();
            ofLog(OF_LOG_ERROR, "ofxParticleEmitter::parseParticleConfig() - No Image file");
            return;
        }
        ofEnableArbTex();
    }else{
        ofLog(OF_LOG_ERROR, "ofxParticleEmitter::parseParticleConfig() - No Image name and no image data");
        return;
    }*/
    
	
	if ( imageFilename != "" )
	{
		ofLog( OF_LOG_WARNING, "ofxParticleEmitter::parseParticleConfig() - loading image file" );
		
        //string imgPth;
        //if(_pth.size()>0)imgPth += _pth;
        //imgPth += imageFilename;
        
        string imgPth = getImagePathFromName(imageFilename, _pth);
        ofDisableArbTex();
		texture = new ofImage();
		texture->loadImage( imgPth );
        applySpriteFlipping();
		texture->setUseTexture( true );
		texture->setAnchorPercent( 0.5f, 0.5f );
        
        textureName = imageFilename;
		
		textureData = texture->getTextureReference().getTextureData();
        ofEnableArbTex();
	}
	else if ( imageData != "" )
	{
		// TODO
		ofLog( OF_LOG_ERROR, "ofxParticleEmitter::parseParticleConfig() - image data found but not yet implemented!" );
        
		return;
	}

    emitterType					= settings->getAttribute( "emitterType", "value", emitterType );
	
	sourcePosition.x			= settings->getAttribute( "sourcePosition", "x", sourcePosition.x );
	sourcePosition.y			= settings->getAttribute( "sourcePosition", "y", sourcePosition.y );
    
    sourcePositionVariance.x			= settings->getAttribute( "sourcePositionVariance", "x", sourcePositionVariance.x );
	sourcePositionVariance.y			= settings->getAttribute( "sourcePositionVariance", "y", sourcePositionVariance.y );
	
	speed						= settings->getAttribute( "speed", "value", speed );
	speedVariance				= settings->getAttribute( "speedVariance", "value", speedVariance );
	particleLifespan			= settings->getAttribute( "particleLifeSpan", "value", particleLifespan );
    
	particleLifespanVariance	= settings->getAttribute( "particleLifespanVariance", "value", particleLifespanVariance );
	angle						= settings->getAttribute( "angle", "value", angle );
	angleVariance				= settings->getAttribute( "angleVariance", "value", angleVariance );
	
	gravity.x					= settings->getAttribute( "gravity", "x", gravity.x );
	gravity.y					= settings->getAttribute( "gravity", "y", gravity.y * yFlipper);
	
	radialAcceleration			= settings->getAttribute( "radialAcceleration", "value", radialAcceleration );
	tangentialAcceleration		= settings->getAttribute( "tangentialAcceleration", "value", tangentialAcceleration );
    radialAccelVariance			= settings->getAttribute( "radialAccelVariance", "value", radialAccelVariance );
	tangentialAccelVariance		= settings->getAttribute( "tangentialAccelVariance", "value", tangentialAccelVariance );
	
	startColor.red				= settings->getAttribute( "startColor", "red", startColor.red );
	startColor.green			= settings->getAttribute( "startColor", "green", startColor.green );
	startColor.blue				= settings->getAttribute( "startColor", "blue", startColor.blue );
	startColor.alpha			= settings->getAttribute( "startColor", "alpha", startColor.alpha );
	
	startColorVariance.red		= settings->getAttribute( "startColorVariance", "red", startColorVariance.red );
	startColorVariance.green	= settings->getAttribute( "startColorVariance", "green", startColorVariance.green );
	startColorVariance.blue		= settings->getAttribute( "startColorVariance", "blue", startColorVariance.blue );
	startColorVariance.alpha	= settings->getAttribute( "startColorVariance", "alpha", startColorVariance.alpha );
	
	finishColor.red				= settings->getAttribute( "finishColor", "red", finishColor.red );
	finishColor.green			= settings->getAttribute( "finishColor", "green", finishColor.green );
	finishColor.blue			= settings->getAttribute( "finishColor", "blue", finishColor.blue );
	finishColor.alpha			= settings->getAttribute( "finishColor", "alpha", finishColor.alpha );
	
	finishColorVariance.red		= settings->getAttribute( "finishColorVariance", "red", finishColorVariance.red );
	finishColorVariance.green	= settings->getAttribute( "finishColorVariance", "green", finishColorVariance.green );
	finishColorVariance.blue	= settings->getAttribute( "finishColorVariance", "blue", finishColorVariance.blue );
	finishColorVariance.alpha	= settings->getAttribute( "finishColorVariance", "alpha", finishColorVariance.alpha );
	
	maxParticles				= settings->getAttribute( "maxParticles", "value", maxParticles );
	startParticleSize			= settings->getAttribute( "startParticleSize", "value", startParticleSize );
	startParticleSizeVariance	= settings->getAttribute( "startParticleSizeVariance", "value", startParticleSizeVariance );
	finishParticleSize			= settings->getAttribute( "finishParticleSize", "value", finishParticleSize );
	finishParticleSizeVariance	= settings->getAttribute( "finishParticleSizeVariance", "value", finishParticleSizeVariance );
	duration					= settings->getAttribute( "duration", "value", duration );
	blendFuncSource				= settings->getAttribute( "blendFuncSource", "value", blendFuncSource );
	blendFuncDestination		= settings->getAttribute( "blendFuncDestination", "value", blendFuncDestination );
	
	maxRadius					= settings->getAttribute( "maxRadius", "value", maxRadius );
	maxRadiusVariance			= settings->getAttribute( "maxRadiusVariance", "value", maxRadiusVariance );
	radiusSpeed					= settings->getAttribute( "radiusSpeed", "value", radiusSpeed );
	minRadius					= settings->getAttribute( "minRadius", "value", minRadius );
	
	rotatePerSecond				= settings->getAttribute( "rotatePerSecond", "value", rotatePerSecond );
	rotatePerSecondVariance		= settings->getAttribute( "rotatePerSecondVariance", "value", rotatePerSecondVariance );
    
#ifdef EMIT_PPS
    int _eRate = getNaturalEmissionRate();
    if(settings->tagExists("emissionRate")){
        _eRate = settings->getAttribute("emissionRate", "value", _eRate);
    }
    setEmissionRate(_eRate);
    
    pctInteract = 0.0;
    if(settings->tagExists("pctInteract")){
        pctInteract = settings->getAttribute("pctInteract", "value", 0.0);
    }
    
    timeInteract = 0.0;
    if(settings->tagExists("timeInteract")){
        timeInteract = settings->getAttribute("timeInteract", "value", 0.0);
    }
    
    pushMultiplier = 1.0;
    if(settings->tagExists("pushMultiplier")){
        pushMultiplier = settings->getAttribute("pushMultiplier", "value", 0.0);
    }
    
    delay = 0.0;
    if(settings->tagExists("delay")){
        delay = settings->getAttribute("delay", "value", 0.0);
    }
    
#endif
}

template<typename T>
void ofxParticleEmitter::initArray(T *a){
    if(a!=NULL){
        delete a;
        a = NULL;
    }
}

void ofxParticleEmitter::setupArrays()
{
	initArray<Particle>(particles);
#ifdef RENDER_FAST
    initArray<ofVec3f>(vtx);
    initArray<ofVec3f>(nlx);
    initArray<ofFloatColor>(clx);
#endif
    
    // Allocate the memory necessary for the particle emitter arrays
	particles = (Particle*)malloc( sizeof( Particle ) * maxParticles );
#ifdef RENDER_FAST
    vtx = (ofVec3f*)malloc(sizeof(ofVec3f) * maxParticles);
    nlx = (ofVec3f*)malloc(sizeof(ofVec3f) * maxParticles);
    clx = (ofFloatColor *)malloc(sizeof(ofFloatColor) * maxParticles);
    assert(particles && vtx && clx && nlx);
    
#else
	vertices = (PointSprite*)malloc( sizeof( PointSprite ) * maxParticles );
	
	// If one of the arrays cannot be allocated throw an assertion as this is bad
	assert( particles && vertices );
	
	// Generate the vertices VBO
	glGenBuffers( 1, &verticesID );
#endif
	// Set the particle count to zero
	particleCount = 0;
	
	// Reset the elapsed time
	elapsedTime = 0;
}

// ------------------------------------------------------------------------
// Particle Management
// ------------------------------------------------------------------------

bool ofxParticleEmitter::hasParticlesLeft(){
    return (particleCount>0);
}

bool ofxParticleEmitter::addParticle()
{
	// If we have already reached the maximum number of particles then do nothing
	if(particleCount >= maxParticles)
		return false;
	
	// Take the next particle out of the particle pool we have created and initialize it
	Particle *particle = &particles[particleCount];
	initParticle( particle );
	
	// Increment the particle count
	particleCount++;
	
	// Return true to show that a particle has been created
	return true;
}

void ofxParticleEmitter::initParticle( Particle* particle )
{
	// Init the position of the particle.  This is based on the source position of the particle emitter
	// plus a configured variance.  The RANDOM_MINUS_1_TO_1 macro allows the number to be both positive
	// and negative
	particle->position.x = sourcePosition.x + sourcePositionVariance.x * RANDOM_MINUS_1_TO_1();
	particle->position.y = sourcePosition.y + sourcePositionVariance.y * RANDOM_MINUS_1_TO_1();
    particle->startPos.x = sourcePosition.x;
    particle->startPos.y = sourcePosition.y;
	
	// Init the direction of the particle.  The newAngle is calculated using the angle passed in and the
	// angle variance.
	float newAngle = (GLfloat)DEGREES_TO_RADIANS(angle + angleVariance * RANDOM_MINUS_1_TO_1());
	
	// Create a new Vector2f using the newAngle
	Vector2f vector = Vector2fMake(cosf(newAngle), sinf(newAngle));
    
	// Calculate the vectorSpeed using the speed and speedVariance which has been passed in
	float vectorSpeed = speed + speedVariance * RANDOM_MINUS_1_TO_1();
	
	// The particles direction vector is calculated by taking the vector calculated above and
	// multiplying that by the speed
	particle->direction = Vector2fMultiply(vector, vectorSpeed);
    
	// Set the default diameter of the particle from the source position
	particle->radius = maxRadius + maxRadiusVariance * RANDOM_MINUS_1_TO_1();
	particle->radiusDelta = (maxRadius / particleLifespan) * (1.0 / MAXIMUM_UPDATE_RATE);
	particle->angle = DEGREES_TO_RADIANS(angle + angleVariance * RANDOM_MINUS_1_TO_1());
	particle->degreesPerSecond = DEGREES_TO_RADIANS(rotatePerSecond + rotatePerSecondVariance * RANDOM_MINUS_1_TO_1());
    
    particle->radialAcceleration = radialAcceleration;
    particle->tangentialAcceleration = tangentialAcceleration;
	
	// Calculate the particles life span using the life span and variance passed in
	particle->timeToLive = MAX(0, particleLifespan + particleLifespanVariance * RANDOM_MINUS_1_TO_1());
	
	// Calculate the particle size using the start and finish particle sizes
	GLfloat particleStartSize = startParticleSize + startParticleSizeVariance * RANDOM_MINUS_1_TO_1();
	GLfloat particleFinishSize = finishParticleSize + finishParticleSizeVariance * RANDOM_MINUS_1_TO_1();
	particle->particleSizeDelta = ((particleFinishSize - particleStartSize) / particle->timeToLive) * (1.0 / MAXIMUM_UPDATE_RATE);
	particle->particleSize = MAX(0, particleStartSize);
	
	// Calculate the color the particle should have when it starts its life.  All the elements
	// of the start color passed in along with the variance are used to calculate the star color
	Color4f start = {0, 0, 0, 0};
	start.red = startColor.red + startColorVariance.red * RANDOM_MINUS_1_TO_1();
	start.green = startColor.green + startColorVariance.green * RANDOM_MINUS_1_TO_1();
	start.blue = startColor.blue + startColorVariance.blue * RANDOM_MINUS_1_TO_1();
	start.alpha = startColor.alpha + startColorVariance.alpha * RANDOM_MINUS_1_TO_1();
	
	// Calculate the color the particle should be when its life is over.  This is done the same
	// way as the start color above
	Color4f end = {0, 0, 0, 0};
	end.red = finishColor.red + finishColorVariance.red * RANDOM_MINUS_1_TO_1();
	end.green = finishColor.green + finishColorVariance.green * RANDOM_MINUS_1_TO_1();
	end.blue = finishColor.blue + finishColorVariance.blue * RANDOM_MINUS_1_TO_1();
	end.alpha = finishColor.alpha + finishColorVariance.alpha * RANDOM_MINUS_1_TO_1();
	
	// Calculate the delta which is to be applied to the particles color during each cycle of its
	// life.  The delta calculation uses the life span of the particle to make sure that the 
	// particles color will transition from the start to end color during its life time.  As the game
	// loop is using a fixed delta value we can calculate the delta color once saving cycles in the 
	// update method
	particle->color = start;
	particle->deltaColor.red = ((end.red - start.red) / particle->timeToLive) * (1.0 / MAXIMUM_UPDATE_RATE);
	particle->deltaColor.green = ((end.green - start.green) / particle->timeToLive)  * (1.0 / MAXIMUM_UPDATE_RATE);
	particle->deltaColor.blue = ((end.blue - start.blue) / particle->timeToLive)  * (1.0 / MAXIMUM_UPDATE_RATE);
	particle->deltaColor.alpha = ((end.alpha - start.alpha) / particle->timeToLive)  * (1.0 / MAXIMUM_UPDATE_RATE);
    
    //multisprite
    if(bMultiSprite){
        particle->sIdx = (int)ofRandom(0, nSprites-1);
    }
    //---
    
    //push
    if(ofRandom(0.0,1.0)<pctInteract){
        particle->z = 1.0;
        particle->direction.x += emissionPush.x;
        particle->direction.y += emissionPush.y;
    }else{
        particle->z = 0.0;
    }
    //---
}

void ofxParticleEmitter::stopParticleEmitter()
{
	active = false;
	elapsedTime = 0;
	//emitCounter = 0;
}

// ------------------------------------------------------------------------
// Update
// ------------------------------------------------------------------------

#ifdef EMIT_PPS
void ofxParticleEmitter::setEmissionRate(int _pps){
    pps = _pps;
    ppf = int(pps * frameTime);
}

int ofxParticleEmitter::getEmissionRate(){
    return pps;
}

int ofxParticleEmitter::getNaturalEmissionRate(){
    return int(maxParticles/particleLifespan);
}

void ofxParticleEmitter::updateFrameTime(){
    frameTime = (ofGetElapsedTimeMillis()-lastUpdateMillis)/1000.0f;
    ppf = int(pps * frameTime);
}
#endif

void ofxParticleEmitter::setMaxParticles(int _maxParts){
    maxParticles = _maxParts;
    setupArrays();
}

#ifdef EMIT_PPS
void ofxParticleEmitter::checkRunningState(){
#else
void ofxParticleEmitter::checkRunningState(float & aDelta){
#endif
    if(active){
        if(duration==-1){
#ifdef EMIT_PPS
            emitParticles(ppf);
#else
            emitParticlesFromDelta(aDelta);
#endif
        }else{
            if(ofGetElapsedTimef()<endTime){
#ifdef EMIT_PPS
                emitParticles(ppf);
#else
                emitParticlesFromDelta(aDelta);
#endif
            }else{
                stopParticleEmitter();
            }
        }
    }else{
        if(loopType==OF_LOOP_NORMAL){
            if(particleCount==0){
                start();
            }
        }
    }
}

void ofxParticleEmitter::emitParticles(int n){
    for(int i=0;i<n;i++){
        addParticle();
    }
}

void ofxParticleEmitter::emitParticlesFromDelta(float &aDelta){
    if(emissionRate){
        float rate = 1/emissionRate;
        emitCounter += aDelta;
        
        while (particleCount < maxParticles && emitCounter > rate) {
            addParticle();
            emitCounter -= rate;
        }
    }
}
    
void ofxParticleEmitter::updateParticle(Particle *currentParticle){
    // If maxRadius is greater than 0 then the particles are going to spin otherwise
    // they are effected by speed and gravity
    if (emitterType == kParticleTypeRadial) {
        
        // FIX 2
        // Update the angle of the particle from the sourcePosition and the radius.  This is only
        // done of the particles are rotating
        currentParticle->angle += currentParticle->degreesPerSecond * frameTime;
        currentParticle->radius -= currentParticle->radiusDelta;
        
        Vector2f tmp;
        tmp.x = sourcePosition.x - cosf(currentParticle->angle) * currentParticle->radius;
        tmp.y = sourcePosition.y - sinf(currentParticle->angle) * currentParticle->radius;
        currentParticle->position = tmp;
        
        if (currentParticle->radius < minRadius)
            currentParticle->timeToLive = 0;
    } else {
        Vector2f tmp, radial, tangential;
        
        radial = Vector2fZero;
        Vector2f diff = Vector2fSub(currentParticle->startPos, Vector2fZero);
        
        currentParticle->position = Vector2fSub(currentParticle->position, diff);
        
        if (currentParticle->position.x || currentParticle->position.y)
            radial = Vector2fNormalize(currentParticle->position);
        
        tangential.x = radial.x;
        tangential.y = radial.y;
        radial = Vector2fMultiply(radial, currentParticle->radialAcceleration);
        
        GLfloat newy = tangential.x;
        tangential.x = -tangential.y;
        tangential.y = newy;
        tangential = Vector2fMultiply(tangential, currentParticle->tangentialAcceleration);
        
        tmp = Vector2fAdd( Vector2fAdd(radial, tangential), gravity);
        tmp = Vector2fMultiply(tmp, frameTime);
        currentParticle->direction = Vector2fAdd(currentParticle->direction, tmp);
        tmp = Vector2fMultiply(currentParticle->direction, frameTime);
        currentParticle->position = Vector2fAdd(currentParticle->position, tmp);
        currentParticle->position = Vector2fAdd(currentParticle->position, diff);
        
        //push
        if(currentParticle->z>=1 && currentParticle->timeToLive<(particleLifespan-timeInteract)){
            ofVec2f psh = vField.getVectorForPoint(currentParticle->position.x, currentParticle->position.y);
            //currentParticle->position += psh;
            //tmp+=psh;
            currentParticle->direction += psh;
        //    currentParticle->direction.x += psh.x;
        //    currentParticle->direction.y += psh.y;
        }
        //---
        
    }
    
    // Update the particles color
    currentParticle->color.red += currentParticle->deltaColor.red;
    currentParticle->color.green += currentParticle->deltaColor.green;
    currentParticle->color.blue += currentParticle->deltaColor.blue;
    currentParticle->color.alpha += currentParticle->deltaColor.alpha;
#ifdef RENDER_FAST
    vtx[particleIndex].set(currentParticle->position.x, currentParticle->position.y, 0);
    clx[particleIndex].set(currentParticle->color.red, currentParticle->color.green, currentParticle->color.blue, currentParticle->color.alpha);
    currentParticle->particleSize += currentParticle->particleSizeDelta;
    nlx[particleIndex].x = MAX(0, currentParticle->particleSize);
    nlx[particleIndex].y = currentParticle->sIdx;
    nlx[particleIndex].z = nSprites;
#else
    // Place the position of the current particle into the vertices array
    vertices[particleIndex].x = currentParticle->position.x;
    vertices[particleIndex].y = currentParticle->position.y;
    
    // Place the size of the current particle in the size array
    currentParticle->particleSize += currentParticle->particleSizeDelta;
    vertices[particleIndex].size = MAX(0, currentParticle->particleSize);
    
    // Place the color of the current particle into the color array
    vertices[particleIndex].color = currentParticle->color;
#endif
}
    
void ofxParticleEmitter::update()
{
	if(!bInited)return;
#ifdef EMIT_PPS
    updateFrameTime();
    checkRunningState();
#else
	GLfloat frameTime = (ofGetElapsedTimeMillis()-lastUpdateMillis)/1000.0f;
    checkRunningState(frameTime);
#endif
    
	// Reset the particle index before updating the particles in this emitter
	particleIndex = 0;
	
	// Loop through all the particles updating their location and color
	while(particleIndex < particleCount) {
		
		// Get the particle for the current particle index
		Particle *currentParticle = &particles[particleIndex];
        
        // FIX 1
        // Reduce the life span of the particle
        currentParticle->timeToLive -= frameTime;
		
		// If the current particle is alive && in viewport then update it
		if(currentParticle->timeToLive > 0 && vField.inside(currentParticle->position.x, currentParticle->position.y)) {
			
			updateParticle(currentParticle);
            
			// Update the particle counter
			particleIndex++;
		} else {
			
			// As the particle is not alive anymore replace it with the last active particle 
			// in the array and reduce the count of particles by one.  This causes all active particles
			// to be packed together at the start of the array so that a particle which has run out of
			// life will only drop into this clause once
			if(particleIndex != particleCount - 1)
				particles[particleIndex] = particles[particleCount - 1];
			particleCount--;
		}
	}

	lastUpdateMillis = ofGetElapsedTimeMillis();
    resetPushers();
}

// ------------------------------------------------------------------------
// Render
// ------------------------------------------------------------------------

void ofxParticleEmitter::draw(int x /* = 0 */, int y /* = 0 */)
{
	if (!bInited || (!active && particleCount==0)) return;
	
	glPushMatrix();
	glTranslatef( x, y, 0.0f );
	
//#ifdef TARGET_OF_IPHONE
	
//	drawPointsOES();
	
//#else
#ifdef RENDER_FAST
    drawVBO();
#else
	drawTextures();
#endif
	//drawPoints();
	
//#endif
	
	glPopMatrix();
}

void ofxParticleEmitter::drawTextures()
{
	glEnable(GL_BLEND);
	glBlendFunc(blendFuncSource, blendFuncDestination);
	
	for( int i = 0; i < particleCount; i++ )
	{
#ifdef RENDER_FAST
        ofSetColor(clx[i]);
        texture->draw(vtx[i].x, vtx[i].y, nlx[i].x, nlx[i].x);
#else
        PointSprite* ps = &vertices[i];
		ofSetColor( ps->color.red*255.0f, ps->color.green*255.0f, 
				   ps->color.blue*255.0f, ps->color.alpha*255.0f );
        //ofEnableAlphaBlending();
		texture->draw( ps->x, ps->y, ps->size, ps->size );
        //ofDisableAlphaBlending();
#endif
	}
	
	glDisable(GL_BLEND);
}

#ifdef RENDER_FAST
void ofxParticleEmitter::drawVBO(){
    vbo.setVertexData(vtx, particleCount, GL_DYNAMIC_DRAW);
    vbo.setColorData(clx, particleCount, GL_DYNAMIC_DRAW);
    vbo.setNormalData(nlx, particleCount, GL_DYNAMIC_DRAW);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, blendFuncDestination);//(blendFuncSource, blendFuncDestination);
    //ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofEnablePointSprites();
    shd.begin();
    texture->bind();
    vbo.draw(GL_POINTS, 0, particleCount);
    texture->unbind();
    shd.end();
    ofDisablePointSprites();
    //ofDisableBlendMode();
    glDisable(GL_BLEND);
    
    //ofSetColor(ofColor::red);
    //texture->draw(100, 100, 100, 100);
}
#else
// this doesn't yet work, it is an attempt to port over the point sprite logic
// from opengles. It draws the point sprites but it doesn't replace the point
// size or color values. I left it here in case anyone wants to fix it :)
void ofxParticleEmitter::drawPoints()
{
	// Disable the texture coord array so that texture information is not copied over when rendering
	// the point sprites.
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	
	// Bind to the verticesID VBO and popuate it with the necessary vertex & color informaiton
	glBindBuffer(GL_ARRAY_BUFFER, verticesID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PointSprite) * maxParticles, vertices, GL_DYNAMIC_DRAW);
	
	// Configure the vertex pointer which will use the currently bound VBO for its data
	glVertexPointer(2, GL_FLOAT, sizeof(PointSprite), 0);
	glColorPointer(4,GL_FLOAT,sizeof(PointSprite),(GLvoid*) (sizeof(GLfloat)*3));
	
	// Bind to the particles texture
	glBindTexture(GL_TEXTURE_2D, (GLuint)textureData.textureID);
	
	// Enable the point size array
	
	
	//glEnableClientState(GL_POINT_SIZE_ARRAY_OES);
	
	
	// Configure the point size pointer which will use the currently bound VBO.  PointSprite contains
	// both the location of the point as well as its size, so the config below tells the point size
	// pointer where in the currently bound VBO it can find the size for each point
	
	
	//glPointSizePointerOES(GL_FLOAT,sizeof(PointSprite),(GLvoid*) (sizeof(GL_FLOAT)*2));
	
	
	// Change the blend function used if blendAdditive has been set
	
    // Set the blend function based on the configuration
    glBlendFunc(blendFuncSource, blendFuncDestination);
	
	// Enable and configure point sprites which we are going to use for our particles
	glEnable(GL_POINT_SPRITE);
	glTexEnvi( GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE );
	
	// Now that all of the VBOs have been used to configure the vertices, pointer size and color
	// use glDrawArrays to draw the points
	glDrawArrays(GL_POINTS, 0, particleIndex);
	
	// Unbind the current VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	// Disable the client states which have been used incase the next draw function does 
	// not need or use them
	
	
	//glDisableClientState(GL_POINT_SIZE_ARRAY_OES);
	
	
	glDisable(GL_POINT_SPRITE);
	
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	// Re-enable the texture coordinates as we use them elsewhere in the game and it is expected that
	// its on
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}
#endif
/*
void ofxParticleEmitter::drawPointsOES()
{
#ifdef TARGET_OF_IPHONE
	
	// Disable the texture coord array so that texture information is not copied over when rendering
	// the point sprites.
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	
	// Bind to the verticesID VBO and popuate it with the necessary vertex & color informaiton
	glBindBuffer(GL_ARRAY_BUFFER, verticesID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PointSprite) * maxParticles, vertices, GL_DYNAMIC_DRAW);
	
	// Configure the vertex pointer which will use the currently bound VBO for its data
	glVertexPointer(2, GL_FLOAT, sizeof(PointSprite), 0);
	glColorPointer(4,GL_FLOAT,sizeof(PointSprite),(GLvoid*) (sizeof(GLfloat)*3));
	
	// Bind to the particles texture
	glBindTexture(GL_TEXTURE_2D, (GLuint)textureData.textureID);
	
	// Enable the point size array
	glEnableClientState(GL_POINT_SIZE_ARRAY_OES);
	
	// Configure the point size pointer which will use the currently bound VBO.  PointSprite contains
	// both the location of the point as well as its size, so the config below tells the point size
	// pointer where in the currently bound VBO it can find the size for each point
	glPointSizePointerOES(GL_FLOAT,sizeof(PointSprite),(GLvoid*) (sizeof(GL_FLOAT)*2));
	
	// Change the blend function used if blendAdditive has been set
	
    // Set the blend function based on the configuration
    glBlendFunc(blendFuncSource, blendFuncDestination);
	
	// Enable and configure point sprites which we are going to use for our particles
	glEnable(GL_POINT_SPRITE_OES);
	glTexEnvi( GL_POINT_SPRITE_OES, GL_COORD_REPLACE_OES, GL_TRUE );
	
	// Now that all of the VBOs have been used to configure the vertices, pointer size and color
	// use glDrawArrays to draw the points
	glDrawArrays(GL_POINTS, 0, particleIndex);
	
	// Unbind the current VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	// Disable the client states which have been used incase the next draw function does 
	// not need or use them
	glDisableClientState(GL_POINT_SIZE_ARRAY_OES);
	glDisable(GL_POINT_SPRITE_OES);
	
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	// Re-enable the texture coordinates as we use them elsewhere in the game and it is expected that
	// its on
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
#endif
}
 */

void ofxParticleEmitter::changeTexture(string filename) {
    
    texture->loadImage(filename);
    textureName = filename;
}

string ofxParticleEmitter::getTextureName() {
    
    return textureName;
}

//emitter controls
void ofxParticleEmitter::setPosition(ofVec2f np){
    sourcePosition.x = np.x;
    sourcePosition.y = np.y;
}

ofVec2f ofxParticleEmitter::getPosition(){
    return ofVec2f(sourcePosition.x, sourcePosition.y);
}

bool ofxParticleEmitter::isRunning(){
    return active;
}

bool ofxParticleEmitter::isReady(){
    return bInited;
}

void ofxParticleEmitter::stop(){
    if(isRunning()){
        stopParticleEmitter();
    }
}

void ofxParticleEmitter::start(){
    if(!isRunning()){
        startTime = ofGetElapsedTimef();
        endTime = startTime + duration;
        active = true;
        bStarted = true;
   }
}

void ofxParticleEmitter::setLoopType(ofLoopType _loop){
    if(_loop==OF_LOOP_PALINDROME){
        ofLogError("ofxParticleEmitter", "You cannot set a palindrome loop.");
        return;
    }else{
        loopType = _loop;
    }
}

void ofxParticleEmitter::setViewport(ofRectangle r, float blockX, float blockY){
    vField.setup(r, ofVec2f(blockX,blockY));
}

//flipping (to be called first)
void ofxParticleEmitter::flipY(bool b){
    yFlipper = b?-1:1;
}

void ofxParticleEmitter::flipSprite(bool bX, bool bY){
    flipSpriteX=bX;
    flipSpriteY=bY;
}

void ofxParticleEmitter::applySpriteFlipping(){
    if(flipSpriteX || flipSpriteY){
        texture->mirror(flipSpriteX, flipSpriteY);
    }
}
//---

//push
void ofxParticleEmitter::resetPushers(){
    emissionPush.set(0,0);
}
    
void ofxParticleEmitter::applyEmissionPush(ofVec2f psh){
    emissionPush=psh;
}
    
//---

ofImage * ofxParticleEmitter::getSprite(){
    return texture;
}
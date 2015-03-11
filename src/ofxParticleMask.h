//
//  ofxParticleMask.h
//
//  Created by nausea on 1/21/15.
//
//

#pragma once
#include "ofMain.h"
#include "ofxPexStuff.h"

class ofxParticleMask{
public:
    ofxParticleMask();
    void clear();
    void reserve(int nPts);
    void setFromBinaryImage(ofImage & img, int bW, int bH);
    
    void transformToRect(ofRectangle r);
    void moveToLine(ofVec2f ptStart, ofVec2f ptStop, int nParts, int idxStart=0);
    
    void setImageBlock(ofImage & img, ofColor col, int sX, int sY);
    float getBlockBrightness(ofImage & img, int sX, int sY);
    
    void attractPex(Particle * part, int & pIdx);
    
    void setAttraction(bool b);
    void setFade(bool b);
    void setRefill(bool b);
    void toggleAttraction();
    void toggleFade();
    void toggleRefill();
    
    vector<ofVec2f> getCurrentConfiguration();
    void setConfiguration(vector<ofVec2f> _pts);
    void moveBy(ofVec2f off);
    
    void drawDebug();
    
    vector<ofVec2f> pts;
    ofVec2f blockSz;
    float bgtThr;
    float strength;
    int numLinked;
    bool bFade;
    bool bAttract;
    bool bRefill;
};
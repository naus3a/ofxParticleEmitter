//
//  ofxBasicVectorfield.h
//
//  Created by nausea on 11/12/14.
//
//

#pragma once
#include "ofMain.h"

class ofxBasicVectorfield{
public:
    struct Coords{
        void set(int _x, int _y){
            x=_x;
            y=_y;
        }
        int x=0;
        int y=0;
    };
    
    ofxBasicVectorfield();
    void setup(ofRectangle _rvp, ofVec2f _block);
    void initMask();
    void clearField();
    void initField();
    void zeroField();
    
    bool inside(float _x, float _y);
    ofVec2f getVector(int _x, int _y);
    ofVec2f getVector(Coords _cc);
    ofVec2f getVectorForPoint(float px, float py);
    Coords getCoords(float px, float py);
    
    void clampCoords(Coords * _cc);
    
    void addVector(int _x, int _y, ofVec2f & _vec);
    void addVector(Coords _cc, ofVec2f & _vec);
    void addVectorForPoint(float px, float py, ofVec2f & _vec);
    
    void addMask(ofTexture * _txt, float mult=1);
    void addMask(ofPixels & _pix, float mult=1);
    float checkPixelsInBlock(ofPixels & pix, ofRectangle _block);
    
    void draw();
    
    ofFbo fboMask;
    ofPixels pixMask;
protected:
    ofRectangle rVp;
    vector<vector<ofVec2f> > field;
    ofVec2f block;
    int cols;
    int rows;
    int xMax;
    int yMax;
};
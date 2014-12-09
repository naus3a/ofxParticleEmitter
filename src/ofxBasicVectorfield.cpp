//
//  ofxBasicVectorfield.cpp
//
//  Created by nausea on 11/12/14.
//
//

#include "ofxBasicVectorfield.h"

ofxBasicVectorfield::ofxBasicVectorfield(){
    cols=0;
    rows=0;
}

void ofxBasicVectorfield::setup(ofRectangle _rvp, ofVec2f _block){
    rVp=_rvp;
    block=_block;
    
    cols = int(rVp.width/block.x);
    rows = int(rVp.height/block.y);
    xMax = cols-1;
    yMax = rows-1;
    
    initMask();
    
    initField();
}

void ofxBasicVectorfield::initMask(){
    ofVec2f mskSz(rVp.width,rVp.height);
    if(mskSz.x>1000){
        mskSz/=5;
    }
    fboMask.allocate(mskSz.x, mskSz.y, GL_RGB);
}

void ofxBasicVectorfield::clearField(){
    for(int i=0;i<field.size();i++){
        field[i].clear();
    }
    field.clear();
}

void ofxBasicVectorfield::initField(){
    clearField();
    
    field.resize(rows);
    for(int i=0;i<rows;i++){
        field[i].resize(cols);
    }
    
    zeroField();
}

void ofxBasicVectorfield::zeroField(){
    for(int y=0;y<field.size();y++){
        for(int x=0;x<field[y].size();x++){
            field[y][x].set(0,0);
        }
    }
}

bool ofxBasicVectorfield::inside(float _x, float _y){
    return rVp.inside(_x, _y);
}

ofVec2f ofxBasicVectorfield::getVector(ofxBasicVectorfield::Coords _cc){
    return getVector(_cc.x, _cc.y);
}

ofVec2f ofxBasicVectorfield::getVector(int _x, int _y){
    return field[_y][_x];
}

ofxBasicVectorfield::Coords ofxBasicVectorfield::getCoords(float px, float py){
    ofxBasicVectorfield::Coords cc;
    cc.set(int(px/block.x), int(py/block.y));
    clampCoords(&cc);
    return cc;
}

ofVec2f ofxBasicVectorfield::getVectorForPoint(float px, float py){
    return getVector(getCoords(px, py));
}

void ofxBasicVectorfield::addVector(int _x, int _y, ofVec2f & _vec){
    field[_y][_x] += _vec;
}

void ofxBasicVectorfield::addVector(ofxBasicVectorfield::Coords _cc, ofVec2f & _vec){
    addVector(_cc.x, _cc.y, _vec);
}

void ofxBasicVectorfield::addVectorForPoint(float px, float py, ofVec2f & _vec){
    addVector(getCoords(px, py), _vec);
}

void ofxBasicVectorfield::clampCoords(ofxBasicVectorfield::Coords *_cc){
    int xMax = cols-1;
    int yMax = rows-1;
    _cc->x = _cc->x < 0? 0 : _cc->x > xMax? xMax : _cc->x;
    _cc->y = _cc->y < 0? 0 : _cc->y > yMax? yMax : _cc->y;
}

void ofxBasicVectorfield::addMask(ofTexture *_txt, float mult){
    fboMask.begin();
    ofClear(0, 0, 0, 255);
    _txt->draw(0,0,fboMask.getWidth(),fboMask.getHeight());
    fboMask.end();
    
    fboMask.readToPixels(pixMask);
    addMask(pixMask, mult);
}

void ofxBasicVectorfield::addMask(ofPixels & _pix, float mult){
    int bw = int(_pix.getWidth()/cols);
    int bh = int(_pix.getHeight()/rows);
    for(int yy=0; yy<rows; yy++){
        for(int xx=0; xx<cols; xx++){
            float score = checkPixelsInBlock(_pix, ofRectangle(xx*bw, yy*bh, bw, bh));
            if(score>0){
                score *= mult;
                ofVec2f vv;
                
                //to save and extra loop, we use a simplified repulsion model with only 4 directions
                vv.x = xx<bw/2?-1:1;
                vv.y = yy<bh/2?-1:1;
                
                vv *= score;
                addVector(xx, yy, vv);
            }
        }
    }
}

float ofxBasicVectorfield::checkPixelsInBlock(ofPixels & pix, ofRectangle _block){
    // ---------
    //|    X    |
    //|X   X   X|
    //|    X    |
    // ---------
    
    float hw = _block.x + (_block.width/2);
    float hh = _block.y + (_block.height/2);
    
    float score = 0;
    
    score += pix.getColor(hw, hh).r;
    score += pix.getColor(_block.x, hh).r;
    score += pix.getColor(_block.x + _block.width, hh).r;
    score += pix.getColor(hw, _block.y).r;
    score += pix.getColor(hw, _block.y + _block.height).r;
    
    score /= 5.0;
    score /= 255;
    
    return score;
}

void ofxBasicVectorfield::draw(){
    ofPushStyle();
    ofEnableAlphaBlending();
    ofNoFill();
    for(int y=0;y<field.size();y++){
        for(int x=0;x<field[y].size();x++){
            ofPushMatrix();
            ofTranslate(x*block.x, y*block.y);
            ofSetColor(255,255,255,20);
            ofRect(0,0,block.x,block.y);
            ofTranslate(block.x/2, block.y/2);
            ofSetColor(255,0,0,80);
            ofSetLineWidth(2);
            ofLine(0, 0, field[y][x].x, field[y][x].y);
            ofPopMatrix();
        }
    }
    ofPopStyle();
}
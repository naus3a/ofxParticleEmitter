//
//  ofxParticleMask.cpp
//
//  Created by nausea on 1/21/15.
//
//

#include "ofxParticleMask.h"

ofxParticleMask::ofxParticleMask(){
    blockSz.set(1,1);
    bgtThr = 127;
    strength = 1.0;
    clear();
    bFade = false;
    bAttract = true;
    bRefill = false;
}

void ofxParticleMask::clear(){
    numLinked = 0;
    pts.clear();
}

void ofxParticleMask::reserve(int nPts){
    clear();
    blockSz.set(5,5);
    for(int i=0;i<nPts;i++){
        ofVec2f pt = ofVec2f(0,0);
        pts.push_back(pt);
    }
}

void ofxParticleMask::setFromBinaryImage(ofImage &img, int bW, int bH){
    clear();
    blockSz.set(bW, bH);
    ofVec2f hB = blockSz/2;
    
    for(int y=0;y<img.getHeight();y+=blockSz.y){
        for(int x=0;x<img.getWidth();x+=blockSz.x){
            float bgt = getBlockBrightness(img, x, y);
            if(bgt>bgtThr){
                ofVec2f pt = ofVec2f(x,y);
                pt += hB;
                pts.push_back(pt);
            }
        }
    }
}

void ofxParticleMask::transformToRect(ofRectangle r){
    int nParts = pts.size();
    int per = (r.width+r.height)*2;
    float pctW = r.width/per;
    float pctH = r.height/per;
    int partsW = nParts*pctW;
    int partsH = nParts*pctH;
    
    ofVec2f v1 = ofVec2f(r.x, r.y);
    ofVec2f v2 = ofVec2f(r.x + r.width, r.y);
    ofVec2f v3 = ofVec2f(v1.x, r.y + r.height);
    ofVec2f v4 = ofVec2f(v2.x, v3.y);
    
    int curIdx = 0;
    moveToLine(v1, v2, partsW, curIdx);
    curIdx += partsW;
    moveToLine(v3, v4, partsW, curIdx);
    curIdx += partsW;
    moveToLine(v1, v3, partsH, curIdx);
    curIdx += partsH;
    moveToLine(v2, v4, partsH, curIdx);
}

void ofxParticleMask::moveToLine(ofVec2f ptStart, ofVec2f ptStop, int nParts, int idxStart){
    int idxStop = idxStart + nParts;
    if(idxStop<pts.size()){
        float pctInc = 1.0/(idxStop-idxStart);
        for(int i=idxStart; i<=idxStop; i++){
            float curInc = (i-idxStart)*pctInc;
            pts[i].x = ofLerp(ptStart.x, ptStop.x, curInc);
            pts[i].y = ofLerp(ptStart.y, ptStop.y, curInc);
        }
    }
}

float ofxParticleMask::getBlockBrightness(ofImage &img, int sX, int sY){
    int xEnd = sX+blockSz.x;
    int yEnd = sY+blockSz.y;
    float bgt = 0;
    int bSz = 0;
    for(int yy=sY;yy<yEnd;yy++){
        for(int xx=sX;xx<xEnd;xx++){
            bgt += img.getColor(xx, yy).r;
            bSz++;
        }
    }
    if(bSz>0)bgt /= bSz;
    return bgt;
}

void ofxParticleMask::setImageBlock(ofImage &img, ofColor col, int sX, int sY){
    int xEnd = sX+blockSz.x;
    int yEnd = sY+blockSz.y;
    if(xEnd>img.getWidth())xEnd=img.getWidth();
    if(yEnd>img.getHeight())yEnd=img.getHeight();
    for(int yy=sY;yy<yEnd;yy++){
        for(int xx=sX;xx<xEnd;xx++){
            img.setColor(xx, yy, col);
        }
    }
}

void ofxParticleMask::attractPex(Particle *part, int &pIdx){
    if(bAttract){
        if(pIdx<numLinked){
            ofVec2f cp = ofVec2f(part->position.x, part->position.y);
            ofVec2f vv = pts[pIdx]-cp;
            vv.normalize();
            vv *= strength;
            
            cp += vv;
            part->position.x = cp.x;
            part->position.y = cp.y;
        }
    }
}

void ofxParticleMask::setAttraction(bool b){
    bAttract = b;
}

void ofxParticleMask::setFade(bool b){
    bFade = b;
}

void ofxParticleMask::setRefill(bool b){
    bRefill = b;
}

void ofxParticleMask::toggleAttraction(){
    bAttract = !bAttract;
}

void ofxParticleMask::toggleFade(){
    bFade = !bFade;
}

void ofxParticleMask::toggleRefill(){
    bRefill = !bRefill;
}

vector<ofVec2f> ofxParticleMask::getCurrentConfiguration(){
    return pts;
}

void ofxParticleMask::setConfiguration(vector<ofVec2f> _pts){
    pts = _pts;
}

void ofxParticleMask::moveBy(ofVec2f off){
    for(int i=0;i<pts.size();i++){
        pts[i]+=off;
    }
}

void ofxParticleMask::drawDebug(){
    ofPushStyle();
    ofNoFill();
    ofSetColor(ofColor::red);
    ofVec2f hB = blockSz/2;
    for(int i=0;i<pts.size();i++){
        ofRect(pts[i].x-hB.x, pts[i].y-hB.y, blockSz.x, blockSz.y);
    }
    ofPopStyle();
}
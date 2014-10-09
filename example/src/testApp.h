#pragma once

#include "ofMain.h"
#include "ofxParticleEmitter.h"
#include "NauMisc.h"

class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
    void checkFiles();
    void setPreset(int idx);
    void setPreset(string name);
    
    ofxParticleEmitter m_emitter;
    vector<ofFile> files;
    string curName;
    int curPreset;
};

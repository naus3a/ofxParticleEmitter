#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
    ofBackground(0);
    ofSetFrameRate(60);
    
    //setPreset(0);
    setPreset("21_blue_burst.pex");
}

void testApp::checkFiles(){
    files = NauMisc::getFilesInDir("", "pex");
    if(curPreset<0 || curPreset>=files.size()){
        curPreset=0;
    }
}

void testApp::setPreset(int idx){
    curPreset = idx;
    checkFiles();
    if(files.size()>=idx){
        curName = files[curPreset].getFileName();
        if(!m_emitter.loadFromXml(curName)){
            ofLog(OF_LOG_ERROR,"testApp::setup() - failed to load emitter config");
        }
    }
}

void testApp::setPreset(string name){
    checkFiles();
    for(int i=0;i<files.size();i++){
        if(files[i].getFileName()==name){
            curPreset = i;
            curName =  name;
            i=files.size()+2;
        }
    }
    if(!m_emitter.loadFromXml(files[curPreset].getFileName())){
        ofLog(OF_LOG_ERROR,"testApp::setup() - failed to load emitter config");
    }
}

//--------------------------------------------------------------
void testApp::update(){
    m_emitter.update();
}

//--------------------------------------------------------------
void testApp::draw(){
    m_emitter.draw();
	
	ofSetColor( 255, 255, 255 );
	ofDrawBitmapString( "fps: " + ofToString( ofGetFrameRate(), 2 ), 20, 20 );
    ofDrawBitmapString("right key to scroll through the pex files in data/", 20,40);
    ofDrawBitmapString("Preset "+ofToString(curPreset+1)+" of "+ofToString(files.size()), 20,60);
    ofDrawBitmapString(curName, 20, 80);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    if(key==OF_KEY_RIGHT){
        setPreset(curPreset+1);
    }
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
    m_emitter.sourcePosition.x = x;
	m_emitter.sourcePosition.y = y;
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}

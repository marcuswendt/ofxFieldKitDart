#include "ofApp.h"

#include "Isolate.h"

using namespace fieldkit;

void ofApp::setup()
{
	dartVM = fieldkit::dart::DartVM::create("snapshots/full.snapshot");

	dartVM->Init(true);

	script = dartVM->LoadScript( "main.dart" );

    script->Invoke("main");
    
    sketch = script->New("Sketch");
}


void ofApp::update()
{
    script->Invoke(sketch, "update");
}

void ofApp::draw()
{
    script->Invoke(sketch, "draw");
}


// --- INPUT EVENTS -----------------------------------------------------------
void ofApp::keyPressed(int key){

}

void ofApp::keyReleased(int key){

}

void ofApp::mouseMoved(int x, int y ){

}

void ofApp::mouseDragged(int x, int y, int button){

}

void ofApp::mousePressed(int x, int y, int button){

}

void ofApp::mouseReleased(int x, int y, int button){

}

void ofApp::dragEvent(ofDragInfo dragInfo){
    
}


// --- SYSTEM EVENTS -----------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

void ofApp::gotMessage(ofMessage msg){

}

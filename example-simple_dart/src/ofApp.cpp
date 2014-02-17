#include "ofApp.h"

#include "Isolate.h"

using namespace fieldkit;

static int64_t ToInt64(Dart_Handle h) {
	int64_t i = 0;
	Dart_Handle res = Dart_IntegerToInt64(h, &i);
	return i;
}


void ofApp::setup()
{

	dartVM = fieldkit::dart::DartVM::create("snapshots/dart.1.0.core.snapshot");

	dartVM->init(true);

	script = shared_ptr<dart::Isolate>(dartVM->loadScript( "main.dart" ));

	script->invoke("main");

	sketch = script->create("Sketch",0,NULL);

	script->invoke("get_int");

}


void ofApp::update()
{
	//dart::Invoke(sketch, "update");

}

void ofApp::draw()
{
	//dart::Invoke(sketch, "draw");
}


// --- INPUT EVENTS -----------------------------------------------------------
void ofApp::keyPressed(int key){

}

void ofApp::keyReleased(int key){
	if (key == ' '){
 		dart::Invoke dart_function(sketch, "getMeaningOfLife");
		ofLogNotice() << (Dart_IsInteger(dart_function.getResult()) ? "integer" : "something else");
		int64_t res=0;
		Dart_IntegerToInt64(dart_function.getResult(), &res);
		ofLogNotice() << res;
	}
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

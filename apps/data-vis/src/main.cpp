#include "precomp.h"

//========================================================================
int main( ){
	ofGLFWWindowSettings settings;
	settings.setSize(800, 600);
	settings.setGLVersion(3, 3);
	ofCreateWindow(settings);

	ofRunApp(new ofApp());
}

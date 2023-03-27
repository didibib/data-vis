#include "precomp.h"
#include "ofApp.h"

// ========================================================================
int main()
{
	ofGLFWWindowSettings settings;
	settings.setSize(800, 600);
	settings.setGLVersion(3, 3);
	settings.title = "Data Visualizer";
	const auto window = ofCreateWindow(settings);

#ifdef _WIN32
	const HWND hwnd = window->getWin32Window();
	ShowWindow(hwnd, SW_MAXIMIZE);
#endif
	ofRunApp(make_shared<ofApp>());
}

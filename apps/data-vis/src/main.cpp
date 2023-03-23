#include "precomp.h"
#include "ofApp.h"

float cross(const glm::vec2& p, const glm::vec2& q)
{
	return p.x * q.y - p.y * q.x;
}

// ========================================================================
int main()
{
	const glm::vec2 P0 = { 1, 3 };
	const glm::vec2 P1 = { 5, 4 };
	const glm::vec2 r = P1 - P0;
	// Pdir.x * X + P0.y = y

	// First projected line along the normal             
	const glm::vec2 Q0 = { 2, 2 };
	const glm::vec2 Q1 = { 5, 1 };
	const glm::vec2 Qdir = Q1 - Q0;
	const glm::vec2 s = glm::vec2(-Qdir.y, Qdir.x);

	auto t0 = cross(Q0 - P0, s) / cross(r, s);
	auto t1 = cross(Q1 - P0, s) / cross(r, s);
	auto i0 = P0 + t0 * r;
	auto i1 = P0 + t1 * r;

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

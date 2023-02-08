#pragma once

// ImGui
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

// ImPlot
#include "implot/implot.h"

typedef unsigned int uint;

uint RandomUInt();
uint RandomUInt(uint& seed);
float RandomFloat();
float RandomRange(float range);

// Own headers
#include "ofMain.h"
#include "ofApp.h"
#include "visuals/graph.h"
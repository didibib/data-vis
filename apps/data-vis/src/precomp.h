#pragma once

// ImGui
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "boost/graph/graphviz.hpp"

// ==============================================
typedef unsigned int uint;

uint RandomUInt();
uint RandomUInt(uint& seed);
float RandomFloat();
float RandomRange(float range);

// ==============================================

// OF header
#include "ofMain.h"

// STD headers
#include <iostream>
#include <map>
#include <vector>

// Own headers
#include "ofApp.h"
#include "visuals/graph.h"

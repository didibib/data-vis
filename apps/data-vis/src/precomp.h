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

// Purple color palette: #635985 #443C68 #393053 #18122B
const ofColor palettePurple_0(99, 89, 133);
const ofColor palettePurple_1(68, 60, 104);
const ofColor palettePurple_2(57, 48, 83);
const ofColor palettePurple_3(24, 18, 43);

// STD headers
#include <iostream>
#include <map>
#include <vector>

// Own headers
#include "ofApp.h"
#include "graph.h"

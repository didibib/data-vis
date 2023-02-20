#pragma once

// ImGui
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

// Boost
#include <boost/graph/graphviz.hpp>
#include <boost/program_options.hpp>

//--------------------------------------------------------------
// OF header
#include "ofMain.h"

// Purple color palette: #635985 #443C68 #393053 #18122B
namespace Color {
	const ofColor palettePurple_0(99, 89, 133);
	const ofColor palettePurple_1(68, 60, 104);
	const ofColor palettePurple_2(57, 48, 83);
	const ofColor palettePurple_3(24, 18, 43);
}

//--------------------------------------------------------------
// STD headers
#include <iostream>
#include <string>
#include <filesystem>
#include <map>
#include <vector>
#include <random>
#include <functional>
#include <map>

//--------------------------------------------------------------
typedef unsigned int uint;

namespace Random {
	std::random_device random_device;
	std::mt19937 MT19937(random_device());

	uint UInt();
	uint UInt(uint& seed);
	uint Range(uint, uint);
	uint Range(uint);
	float Float();
	float RangeF(float range);
}

void ParseCmdline(const boost::program_options::options_description&, std::string);

//--------------------------------------------------------------
// Own headers
#include "graph.h"
#include "tree.h"
#include "layout.h"
#include "optimizer.h"
#include "ofApp.h"

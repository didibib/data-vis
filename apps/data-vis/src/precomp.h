#pragma once

// ImGui
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

// Boost
#include <boost/graph/graphviz.hpp>
#include <boost/program_options.hpp>
#include <boost/exception/exception.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/lexical_cast.hpp>

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
#include <fstream>
#include <iostream>
#include <deque>

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

namespace Curves {
/// <summary>
/// Gives a value in the range [0,1]
/// </summary>
/// <param name="time">: in of [0,1]</param>
/// <returns></returns>
float Bezier(float time);
}

namespace Parser {
	void Cmdline(const boost::program_options::options_description&, std::string);
}

//--------------------------------------------------------------
// External
#include "external/spirit-graphviz.hpp"

//--------------------------------------------------------------
// Own headers
#include "layout.h"
#include "graph.h"
#include "tree.h"
#include "optimizer.h"

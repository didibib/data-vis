#pragma once

// ImGui
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

// ImPlot
#include "implot/implot.h"

// Boost
#include <boost/graph/graphviz.hpp>
#include <boost/program_options.hpp>
#include <boost/exception/exception.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/lexical_cast.hpp>

//--------------------------------------------------------------
// OF header
#include "ofMain.h"

namespace Color
{
    // Purple color palette: #635985 #443C68 #393053 #18122B
    const ofColor palettePurple_0(99, 89, 133);
    const ofColor palettePurple_1(68, 60, 104);
    const ofColor palettePurple_2(57, 48, 83);
    const ofColor palettePurple_3(24, 18, 43);

    // Cold color pallete: #F1F6F9 #14274E #394867 #9BA4B4
    const ofColor paletteCold_0(246, 246, 246);
    const ofColor paletteCold_1(20, 39, 78);
    const ofColor paletteCold_2(57, 72, 103);
    const ofColor paletteCold_3(155, 164, 180);
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
#include <variant>
#include <limits.h>

//--------------------------------------------------------------
typedef unsigned int uint;

namespace DataVis
{
    constexpr float MAX_FLOAT = std::numeric_limits<float>::max();
    constexpr float MIN_FLOAT = std::numeric_limits<float>::min();
    constexpr int MAX_INT = std::numeric_limits<int>::max();
    constexpr int MIN_INT = std::numeric_limits<int>::min();

    namespace Random
    {
        inline std::random_device random_device;
        inline std::mt19937 MT19937(random_device());

        uint UInt();
        uint UInt(uint& seed);
        uint Range(uint, uint);
        uint Range(uint);
        float Float();
        float RangeF(float range);
    } // namespace Random

    namespace Curves
    {
        /// <summary>
        /// Returns value in the range [0,1]
        /// </summary>
        /// <param name="time">: in range of [0,1]</param>
        /// <returns></returns>
        float Bezier(float time);
    } // namespace Curves

    namespace ImGuiExtensions
    {
        // Converts color from [0..255] to [0..1] for displaying and back
        bool ColorEdit3(const char* label, glm::vec3& color, int flags);
        bool ColorEdit4(const char* label, glm::vec4& color, int flags);
        ofColor Vec3ToOfColor(const glm::vec3& color);
        ofColor Vec4ToOfColor(const glm::vec4& color);
    }

    namespace Math
    {
        float Cross(const glm::vec2& p, const glm::vec2& q);
    }
} // namespace DataVis

//--------------------------------------------------------------
// External
// Dot Parser
#include "spirit-graphviz.hpp"
// T-SNE
#define QDTSNE_CUSTOM_NEIGHBORS
#include "qdtsne/tsne.hpp"
// MDS
#include "SimpleMatrix/SimpleMatrix.h"

//--------------------------------------------------------------
// Own headers
#include "dataset.h"
#include "floyd_warshall.h"
#include "animator.h"
#include "aabb.h"
#include "structure/node.h"
#include "structure/edge_path.h"
#include "structure/structure.h"
#include "structure/graph.h"
#include "structure/tree.h"
#include "structure/clusters.h"
#include "layout/layout.h"

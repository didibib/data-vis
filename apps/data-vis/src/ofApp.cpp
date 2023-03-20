#include "precomp.h"
#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    // Init ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsLight();
    ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)ofGetWindowPtr()->getWindowContext(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Set OpenGL 
    ofEnableDepthTest();

    // Setup 2D camera https://gist.github.com/roymacdonald/cd92c6d5adfa2c8aeffb22fc6c293bcc
    m_camera.removeAllInteractions();
    m_camera.addInteraction(ofEasyCam::TRANSFORM_TRANSLATE_XY, OF_MOUSE_BUTTON_RIGHT);
    m_camera.addInteraction(ofEasyCam::TRANSFORM_TRANSLATE_Z, OF_MOUSE_BUTTON_MIDDLE);
    m_camera.setScrollSensitivity(20);
    m_camera.setVFlip(true);
    m_camera.setFarClip(1e30);
    m_camera.setGlobalPosition(glm::vec3(0, 0, 2500));

    // Load
    LoadDotFiles();

    m_factories.emplace_back("Graph",
                             [](std::shared_ptr<Dataset> _dataset)
                             {
                                 auto g = make_shared<Graph>();
                                 g->Init(_dataset);
                                 RandomLayout::Apply(*g, 800, 800);
                                 return g;
                             });
    m_factories.emplace_back("MSP Tree",
                             [](std::shared_ptr<Dataset> _dataset)
                             {
                                 auto m = make_shared<MSP>();
                                 m->Init(_dataset);
                                 RandomLayout::Apply(*m, 800, 800);
                                 return m;
                             });
	m_factories.emplace_back("Clusters",
                             [](std::shared_ptr<Dataset> _dataset)
                             {
                                 auto c = make_shared<Clusters>();
                                 c->Init(_dataset);
                                 return c;
                             });
}

//--------------------------------------------------------------
void ofApp::LoadDotFiles()
{
    const std::string data_path = ofToDataPath("", false);
    for (const auto& entry : filesystem::directory_iterator(data_path))
    {
        if (entry.path().extension() == ".dot")
        {
            std::string filename = entry.path().filename().string();
            m_graph_file_names.push_back(filename);
            if (Model::MainGraph graph; DataVis::Parser::DotFile(filename, graph))
            {
                auto dataset = std::make_unique<DataVis::Dataset>();
                if (not graph.graph.subgraphs.empty())
                    dataset = std::make_unique<DataVis::DatasetClusters>();
                dataset->Load(graph, filename);
                m_datasets.push_back(std::move(dataset));
            }
        }
    }
    m_current_graph_file = m_graph_file_names[m_imgui_data.combo_graph_file_index];
}

//--------------------------------------------------------------
void ofApp::update()
{
    const ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse || io.WantCaptureKeyboard)
        m_camera.disableMouseInput();
    else m_camera.enableMouseInput();
    for (auto& layout : m_structures)
    {
        layout.get()->Update(ofGetLastFrameTime());
    }
}

//--------------------------------------------------------------
void ofApp::draw()
{
    ofBackgroundGradient(Color::paletteCold_0, Color::paletteCold_0, OF_GRADIENT_CIRCULAR);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    m_camera.begin();

    for (auto& layout : m_structures)
    {
        layout.get()->Draw(m_focussed_structure == layout);
    }

    m_camera.end();

    Gui();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

//--------------------------------------------------------------
void ofApp::exit()
{
    // Destroy ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ofApp::Gui()
{
    if (ImGui::BeginMainMenuBar())
    {
        //--------------------------------------------------------------
        // Create IStructure 
        //--------------------------------------------------------------
        if (!m_datasets.empty())
        {
            if (ImGui::BeginMenu("Create"))
            {
                const char* select_dataset_preview = m_datasets[m_imgui_data.combo_dataset_index]->GetFilename().c_str();
                if (ImGui::BeginCombo("Select Dataset", select_dataset_preview))
                {
                    for (int n = 0; n < m_datasets.size(); n++)
                    {
                        const bool is_selected = (m_imgui_data.combo_dataset_index == n);
                        if (ImGui::Selectable(m_datasets[n]->GetFilename().c_str(), is_selected))
                            m_imgui_data.combo_dataset_index = n;
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                const char* select_structure_preview = m_factories[m_imgui_data.combo_structure_index].first.c_str();
                if (ImGui::BeginCombo("Select Structure", select_structure_preview))
                {
                    for (int n = 0; n < m_factories.size(); n++)
                    {
                        const bool is_selected = (m_imgui_data.combo_structure_index == n);
                        if (ImGui::Selectable(m_factories[n].first.c_str(), is_selected))
                            m_imgui_data.combo_structure_index = n;
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                if (ImGui::Button("Create Structure"))
                {
                    auto new_dataset_file = m_datasets[m_imgui_data.combo_dataset_index];
                    auto factory = m_factories[m_imgui_data.combo_structure_index].second;
                    auto structure = factory(new_dataset_file);
                    structure->SetOnDeleteCallback(std::bind(&ofApp::DeleteStructure, this, std::placeholders::_1));
                    m_structures.push_back(std::move(structure));
                }
                ImGui::EndMenu();
            }
        }

        ImGui::EndMainMenuBar();
    }

    if (m_focussed_structure)
        m_focussed_structure.get()->Gui();
}

void ofApp::DeleteStructure(DataVis::IStructure& _structure)
{
    auto it = m_structures.begin();
    while (it != m_structures.end())
    {
        if (it->get() == addressof(_structure))
        {
            m_structures.erase(it);
            if (m_focussed_structure.get() == addressof(_structure))
                m_focussed_structure = nullptr;
            return;
        }
        it++;
    }
}

glm::vec3 ofApp::ScreenToWorld(const glm::vec2& _position)
{
    auto cam = m_camera.getGlobalPosition();
    auto world = m_camera.screenToWorld(glm::vec3(_position.x, _position.y, 0));
    // Ray from camera origin through mouse click
    auto dir = world - cam;
    // Make dir with z-length of 1
    dir *= -1 / dir.z;
    // World pos on z=0 plane
    return cam + cam.z * dir;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
    if (key == OF_KEY_DEL)
    {
        if (m_focussed_structure)
        {
            DeleteStructure(*m_focussed_structure);
            m_focussed_structure = nullptr;
        }
    }
}

void ofApp::keyReleased(int key)
{
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y)
{
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{
    if (button == OF_MOUSE_BUTTON_LEFT && m_dragging_structure)
    {
        auto world = ScreenToWorld(glm::vec2(x, y));
        auto diff = world - m_prev_mouse_drag;
        m_dragging_structure->Move(diff);
        m_prev_mouse_drag = world;
    }
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{
    const ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;

    if (button == OF_MOUSE_BUTTON_LEFT)
    {
        auto world = ScreenToWorld(glm::vec2(x, y));

        // Check if we are beginning to drag a layout
        for (auto& layout : m_structures)
        {
            if (layout->InsideDraggable(world))
            {
                m_dragging_structure = layout;
                m_prev_mouse_drag = world;
                return;
            }
        }

        // Check if click is inside focussed layout
        if (m_focussed_structure not_eq nullptr && m_focussed_structure->Inside(world))
        {
            m_focussed_structure->Select(world);
        }
        else
        {
            m_focussed_structure = nullptr;
            // Check all layouts
            for (auto& layout : m_structures)
            {
                if (layout->Inside(world))
                {
                    m_focussed_structure = layout;
                    break;
                }
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{
    m_dragging_structure = nullptr;
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y)
{
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y)
{
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg)
{
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo)
{
}

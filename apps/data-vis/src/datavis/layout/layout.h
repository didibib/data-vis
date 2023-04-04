#pragma once

namespace DataVis
{
//--------------------------------------------------------------
// Layout
//--------------------------------------------------------------
class ILayout
{
public:
    virtual bool Gui(IStructure&) = 0;

    virtual void Update(float)
    {
    }

    virtual void Draw()
    {
    }

    virtual ~ILayout() = default;
};

//--------------------------------------------------------------
// Random
//--------------------------------------------------------------
class RandomLayout final : public ILayout
{
public:
    bool Gui(IStructure&) override;
    static void Apply(IStructure&, int width, int height);

private:
    int m_width = 800, m_height = 800;
};

//--------------------------------------------------------------
// Grid
//--------------------------------------------------------------
class GridLayout final : public ILayout
{
public:
    bool Gui(IStructure&) override;
    static void Apply(IStructure&, int width, int height, float step);

private:
    int m_width = 800, m_height = 800;
    float m_step = 100;
};

//--------------------------------------------------------------
// Local Search
//--------------------------------------------------------------
class LocalSearch final : public ILayout
{
public:
    bool Gui(IStructure&) override;
    static void Apply(IStructure&, int _iterations);

private:
    static float CalculateCost(const IStructure& _structure);
    static float CalculateIncrementalCost(const IStructure& _structure, uint _idx0, uint _idx1);
    static float CalculateNodeCost(const IStructure& _structure, uint _idx);
    static void SwapPos(IStructure& _structure, float& _currCost);
    int m_iterations = 10000;
};

//--------------------------------------------------------------
// Radial
//--------------------------------------------------------------
class RadialLayout final : public ILayout
{
public:
    bool Gui(IStructure&) override;
    void Update(float) override;
    void Draw() override;
    static void Apply(ITree&, float start, float step);

private:
    // Pavlo, 2006 https://scholarworks.rit.edu/cgi/viewcontent.cgi?referer=&httpsredir=1&article=1355&context=theses
    static void SubTree(ITree::TreeNode&, float angle_start, float angle_end, int depth, float step, float delta_angle);
    float m_start = 100, m_step = 100;
    Rings m_rings;
};

//--------------------------------------------------------------
// Force Directed
//--------------------------------------------------------------
class ForceDirectedLayout final : public ILayout
{
public:
    bool Gui(IStructure&) override;
    static void Apply(IStructure&, float _C, float _t, int _iterations);

private:
    float m_C = 0.5, m_T = 0.002;
    int m_iterations = 10;
    bool m_enabled = false;
};

//--------------------------------------------------------------
// Sugiyama Framework
//--------------------------------------------------------------
class SugiyamaLayout final : public ILayout
{
public:
    using Layer = std::vector<int>;
    using GetNeighbors = std::function<std::vector<Neighbor>(Vertex&)>;
    using OSCMHeuristic = std::function<bool(const Dataset&, const Layer&, Layer&, Layer&, const GetNeighbors)>;

    SugiyamaLayout();
    bool Gui(IStructure&) override;
    static void Apply(Graph&, const OSCMHeuristic& oscm_heuristic, const glm::vec2& node_offset,
                      const int& oscm_iterations, bool _curved_edges = true);

private:
    std::vector<std::pair<std::string, OSCMHeuristic>> m_oscm_heuristics;
    OSCMHeuristic m_oscm_heuristic;
    int m_oscm_heuristic_idx = 0;
    int m_oscm_iterations = 100;
    glm::vec2 m_node_offset;
    bool m_curved_edges = true;

    // Sugiyama Framework
    static Dataset BreakCycles(Dataset&, std::vector<int>& reversed_edges);
    static void LayerAssignment(const Dataset&, std::vector<Layer>& vertices_per_layer, Layer& layer_per_vertex);
    static void AddDummyVertices(Dataset&, std::vector<Layer>& vertices_per_layer, Layer& layer_per_vertex);
    static int CrossingMinimization(Dataset&, std::vector<Layer>& vertices_per_layer, const OSCMHeuristic& heuristic,
                                    int iterations);
    
    static std::vector<float> VertexPositioning(
        const Dataset& dataset,
        const std::vector<Layer>& vertices_per_layer,
        Layer& layer_per_vertex,
        float delta_x);

    //--------------------------------------------------------------
    // Layer Assignment
    //--------------------------------------------------------------
    static bool IsSink(const Vertex&);
    static bool IsSource(const Vertex&);
    static bool HasUnvisited(const std::function<bool(Vertex&)>&, std::vector<std::shared_ptr<Vertex>>, Vertex& out);
    static void RemoveOutgoingNeighbors(Dataset&, Vertex&);
    static void RemoveIncomingNeighbors(const Dataset&, Vertex&);
    static void RemoveNeighbors(Dataset&, const Edge&);
    static void AddNeighbors(const Dataset&, Edge&);

    //--------------------------------------------------------------
    // OSCM
    //--------------------------------------------------------------
    static bool OSCMBarycenterHeuristic(
        const Dataset& dataset,
        const Layer& layer_fixed,
        Layer& layer,
        Layer& new_layer,
        const GetNeighbors& get_neighbors);
    
    static bool OSCMMedianHeuristic(
        const Dataset& dataset,
        const Layer& layer_fixed,
        Layer& layer,
        Layer& new_layer,
        const GetNeighbors& get_neighbors);

    static int Crossings(const Dataset&, const Layer& layer_1, const Layer& layer_2);

    //--------------------------------------------------------------
    // Node Positioning
    //--------------------------------------------------------------
    static void FlagType1Conflicts(
        const Dataset&,
        const std::vector<Layer>& vertices_per_layer,
        Layer& layer_per_vertex,
        std::vector<std::pair<int, int>>& flags);

    static void VerticalAlignment(
        const Dataset&,
        const std::vector<Layer>& vertices_per_layer,
        Layer& layer_per_vertex,
        const Layer& pos_per_vertex,
        std::vector<int>& root,
        std::vector<int>& align,
        std::vector<std::pair<int, int>>& flags);

    static void HorizontalCompaction(
        const Dataset&,
        const std::vector<Layer>& vertices_per_layer,
        const Layer& layer_per_vertex,
        const Layer& pos_per_vertex,
        const std::vector<int>& root,
        const std::vector<int>& align,
        std::vector<float>& x_per_vertex,
        float delta);

    static void ReverseEdges(Dataset&, const std::vector<int>& reversed_edges);
    static void CreateEdges(
        Graph&,
        const std::vector<glm::vec3>& new_positions,
        const glm::vec2& _node_offset,
        bool _curved_edges = true);
};
//--------------------------------------------------------------
// Edge Bundling
//--------------------------------------------------------------
class EdgeBundlingLayout : public ILayout
{
    using CompatibilityFunction = std::function<float(const EdgePath&, const EdgePath&)>;
public:
    EdgeBundlingLayout() = default;
    // Inherited via Layout
    bool Gui(IStructure&) override;
    static void Apply(IStructure&, int C, int l, float K, int n, float s, float threshold, bool quadratic, bool check_owners, CompatibilityFunction f);

private:
    float m_K = 0.1f;          // Stiffnes
    int m_C = 3;        // Cycles
    int m_l = 30;       // Iterations
    int m_n = 1;        // Initial subdivisions
    float m_s = 0.004;   // Step size
    float m_comp_threshold = 0.05f; // Compatibility threshold
    bool m_quadratic = true;
    bool m_check_owners = true;
    
    //--------------------------------------------------------------
    // Compatibility
    //--------------------------------------------------------------
    static float Compatibility(const EdgePath&, const EdgePath&);
    static float AngleCompatibility(const EdgePath&, const EdgePath&);
    static float ScaleCompatibility(const EdgePath&, const EdgePath&);
    static float DistanceCompatibility(const EdgePath&, const EdgePath&);
    static float VisibilityCompatibility(const EdgePath&, const EdgePath&);
    static float Visibility(const EdgePath&, const EdgePath&);
};

//--------------------------------------------------------------
// Dimensionality Reduction
//--------------------------------------------------------------
class DRQualityMetrics
{
    public:
        DRQualityMetrics() = default;
        virtual ~DRQualityMetrics();
        static float NormalizedStress(IStructure&);
        static std::pair<std::vector<float>, std::vector<float>> ShepardPoints(IStructure&);
        static float Trustworthiness(IStructure&, int K);
        static float Continuity(IStructure&, int K);

    protected:
        void MetricGui();
        void ComputeMetrics(IStructure&);

    private:
        std::vector<std::pair<std::string, std::string>> m_metrics;
        float* m_shepard_xs;
        float* m_shepard_ys;
        int m_shepard_count;
};

class TSNELayout : public ILayout, public DRQualityMetrics
{
public:
    TSNELayout() = default;
    bool Gui(IStructure&) override;
    static void Apply(IStructure&, const int iterations, const int scale);

private:
    int m_iterations = 1000;
    int m_scale = 100;
};

class MDSLayout : public ILayout, public DRQualityMetrics
{
public:
    MDSLayout() = default;
    bool Gui(IStructure&) override;
    static void Apply(IStructure&, const int iterations, const int scale);

private:
    int m_iterations = 1000;
    int m_scale = 100;
};


} // namespace DataVis

#pragma once

namespace DataVis
{
class EdgePath final : public Animator
{
public:
    enum class Style { Line, Curve };

    EdgePath() = default;
    EdgePath(const Dataset::Kind&);
    void Draw() override;
    void SetEdgeIdx(EdgeIdx);
    void SetIsDirected(bool);
    void SetStyle(const Style&);
    void SetArrowOffset(float);
    void Clear();
    void AddPoint(const glm::vec3&);
    void SetStartCtrlPoint(const glm::vec3&);
    void SetEndCtrlPoint(const glm::vec3&);
    void UpdateStartPoint(const glm::vec3&);
    void UpdateEndPoint(const glm::vec3&);
    void UpdatePoint(const size_t index, const glm::vec3&);
    void ForceUpdate();

protected:
    void OnStopAnimation() override;
    void Interpolate(float percentage) override;

private:
    void UpdatePath();
    // Arrow can be updated after path has been build
    void UpdateArrow();
    void BuildPath();
    // Line path will be animated
    void BuildLinePath();
    // Cuve path will not be animated
    void BuildCurvePath();

    
    EdgeIdx m_edge_idx = UNINIT_IDX;
    ofPolyline m_path;
    std::vector<InterpolateValue3> m_points;
    glm::vec3 m_start_arrow, m_end_arrow;
    float m_arrow_offset = 10;
    InterpolateValue3 m_start_ctrl_point, m_end_ctrl_point;

    Style m_style = Style::Line;
    bool m_is_directed = false;
    bool m_update_path = false;
    bool m_start_ctrl_point_set = false;
    bool m_end_ctrl_point_set = false;
};

using VectorOfEdgePaths = std::vector<std::shared_ptr<EdgePath>>;
}

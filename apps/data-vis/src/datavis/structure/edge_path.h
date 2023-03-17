#pragma once

namespace DataVis
{
class EdgePath final : public Animator
{
public:
    EdgePath() = default;
    EdgePath(bool is_directed);
    void Init(EdgeIdx);
    void SetIsDirected(bool);
    void Draw() override;
    void AddPoint(const glm::vec3&);
    void UpdatePoint(const size_t index, const glm::vec3&);

protected:
    void OnStopAnimation() override;
    void Interpolate(float percentage) override;

private:
    EdgeIdx m_edge_idx;
    ofPolyline m_path;
    std::vector<glm::vec3> m_points;
    bool m_is_directed = false;
    bool m_update_arrow = false;
    glm::vec3 m_start_arrow, m_end_arrow;
};
using VectorOfEdgePaths = std::vector<std::shared_ptr<EdgePath>>;
}

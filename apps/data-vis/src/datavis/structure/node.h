#pragma once

namespace DataVis
{
    class Node : public Animator
    {
    public:
        Node(std::string vertex_id, VertexIdx vertex_index, const glm::vec3& position = glm::vec3(0));

        void Draw() override;
        [[nodiscard]] const std::string& GetVertexId() const;
        [[nodiscard]] VertexIdx GetVertexIdx() const;
        const glm::vec3& GetPosition();
        void SetPosition(const glm::vec3& position);
        const glm::vec3& GetNewPosition();
        void SetNewPosition(const glm::vec3& new_position);
        void SetDisplacement(glm::vec3 displacement);
        const glm::vec3& GetDisplacement();
        [[nodiscard]] const float& GetRadius() const;
        void SetRadius(float radius);
        [[nodiscard]] bool Inside(const glm::vec3& position) const;

        ofColor color = ofColor::black;
        std::vector<std::shared_ptr<Node>> neighbors;

    protected:
        void OnStopAnimation() override;
        void Interpolate(float percentage) override;

        std::string m_vertex_id;
        VertexIdx m_vertex_idx;

        glm::vec3 m_position = glm::vec3(0);
        glm::vec3 m_new_position = glm::vec3(0);
        glm::vec3 m_old_position = glm::vec3(0);
        glm::vec3 m_displacement = glm::vec3(0);
        ofRectangle m_aabb = {};
        float m_radius = 10;
    };
    
    using VectorOfNodes = std::vector<std::shared_ptr<Node>>;
}

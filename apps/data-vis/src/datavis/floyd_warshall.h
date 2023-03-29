#pragma once

namespace DataVis
{
    void FloydWarshall(Dataset& _dataset, qdtsne::NeighborList<int, float>& _D1, std::unique_ptr<smat::Matrix<float>>& _D2  )
    {
        const auto& vertices = _dataset.vertices;
        auto& edges = _dataset.edges;
        const size_t size = vertices.size();
        // Initialize D1
        _D1.clear();
        _D1.resize( size);
        for(auto& entry : _D1)
            entry.resize(size);
        
        // Initialize D2
        _D2 = std::make_unique<smat::Matrix<float>>(size, size, MAX_FLOAT);

        // Insert default values
        for(size_t i = 0; i < size; i++)
        {
            for(size_t j = 0; j < size; j++)
            {
                _D1[i][j].first = j;
                if(i == j)
                {
                    _D1[i][j].second = 0;
                    _D2->set(i, j, 0);
                }
                else
                    _D1[i][j].second = MAX_FLOAT;
            }
        }

        // Insert edge weights
        for(auto& edge: edges)
        {
            const float weight = edge.attributes.FindFloat("weight");
            _D1[edge.from_idx][edge.to_idx].second = weight;
            _D2->set(edge.from_idx, edge.to_idx, weight);
            if(_dataset.GetKind() == Dataset::Kind::Undirected)
            {
                _D1[edge.to_idx][edge.from_idx].second = weight;
                _D2->set(edge.to_idx, edge.from_idx, weight);
            }
        }

        // Calculate graph-theoretic distance
        for(size_t k = 0; k < size; k++)
        {
            for(size_t i = 0; i < size; i++)
                for(size_t j = 0; j < size; j++)
                {
                    const float D1_value = _D1[i][k].second + _D1[k][j].second;
                    if(_D1[i][j].second > D1_value)
                        _D1[i][j].second = D1_value;
                    const float D2_value = _D2->get(i, k) + _D2->get(k, j);
                    if(_D2->get(i, j) > D2_value)
                        _D2->set(i, j, D2_value);
                }
        }
    }
}

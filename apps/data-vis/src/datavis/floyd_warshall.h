#pragma once

namespace DataVis
{
/**
 * \brief Calculates the Calculate graph-theoretic distance to be used for dimensionality reduction algorithms.
 * \param _dataset
 * \param _D T-SNE datastructure
 */
inline void FloydWarshall(Dataset& _dataset, qdtsne::NeighborList<int, double>& _D)
{
	const auto& vertices = _dataset.vertices;
	auto& edges = _dataset.edges;
	const size_t size = vertices.size();
	// Initialize D1
	_D.clear();
	_D.resize(size);
	for (auto& entry : _D)
		entry.resize(size);

	// Insert default values
	for (size_t i = 0; i < size; i++)
	{
		for (size_t j = 0; j < size; j++)
		{
			_D[i][j].first = j;
			if (i == j)
				_D[i][j].second = 0;
			else
				_D[i][j].second = MAX_FLOAT;
		}
	}

	// Insert edge weights
	for (auto& edge : edges)
	{
		const float weight = edge.attributes.FindFloat("weight");
		_D[edge.from_idx][edge.to_idx].second = weight;
		if (_dataset.GetKind() == Dataset::Kind::Undirected)
		{
			_D[edge.to_idx][edge.from_idx].second = weight;
		}
	}

	// Calculate graph-theoretic distance
	for (size_t k = 0; k < size; k++)
	{
		for (size_t i = 0; i < size; i++)
			for (size_t j = 0; j < size; j++)
			{
				const float D_value = _D[i][k].second + _D[k][j].second;
				if (_D[i][j].second > D_value)
					_D[i][j].second = D_value;
			}
	}

	// Remove itself and sort in increasing distance
    for(size_t i = 0; i < size; i++)
    {
        auto& entry = _D[i];
        entry[i] = entry.back();
        entry.resize(size - 1);
        
        std::sort(entry.begin(), entry.end(), [](std::pair<int, double> lhs, std::pair<int, double> rhs ){
            return lhs.second < rhs.second;
        });
    }
}
/**
 * \brief Calculates the Calculate graph-theoretic distance to be used for dimensionality reduction algorithms.
 * \param _dataset
 * \param _D MDS datastructure
 */
void FloydWarshall(Dataset& _dataset, std::unique_ptr<smat::Matrix<double>>& _D)
{
	const auto& vertices = _dataset.vertices;
	auto& edges = _dataset.edges;
	const size_t size = vertices.size();

	// Initialize D
	_D = std::make_unique<smat::Matrix<double>>(size, size, MAX_FLOAT);

	// Insert default values
	for (size_t i = 0; i < size; i++)
	{
		_D->set(i, i, 0);
	}

	// Insert edge weights
	for (auto& edge : edges)
	{
		const float weight = edge.attributes.FindFloat("weight");
		_D->set(edge.from_idx, edge.to_idx, weight);
		if (_dataset.GetKind() == Dataset::Kind::Undirected)
		{
			_D->set(edge.to_idx, edge.from_idx, weight);
		}
	}

	// Calculate graph-theoretic distance
	for (size_t k = 0; k < size; k++)
	{
		for (size_t i = 0; i < size; i++)
			for (size_t j = 0; j < size; j++)
			{
				const float D_value = _D->get(i, k) + _D->get(k, j);
				if (_D->get(i, j) > D_value)
					_D->set(i, j, D_value);
			}
	}
}
}

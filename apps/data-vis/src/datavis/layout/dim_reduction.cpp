#include "precomp.h"

namespace DataVis
{
bool DimReductionLayout::Gui(IStructure& _structure)
{
    bool active = false;
    if (ImGui::TreeNode("Dim Reduction"))
    {
        if (ImGui::Button("Apply"))
        {
            Apply(_structure);
            active = true;
        }

        ImGui::TreePop();
        ImGui::Separator();
    }
    return active;
}

void DimReductionLayout::Apply(IStructure& _structure)
{
    qdtsne::NeighborList<int, float> D1;
    std::unique_ptr<smat::Matrix<float>> D2;
    FloydWarshall(*_structure.dataset, D1, D2);
}

} 

#pragma once 

namespace DataVis
{
class IStructureFactory
{
public:
    std::string name;
    virtual std::shared_ptr<IStructure> Create() = 0;
    virtual ~IStructureFactory() {};
};

} // namespace DataVis

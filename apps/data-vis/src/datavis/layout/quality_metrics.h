#pragma once

namespace DataVis
{
    class IQualityMetrics
    {
    public:
        virtual void MetricGui() = 0;
        virtual void ComputeMetrics( IStructure& ) = 0;
    protected:
        std::vector<std::pair<std::string, std::string>> m_metrics;

    };

    class GraphQualityMetrics : public IQualityMetrics
    {
    public:
        GraphQualityMetrics() = default;
        // Returns <number of crossings, minimum crossing angle>
        static std::pair<int, float> Crossings( IStructure& );
        static float CrossingResolution( IStructure& );
        static float Stress( IStructure& );
        void MetricGui() override;
        void ComputeMetrics( IStructure& ) override;
    };

    class DRQualityMetrics : public IQualityMetrics
    {
    public:
        DRQualityMetrics() = default;
        virtual ~DRQualityMetrics();
        static float NormalizedStress( IStructure& );
        static std::pair<std::vector<float>, std::vector<float>> ShepardPoints( IStructure& );
        static float Trustworthiness( IStructure&, int K );
        static float Continuity( IStructure&, int K );
        void MetricGui() override;
        void ComputeMetrics( IStructure& ) override;

    private:
        float* m_shepard_xs;
        float* m_shepard_ys;
        int m_shepard_count;
    };
}
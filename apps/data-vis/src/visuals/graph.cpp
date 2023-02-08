#include "precomp.h"

Graph::Graph() {
	for (size_t i = 0; i < 11; i++){
		bar_data[i] = 50 * i;
	}
	for (uint i = 0; i < 500; i++)
	{
		x_data[i] = 11 * i / (float)500;
		y_data[i] = sinf(2 * PI * i * 0.01f) * 100 + 200;
	}
}

void Graph::draw()
{
	if (ImGui::Begin("ImGui Window")) {
		ImGui::Text("Hello");
		ImGui::End();
	}

	if (ImGui::Begin("My Window")) {
		if (ImPlot::BeginPlot("My Plot")) {

			ImPlot::PlotBars("My Bar Plot", bar_data, 11);
			ImPlot::PlotLine("My Line Plot", x_data, y_data, 500);
			ImPlot::EndPlot();
		}
		ImGui::End();
	}
}

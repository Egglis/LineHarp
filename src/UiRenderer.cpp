#include "UiRenderer.h"

#include <imgui.h>
#include <tinyfiledialogs.h>

using namespace lineweaver;
using namespace gl;
using namespace glm;
using namespace globjects;

UiRenderer::UiRenderer() {

}

void UiRenderer::renderUi() {


}

void lineweaver::UiRenderer::setFocusId(int id)
{
	focusLineID = id;
}

bool UiRenderer::dataFile() {	
	fileMode = 0;
	ImGui::Combo("File Mode", &fileMode, "Trajectory\0Series\0");

	std::string oldDataFilename = dataFilename;

	if (ImGui::Button("Browse##1"))
	{
		const char* filterExtensions[] = { "*.csv" };
		const char* openfileName = tinyfd_openFileDialog("Open Data File", "./", 1, filterExtensions, "CSV Files (*.csv)", 0);

		if (openfileName) dataFilename = std::string(openfileName);
	}

	ImGui::SameLine();
	ImGui::InputTextWithHint("Data File", "Press button to load new file", (char*)dataFilename.c_str(), dataFilename.size(), ImGuiInputTextFlags_ReadOnly);

	return dataFilename != oldDataFilename;
}


bool UiRenderer::impFile() {
	std::string oldImportanceFilename = importanceFilename;

	if (ImGui::Button("Browse##2"))
	{
		const char* filterExtensions[] = { "*.csv" };
		const char* openfileName = tinyfd_openFileDialog("Open Data File", "./", 1, filterExtensions, "CSV Files (*.csv)", 0);

		if (openfileName)
			importanceFilename = std::string(openfileName);
	}

	ImGui::SameLine();
	ImGui::InputTextWithHint("Importance File", "Press button to load new file", (char*)importanceFilename.c_str(), importanceFilename.size(), ImGuiInputTextFlags_ReadOnly);

	return oldImportanceFilename != importanceFilename;
}

void UiRenderer::scaling() {

	// allow the user to arbitrarily scale both axes
	ImGui::SliderFloat("x-Axis Scale", &xAxisScaling, 0.1f, 10.0f);
	ImGui::SliderFloat("y-Axis Scale", &yAxisScaling, 0.1f, 10.0f);

	if (ImGui::Button("Reset"))
	{
		xAxisScaling = 1.0f;
		yAxisScaling = 1.0f;
	}

}


void UiRenderer::linePropreties() {
	if (ImGui::CollapsingHeader("Line Properties", ImGuiTreeNodeFlags_DefaultOpen))
	{

		ImGui::Combo("Color Mode", &coloringMode, "None\0Importance\0Depth\0Random\0");
		ImGui::SliderFloat("Line Width", &lineWidth, 1.0f, 128.0f);
		ImGui::SliderFloat("Smoothness", &smoothness, 0.0f, 1.0f);

		ImGui::Checkbox("Enable Line-Halos", &enableLineHalos);
	}
}

void lineweaver::UiRenderer::selectionSettings(Viewer* viewer)
{
	if(ImGui::CollapsingHeader("Selection Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Checkbox("Enable Focus-Line", &enableFocusLine);
		ImGui::SliderInt("Focus-Line", &focusLineID, 0, viewer->scene()->tableData()->m_numberOfTrajectories - 1);

		

		ImGui::Combo("Selection Mode", &selectionMode, "Single\0Importance\0Mid Point\0Min Dist\0Hausdorff Dist\0Frechet Dist");
		if(selectionMode != 0) {
			ImGui::SliderFloat("Selection Range", &selectionRange, 0.0f, 1.0f);
		}
		ImGui::Checkbox("Pull Background", &pullBackgorund);
	}

}

void UiRenderer::lensFeature(Viewer* viewer) {
	if (ImGui::CollapsingHeader("Lens Feature", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::SliderFloat("Lens Radius", &lensRadius, 0.0f, 1.0f);

		ImGui::Checkbox("Enable Focus-Lens", &enableLens);
		ImGui::Checkbox("Enable Angular-Brushing", &enableAngularBrush);
		ImGui::Checkbox("Enable Lens Depth", &enableLensDepth);

		ImGui::SliderFloat("Lens Depth", &viewer->m_lensDepthValue, 0.0f, 1.0f);
		lensDepthValue = viewer->getLensDepthValue();

		ImGui::SliderFloat("Lens Displacment ", &lensDisp, 0.0f, 1.0f);

		// If angual brushing then scroll wheel on angular brushing is prioratized
		if (!enableAngularBrush) {

			// Convert angle into 0 -> 1 range
			const float oldRange = (90.0f - (-90.0f));
			const float newRange = (1.0f - 0.0f);
			float newValue = ((viewer->m_scrollWheelAngle - (-90)) * newRange) / oldRange;
			lensDisp = newValue;
		}

		ImGui::SliderFloat("Brushing Angle", &brushingAngle, -90.0f, 90.0f);

		if (enableAngularBrush) {
			brushingAngle = viewer->m_scrollWheelAngle;
		}

	}
}

void UiRenderer::overplottingMeasurment(Viewer* viewer) {
	if (ImGui::CollapsingHeader("Overplotting Measurement", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::Button("Compute") || viewer->enforcedOverplottingComp())
		{
			calculateOverplottingIndex = true;
			displayOverplottingGUI = false;
		}

		if (displayOverplottingGUI) {

			// print result
			ImGui::Text("Overplotting index = %f", overplottingRatio);

			// print individual sub-results
			for (int i = 0; i < viewer->scene()->tableData()->m_numberOfTrajectories; i++) {
				ImGui::Text("Trajectory %i = %u out of %u", i, visiblePixelsPerTrajectory.at(i), totalPixelsPerTrajectory.at(i));
			}
		}
	}
}


void UiRenderer::animationSettings() {
	if (ImGui::BeginMenu("Animations")) {
		ImGui::SliderFloat("Global Animation Speed", &globalAnimationFactor, 0.0f, 5.0f);
		ImGui::SliderFloat("Fold Animation Speed", &foldAnimationSpeed, 0.0f, 5.0f);
		ImGui::SliderFloat("Pull Animation Speed", &pullAnimationSpeed, 0.0f, 5.0f);

		ImGui::Checkbox("Enable Moving Animation:", &movingAnimation);
		ImGui::EndMenu();
	}

}

std::string UiRenderer::generateDefines() {
	std::string defines = "";

	if (coloringMode == 1)
		defines += "#define IMPORTANCE_AS_OPACITY\n";
	else if (coloringMode == 2)
		defines += "#define DEPTH_LUMINANCE_COLOR\n";
	else if (coloringMode == 3)
		defines += "#define RANDOM_LINE_COLORS\n";

	if (enableFocusLine)
		defines += "#define FOCUS_LINE\n";

	if (enableLineHalos)
		defines += "#define LINE_HALOS\n";

	// Depricated now, as RS_LINKEDLIST is always used
	if (true /*LinkedListRendering* r = dynamic_cast<LinkedListRendering*>(renderingStrategy)*/)
		defines += "#define RS_LINKEDLIST\n";

	if (calculateOverplottingIndex)
		defines += "#define CALCULATE_OVERPLOTTING_INDEX\n";

	if (enableLens)
		defines += "#define LENS_FEATURE\n";

	if (enableAngularBrush)
		defines += "#define ANGULAR_BRUSHING\n";

	if (enableLensDepth)
		defines += "#define LENS_DEPTH\n";

	if (pullBackgorund)
		defines += "#define PULL_BACKGROUND\n";

	if (easeFunctionID == 0)
		defines += "#define EASE_LINEAR\n";
	else if (easeFunctionID == 1)
		defines += "#define EASE_IN_SINE\n";
	else if (easeFunctionID == 2)
		defines += "#define EASE_OUT_SINE\n";
	else if (easeFunctionID == 3)
		defines += "#define EASE_IN_OUT_SINE\n";
	else if (easeFunctionID == 4)
		defines += "#define EASE_IN_QUAD\n";
	else if (easeFunctionID == 5)
		defines += "#define EASE_OUT_QUAD\n";
	else if (easeFunctionID == 6)
		defines += "#define EASE_IN_OUT_QUAD\n";
	else if (easeFunctionID == 7)
		defines += "#define EASE_IN_CUBIC\n";
	else if (easeFunctionID == 8)
		defines += "#define EASE_OUT_CUBIC\n";
	else if (easeFunctionID == 9)
		defines += "#define EASE_IN_OUT_CUBIC\n";
	else if (easeFunctionID == 10)
		defines += "#define EASE_IN_QUART\n";
	else if (easeFunctionID == 11)
		defines += "#define EASE_OUT_QUART\n";
	else if (easeFunctionID == 12)
		defines += "#define EASE_IN_OUT_QUART\n";
	else if (easeFunctionID == 13)
		defines += "#define EASE_IN_QUINT\n";
	else if (easeFunctionID == 14)
		defines += "#define EASE_OUT_QUINT\n";
	else if (easeFunctionID == 15)
		defines += "#define EASE_IN_OUT_QUINT\n";
	else if (easeFunctionID == 16)
		defines += "#define EASE_IN_EXPO\n";
	else if (easeFunctionID == 17)
		defines += "#define EASE_OUT_EXPO\n";
	else if (easeFunctionID == 18)
		defines += "#define EASE_IN_OUT_EXPO\n";
	return defines;
}


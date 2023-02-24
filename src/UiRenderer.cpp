#include "UiRenderer.h"

using namespace lineweaver;
using namespace gl;
using namespace glm;
using namespace globjects;

UiRenderer::UiRenderer() {

}

void UiRenderer::setFocusId(int id)
{
	m_selectionSettings.focusLineId = id;
}

bool UiRenderer::dataFileGUI() {	
	ImGui::Combo("File Mode", &m_fileSettings.fileMode, "Trajectory\0Series\0");

	std::string oldDataFilename = m_fileSettings.dataFilename;

	if (ImGui::Button("Browse##1"))
	{
		const char* filterExtensions[] = { "*.csv" };
		const char* openfileName = tinyfd_openFileDialog("Open Data File", "./", 1, filterExtensions, "CSV Files (*.csv)", 0);

		if (openfileName) m_fileSettings.dataFilename = std::string(openfileName);
	}

	ImGui::SameLine();
	ImGui::InputTextWithHint("Data File", "Press button to load new file", (char*)m_fileSettings.dataFilename.c_str(), m_fileSettings.dataFilename.size(), ImGuiInputTextFlags_ReadOnly);

	return m_fileSettings.dataFilename != oldDataFilename;
}


bool UiRenderer::impFileGUI() {
	std::string oldImportanceFilename = m_fileSettings.importanceFilename;

	if (ImGui::Button("Browse##2"))
	{
		const char* filterExtensions[] = { "*.csv" };
		const char* openfileName = tinyfd_openFileDialog("Open Data File", "./", 1, filterExtensions, "CSV Files (*.csv)", 0);

		if (openfileName)
			m_fileSettings.importanceFilename = std::string(openfileName);
	}

	ImGui::SameLine();
	ImGui::InputTextWithHint("Importance File", "Press button to load new file",
		(char*)m_fileSettings.importanceFilename.c_str(), m_fileSettings.importanceFilename.size(), ImGuiInputTextFlags_ReadOnly);

	return oldImportanceFilename != m_fileSettings.importanceFilename;
}

void UiRenderer::scalingGUI() {

	// allow the user to arbitrarily scale both axes
	ImGui::SliderFloat("x-Axis Scale", &m_scalingSettings.xAxisScaling, 0.1f, 10.0f);
	ImGui::SliderFloat("y-Axis Scale", &m_scalingSettings.yAxisScaling, 0.1f, 10.0f);

	if (ImGui::Button("Reset"))
	{
		m_scalingSettings.xAxisScaling = 1.0f;
		m_scalingSettings.yAxisScaling = 1.0f;
	}

}


void UiRenderer::linePropretiesGUI() {
	if (ImGui::CollapsingHeader("Line Properties", ImGuiTreeNodeFlags_DefaultOpen))
	{

		ImGui::Combo("Color Mode", &m_lineSettings.coloringMode, "None\0Importance\0Depth\0Random\0");
		ImGui::SliderFloat("Line Width", &m_lineSettings.lineWidth, 1.0f, 128.0f);
		ImGui::SliderFloat("Smoothness", &m_lineSettings.smoothness, 0.0f, 1.0f);

		ImGui::Checkbox("Enable Line-Halos", &m_lineSettings.enableLineHalos);
	}
}

void lineweaver::UiRenderer::selectionGUI(Viewer* viewer)
{
	if(ImGui::CollapsingHeader("Selection Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Checkbox("Enable Focus-Line", &m_selectionSettings.enableFocusLine);
		ImGui::SliderInt("Focus-Line", &m_selectionSettings.focusLineId, 0, viewer->scene()->tableData()->m_numberOfTrajectories - 1);

		

		ImGui::Combo("Selection Mode", &m_selectionSettings.selectionMode, "Single\0Importance\0Mid Point\0Min Dist\0Hausdorff Dist\0Frechet Dist");
		if(m_selectionSettings.selectionMode != 0) {
			ImGui::SliderFloat("Selection Range", &m_selectionSettings.selectionRange, 0.0f, 1.0f);
		}
		ImGui::Checkbox("Pull Background", &m_selectionSettings.enablePullBackground);
	}

}

void UiRenderer::lensSettingsGUI(Viewer* viewer) {
	if (ImGui::CollapsingHeader("Lens Feature", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::SliderFloat("Lens Radius", &m_lensSettings.lensRadius, 0.0f, 1.0f);

		ImGui::Checkbox("Enable Focus-Lens", &m_lensSettings.enableLens);
		ImGui::Checkbox("Enable Angular-Brushing", &m_lensSettings.enableAngularBrush);
		ImGui::Checkbox("Enable Lens Depth", &m_lensSettings.enableLensDepth);

		if (m_lensSettings.enableLensDepth) {
			ImGui::SliderFloat("Lens Depth", &viewer->m_lensDepthValue, 0.0f, 1.0f);
			ImGui::SliderFloat("Lend Depth Scaling", &m_lensSettings.lensDepthScaling, 0.0f, 1.0f);
			m_lensSettings.lensDepthValue = viewer->getLensDepthValue();
		}


		ImGui::SliderFloat("Lens Displacment ", &m_lensSettings.lensDisp, 0.0f, 1.0f);

		// If angual brushing then scroll wheel on angular brushing is prioratized
		if (!m_lensSettings.enableAngularBrush) {

			// Convert angle into 0 -> 1 range
			const float oldRange = (90.0f - (-90.0f));
			const float newRange = (1.0f - 0.0f);
			float newValue = ((viewer->m_scrollWheelAngle - (-90)) * newRange) / oldRange;
			m_lensSettings.lensDisp = newValue;
		}

		ImGui::SliderFloat("Brushing Angle", &m_lensSettings.brushingAngle, -90.0f, 90.0f);

		if (m_lensSettings.enableAngularBrush) {
			m_lensSettings.brushingAngle = viewer->m_scrollWheelAngle;
		}

	}
}

void UiRenderer::overplottingMeasurmentGUI(Viewer* viewer) {
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


void UiRenderer::animationSettingsGUI() {
	if (ImGui::BeginMenu("Animations")) {
		ImGui::SliderFloat("Global Animation Speed", &m_animationSettings.globalAnimationFactor, 0.0f, 5.0f);
		ImGui::SliderFloat("Fold Animation Speed", &m_animationSettings.foldAnimationSpeed, 0.0f, 5.0f);
		ImGui::SliderFloat("Pull Animation Speed", &m_animationSettings.pullAnimationSpeed, 0.0f, 5.0f);

		ImGui::Checkbox("Enable Moving Animation:", &m_animationSettings.movingAnimation);
		ImGui::EndMenu();
	}

}

void UiRenderer::audioSettingsMenuGUI() {
	if (ImGui::BeginMenu("Audio Settings")) {
		ImGui::SliderFloat("Interval between each note:", &m_audioSettings.note_interval, 0.0, 1.0);
		ImGui::SliderFloat("Global volume", &m_audioSettings.volume, 0.0, 1.0);

		ImGui::Spacing();
		m_audioSettings.mute = m_audioSettings.volume <= 0.0f;

		ImGui::Combo("Audio Frequency Metric", &m_audioSettings.metric, "Importance\0Similarity");


		ImGui::Spacing();
		ImGui::Text("Current Audio Device: ");
		ImGui::SameLine();
		ImGui::Text(m_audioSettings.defaultDevice.c_str());

		ImGui::EndMenu();
	}

}

void UiRenderer::audioSettingsGUI() {
	if (ImGui::CollapsingHeader("Audio", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox("Mute Audio", &m_audioSettings.mute);
		ImGui::Checkbox("Enable Visual Audio Guide", &m_selectionSettings.enableVisualAudioGuide);

		std::string text;
		if (m_audioSettings.importance) {
			m_audioSettings.metric = 0;
			text = "Playback on importance";
		}
		else {
			m_audioSettings.metric = 1;
			m_selectionSettings.enableFocusLine = true;
			text = "Playback on similarity";
		}
		ImGui::Checkbox(text.c_str(), &m_audioSettings.importance);
	}

}

void lineweaver::UiRenderer::keybindingsInfoGUI()
{
	if (ImGui::BeginMenu("Info & Keybindings")) {

		ImGui::Text("Displacment:");
		ImGui::Text("Mouse Scroll Wheel - Lens Displacment / Angular brush angle");
		ImGui::Text("Shift + W or S - Hold to change lens radius");
		ImGui::Text("Ctrl + W or S - Hold to change lens depth up or down");

		ImGui::Spacing();

		ImGui::Text("Lens:");
		ImGui::Text("F - Fold animation / Close the displacment of the lens");
		ImGui::Text("P - Pull animation / Pull out the selected lines");

		ImGui::Spacing();

		ImGui::Text("Audio:");
		ImGui::Text("P - Play audio for selected lines when they are within the lens");
		ImGui::Text("P - While similarity audio feedback mode is enabled, only play selected notes");
		ImGui::Text("G - Play every line within the the lens in importance order");
		ImGui::Text("M - Change the audiofeedback mode between: Importance and Similarity");

		ImGui::EndMenu();

	}

}



std::string UiRenderer::generateDefines() {
	std::string defines = "";

	if (m_lineSettings.coloringMode == 1)
		defines += "#define IMPORTANCE_AS_OPACITY\n";
	else if (m_lineSettings.coloringMode == 2)
		defines += "#define DEPTH_LUMINANCE_COLOR\n";
	else if (m_lineSettings.coloringMode == 3)
		defines += "#define RANDOM_LINE_COLORS\n";

	if (m_selectionSettings.enableFocusLine)
		defines += "#define FOCUS_LINE\n";

	if (m_lineSettings.enableLineHalos)
		defines += "#define LINE_HALOS\n";

	if (m_selectionSettings.enableVisualAudioGuide)
		defines += "#define AUDIO_GUIDE\n";

	// Depricated now, as RS_LINKEDLIST is always used
	if (true /*LinkedListRendering* r = dynamic_cast<LinkedListRendering*>(renderingStrategy)*/)
		defines += "#define RS_LINKEDLIST\n";

	if (calculateOverplottingIndex)
		defines += "#define CALCULATE_OVERPLOTTING_INDEX\n";

	if (m_lensSettings.enableLens)
		defines += "#define LENS_FEATURE\n";

	if (m_lensSettings.enableAngularBrush)
		defines += "#define ANGULAR_BRUSHING\n";

	if (m_lensSettings.enableLensDepth)
		defines += "#define LENS_DEPTH\n";

	if (m_selectionSettings.enablePullBackground)
		defines += "#define PULL_BACKGROUND\n";

	if (m_lensSettings.enableBinaryLensDepth)
		defines += "#define BINARY_LENS_DEPTH\n";

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


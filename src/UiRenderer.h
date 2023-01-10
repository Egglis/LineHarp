#pragma once

#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <imgui.h>
#include <tinyfiledialogs.h>
#include <iostream>
#include <iomanip>

#include "Viewer.h"
#include "Audio/NoteMap.h"

#include "RenderingStrategies/RenderingStrategy.h"
#include "RenderingStrategies/LinkedListRendering.h"

namespace Settings {

	struct Animation
	{
		float globalAnimationFactor = 1.0f;
		float foldAnimationSpeed = 1.0f;
		float pullAnimationSpeed = 1.0f;
		bool movingAnimation = true;
	};

	struct Audio
	{
		int min_note = 0;
		int max_note = 71;

		float note_interval = 0.2f;
		float volume = 0.2f;

		bool mute = false;
		bool enableNotesWhileClicking = true;
		bool reset = false;

		int playingMode = 0;
		int metric = 0;
	};

	struct File {
		int fileMode = 0;
		std::string dataFilename;
		std::string importanceFilename;
	};

	struct Scaling
	{
		float xAxisScaling = 1.0f;
		float yAxisScaling = 1.0f;
	};

	struct Line {
		int coloringMode = 0; // 0-None, 1-Importance, 2-Depth, 3-Random
		float lineWidth = 16.0f;
		float smoothness = (1.0f / 3.0f); // weight used for soft depth compositing
		bool enableLineHalos = true;
	};

	struct Selection
	{
		// TODO
		bool TODO = true;
	};
	struct Lens {
		// TODO
		bool TODO = true;
	};
	struct OverPlotting {
		//TODO
		bool TODO = true;
	};

}



namespace lineweaver
{
	class UiRenderer {
	public:
		UiRenderer();

		void setFocusId(int id);
		void setNoteMap(gam::NoteMap* noteMap) { m_noteMap = noteMap; };

		// Animation Settings
		void animationSettingsGUI();
		Settings::Animation* Animation() { return &m_animationSettings; };

		// Audio Setttings
		void audioSettingsGUI();
		Settings::Audio* Audio() { return &m_audioSettings; };


		// Collapsing headers for Line Graph: ------------
		// File Settings
		bool dataFileGUI();
		bool impFileGUI();
		Settings::File* File() { return &m_fileSettings; };
		
		// Scaling Settings
		void scalingGUI();
		Settings::Scaling* Scaling() { return &m_scalingSettings; };

		// Line Settings
		void linePropretiesGUI();
		Settings::Line* Line() { return &m_lineSettings; };

		void selectionGUI(Viewer* viewer);
		void lensSettingsGUI(Viewer* viewer);
		void overplottingMeasurmentGUI(Viewer* viewer);



		// Generates the defines.glsl file
		std::string generateDefines();

		// GUI variables ----------------------------------------------------------------------------

		
		// allow highlighting a single trajectory
		bool enableFocusLine = false;
		int focusLineID = 0;

		// Selection:
		int selectionMode = 0; // 0-SingleSelection, 1-Importance, 2-Similarity, 3-Distance
		float selectionRange = 0.1f;
		bool pullBackgorund = false;

		// provide modulation of importance
		int easeFunctionID = 0;

		// since overplotting measuring reduced performance, it is triggered by a button
		bool calculateOverplottingIndex = false;
		bool displayOverplottingGUI = false;

		// support focus lense feature
		bool enableLens = false;
		float lensRadius = 0.15f;

		glm::vec2 lensPosition = glm::vec2(0, 0);
		glm::vec2 delayedLensPosition = glm::vec2(0, 0);

		// support for angular brush
		bool enableAngularBrush = false;
		float lensDisp = 0.0f;
		float brushingAngle = 0.0f;

		// Overplotting
		std::vector<unsigned int> totalPixelsPerTrajectory;
		std::vector<unsigned int> visiblePixelsPerTrajectory;
		double overplottingRatio = 0.0;

		// Lens Depth Mode
		float lensDepthValue = 1.0f;
		float lensDepthScaling = 0.5f;
		bool enableLensDepth = false;
		bool binaryLensDepth = false;

		bool dispAction = false;

	private:
		Settings::Animation m_animationSettings;
		Settings::Audio m_audioSettings;
		Settings::File m_fileSettings;
		Settings::Scaling m_scalingSettings;
		Settings::Line m_lineSettings;

		gam::NoteMap* m_noteMap;
	};


}



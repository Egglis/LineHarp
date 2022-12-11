#pragma once

#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include "Viewer.h"

#include "RenderingStrategies/RenderingStrategy.h"
#include "RenderingStrategies/LinkedListRendering.h"

namespace lineweaver
{
	class UiRenderer {
	public:
		UiRenderer();
		void renderUi(); // TODO remove

		void setFocusId(int id);
		
		// Animation Settings
		void animationSettings();

		// Collapsing headers for Line Graph:
		bool dataFile();
		bool impFile();
		void scaling();
		void linePropreties();
		void selectionSettings(Viewer* viewer);
		void lensFeature(Viewer* viewer);
		void overplottingMeasurment(Viewer* viewer);

		// Generates the defines.glsl file
		std::string generateDefines();


		int fileMode = 0;

		// GUI variables ----------------------------------------------------------------------------

		// Supported redner modes
		int coloringMode = 0;		// 0-None, 1-Importance, 2-Depth, 3-Random

		// allow the user to arbitrarily scale both axes
		float xAxisScaling = 1.0f;
		float yAxisScaling = 1.0f;

		// store combo ID of selected file
		std::string dataFilename;
		std::string importanceFilename;

		// allow highlighting a single trajectory
		bool enableFocusLine = false;
		int focusLineID = 0;

		// Selection:
		int selectionMode = 0; // 0-SingleSelection, 1-Importance, 2-Similarity, 3-Distance
		float selectionRange = 0.1f;
		bool pullBackgorund = false;

		// add support for line halos
		bool enableLineHalos = true;

		// Line Parameters
		float lineWidth = 16.0f;
		float smoothness = (1.0f / 3.0f);		// weight used for soft depth compositing

		// provide modulation of importance
		int easeFunctionID = 0;

		// since overplotting measuring reduced performance, it is triggered by a button
		bool calculateOverplottingIndex = false;
		bool displayOverplottingGUI = false;

		// support focus lense feature
		bool enableLens = false;
		float lensRadius = 0.15f;

		glm::vec2 lensPosition;
		glm::vec2 delayedLensPosition;

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
		bool enableLensDepth = false;

		// Animation
		float globalAnimationFactor = 1.0f;
		float foldAnimationSpeed = 1.0f;
		float pullAnimationSpeed = 1.0f;

		bool dispAction = false;

		bool movingAnimation = true;

		
	};


}



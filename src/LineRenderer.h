#pragma once
#include "Renderer.h"

// currently supported rendering strategies
#include "RenderingStrategies/RenderingStrategy.h"
#include "RenderingStrategies/LinkedListRendering.h"

#include <memory>

#include <glm/glm.hpp>
#include <glbinding/gl/gl.h>
#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>

#include <globjects/VertexArray.h>
#include <globjects/VertexAttributeBinding.h>
#include <globjects/Buffer.h>
#include <globjects/Program.h>
#include <globjects/Shader.h>
#include <globjects/Framebuffer.h>
#include <globjects/Renderbuffer.h>
#include <globjects/Texture.h>
#include <globjects/base/File.h>
#include <globjects/TextureHandle.h>
#include <globjects/NamedString.h>
#include <globjects/base/StaticStringSource.h>
#include <globjects/Query.h>

namespace lineweaver
{
	class Viewer;

	class LineRenderer : public Renderer
	{
	public:
		LineRenderer(Viewer *viewer);
		virtual void display();
		void handleAction(bool start);
	private:
		
		LinkedListRendering* renderingStrategy = NULL;

		std::unique_ptr<globjects::VertexArray> m_vao = std::make_unique<globjects::VertexArray>();
		std::unique_ptr<globjects::Buffer> m_xColumnBuffer = std::make_unique<globjects::Buffer>();
		std::unique_ptr<globjects::Buffer> m_yColumnBuffer = std::make_unique<globjects::Buffer>();
		std::unique_ptr<globjects::Buffer> m_importanceColumnBuffer = std::make_unique<globjects::Buffer>();
		
		std::unique_ptr<globjects::VertexArray> m_vaoQuad = std::make_unique<globjects::VertexArray>();
		std::unique_ptr<globjects::Buffer> m_verticesQuad = std::make_unique<globjects::Buffer>();
		

		std::unique_ptr<globjects::StaticStringSource> m_shaderSourceDefines = nullptr;
		std::unique_ptr<globjects::NamedString> m_shaderDefines = nullptr;

		//SSBO
		std::unique_ptr<globjects::Buffer> m_intersectionBuffer = std::make_unique<globjects::Buffer>();

		//SSBO for overplotting measure implementation
		std::unique_ptr<globjects::Buffer> m_totalPixelBuffer = std::make_unique<globjects::Buffer>();
		std::unique_ptr<globjects::Buffer> m_visiblePixelBuffer = std::make_unique<globjects::Buffer>();
		

		std::unique_ptr<globjects::Texture> m_lineChartTexture = nullptr;
		std::unique_ptr<globjects::Texture> m_depthTexture = nullptr;
		std::unique_ptr<globjects::Texture> m_offsetTexture = nullptr;
		std::unique_ptr<globjects::Texture> m_blurTexture[2] = { nullptr, nullptr };

		glm::ivec2 m_framebufferSize;

		std::unique_ptr<globjects::Framebuffer> m_blurFramebuffer = nullptr;
		std::unique_ptr<globjects::Framebuffer> m_lineFramebuffer = nullptr;

		// GUI variables ----------------------------------------------------------------------------

		// supported render modes
		int m_coloringMode = 0;			// 0-None, 1-Importance, 2-Depth, 3-Random

		// allow the user to arbitrarily scale both axes
		float m_xAxisScaling = 1.0f;
		float m_yAxisScaling = 1.0f;

		// store combo ID of selected file
		std::string m_dataFilename;
		std::string m_importanceFilename;
		
		// allow highlighting a single trajectory
		bool m_enableFocusLine = false;
		int m_focusLineID = 0;

		// add support for line halos
		bool m_enableLineHalos = true;

		// Line Parameters
		float m_lineWidth = 16.0f;
		float m_smoothness = (1.0f/3.0f);		// weight used for soft depth compositing

		// provide modulation of importance
		int m_easeFunctionID = 0;

		// since overplotting measuring reduced performance, it is triggered by a button
		bool m_calculateOverplottingIndex = false;
		bool m_displayOverplottingGUI = false;

		// support focus lense feature
		bool m_enableLens = false;
		float m_lensRadius = 0.15f;

		glm::vec2 m_lensPosition;
		glm::vec2 m_delayedLensPosition;

		// support for angular brush
		bool m_enableAngularBrush = false;
		float m_lensDisp= 0.0f;
		float m_brushingAngle = 0.0f;

		std::vector<unsigned int> m_totalPixelsPerTrajectory;
		std::vector<unsigned int> m_visiblePixelsPerTrajectory;
		double m_overplottingRatio = 0.0;

		float m_actionStart = 0.0f;
		float m_actionEnd = 1.0f;
		double m_prevTime = NULL;
		bool m_action = false;
		float m_time = 0.0;
		float m_lensDepthValue = 1.0f;
		bool m_enableLensDepth = false;

		// ------------------------------------------------------------------------------------------
	};

}
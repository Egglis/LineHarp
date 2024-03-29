#include "LineRenderer.h"
#include <globjects/base/File.h>
#include <globjects/State.h>
#include <iostream>
#include <filesystem>
#include <algorithm>

#include <imgui.h>
#include "Viewer.h"
#include "Scene.h"
#include "CSV/TableData.h"
#include <sstream>
#include <tinyfiledialogs.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#endif

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <chrono>


using namespace lineweaver;
using namespace gl;
using namespace glm;
using namespace globjects;

LineRenderer::LineRenderer(Viewer* viewer) : Renderer(viewer), m_uiRenderer(), m_AudioPlayer(&m_uiRenderer)
{
	m_AudioPlayer.start();

	Shader::hintIncludeImplementation(Shader::IncludeImplementation::Fallback);

	m_intersectionBuffer->setStorage(sizeof(vec3) * 2560 * 1440 * 128 + sizeof(uint), nullptr, gl::GL_NONE_BIT);

	m_verticesQuad->setStorage(std::array<vec3, 1>({ vec3(0.0f, 0.0f, 0.0f) }), gl::GL_NONE_BIT);
	auto vertexBindingQuad = m_vaoQuad->binding(0);
	vertexBindingQuad->setBuffer(m_verticesQuad.get(), 0, sizeof(vec3));
	vertexBindingQuad->setFormat(3, GL_FLOAT);
	m_vaoQuad->enable(0);
	m_vaoQuad->unbind();

	m_shaderSourceDefines = StaticStringSource::create("");
	m_shaderDefines = NamedString::create("/defines.glsl", m_shaderSourceDefines.get());


	createShaderProgram("line", {
	{ GL_VERTEX_SHADER,"./res/line/line-vs.glsl" },
	{ GL_TESS_CONTROL_SHADER, "./res/line/line-tcs.glsl"},
	{ GL_TESS_EVALUATION_SHADER, "./res/line/line-tes.glsl"},
	{ GL_GEOMETRY_SHADER,"./res/line/line-gs.glsl" },
	{ GL_FRAGMENT_SHADER,"./res/line/line-fs.glsl" },
		},
	{
		"./res/line/globals.glsl",
		"./res/line/lens.glsl"
	});

	createShaderProgram("blur", {
		{ GL_VERTEX_SHADER,"./res/line/image-vs.glsl" },
		{ GL_GEOMETRY_SHADER,"./res/line/image-gs.glsl" },
		{ GL_FRAGMENT_SHADER,"./res/line/blur-fs.glsl" }
		});

	createShaderProgram("blend", {
		{ GL_VERTEX_SHADER,"./res/line/image-vs.glsl" },
		{ GL_GEOMETRY_SHADER,"./res/line/image-gs.glsl" },
		{ GL_FRAGMENT_SHADER,"./res/line/blend-fs.glsl" }
		});


	m_framebufferSize = viewer->viewportSize();

	m_lineChartTexture = Texture::create(GL_TEXTURE_2D);
	m_lineChartTexture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_lineChartTexture->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_lineChartTexture->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	m_lineChartTexture->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	m_lineChartTexture->image2D(0, GL_RGBA32F, m_framebufferSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	m_depthTexture = Texture::create(GL_TEXTURE_2D);
	m_depthTexture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_depthTexture->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_depthTexture->setParameter(GL_TEXTURE_BORDER_COLOR, vec4(1.0, 1.0, 1.0, 1.0));
	m_depthTexture->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	m_depthTexture->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	m_depthTexture->image2D(0, GL_DEPTH_COMPONENT, m_framebufferSize, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);

	for (int i = 0; i < 2; i++)
	{
		m_blurTexture[i] = Texture::create(GL_TEXTURE_2D);
		m_blurTexture[i]->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		m_blurTexture[i]->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		m_blurTexture[i]->setParameter(GL_TEXTURE_BORDER_COLOR, vec4(1.0, 1.0, 1.0, 1.0));
		m_blurTexture[i]->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		m_blurTexture[i]->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		m_blurTexture[i]->image2D(0, GL_RGBA, m_framebufferSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	}

	m_offsetTexture = Texture::create(GL_TEXTURE_2D);
	m_offsetTexture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_offsetTexture->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_offsetTexture->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	m_offsetTexture->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	m_offsetTexture->image2D(0, GL_R32UI, m_framebufferSize, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);


	m_segmentAngleTexture = Texture::create(GL_TEXTURE_2D);
	m_segmentAngleTexture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_segmentAngleTexture->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_segmentAngleTexture->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	m_segmentAngleTexture->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	m_segmentAngleTexture->image2D(0, GL_RGBA32F, m_framebufferSize, 0, GL_RGBA, GL_UNSIGNED_INT, nullptr);

	m_blurFramebuffer = Framebuffer::create();
	m_blurFramebuffer->attachTexture(GL_COLOR_ATTACHMENT0, m_blurTexture[0].get());
	m_blurFramebuffer->attachTexture(GL_COLOR_ATTACHMENT1, m_blurTexture[1].get());
	m_blurFramebuffer->attachTexture(GL_DEPTH_ATTACHMENT, m_depthTexture.get());
	m_blurFramebuffer->setDrawBuffers({ GL_COLOR_ATTACHMENT0 });


	m_lineFramebuffer = Framebuffer::create();
	m_lineFramebuffer->attachTexture(GL_COLOR_ATTACHMENT0, m_lineChartTexture.get());
	m_lineFramebuffer->attachTexture(GL_DEPTH_ATTACHMENT, m_depthTexture.get());
	m_lineFramebuffer->attachTexture(GL_COLOR_ATTACHMENT2, m_segmentAngleTexture.get());
	m_lineFramebuffer->setDrawBuffers({ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 });
	

}

void LineRenderer::display()
{
	

	auto currentState = State::currentState();

	if (viewer()->viewportSize() != m_framebufferSize)
	{
		m_framebufferSize = viewer()->viewportSize();

		m_lineChartTexture->image2D(0, GL_RGBA32F, m_framebufferSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		m_depthTexture->image2D(0, GL_DEPTH_COMPONENT, m_framebufferSize, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
		m_offsetTexture->image2D(0, GL_R32UI, m_framebufferSize, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);
		m_segmentAngleTexture->image2D(0, GL_RGBA32F, m_framebufferSize, 0, GL_RGBA, GL_UNSIGNED_INT, nullptr);

		for (int i = 0; i < 2; i++)
			m_blurTexture[i]->image2D(0, GL_RGBA, m_framebufferSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	}

	// retrieve/compute all necessary matrices and related properties
	const mat4 viewMatrix = viewer()->viewTransform();
	const mat4 inverseViewMatrix = inverse(viewMatrix);
	const mat4 modelViewMatrix = viewer()->modelViewTransform();
	const mat4 inverseModelViewMatrix = inverse(modelViewMatrix);
	const mat4 modelLightMatrix = viewer()->modelLightTransform();
	const mat4 inverseModelLightMatrix = inverse(modelLightMatrix);
	const mat4 modelViewProjectionMatrix = viewer()->modelViewProjectionTransform();
	const mat4 inverseModelViewProjectionMatrix = inverse(modelViewProjectionMatrix);
	const mat4 projectionMatrix = viewer()->projectionTransform();
	const mat4 inverseProjectionMatrix = inverse(projectionMatrix);
	const mat3 normalMatrix = mat3(transpose(inverseModelViewMatrix));
	const mat3 inverseNormalMatrix = inverse(normalMatrix);
	const ivec2 viewportSize = viewer()->viewportSize();

	/*
	globjects::debug() << inverseModelViewProjectionMatrix[0][0] << "," << inverseModelViewProjectionMatrix[0][1] << "," << inverseModelViewProjectionMatrix[0][2] << std::endl;
	globjects::debug() << inverseModelViewProjectionMatrix[1][0] << "," << inverseModelViewProjectionMatrix[1][1] << "," << inverseModelViewProjectionMatrix[1][2] << std::endl;
	globjects::debug() << inverseModelViewProjectionMatrix[2][0] << "," << inverseModelViewProjectionMatrix[2][1] << "," << inverseModelViewProjectionMatrix[2][2] << std::endl;
	globjects::debug() << "---" << std::endl;
	*/

	auto programLine = shaderProgram("line");
	auto programBlur = shaderProgram("blur");
	auto programBlend = shaderProgram("blend");

	vec4 worldLightPosition = inverseModelLightMatrix * vec4(0.0f, 0.0f, 0.0f, 1.0f);

	vec4 projectionInfo(float(-2.0 / (viewportSize.x * projectionMatrix[0][0])),
		float(-2.0 / (viewportSize.y * projectionMatrix[1][1])),
		float((1.0 - (double)projectionMatrix[0][2]) / projectionMatrix[0][0]),
		float((1.0 + (double)projectionMatrix[1][2]) / projectionMatrix[1][1]));

	float projectionScale = float(viewportSize.y) / fabs(2.0f / projectionMatrix[1][1]);

	vec4 nearPlane = inverseProjectionMatrix * vec4(0.0, 0.0, -1.0, 1.0);
	nearPlane /= nearPlane.w;

	// boolean variables used to automatically update data and importance
	static bool dataChanged = false;
	static bool importanceChanged = false;

	m_uiRenderer.animationSettingsGUI();

	if (ImGui::BeginMenu("Audio Settings")) {
		m_uiRenderer.audioSettingsMenuGUI();

		
		static std::list<float> frameratesList;
		frameratesList.push_back(m_AudioPlayer.mSound);

		while (frameratesList.size() > 128)
			frameratesList.pop_front();

		static float framerates[128];
		int i = 0;
		for (auto v : frameratesList)
			framerates[i++] = v;


		std::string s = "";

		// ImGui::Begin("Information");
		//ImGui::SameLine( - 220.0f);
		ImGui::PlotLines(s.c_str(), framerates, frameratesList.size(), 0, 0, -0.3f, 0.3f, ImVec2(ImGui::CalcItemWidth(), 128.0f));
		// ImGui::End();
		
		ImGui::EndMenu();
	}

	m_uiRenderer.keybindingsInfoGUI();

	ImGui::Begin("Importance Driven Dense Line Graphs");

	if (ImGui::CollapsingHeader("CSV-Files", ImGuiTreeNodeFlags_DefaultOpen)) {
		bool fileChanged = m_uiRenderer.dataFileGUI();

		if (fileChanged || viewer()->enforcedDataRefresh()) {

			// initialize data table
			if (m_uiRenderer.File()->fileMode == 0) {
				viewer()->scene()->tableData()->load(m_uiRenderer.File()->dataFilename);
			}
			else {
				if (m_uiRenderer.File()->dataFilename.find("AndrewsPlot") != std::string::npos) {

					// if data set is an andrews plot (currently indicated by name) 
					// first, load data without duplicating first and last entry
					viewer()->scene()->tableData()->loadAndrewsSeries(m_uiRenderer.File()->dataFilename);

					// second, perform andrews transformation
					viewer()->scene()->tableData()->andrewsTransform(96);

				}
				else {

					// otherwise, load series duplicating first and last element as usual
					viewer()->scene()->tableData()->loadSeries(m_uiRenderer.File()->dataFilename);
				}

			}

			// assign default rendering strategy 
			renderingStrategy = new LinkedListRendering(viewer()->scene()->tableData());

			// load data and apply implicite importance metric
			renderingStrategy->prepareDataBuffers();

			if (m_uiRenderer.File()->fileMode == 0)
				renderingStrategy->prepareImportanceBuffer();
			else
				renderingStrategy->weaveSeries(*viewer()->scene()->tableData());


			if (renderingStrategy != nullptr)
			{
				// reset importance to default if file was updated otherwise keep it
				if (fileChanged)
					importanceChanged = true;

				// update status
				dataChanged = true;
				m_uiRenderer.displayOverplottingGUI = false;
			}
		}
		fileChanged = m_uiRenderer.impFileGUI();

		if ((fileChanged || viewer()->enforcedImportanceRefresh()) && renderingStrategy != NULL)
		{
			// initialize importance table
			viewer()->scene()->tableImportance()->load(m_uiRenderer.File()->importanceFilename);

			// load external importance data
			renderingStrategy->prepareImportanceBuffer(viewer()->scene()->tableImportance());

			importanceChanged = true;
			m_uiRenderer.displayOverplottingGUI = false;
		}

		if (dataChanged)
		{
			// update VBOs for both columns
			m_xColumnBuffer->setData(renderingStrategy->activeXColumn(), GL_STATIC_DRAW);
			m_yColumnBuffer->setData(renderingStrategy->activeYColumn(), GL_STATIC_DRAW);

			auto vertexBinding = m_vao->binding(0);
			vertexBinding->setAttribute(0);
			vertexBinding->setBuffer(m_xColumnBuffer.get(), 0, sizeof(float));
			vertexBinding->setFormat(1, GL_FLOAT);
			m_vao->enable(0);

			vertexBinding = m_vao->binding(1);
			vertexBinding->setAttribute(1);
			vertexBinding->setBuffer(m_yColumnBuffer.get(), 0, sizeof(float));
			vertexBinding->setFormat(1, GL_FLOAT);
			m_vao->enable(1);

			// Scaling the model's bounding box to the canonical view volume
			vec3 boundingBoxSize = viewer()->scene()->tableData()->maximumBounds() - viewer()->scene()->tableData()->minimumBounds();
			float maximumSize = std::max({ boundingBoxSize.x, boundingBoxSize.y, boundingBoxSize.z });
			mat4 modelTransform = scale(vec3(2.0f) / vec3(1.25f * boundingBoxSize.x, 1.25f * boundingBoxSize.y, 1.0));
			modelTransform = modelTransform * translate(-0.5f * (viewer()->scene()->tableData()->minimumBounds() + viewer()->scene()->tableData()->maximumBounds()));
			viewer()->setModelTransform(modelTransform);

			// store diameter of current line chart and initialize light position
			viewer()->m_lineChartDiameter = sqrt(pow(boundingBoxSize.x, 2) + pow(boundingBoxSize.y, 2));

			// initial position of the light source (azimuth 120 degrees, elevation 45 degrees, 5 times the distance to the object in center) ---------------------------------------------------------------------------------------------------------
			glm::mat4 viewTransform = viewer()->viewTransform();
			glm::vec3 initLightDir = normalize(glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(120.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
			glm::mat4 newLightTransform = glm::inverse(viewTransform) * glm::translate(mat4(1.0f), (5 * viewer()->m_lineChartDiameter * initLightDir)) * viewTransform;
			viewer()->setLightTransform(newLightTransform);
			//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

			// reset values
			m_uiRenderer.Selection()->focusLineId = 0;

			// update status
			dataChanged = false;
			initLensPosition = true;

		}

		if (importanceChanged)
		{
			if (renderingStrategy->activeImportance().empty() == false) {
				m_importanceColumnBuffer->setData(renderingStrategy->activeImportance(), GL_STATIC_DRAW);

				auto vertexBinding = m_vao->binding(2);
				vertexBinding->setAttribute(2);
				vertexBinding->setBuffer(m_importanceColumnBuffer.get(), 0, sizeof(float));
				vertexBinding->setFormat(1, GL_FLOAT);
				m_vao->enable(2);

			}

			// update status
			importanceChanged = false;
			initLensPosition = true;
		}

		ImGui::Combo("Ease Function", &m_uiRenderer.easeFunctionID, "Linear\0In Sine\0Out Sine\0In Out Sine\0In Quad\0Out Quad\0In Out Quad\0In Cubic\0Out Cubic\0In Out Cubic\0In Quart\0Out Quart\0In Out Quart\0In Quint\0Out Quint\0In Out Quitn\0In Expo\0Out Expo\0In Out Expo\0");
	}

	m_uiRenderer.scalingGUI();
	m_uiRenderer.linePropretiesGUI();
	m_uiRenderer.selectionGUI(viewer());
	m_uiRenderer.audioSettingsGUI();
	m_uiRenderer.lensSettingsGUI(viewer());
	m_uiRenderer.overplottingMeasurmentGUI(viewer());

	ImGui::End();


	if (!m_dispAction) {
		if (m_uiRenderer.Lens()->lensDisp != m_previousLensDisp) {
			m_dispAction = true;
			m_time = 0.0f;
		}
	}

	m_previousLensDisp = m_uiRenderer.Lens()->lensDisp;

	// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// For custome fixed speed mouse movement:
	// 500, 45
	const float r = 200;
	const glm::vec2 start = vec2(853, 950);
	const glm::vec2 end = vec2(853, 950);
	const float total_time = 5.0;
	// 548, 513
	// 
	// Perfromance Measures 
	// vec2(853, 50);
	// vec2(853, 950);
	// 
	// 
	// Iris:
	// vec2(1250, 50);
	// vec2(1250, 950);
	// 
	// Figure: X = (853, 50) -> (853, 950), Duration = 5sec

	if (mAudioMode != GLOBAL) {
		double mouseX, mouseY;
		glfwGetCursorPos(viewer()->window(), &mouseX, &mouseY);

		// Audio Testing
		if (viewer()->m_audioTest) {
			// mouseY = glm::mix(0, 180, m_audioTestTimer / total_time);
			mouseX = glm::mix(start.x, end.x, m_audioTestTimer / total_time);
			mouseY = glm::mix(start.y, end.y, m_audioTestTimer / total_time);
		}

		glm::vec2 tempLensPosition = m_lensPosition;
		m_lensPosition = vec2(2.0f * float(mouseX) / float(viewportSize.x) - 1.0f, -2.0f * float(mouseY) / float(viewportSize.y) + 1.0f);

		if (initLensPosition || !m_uiRenderer.Animation()->movingAnimation) {
			m_delayedLensPosition = m_lensPosition;

			initLensPosition = false;
		}

	}


	
	double mouseX, mouseY;
	glfwGetCursorPos(viewer()->window(), &mouseX, &mouseY);


	if (viewer()->m_audioTest) {
		mouseX = glm::mix(start.x, end.x, m_audioTestTimer / total_time);
		mouseY = glm::mix(start.y, end.y, m_audioTestTimer / total_time);
	}


	glm::vec2 position = vec2(2.0f * float(mouseX) / float(viewportSize.x) - 1.0f, -2.0f * float(mouseY) / float(viewportSize.y) + 1.0f);

	// Translate Mouse Position into pixel position
	int xPixel = (position.x * viewportSize.x / 2) + viewportSize.x / 2;
	int yPixel = (position.y * viewportSize.y / 2) + viewportSize.y / 2;



	int numberOfTrajectories = viewer()->scene()->tableData()->m_numberOfTrajectories;
	std::vector<int> numberOfTimesteps = viewer()->scene()->tableData()->m_numberOfTimesteps;


	// do not render if either the dataset was not loaded or the window is minimized 
	if (renderingStrategy == NULL || viewer()->viewportSize().x == 0 || viewer()->viewportSize().y == 0) {
		return;
	}

	std::string defines = m_uiRenderer.generateDefines();

	if (defines != m_shaderSourceDefines->string())
	{
		globjects::debug() << defines << std::endl;
		m_shaderSourceDefines->setString(defines);
		reloadShaders();
	}


	// FPS independant animation setup
	if (m_prevTime == NULL) m_prevTime = glfwGetTime();
	float currTime = glfwGetTime();
	float deltaTime = currTime - m_prevTime;

	deltaTime *= m_uiRenderer.Animation()->globalAnimationFactor;
	audioTimer += deltaTime;


	// Displacement timer
	if (m_dispAction) {
		m_time += deltaTime;
		if (m_time >= ANIMATION_LENGTH) {
			m_time = 1.0f;
			m_dispAction = false;
		}
	}

	// Compute the direction vector from delayedLensPosition and lensPosition
	glm::vec2 target = m_lensPosition;
	glm::vec2 lensDir = normalize(target - m_delayedLensPosition);
	float dist = distance(target, m_delayedLensPosition);

	// Computes the force which we push the delayed lens
	glm::vec2 force = (lensDir * (dist));
	force *= (m_uiRenderer.Animation()->globalAnimationFactor * deltaTime);
	force *= 2.0f; // Feels better

	float t;
	if (length(force) < dist) {
		m_delayedLensPosition += force;
		t = clamp(dist, 0.0f, 1.0f);
	}
	else {
		t = 0.0;
	}

	// Test timer for various stuff
	m_testTimer += deltaTime;
	if (m_testTimer > 0.1) {
		m_testTimer = 0.0;
	}
	else {
		if (m_testTimer > 0.1) {
			m_testTimer = 0.1;
		}
	}


	// Fold Animation:
	const float foldDelta = (deltaTime * m_uiRenderer.Animation()->foldAnimationSpeed);;
	if (viewer()->m_foldButton.pressed) {
		m_foldTimer = min(1.0f, m_foldTimer + foldDelta);
		viewer()->m_lensDepthValue = min(viewer()->m_lensDepthValue, 1.0f);
	}
	else {
		m_foldTimer = max(0.0f, m_foldTimer - foldDelta);
	}

	// Pulling Animation:
	const float pullDelta = (deltaTime * m_uiRenderer.Animation()->pullAnimationSpeed);
	if (viewer()->m_pullButton.pressed) {
		m_pullTimer = min(1.0f, m_pullTimer + pullDelta);
	}
	else {
		m_pullTimer = max(0.0f, m_pullTimer - pullDelta);
	}

	// All constants are choosen based on what feel best
	if (viewer()->m_incLensRadius != 0) {
		float sign = viewer()->m_incLensRadius;
		viewer()->setLensRadiusValue((sign / 10.0f) * deltaTime * (m_incHoldCounter / 100.0f));
		viewer()->m_lensRadiusChanging = true;
		m_incHoldCounter++;

	}
	else if (viewer()->m_incLensDepth != 0) {
		float sign = viewer()->m_incLensDepth;
		viewer()->setLensDepthValue((sign / 10.0f) * deltaTime * (m_incHoldCounter/100.0f));
		viewer()->m_lensDepthChanging = true;
		m_incHoldCounter++;
	}
	else {
		viewer()->m_lensRadiusChanging = false;
		viewer()->m_lensDepthChanging = false;
		m_incHoldCounter = 1;
	}

	if (viewer()->m_playbackModeButton.pressed) {
		Settings::Audio* audioSet = m_uiRenderer.Audio();
		audioSet->importance = !audioSet->importance;
		viewer()->m_playbackModeButton.release();
	}


	if (viewer()->m_audioTest) {
		m_audioTestTimer += deltaTime;

		float fps = ImGui::GetIO().Framerate;
		
		if (fps > maxFps) maxFps = fps;
		if (fps < minFps) minFps = fps;
		totFps += fps;
		contFps += 1;

		if (m_audioTestTimer > total_time) {

			globjects::debug() << "Audio Test complete, Total beats: " << m_totalBpm <<  std::endl;
			globjects::debug() << "Max: " << maxFps << " - Min: " << minFps << " - Avg: " << totFps / contFps << std::endl;
			
			m_audioTestTimer = 0.0f;
			viewer()->m_audioTest = false;
			m_totalBpm = 0;

			maxFps = 0;
			minFps = 10000000;
			totFps = 0.0;
			contFps = 0;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Line rendering pass and linked list generation
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	m_lineFramebuffer->bind();

	glClearDepth(1.0f);
	glClearColor(viewer()->backgroundColor().r, viewer()->backgroundColor().g, viewer()->backgroundColor().b, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	// test different blending options interactively: --------------------------------------------------
	// https://andersriggelsen.dk/glblendfunc.php

	// allow blending for the classical line chart color-attachment (0) of the line frame-buffer
	/*
	glEnablei(GL_BLEND, 0);
	glBlendFunci(0, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquationi(0, GL_FUNC_ADD);
	*/

	// SSBO --------------------------------------------------------------------------------------------
	m_intersectionBuffer->bindBase(GL_SHADER_STORAGE_BUFFER, 1);

	const uint intersectionClearValue = 1;
	m_intersectionBuffer->clearSubData(GL_R32UI, 0, sizeof(uint), GL_RED_INTEGER, GL_UNSIGNED_INT, &intersectionClearValue);

	const uint offsetClearValue = 0;
	m_offsetTexture->clearImage(0, GL_RED_INTEGER, GL_UNSIGNED_INT, &offsetClearValue);


	// -------------------------------------------------------------------------------------------------


	m_offsetTexture->bindImageTexture(0, 0, false, 0, GL_READ_WRITE, GL_R32UI);

	mat4 modelTransform = viewer()->modelTransform();
	mat4 inverseModelTransform = inverse(modelTransform);
	float scaledLineWidth = length(vec2(inverseModelViewProjectionMatrix * vec4(0.0f, 0.0f, 0.0f, 1.0f) - inverseModelViewProjectionMatrix * (vec4(m_uiRenderer.Line()->lineWidth, 0.0f, 0.0f, 1.0f))) / vec2(viewportSize));

	programLine->setUniform("modelViewProjectionMatrix", modelViewProjectionMatrix);
	programLine->setUniform("inverseModelViewProjectionMatrix", inverseModelViewProjectionMatrix);


	programLine->setUniform("xAxisScaling", m_uiRenderer.Scaling()->xAxisScaling);
	programLine->setUniform("yAxisScaling", m_uiRenderer.Scaling()->yAxisScaling);

	programLine->setUniform("lineWidth", scaledLineWidth);
	programLine->setUniform("lineColor", viewer()->lineChartColor());

	programLine->setUniform("viewportSize", vec2(viewportSize));

	programLine->setUniform("haloColor", viewer()->haloColor());
	programLine->setUniform("focusLineColor", viewer()->focusLineColor());

	if (m_uiRenderer.Selection()->enableFocusLine) {
		programLine->setUniform("focusLineID", m_uiRenderer.Selection()->focusLineId);
	}
	else {
		programLine->setUniform("focusLineID", -1);
	}
	programLine->setUniform("audioLineID", m_uiRenderer.Selection()->audioLineId);
	programLine->setUniform("lensPosition", m_lensPosition);
	programLine->setUniform("delayedLensPosition", m_delayedLensPosition);
	programLine->setUniform("delayedTValue", t);

	m_uiRenderer.Lens()->lensRadius = viewer()->m_lensRadiusValue;
	programLine->setUniform("lensRadius", m_uiRenderer.Lens()->lensRadius);
	programLine->setUniform("viewMatrix", viewMatrix);
	programLine->setUniform("inverseViewMatrix", inverseViewMatrix);


	programLine->setUniform("brushingAngle", m_uiRenderer.Lens()->brushingAngle);
	programLine->setUniform("lensDepthValue", m_uiRenderer.Lens()->lensDepthValue);
	programLine->setUniform("lensDepthScaling", m_uiRenderer.Lens()->lensDepthScaling);
	programLine->setUniform("lensDisp", m_uiRenderer.Lens()->lensDisp);

	// programLine->setUniform("prevLensDisp", m_uiRenderer.previousLensDisp);

	programLine->setUniform("time", m_time);
	programLine->setUniform("testTime", m_testTimer);
	programLine->setUniform("foldTime", m_foldTimer);
	programLine->setUniform("pullTime", m_pullTimer);


	m_vao->bind();
	glPatchParameteri(GL_PATCH_VERTICES, 4);

	programLine->use();
	renderingStrategy->updateSettings(
		m_uiRenderer.Selection()->focusLineId,
		m_uiRenderer.Selection()->selectionMode,
		m_uiRenderer.Selection()->selectionRange);

	renderingStrategy->setOscMap(m_AudioPlayer.peekBufferOsc());
	renderingStrategy->performRendering(programLine, m_vao.get());

	programLine->release();
	m_vao->unbind();



	//m_intersectionBuffer->unbind(GL_SHADER_STORAGE_BUFFER);

	m_offsetTexture->unbindImageTexture(0);

	//m_lineFramebuffer->unbind();

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	// disable blending for draw buffer 0 (line graph texture)
	//glDisablei(GL_BLEND, 0);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Blending dependent on current rendering strategy
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// make sure lines are drawn on top of each other
	// glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	m_blurFramebuffer->bind();
	glDepthMask(GL_FALSE);


	m_vaoQuad->bind();
	programBlur->use();
	programBlur->setUniform("blurTexture", 0);

	m_depthTexture->bindActive(0);

	const int blutIterations = 3;
	float blurOffset = 8.0;

	int blurIndex = 0;

	for (int i = 0; i < blutIterations; i++)
	{
		programBlur->setUniform("offset", blurOffset);
		m_blurFramebuffer->setDrawBuffers({ GL_COLOR_ATTACHMENT0 + (1 - blurIndex) });
		m_vaoQuad->drawArrays(GL_POINTS, 0, 1);

		blurIndex = 1 - blurIndex;
		blurOffset = max(1.0f, 0.5f * blurOffset);

		m_blurTexture[blurIndex]->bindActive(0);
	}

	programBlur->release();
	m_vaoQuad->unbind();


	glDepthMask(GL_TRUE);
	m_blurFramebuffer->unbind();



	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	m_lineFramebuffer->bind();

	// SSBO --------------------------------------------------------------------------------------------
	m_intersectionBuffer->bindBase(GL_SHADER_STORAGE_BUFFER, 1);

	if (m_uiRenderer.calculateOverplottingIndex) {

		// reset buffers in case they were already initialized
		m_totalPixelBuffer = std::make_unique<globjects::Buffer>();
		m_visiblePixelBuffer = std::make_unique<globjects::Buffer>();

		// initialize SSAOs
		m_totalPixelBuffer->setStorage(sizeof(uint) * numberOfTrajectories, nullptr, gl::GL_NONE_BIT);
		m_visiblePixelBuffer->setStorage(sizeof(uint) * numberOfTrajectories, nullptr, gl::GL_NONE_BIT);

		// clear and prepare buffers
		m_totalPixelBuffer->bindBase(GL_SHADER_STORAGE_BUFFER, 2);
		m_visiblePixelBuffer->bindBase(GL_SHADER_STORAGE_BUFFER, 3);

		const uint bufferCounterClearValue = 0;
		m_totalPixelBuffer->clearSubData(GL_R32UI, 0, sizeof(uint) * numberOfTrajectories, GL_RED_INTEGER, GL_UNSIGNED_INT, &bufferCounterClearValue);
		m_visiblePixelBuffer->clearSubData(GL_R32UI, 0, sizeof(uint) * numberOfTrajectories, GL_RED_INTEGER, GL_UNSIGNED_INT, &bufferCounterClearValue);
	}


	// Clear id buffer
	m_lensBuffer = std::make_unique <globjects::Buffer>();
	m_lensBuffer->setStorage(sizeof(vec4) * numberOfTrajectories, nullptr, gl::GL_NONE_BIT);
	m_lensBuffer->bindBase(GL_SHADER_STORAGE_BUFFER, 4);

	const vec4 zero = vec4(0.0f);
	m_lensBuffer->clearSubData(GL_RGBA32F, 0, sizeof(vec4) * numberOfTrajectories, GL_RGBA, GL_FLOAT, &zero);
	// ------

	// -------------------------------------------------------------------------------------------------

	m_offsetTexture->bindActive(0);
	m_blurTexture[blurIndex]->bindActive(1);

	programBlend->setUniform("offsetTexture", 0);
	programBlend->setUniform("blurTexture", 1);

	programBlend->setUniform("backgroundColor", viewer()->backgroundColor());
	programBlend->setUniform("smoothness", m_uiRenderer.Line()->smoothness);

	programBlend->setUniform("viewportSize", vec2(viewportSize));
	programBlend->setUniform("lensPosition", m_lensPosition);
	programBlend->setUniform("delayedLensPosition", m_delayedLensPosition);
	programBlend->setUniform("lensRadius", m_uiRenderer.Lens()->lensRadius);
	programBlend->setUniform("lensBorderColor", viewer()->lensColor());
	programBlend->setUniform("lensDepthValue", m_uiRenderer.Lens()->lensDepthValue);

	m_vaoQuad->bind();

	programBlend->use();
	m_vaoQuad->drawArrays(GL_POINTS, 0, 1);
	programBlend->release();

	m_vaoQuad->unbind();

	/* AUDIO -------------------------------------------------- */

	// Read the angle from texture
	glReadBuffer(GL_COLOR_ATTACHMENT2);


	GLfloat data[4];
	glReadPixels(xPixel, yPixel, 1, 1, GL_RGBA, GL_FLOAT, &data);

	int id = int(data[0]);
	float xDir = float(data[1]);
	float yDir = float(data[2]);
	
	// Transform data into degrees
	const float radians = acos(dot(vec2(xDir, yDir), vec2(0.0, -1.0)));
	const int degrees = int(round(radians * (180.0 / glm::pi<float>())));


	glReadBuffer(GL_NONE);

	// Check for any special key presses
	bool readSubData = false;
	AudioMode mode = NONE;

	// TODO make the syntax consistent
	if (viewer()->playAudioQueue()) mode = GLOBAL;
	else if (viewer()->m_pullButton.pressed) mode = PULL;
	else if (viewer()->m_lensDepthChanging) mode = DRAG;
	else if (viewer()->m_lensRadiusChanging) mode = RADIUS;

	const int metric = m_uiRenderer.Audio()->metric; // Get the current metric we want to playback the audio on

	// Update the focusID when clicking, independant of Audio

	const bool idWithinBounds = (id >= 0) && (id <= numberOfTrajectories);
	const bool legalDegree = degrees < 181;
	const bool isEnabled = mode == NONE || (mode == PULL && !m_uiRenderer.Audio()->importance);

	// Only used for checking mouse positions, when making audio tests
	if (viewer()->m_mousePressed[0]) {
		//globjects::debug() << mouseX << ", " << mouseY << std::endl;
	}
	



	if (idWithinBounds && legalDegree && isEnabled && !viewer()->m_lensRadiusChanging) {


		if (viewer()->m_mousePressed[0]) {
			globjects::debug() << mouseX << ", " << mouseY << std::endl;
			m_uiRenderer.setFocusId(id);
		}

		const bool mouseMoved = (xPixel != prevPixelX || yPixel != prevPixelY);
		const bool timerReset = audioTimer >= m_uiRenderer.Audio()->note_interval;

		if (mouseMoved && timerReset && !m_uiRenderer.Audio()->mute) {
			SimTable* simTable = renderingStrategy->getSimTable();
			float vol = 0.0;

			bool skipPlayNote = false;
			if (metric == 0) {
				vol = simTable->getImportanceTable().at(id);
				skipPlayNote = vol > m_uiRenderer.Lens()->lensDepthValue;
			}
			else if (metric == 1) {
				vol = simTable->get(m_uiRenderer.Selection()->focusLineId, id, m_uiRenderer.Selection()->selectionRange);
			}

			if (!skipPlayNote) {
				m_AudioPlayer.playNote(id, vol, degrees);
				m_totalBpm += 1;
			}

			prevID = id;
			audioTimer = 0.0f;
		}
	
	}

	// Audio for speciall key presses
	if(mode != NONE && mode != mPrevFrameAudioMode && mode != RADIUS|| mode == PULL){

		if (m_AudioPlayer.current_id == 0) {
			m_AudioPlayer.startTime = std::chrono::high_resolution_clock::now();
			m_AudioPlayer.current_id = 1;
		}
		

		SimTable* simTable = renderingStrategy->getSimTable();
		std::vector<std::pair<int,  std::pair<float, int>>> tmpLines;


		for (int i = 0; i < numberOfTrajectories; i++) {

			SubData subData = getSubDataFromLensBuffer(i);
			const Settings::Selection* selSettings = m_uiRenderer.Selection();


			const bool isInsideLens = subData.dist > 0;
			if (isInsideLens) {

				float vol = 0.0f;
				if (mode == GLOBAL || mode == DRAG) {

					if (metric == 0) vol = simTable->getImportanceTable().at(i);
					else if (metric == 1) vol = simTable->get(selSettings->focusLineId, subData.id, selSettings->selectionRange);

					tmpLines.push_back(std::make_pair(i, std::make_pair(vol, subData.degree)));
				}
				else if (mode == PULL && m_uiRenderer.Audio()->importance) {
					vol = simTable->get(selSettings->focusLineId, i, selSettings->selectionRange);
					if (vol > 0) {
						tmpLines.push_back(std::make_pair(i, std::make_pair(vol, subData.degree)));
					}
				}
				
					
			}
		}

		m_AudioPlayer.setAvailableNotes(tmpLines, 1);
	}

	// Continues this thread within the audio object!
	m_AudioPlayer.mainThread(deltaTime, mode);
	mPrevFrameAudioMode = mode;


	prevPixelX = xPixel;
	prevPixelY = yPixel;
	m_prevTime = currTime;

	// SSBO --------------------------------------------------------------------------------------------
	m_lensBuffer->unbind(GL_SHADER_STORAGE_BUFFER);
	m_intersectionBuffer->unbind(GL_SHADER_STORAGE_BUFFER);


	if (m_uiRenderer.calculateOverplottingIndex) {

		// force GPU to finish work before we start reading
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		//		     # visible pixels (per trajectory)
		//	    Sum ------------------------------------
		//		     # total pixels   (per trajectory)
		// 1 -	-------------------------------------------
		//		     # number of trajectories (total)

		m_overplottingRatio = 0;
		m_totalPixelsPerTrajectory.clear();
		m_visiblePixelsPerTrajectory.clear();

		for (int i = 0; i < numberOfTrajectories; i++) {

			uint currentTotalPixels = 0;
			m_totalPixelBuffer->getSubData(i * sizeof(uint), sizeof(uint), &currentTotalPixels);
			m_totalPixelsPerTrajectory.push_back(currentTotalPixels);

			uint currentVisiblePixels = 0;
			m_visiblePixelBuffer->getSubData(i * sizeof(uint), sizeof(uint), &currentVisiblePixels);
			m_visiblePixelsPerTrajectory.push_back(currentVisiblePixels);

			m_overplottingRatio += (double)currentVisiblePixels / (double)currentTotalPixels;
		}

		m_overplottingRatio = 1 - (m_overplottingRatio / numberOfTrajectories);

		// release bound SSBO
		m_totalPixelBuffer->unbind(GL_SHADER_STORAGE_BUFFER);
		m_visiblePixelBuffer->unbind(GL_SHADER_STORAGE_BUFFER);

		m_uiRenderer.calculateOverplottingIndex = false;

		// display results in GUI
		m_uiRenderer.displayOverplottingGUI = true;
	}
	// -------------------------------------------------------------------------------------------------

	m_blurTexture[blurIndex]->unbindActive(1);
	m_offsetTexture->unbindActive(0);

	m_lineFramebuffer->unbind();

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	// Blit final image into visible framebuffer
	m_lineFramebuffer->blit(GL_COLOR_ATTACHMENT0, { 0,0,viewer()->viewportSize().x, viewer()->viewportSize().y }, Framebuffer::defaultFBO().get(), GL_BACK, { 0,0,viewer()->viewportSize().x, viewer()->viewportSize().y }, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	// Restore OpenGL state
	currentState->apply();

}

// Simply retrives the subData for a speciffic texture offsett
SubData LineRenderer::getSubDataFromLensBuffer(int offset) {
	vec4 vecData;
	m_lensBuffer->getSubData(offset * sizeof(vec4), sizeof(vec4), &vecData);

	const int id = int(vecData.x);
	const float xDir = float(vecData.y);
	const float yDir = float(vecData.z);
	const float dist = float(vecData.a);

	const float radians = acos(dot(vec2(xDir, yDir), vec2(0.0, -1.0)));
	const int degrees = int(round(radians * (180.0 / glm::pi<float>())));

	return SubData(id, degrees, dist);
}


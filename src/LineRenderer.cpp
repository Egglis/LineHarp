#include "LineRenderer.h"
#include <globjects/base/File.h>
#include <globjects/State.h>
#include <iostream>
#include <filesystem>

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

using namespace lineweaver;
using namespace gl;
using namespace glm;
using namespace globjects;

LineRenderer::LineRenderer(Viewer* viewer) : Renderer(viewer), m_uiRenderer()
{
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

	m_IdTexture = Texture::create(GL_TEXTURE_2D);
	m_IdTexture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_IdTexture->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_IdTexture->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	m_IdTexture->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	m_IdTexture->image2D(0, GL_R32I, m_framebufferSize, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);


	m_blurFramebuffer = Framebuffer::create();
	m_blurFramebuffer->attachTexture(GL_COLOR_ATTACHMENT0, m_blurTexture[0].get());
	m_blurFramebuffer->attachTexture(GL_COLOR_ATTACHMENT1, m_blurTexture[1].get());
	m_blurFramebuffer->attachTexture(GL_DEPTH_ATTACHMENT, m_depthTexture.get());
	m_blurFramebuffer->setDrawBuffers({ GL_COLOR_ATTACHMENT0 });


	m_lineFramebuffer = Framebuffer::create();
	m_lineFramebuffer->attachTexture(GL_COLOR_ATTACHMENT0, m_lineChartTexture.get());
	m_lineFramebuffer->attachTexture(GL_DEPTH_ATTACHMENT, m_depthTexture.get());
	m_lineFramebuffer->attachTexture(GL_COLOR_ATTACHMENT2, m_IdTexture.get());
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
		m_IdTexture->image2D(0, GL_R32I, m_framebufferSize, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);

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

	m_uiRenderer.animationSettings();


	ImGui::Begin("Importance Driven Dense Line Graphs");

	if (ImGui::CollapsingHeader("CSV-Files", ImGuiTreeNodeFlags_DefaultOpen)) {
		bool fileChanged = m_uiRenderer.dataFile();

		if (fileChanged || viewer()->enforcedDataRefresh()) {

			// initialize data table
			if (m_uiRenderer.fileMode == 0) {
				viewer()->scene()->tableData()->load(m_uiRenderer.dataFilename);
			}
			else {
				if (m_uiRenderer.dataFilename.find("AndrewsPlot") != std::string::npos) {

					// if data set is an andrews plot (currently indicated by name) 
					// first, load data without duplicating first and last entry
					viewer()->scene()->tableData()->loadAndrewsSeries(m_uiRenderer.dataFilename);

					// second, perform andrews transformation
					viewer()->scene()->tableData()->andrewsTransform(96);

				}
				else {

					// otherwise, load series duplicating first and last element as usual
					viewer()->scene()->tableData()->loadSeries(m_uiRenderer.dataFilename);
				}

			}

			// assign default rendering strategy 
			renderingStrategy = new LinkedListRendering(viewer()->scene()->tableData());

			// load data and apply implicite importance metric
			renderingStrategy->prepareDataBuffers();

			if (m_uiRenderer.fileMode == 0)
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
		fileChanged = m_uiRenderer.impFile();

		if ((fileChanged || viewer()->enforcedImportanceRefresh()) && renderingStrategy != NULL)
		{
			// initialize importance table
			viewer()->scene()->tableImportance()->load(m_uiRenderer.importanceFilename);

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
			m_uiRenderer.focusLineID = 0;

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

	m_uiRenderer.scaling();
	m_uiRenderer.linePropreties();
	m_uiRenderer.selectionSettings(viewer());
	m_uiRenderer.lensFeature(viewer());
	m_uiRenderer.overplottingMeasurment(viewer());

	ImGui::End();
	if (!m_dispAction) {
		if (m_uiRenderer.lensDisp != m_previousLensDisp) {
			m_dispAction = true;
			m_time = 0.0f;
		}
	}

	m_previousLensDisp = m_uiRenderer.lensDisp;

	// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	if (!viewer()->m_lensDepthChanging) {
		double mouseX, mouseY;
		glfwGetCursorPos(viewer()->window(), &mouseX, &mouseY);

		glm::vec2 tempLensPosition = m_lensPosition;
		m_lensPosition = vec2(2.0f * float(mouseX) / float(viewportSize.x) - 1.0f, -2.0f * float(mouseY) / float(viewportSize.y) + 1.0f);

		if (initLensPosition || !m_uiRenderer.movingAnimation) {
			m_delayedLensPosition = m_lensPosition;

			initLensPosition = false;
		}

	}


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

	deltaTime *= m_uiRenderer.globalAnimationFactor;




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
	force *= (m_uiRenderer.globalAnimationFactor * deltaTime);
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
	if (viewer()->foldAnimation()) {
		m_foldTimer = min(1.0f, m_foldTimer + (deltaTime * m_uiRenderer.foldAnimationSpeed));
		viewer()->m_lensDepthValue = min(viewer()->m_lensDepthValue, 1.0f);
	} else {
		m_foldTimer = max(0.0f, m_foldTimer - (deltaTime * m_uiRenderer.foldAnimationSpeed));
	}

	// Pulling Animation:
	if (viewer()->pullAnimation()) {
		m_pullTimer = min(1.0f, m_pullTimer + (deltaTime * m_uiRenderer.pullAnimationSpeed));
	}
	else {
		m_pullTimer = max(0.0f, m_pullTimer - (deltaTime * m_uiRenderer.pullAnimationSpeed));
	}


	m_prevTime = currTime;
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
	float scaledLineWidth = length(vec2(inverseModelViewProjectionMatrix * vec4(0.0f, 0.0f, 0.0f, 1.0f) - inverseModelViewProjectionMatrix * (vec4(m_uiRenderer.lineWidth, 0.0f, 0.0f, 1.0f))) / vec2(viewportSize));

	programLine->setUniform("modelViewProjectionMatrix", modelViewProjectionMatrix);
	programLine->setUniform("inverseModelViewProjectionMatrix", inverseModelViewProjectionMatrix);


	programLine->setUniform("xAxisScaling", m_uiRenderer.xAxisScaling);
	programLine->setUniform("yAxisScaling", m_uiRenderer.yAxisScaling);

	programLine->setUniform("lineWidth", scaledLineWidth);
	programLine->setUniform("lineColor", viewer()->lineChartColor());

	programLine->setUniform("viewportSize", vec2(viewportSize));

	programLine->setUniform("haloColor", viewer()->haloColor());
	programLine->setUniform("focusLineColor", viewer()->focusLineColor());

	if (m_uiRenderer.enableFocusLine) {
		programLine->setUniform("focusLineID", m_uiRenderer.focusLineID);
	}
	else {
		programLine->setUniform("focusLineID", -1);
	}

	programLine->setUniform("lensPosition", m_lensPosition);
	programLine->setUniform("delayedLensPosition", m_delayedLensPosition);
	programLine->setUniform("delayedTValue", t);


	programLine->setUniform("lensRadius", m_uiRenderer.lensRadius);
	programLine->setUniform("viewMatrix", viewMatrix);
	programLine->setUniform("inverseViewMatrix", inverseViewMatrix);


	programLine->setUniform("brushingAngle", m_uiRenderer.brushingAngle);
	programLine->setUniform("lensDepthValue", m_uiRenderer.lensDepthValue);
	programLine->setUniform("lensDepthScaling", m_uiRenderer.lensDepthScaling);
	programLine->setUniform("lensDisp", m_uiRenderer.lensDisp);

	// programLine->setUniform("prevLensDisp", m_uiRenderer.previousLensDisp);

	programLine->setUniform("time", m_time);
	programLine->setUniform("testTime", m_testTimer);
	programLine->setUniform("foldTime", m_foldTimer);
	programLine->setUniform("pullTime", m_pullTimer);

	
	m_vao->bind();
	glPatchParameteri(GL_PATCH_VERTICES, 4);

	programLine->use();
	renderingStrategy->updateSettings(m_uiRenderer.focusLineID, m_uiRenderer.selectionMode, m_uiRenderer.selectionRange);
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
	// -------------------------------------------------------------------------------------------------

	m_offsetTexture->bindActive(0);
	m_blurTexture[blurIndex]->bindActive(1);

	programBlend->setUniform("offsetTexture", 0);
	programBlend->setUniform("blurTexture", 1);

	programBlend->setUniform("backgroundColor", viewer()->backgroundColor());
	programBlend->setUniform("smoothness", m_uiRenderer.smoothness);

	programBlend->setUniform("viewportSize", vec2(viewportSize));
	programBlend->setUniform("lensPosition", m_lensPosition);
	programBlend->setUniform("delayedLensPosition", m_delayedLensPosition);
	programBlend->setUniform("lensRadius", m_uiRenderer.lensRadius);
	programBlend->setUniform("lensBorderColor", viewer()->lensColor());
	programBlend->setUniform("lensDepthValue", m_uiRenderer.lensDepthValue);

	m_vaoQuad->bind();

	programBlend->use();
	m_vaoQuad->drawArrays(GL_POINTS, 0, 1);
	programBlend->release();

	m_vaoQuad->unbind();

	// Read pixel at clicked location
	if (viewer()->m_mousePressed[0]) {
		glReadBuffer(GL_COLOR_ATTACHMENT2);
		double mouseX, mouseY;
		glfwGetCursorPos(viewer()->window(), &mouseX, &mouseY);

		glm::vec2 position = vec2(2.0f * float(mouseX) / float(viewportSize.x) - 1.0f, -2.0f * float(mouseY) / float(viewportSize.y) + 1.0f);
		
		// Translate Mouse Position into pixel position
		int x = (position.x * viewportSize.x / 2) + viewportSize.x / 2;
		int y = (position.y * viewportSize.y / 2) + viewportSize.y / 2;

		// id = 0: Nothing, (id - 1) = Trajectory ID
		int id = 0;
		glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &id);
		if(id != 0 && id <= numberOfTrajectories){
			m_uiRenderer.setFocusId(id - 1);
		}

		viewer()->m_mousePressed[0] = false;
	}



	// SSBO --------------------------------------------------------------------------------------------
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


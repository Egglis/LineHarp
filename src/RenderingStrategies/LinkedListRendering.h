#pragma once
#include "RenderingStrategy.h"
#include "../SimilarityTables/SimTable.h"

using namespace gl;

namespace lineweaver
{

	class LinkedListRendering : public RenderingStrategy {

	public:
		LinkedListRendering(TableData* data);

		void prepareDataBuffers();

		void prepareImportanceBuffer();
		void prepareImportanceBuffer(TableImportance* importance);

		void performRendering(globjects::Program* p, globjects::VertexArray* va);
		void weaveSeries(const TableData& table);

		void updateSettings(int focusId, int mode, float range) { 
			m_simTable.setMode(mode);
			m_focusID = focusId;
			m_selectionRange = range;
		};

		SimTable* getSimTable() { return &m_simTable; };

	private:
		void prepareIndicesBuffer();



		std::vector<std::vector<GLuint>> m_indices;
		
		int m_focusID = 0;
		float m_selectionRange = 0.1f;

		SimTable m_simTable; 
	};
	
}
#pragma once
#include "RenderingStrategy.h"
#include "../SimilarityTables/ImportanceTable.h"

using namespace gl;

enum SelectionModes {
	SINGLE,
	IMPORTANCE,
	SIMILARITY,
	EUCLIDEAN
};



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

		void prepareSimilarityTables(Table* table);
		void updateSettings(int focusId, int mode, float range) {
			m_focusID = focusId;
			m_selectionMode = mode;
			m_selectionRange = range;
		};

		float getSimilarity(int index);

	private:
		void prepareIndicesBuffer();
		void prepareTrajectoryImportance(Table* table);
		void prepareTrajectoryDistance(Table* table);
		void prepareEuclidean(Table* table);


		std::vector<std::vector<GLuint>> m_indices;
		
		int m_focusID = 0;
		int m_selectionMode = 0;
		float m_selectionRange = 0.1f;

		SimTable m_importanceTable = SimTable(SelectionModes::IMPORTANCE);

		std::vector<float> m_importanceLookup;
		std::vector<float> m_distanceLookup;
		std::vector<float> m_similarityLookup;
		std::vector<std::vector<float>> m_eculideanLookup;

	};
	
}
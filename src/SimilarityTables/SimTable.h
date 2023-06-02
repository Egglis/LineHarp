#pragma once
#include <vector>
#include <fstream>
#include <numeric>
#include <filesystem>


#include "../CSV/Table.h"

enum SelectionModes {
	SINGLE,
	IMPORTANCE,
	MIDPOINT,
	MINDIST,
	HAUSDORFF,
	FRECHET
};

namespace lineweaver {

	class SimTableCache {
	public:
		int makeCacheFolder(std::string& fileName);
		int save(const std::string& filepath, const std::vector<std::vector<float>>& data, int type);
		std::vector<std::vector<float>> load(const std::string& filepath, int type);
		std::vector<float> loadImportance(const std::string& filepath);
		int saveImportance(const std::string& filepath, const std::vector<float>& data);
		bool isCache(std::string& fileName);
	private:
		std::vector<std::string> m_fileNames{
			"importance.bin",
			"midPoint.bin",
			"minDist.bin",
			"hausdorf.bin",
			"frechet.bin"
		};
		std::string newPath = "src\\SimilarityTables\\Cache";
	};

	class SimTable {
	public:
		float get(int focus, int current, float range); // row, col, range
		void setup(Table* table, int mode);
		void setup(const Table* table, int mode);

		void setActiveX(std::vector<float> x) { m_x = normalize(x); };
		void setActiveY(std::vector<float> y) { m_y = normalize(y); };
		void setActiveImportance(std::vector<float> importance) { m_importance = importance; };


		SelectionModes getMode() { return m_mode; };
		void setMode(int mode) { m_mode = static_cast<SelectionModes>(mode); };
		void setMode(SelectionModes mode) { m_mode = mode; };

		std::vector<float> getImportanceTable() { return m_importanceTable; };
	protected:
		void setupImportanceVector(Table* table);
		void iterateLines(Table* table);
		float computeEuclideanDistance(int a, int b);
		float computeMidPointDistance(int a, int b);

		float getFrechetDist(int i, int j, float d, std::vector<std::vector<float>> *matrix);

		bool isLegalIndex(int i, int size);

		std::vector<float> normalize(std::vector<float> list);


		std::vector<float> m_importanceTable;
		std::vector<std::vector<float>> m_midPointTable;
		std::vector<std::vector<float>> m_minDistTable;
		std::vector<std::vector<float>> m_hausdorffTable;
		std::vector<std::vector<float>> m_frechetTable;


		std::vector<float> m_x;
		std::vector<float> m_y;
		std::vector<float> m_importance;
		
		SelectionModes m_mode;

		SimTableCache mCache;
	};

};
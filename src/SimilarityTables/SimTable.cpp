#include "SimTable.h"
#include <memory>
#include <numeric>
#include <chrono>


#include <globjects/Program.h>
using namespace std;
using namespace std::chrono;


using namespace lineweaver;

// Gets the similarity for, focus: row, current: col
float SimTable::get(int focus, int current, float range) {

	if (SelectionModes::SINGLE == m_mode || range <= 0.0f) {
		if (focus == current) return 1.0f;
	}
	else if (SelectionModes::IMPORTANCE == m_mode) {
		float selectedImportance = m_importanceTable.at(focus);
		float currentImportance = m_importanceTable.at(current);
		float diff = abs(selectedImportance - currentImportance);
		if (diff <= range) {
			return 1.0f - ((diff * 1.0f) / range);
		}
	}
	else {
		float dist = 0.0f;
		if (SelectionModes::MIDPOINT == m_mode) dist = m_midPointTable.at(focus).at(current);
		else if (SelectionModes::MINDIST == m_mode) dist = m_minDistTable.at(focus).at(current);
		else if (SelectionModes::HAUSDORFF == m_mode) dist = m_hausdorffTable.at(focus).at(current);
		else if (SelectionModes::FRECHET == m_mode) dist = m_frechetTable.at(focus).at(current);

		if (dist <= range) {
			return 1.0f - (dist / range);
		}
	}

	return 0.0f;
}


void SimTable::setup(Table* table) {
	iterateLines(table);
	setupImportanceVector(table);
}

// Sets up a list of average importance for each trajectory
void SimTable::setupImportanceVector(Table* table)
{
	m_importanceTable.clear();

	for (int i = 0; i < table->m_numberOfTrajectories; i++) {
		int count = static_cast<int>(table->m_numberOfTimesteps.at(i));
		float sum = 0.0f;
		for (int j = 0; j < table->m_numberOfTimesteps.at(i); j++) {
			sum += m_importance.at((i * count) + j);
		}
		m_importanceTable.push_back(sum / static_cast<float>(count));
	}
}

// Iteretes all lines and steps with eachother and computes all the similarity measures
void SimTable::iterateLines(Table* table)
{
	m_midPointTable.clear();
	m_minDistTable.clear();
	m_hausdorffTable.clear();
	m_frechetTable.clear();

	auto start = high_resolution_clock::now();

	for (int lineA = 0; lineA < table->m_numberOfTrajectories; lineA++) {
		std::vector<float> tmpMidPoint;
		std::vector<float> tmpMinDist;
		std::vector<float> tmphaus;
		std::vector<float> tmpfrechet;


		for (int lineB = 0; lineB < table->m_numberOfTrajectories; lineB++) {
			const int stepsA = table->m_numberOfTimesteps.at(lineA);
			const int stepsB = table->m_numberOfTimesteps.at(lineB);

			std::vector<std::vector<float>> frechetMatrix(stepsA, std::vector<float>(stepsB, 0.0f));
			float minDist = INFINITY;
			float midPointSum = 0.0f;
			float h = 0.0f;
			int total = 0;

			// Loop steps in each trajectory
			for (int sa = 0; sa < stepsA; sa++) {


				float hausdorf_min = INFINITY;
				for (int sb = 0; sb < stepsB; sb++) {

					
					// Transform row, col index into 1D indexes
					int a = lineA * stepsA + sa;
					int b = lineB * stepsB + sb;
					
					// Using Euclidean Distance:
					const float d = computeEuclideanDistance(a, b);

					if (sa == sb) {
						// Mid-Point segment indexes:
						int seg_a = lineA * stepsA + std::min(sa, stepsA - 2);
						int seg_b = lineB * stepsB + std::min(sa, stepsB - 2);
						midPointSum += computeMidPointDistance(seg_a, seg_b);

						// Min Dist measure:
						minDist = std::min(minDist, d);
						total++;
					}


					// Hausdorf Distance:
					hausdorf_min = std::min(hausdorf_min, d);

					// Frechet Distance:
					frechetMatrix[sa][sb] = getFrechetDist(sa, sb, d, &frechetMatrix);
				}

				// Update haudorf distance
				h = std::max(h, hausdorf_min);
			}

			tmpMinDist.push_back(minDist);
			tmpMidPoint.push_back(midPointSum / total);
			tmphaus.push_back(h);
			tmpfrechet.push_back(frechetMatrix[stepsA - 1][stepsB - 1]);
		}

		m_minDistTable.push_back(tmpMinDist);
		m_midPointTable.push_back(tmpMidPoint);
		m_hausdorffTable.push_back(tmphaus);
		m_frechetTable.push_back(tmpfrechet);
	}
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);
	globjects::debug() << duration.count() << "ms : Similariy Tables Load Time" << std::endl;
}
 
float SimTable::getFrechetDist(int i, int j, float d, std::vector<std::vector<float>>* matrix)
{
	if (i > 0 && j > 0) {
		const float minM = glm::min(matrix->at(i - 1).at(j), glm::min(matrix->at(i - 1).at(j - 1), matrix->at(i).at(j - 1)));
		return glm::max(minM, d);
	}
	else if (i > 0 && j == 0) {
		return glm::max(matrix->at(i - 1).at(0), d);
	}
	else if (i == 0 && j > 0) {
		return glm::max(matrix->at(0).at(j - 1), d);
	}
	else if (i == 0 && j == 0) {
		return d;
	}

	return INFINITY;
	
}


float lineweaver::SimTable::computeMidPointDistance(int a, int b)
{
	const glm::vec3 a_from = glm::vec3(m_x[a], m_y[a], m_importance[a]);
	const glm::vec3 a_to = glm::vec3(m_x[a + 1], m_y[a + 1], m_importance[a + 1]);
	const glm::vec3 b_from = glm::vec3(m_x[b], m_y[b], m_importance[b]);
	const glm::vec3 b_to = glm::vec3(m_x[b + 1], m_y[b + 1], m_importance[b + 1]);
	const glm::vec3 p0 = glm::mix(a_from, a_to, 0.5);
	const glm::vec3 p1 = glm::mix(b_from, b_to, 0.5);
	return glm::distance(p0, p1);
}

float SimTable::computeEuclideanDistance(int a, int b)
{
	const glm::vec3 p0 = glm::vec3(m_x[a], m_y[a], m_importance[a]);
	const glm::vec3 p1 = glm::vec3(m_x[b], m_y[b], m_importance[b]);
	return glm::distance(p0, p1);
}

std::vector<float> SimTable::normalize(std::vector<float> list)
{
	std::vector<float> tmp;
	const float max_elm = *std::max_element(list.begin(), list.end());
	for (int i = 0; i < list.size(); i++) {
		tmp.push_back(list.at(i) / max_elm);
	}
	return tmp;
}


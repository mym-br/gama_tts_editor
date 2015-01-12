/***************************************************************************
 *  Copyright 2014, 2015 Marcelo Y. Matuda                                 *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/

#ifndef TRANSITION_POINT_H
#define TRANSITION_POINT_H

#include <memory>
#include <string>
#include <vector>

#include "Transition.h"



namespace GS {

namespace TRMControlModel {
class Model;
}

struct TransitionPoint {
	TRMControlModel::Transition::Point::Type type;
	float value;

	// If timeExpression is not empty, time = timeExpression, otherwise time = freeTime.
	std::shared_ptr<TRMControlModel::Equation> timeExpression;
	float freeTime; // milliseconds
	float time;

	bool isPhantom;

	// The point is at the start of a segment for which a slope is defined.
	bool hasSlope;
	float slope;

	TransitionPoint()
		: type(TRMControlModel::Transition::Point::TYPE_INVALID)
		, value(0.0)
		, timeExpression()
		, freeTime(0.0)
		, time(0.0)
		, isPhantom(false)
		, hasSlope(false)
		, slope(0.0) {}

	static void copyPointsFromTransition(const TRMControlModel::Transition& transition, std::vector<TransitionPoint>& pointList);
	static void sortPointListByTypeAndTime(std::vector<TransitionPoint>& pointList);
	static void calculateTimes(const TRMControlModel::Model& model, std::vector<TransitionPoint>& pointList);
	static void adjustValuesInSlopeRatios(std::vector<TransitionPoint>& pointList);
	static void copyPointsToTransition(TRMControlModel::Transition::Type type, const std::vector<TransitionPoint>& pointList, TRMControlModel::Transition& transition);
	static std::unique_ptr<TRMControlModel::Transition::Point> makeNewPoint(const TransitionPoint& sourcePoint);
};

} // namespace GS

#endif // TRANSITION_POINT_H

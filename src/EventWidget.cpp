/***************************************************************************
 *  Copyright 2014, 2015 Marcelo Y. Matuda                                 *
 *  Copyright 1991, 1992, 1993, 1994, 1995, 1996, 2001, 2002               *
 *    David R. Hill, Leonard Manzara, Craig Schock                         *
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
// 2015-01
// This file was created by Marcelo Y. Matuda, and code/information
// from Gnuspeech was added to it later.

#include "EventWidget.h"

#include <cmath>
#include <cstring> /* strlen */

#include <QMouseEvent>
#include <QPainter>
#include <QPointF>
#include <QRectF>
#include <QSizePolicy>

#include "EventList.h"
#include "Model.h"

#define MARGIN 10.0
#define TRACK_HEIGHT 20.0
#define GRAPH_HEIGHT 120.0
#define SPECIAL_STRING "(special)"
#define MININUM_WIDTH 1024
#define MININUM_HEIGHT 768
#define TEXT_MARGIN 5.0
#define DEFAULT_TIME_SCALE 0.7



namespace GS {

EventWidget::EventWidget(QWidget* parent)
		: QWidget(parent)
		, eventList_(nullptr)
		, model_(nullptr)
		, timeScale_(DEFAULT_TIME_SCALE)
		, modelUpdated_(false)
		, textAscent_(0.0)
		, textYOffset_(0.0)
		, labelWidth_(0.0)
		, maxLabelSize_(0)
		, totalWidth_(MININUM_WIDTH)
		, totalHeight_(MININUM_HEIGHT)
{
	//setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	setMinimumWidth(totalWidth_);
	setMinimumHeight(totalHeight_);

	QPalette pal;
	pal.setColor(QPalette::Window, Qt::white);
	setPalette(pal);
	setAutoFillBackground(true);

	setMouseTracking(true);
}

// Note: with no antialiasing, the coordinates in QPointF are rounded to the nearest integer.
void
EventWidget::paintEvent(QPaintEvent* /*event*/)
{
	if (eventList_ == nullptr || eventList_->list().empty()) {
		return;
	}

	QPainter painter(this);
	painter.setFont(QFont("monospace"));

	if (modelUpdated_) {
		QFontMetrics fm = painter.fontMetrics();
		textAscent_ = fm.ascent();
		textYOffset_ = 0.5 * textAscent_;

		int maxWidth = 0;
		for (unsigned int i = 0; i < model_->parameterList().size(); ++i) {
			unsigned int labelSize = model_->parameterList()[i].name().size();
			if (labelSize > maxLabelSize_) {
				maxLabelSize_ = labelSize;
			}
			int width = fm.width(model_->parameterList()[i].name().c_str());
			if (width > maxWidth) {
				maxWidth = width;
			}
		}
		unsigned int labelSize = std::strlen(SPECIAL_STRING);
		if (labelSize > maxLabelSize_) {
			maxLabelSize_ = labelSize;
		}
		int width = fm.width(SPECIAL_STRING);
		if (width > maxWidth) {
			maxWidth = width;
		}
		labelWidth_ = maxWidth;

		modelUpdated_ = false;
	}

	double xEnd, yEnd;
	if (selectedParamList_.empty()) {
		xEnd = 1024.0;
		yEnd = 768.0;
	} else {
		xEnd = 2.0 * MARGIN + labelWidth_ + eventList_->list().back()->time * timeScale_;
		yEnd = 2.0 * TRACK_HEIGHT + (MARGIN + GRAPH_HEIGHT) * selectedParamList_.size();
	}
	totalWidth_ = std::ceil(xEnd + 3.0 * MARGIN);
	if (totalWidth_ < MININUM_WIDTH) {
		totalWidth_ = MININUM_WIDTH;
	}
	totalHeight_ = std::ceil(yEnd + MARGIN);
	if (totalHeight_ < MININUM_HEIGHT) {
		totalHeight_ = MININUM_HEIGHT;
	}
	setMinimumWidth(totalWidth_);
	setMinimumHeight(totalHeight_);

	if (!selectedParamList_.empty()) {

		postureTimeList_.clear();

		double y = MARGIN + 2.0 * TRACK_HEIGHT - 0.5 * (TRACK_HEIGHT - 1.0) + textYOffset_;
		unsigned int postureIndex = 0;
		for (const VTMControlModel::Event_ptr& ev : eventList_->list()) {
			double x = 2.0 * MARGIN + labelWidth_ + ev->time * timeScale_;
			if (ev->flag) {
				postureTimeList_.push_back(ev->time);
				painter.setPen(Qt::black);
				const VTMControlModel::Posture* posture = eventList_->getPostureAtIndex(postureIndex++);
				if (posture) {
					// Posture name.
					painter.drawText(QPointF(x, y), posture->name().c_str());
				}
				// Event vertical line.
				painter.drawLine(QPointF(x, MARGIN + 2.0 * TRACK_HEIGHT), QPointF(x, yEnd));
			} else {
				painter.setPen(Qt::lightGray);
				// Event vertical line.
				painter.drawLine(QPointF(x, MARGIN + 2.0 * TRACK_HEIGHT), QPointF(x, yEnd));
			}
		}
		painter.setPen(Qt::black);

		double xRule = 2.0 * MARGIN + labelWidth_;
		double yRuleText = MARGIN + TRACK_HEIGHT - 0.5 * (TRACK_HEIGHT - 1.0) + textYOffset_;
		painter.drawText(QPointF(MARGIN, yRuleText), tr("Rule"));
		for (int i = 0; i < eventList_->numberOfRules(); ++i) {
			auto* ruleData = eventList_->getRuleAtIndex(i);
			if (ruleData) {
				unsigned int firstPosture = ruleData->firstPosture;
				unsigned int lastPosture = ruleData->lastPosture;

				int postureTime1, postureTime2;
				if (firstPosture < postureTimeList_.size()) {
					postureTime1 = postureTimeList_[firstPosture];
				} else {
					postureTime1 = 0; // invalid
				}
				if (lastPosture < postureTimeList_.size()) {
					postureTime2 = postureTimeList_[lastPosture];
				} else {
					postureTime2 = postureTime1 + ruleData->duration;
				}

				// Rule frame.
				painter.drawRect(QRectF(
						QPointF(xRule + postureTime1 * timeScale_, MARGIN),
						QPointF(xRule + postureTime2 * timeScale_, MARGIN + TRACK_HEIGHT)));
				// Rule number.
				painter.drawText(QPointF(xRule + postureTime1 * timeScale_ + TEXT_MARGIN, yRuleText), QString::number(ruleData->number));
			}
		}
	}

	QPen pen;
	QPen pen2;
	pen2.setWidth(2);

	for (unsigned int i = 0; i < selectedParamList_.size(); ++i) {
		unsigned int paramIndex = selectedParamList_[i];
		unsigned int modelParamIndex;
		if (paramIndex >= NUM_PARAM) { // special
			modelParamIndex = paramIndex - NUM_PARAM;
		} else {
			modelParamIndex = paramIndex;
		}
		double currentMin = model_->parameterList()[modelParamIndex].minimum();
		double currentMax = model_->parameterList()[modelParamIndex].maximum();

		double yBase = 2.0 * TRACK_HEIGHT + (MARGIN + GRAPH_HEIGHT) * (i + 1U);

		// Label.
		painter.drawText(QPointF(MARGIN, yBase - 0.5 * GRAPH_HEIGHT + textYOffset_), model_->parameterList()[modelParamIndex].name().c_str());
		if (paramIndex >= NUM_PARAM) {
			painter.drawText(QPointF(MARGIN, yBase - 0.5 * GRAPH_HEIGHT + textYOffset_ + TRACK_HEIGHT), SPECIAL_STRING);
		}
		// Limits.
		painter.drawText(QPointF(MARGIN, yBase), QString("%1").arg(currentMin, maxLabelSize_));
		painter.drawText(QPointF(MARGIN, yBase - GRAPH_HEIGHT + textAscent_), QString("%1").arg(currentMax, maxLabelSize_));

		// Graph frame.
		painter.drawLine(QPointF(2.0 * MARGIN + labelWidth_, yBase - GRAPH_HEIGHT), QPointF(xEnd, yBase - GRAPH_HEIGHT));
		painter.drawLine(QPointF(2.0 * MARGIN + labelWidth_, yBase)               , QPointF(xEnd, yBase));
		painter.drawLine(QPointF(2.0 * MARGIN + labelWidth_, yBase - GRAPH_HEIGHT), QPointF(2.0 * MARGIN + labelWidth_, yBase));
		painter.drawLine(QPointF(xEnd, yBase - GRAPH_HEIGHT)                      , QPointF(xEnd, yBase));

		// Graph curve.
		painter.setPen(pen2);
		painter.setRenderHint(QPainter::Antialiasing);
		QPointF prevPoint;
		const double valueFactor = 1.0 / (currentMax - currentMin);
		for (const VTMControlModel::Event_ptr& ev : eventList_->list()) {
			double x = 0.5 + 2.0 * MARGIN + labelWidth_ + ev->time * timeScale_; // 0.5 added because of antialiasing
			double value = ev->getParameter(paramIndex);
			if (value != VTMControlModel::Event::EMPTY_PARAMETER) {
				double y = 0.5 + yBase - (value - currentMin) * valueFactor * GRAPH_HEIGHT; // 0.5 added because of antialiasing
				if (!prevPoint.isNull()) {
					painter.drawLine(prevPoint, QPointF(x, y));
				}
				prevPoint.setX(x);
				prevPoint.setY(y);
			}
		}
		painter.setRenderHint(QPainter::Antialiasing, false);
		painter.setPen(pen);
	}
}

void
EventWidget::mouseMoveEvent(QMouseEvent* event)
{
	double time = -1.0;
	double value = 0.0;

	if (model_ == nullptr || eventList_ == nullptr || eventList_->list().empty() || selectedParamList_.empty()) {
		emit mouseMoved(time, value);
		return;
	}

	double x = event->x();
	double y = event->y();
	double xStart = 2.0 * MARGIN + labelWidth_;
	double xEnd = xStart + eventList_->list().back()->time * timeScale_;

	if (x < xStart || x > xEnd) {
		emit mouseMoved(time, value);
		return;
	}

	for (unsigned int i = 0; i < selectedParamList_.size(); ++i) {
		double yStart = 2.0 * TRACK_HEIGHT + (MARGIN + GRAPH_HEIGHT) * (i + 1U);
		double yEnd = yStart - GRAPH_HEIGHT;
		if (yEnd <= y && y <= yStart) {
			unsigned int paramIndex = selectedParamList_[i];
			unsigned int modelParamIndex;
			if (paramIndex >= NUM_PARAM) { // special
				modelParamIndex = paramIndex - NUM_PARAM;
			} else {
				modelParamIndex = paramIndex;
			}
			double currentMin = model_->parameterList()[modelParamIndex].minimum();
			double currentMax = model_->parameterList()[modelParamIndex].maximum();
			value = ((yStart - y) / GRAPH_HEIGHT) * (currentMax - currentMin) + currentMin;

			time = (x - xStart) / timeScale_;

			emit mouseMoved(time, value);
			return;
		}
	}

	emit mouseMoved(time, value);
}

QSize
EventWidget::sizeHint() const
{
	return QSize(totalWidth_, totalHeight_);
}

void
EventWidget::updateData(VTMControlModel::EventList* eventList, VTMControlModel::Model* model)
{
	eventList_ = eventList;
	model_ = model;

	if (model_ && model_->parameterList().size() != NUM_PARAM) {
		qWarning("[EventWidget::updateData] Wrong number of parameters: %lu (should be %d).", model_->parameterList().size(), NUM_PARAM);
		model_ = nullptr;
		eventList_ = nullptr;
		return;
	}

	modelUpdated_ = true;

	update();
}

void
EventWidget::clearParameterSelection()
{
	selectedParamList_.clear();
	update();
}

void
EventWidget::changeParameterSelection(unsigned int paramIndex, bool special, bool selected)
{
	if (special) {
		paramIndex += NUM_PARAM;
	}

	if (selected) {
		selectedParamList_.push_back(paramIndex);
	} else {
		for (auto iter = selectedParamList_.begin(); iter != selectedParamList_.end(); ++iter) {
			if (*iter == paramIndex) {
				selectedParamList_.erase(iter);
				break;
			}
		}
	}
	emit mouseMoved(-1.0, 0.0);
	update();
}

} // namespace GS

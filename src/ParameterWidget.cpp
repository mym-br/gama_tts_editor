/***************************************************************************
 *  Copyright 2014, 2015, 2017 Marcelo Y. Matuda                           *
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

#include "ParameterWidget.h"

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
#define DEFAULT_GRAPH_HEIGHT 120.0
#define MININUM_WIDTH 1024
#define MININUM_HEIGHT 768
#define TEXT_MARGIN 5.0
#define DEFAULT_TIME_SCALE 0.7
#define POINT_RADIUS 2.0



namespace GS {

ParameterWidget::ParameterWidget(QWidget* parent)
		: QWidget{parent}
		, eventList_{}
		, model_{}
		, timeScale_{DEFAULT_TIME_SCALE}
		, graphHeight_{DEFAULT_GRAPH_HEIGHT}
		, modelUpdated_{}
		, labelWidth_{}
		, maxLabelSize_{}
		, totalWidth_{MININUM_WIDTH}
		, totalHeight_{MININUM_HEIGHT}
		, verticalScrollbarValue_{}
		, horizontalScrollbarValue_{}
		, textTotalHeight_{}
{
	setMinimumWidth(totalWidth_);
	setMinimumHeight(totalHeight_);

	setMouseTracking(true);
}

// Note: with no antialiasing, the coordinates in QPointF are rounded to the nearest integer.
void
ParameterWidget::paintEvent(QPaintEvent* /*event*/)
{
	if (eventList_ == nullptr || eventList_->list().empty()) {
		return;
	}

	QPainter painter(this);
	painter.setFont(QFont("monospace"));
	const QFontMetrics fm = painter.fontMetrics();
	const int fontAscent = fm.ascent();
	const int fontLeading = fm.leading();
	const int textYOffset = fontAscent + fontLeading + 1;
	textTotalHeight_ = fm.lineSpacing() + fontLeading + 1;

	if (modelUpdated_) {
		int maxWidth = 0;
		for (unsigned int i = 0; i < model_->parameterList().size(); ++i) {
			const unsigned int labelSize = model_->parameterList()[i].name().size();
			if (labelSize > maxLabelSize_) {
				maxLabelSize_ = labelSize;
			}
			const int width = fm.width(model_->parameterList()[i].name().c_str());
			if (width > maxWidth) {
				maxWidth = width;
			}
		}
		labelWidth_ = maxWidth;

		modelUpdated_ = false;
	}

	const double xBase = 3.0 * MARGIN + labelWidth_;
	double xEnd, yEnd;
	if (selectedParamList_.empty()) {
		xEnd = MININUM_WIDTH;
		yEnd = MININUM_HEIGHT;
	} else {
		xEnd = xBase + eventList_->list().back()->time * timeScale_;
		yEnd = getGraphBaseY(selectedParamList_.size() - 1U);
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

	const double headerBottomY = 2.0 * textTotalHeight_ + MARGIN;
	const QPalette pal;

	if (!selectedParamList_.empty()) {

		postureTimeList_.clear();

		painter.setPen(Qt::black);

		const double yPosture = MARGIN + textTotalHeight_ + textYOffset + verticalScrollbarValue_;
		unsigned int postureIndex = 0;
		for (const VTMControlModel::Event_ptr& ev : eventList_->list()) {
			const double x = xBase + ev->time * timeScale_;
			if (ev->flag) {
				postureTimeList_.push_back(ev->time);
				const VTMControlModel::Posture* posture = eventList_->getPostureAtIndex(postureIndex++);
				if (posture) {
					// Posture name.
					painter.drawText(QPointF(x, yPosture), posture->name().c_str());
				}
				// Event vertical lines.
				for (unsigned int i = 0; i < selectedParamList_.size(); ++i) {
					const double yBase = getGraphBaseY(i);
					const double yTop = yBase - graphHeight_;
					if (yTop - verticalScrollbarValue_ < headerBottomY) {
						continue;
					}
					painter.drawLine(QPointF(x, yTop), QPointF(x, yBase));
				}
			}
		}

		const double yRuleText = MARGIN + textYOffset + verticalScrollbarValue_;

		for (int i = 0; i < eventList_->numberOfRules(); ++i) {
			const auto* ruleData = eventList_->getRuleAtIndex(i);
			if (ruleData) {
				const unsigned int firstPosture = ruleData->firstPosture;
				const unsigned int lastPosture = ruleData->lastPosture;

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

				const double xPost1 = xBase + postureTime1 * timeScale_;
				const double xPost2 = xBase + postureTime2 * timeScale_;
				// Rule frame.
				painter.drawRect(QRectF(
						QPointF(xPost1, MARGIN + verticalScrollbarValue_),
						QPointF(xPost2, MARGIN + textTotalHeight_ + verticalScrollbarValue_)));
				// Rule number.
				painter.drawText(QPointF(xPost1 + TEXT_MARGIN, yRuleText), QString::number(ruleData->number));
			}
		}

		// Background for "Rule" label.
		painter.fillRect(QRectF(
					QPointF(horizontalScrollbarValue_                 , verticalScrollbarValue_),
					QPointF(xBase - MARGIN + horizontalScrollbarValue_, headerBottomY + graphHeight_ + MARGIN + verticalScrollbarValue_)
					), pal.window());

		QString ruleLabel = tr("Rule");
		painter.drawText(QPointF(MARGIN + (labelWidth_ - fm.width(ruleLabel)) + horizontalScrollbarValue_, yRuleText), ruleLabel);
	}

	const QPen pen;
	QPen pen2;
	pen2.setWidth(2);

	const unsigned int numParameters = model_->parameterList().size();
	const double xText = MARGIN + horizontalScrollbarValue_;

	for (unsigned int i = 0; i < selectedParamList_.size(); ++i) {
		const double yBase = getGraphBaseY(i);
		const double yTop = yBase - graphHeight_;
		if (yTop < headerBottomY + verticalScrollbarValue_) {
			continue;
		}

		const unsigned int paramIndex = selectedParamList_[i];
		const double currentMin = model_->parameterList()[paramIndex].minimum();
		const double currentMax = model_->parameterList()[paramIndex].maximum();

		// Graph frame.
		painter.drawLine(QPointF(xBase, yTop) , QPointF(xEnd , yTop));
		painter.drawLine(QPointF(xBase, yBase), QPointF(xEnd , yBase));
		painter.drawLine(QPointF(xBase, yTop) , QPointF(xBase, yBase));
		painter.drawLine(QPointF(xEnd , yTop) , QPointF(xEnd , yBase));

		// Graph curve.
		painter.setPen(pen2);
		painter.setRenderHint(QPainter::Antialiasing);
		QPointF prevPoint;
		const double valueFactor = 1.0 / (currentMax - currentMin);
		// Normal events.
		for (const VTMControlModel::Event_ptr& ev : eventList_->list()) {
			const double x = 0.5 + xBase + ev->time * timeScale_; // 0.5 added because of antialiasing
			const double value = ev->getParameter(paramIndex);
			if (value != VTMControlModel::Event::EMPTY_PARAMETER) {
				const double y = 0.5 + yBase - (value - currentMin) * valueFactor * graphHeight_; // 0.5 added because of antialiasing
				const QPointF point(x, y);
				if (!prevPoint.isNull()) {
					painter.drawLine(prevPoint, point);
				}
				painter.drawEllipse(point, POINT_RADIUS, POINT_RADIUS);
				prevPoint.setX(x);
				prevPoint.setY(y);
			}
		}
		prevPoint.setX(0.0); prevPoint.setY(0.0);
		// Special events.
		pen2.setColor(Qt::red);
		painter.setPen(pen2);
		for (const VTMControlModel::Event_ptr& ev : eventList_->list()) {
			const double x = 0.5 + xBase + ev->time * timeScale_; // 0.5 added because of antialiasing
			const double value = ev->getParameter(paramIndex + numParameters);
			if (value != VTMControlModel::Event::EMPTY_PARAMETER) {
				const double y = 0.5 + yBase - (value - currentMin) * valueFactor * graphHeight_; // 0.5 added because of antialiasing
				const QPointF point(x, y);
				if (!prevPoint.isNull()) {
					painter.drawLine(prevPoint, point);
				}
				painter.drawEllipse(point, POINT_RADIUS, POINT_RADIUS);
				prevPoint.setX(x);
				prevPoint.setY(y);
			}
		}
		pen2.setColor(Qt::black);
		painter.setPen(pen2);

		painter.setRenderHint(QPainter::Antialiasing, false);
		painter.setPen(pen);

		// Background for labels and limits.
		painter.fillRect(QRectF(
					QPointF(horizontalScrollbarValue_                 , yTop  - MARGIN),
					QPointF(xBase - MARGIN + horizontalScrollbarValue_, yBase + MARGIN)
					), pal.window());

		// Label.
		painter.drawText(QPointF(xText, yBase - 0.5 * graphHeight_), model_->parameterList()[paramIndex].name().c_str());
		// Limits.
		painter.drawText(QPointF(xText, yBase)                            , QString("%1").arg(currentMin, maxLabelSize_));
		painter.drawText(QPointF(xText, yBase - graphHeight_ + fontAscent), QString("%1").arg(currentMax, maxLabelSize_));
	}
}

void
ParameterWidget::mouseMoveEvent(QMouseEvent* event)
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
		double yStart = getGraphBaseY(i);
		double yEnd = yStart - graphHeight_;
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
			value = ((yStart - y) / graphHeight_) * (currentMax - currentMin) + currentMin;

			time = (x - xStart) / timeScale_;

			emit mouseMoved(time, value);
			return;
		}
	}

	emit mouseMoved(time, value);
}

void
ParameterWidget::mouseDoubleClickEvent(QMouseEvent* /*event*/)
{
	// Reset zoom.
	timeScale_ = DEFAULT_TIME_SCALE;
	graphHeight_ = DEFAULT_GRAPH_HEIGHT;

	update();

	emit zoomReset();
}

double
ParameterWidget::getGraphBaseY(unsigned int index)
{
	return 2.0 * textTotalHeight_ + (MARGIN + graphHeight_) * (index + 1U);
}

QSize
ParameterWidget::sizeHint() const
{
	return QSize(totalWidth_, totalHeight_);
}

void
ParameterWidget::updateData(VTMControlModel::EventList* eventList, VTMControlModel::Model* model)
{
	eventList_ = eventList;
	model_ = model;

	if (model_ && model_->parameterList().size() != NUM_PARAM) {
		THROW_EXCEPTION(InvalidValueException, "[ParameterWidget::updateData] Wrong number of parameters: " <<
				model_->parameterList().size() << " (should be " << NUM_PARAM << ").");
		model_ = nullptr;
		eventList_ = nullptr;
		return;
	}

	modelUpdated_ = true;

	selectedParamList_.clear();

	update();
}

void
ParameterWidget::changeParameterSelection(unsigned int paramIndex, bool selected)
{
	if (selected) {
		if (paramIndex < model_->parameterList().size()) {
			selectedParamList_.push_back(paramIndex);
		}
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

void
ParameterWidget::changeXZoom(double zoom)
{
	zoom = qBound(xZoomMin(), zoom, xZoomMax());
	timeScale_ = DEFAULT_TIME_SCALE * zoom;

	update();
}

void
ParameterWidget::changeYZoom(double zoom)
{
	zoom = qBound(yZoomMin(), zoom, yZoomMax());
	graphHeight_ = DEFAULT_GRAPH_HEIGHT * zoom;

	update();
}

void
ParameterWidget::getVerticalScrollbarValue(int value)
{
	verticalScrollbarValue_ = value;
}

void
ParameterWidget::getHorizontalScrollbarValue(int value)
{
	horizontalScrollbarValue_ = value;
}

} // namespace GS

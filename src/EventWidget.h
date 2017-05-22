/***************************************************************************
 *  Copyright 2014, 2015, 2017 Marcelo Y. Matuda                           *
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

#ifndef EVENT_WIDGET_H
#define EVENT_WIDGET_H

#include <vector>

#include <QWidget>



namespace GS {

namespace VTMControlModel {
class Model;
class EventList;
}

class EventWidget : public QWidget {
	Q_OBJECT
public:
	explicit EventWidget(QWidget* parent=0);

	virtual QSize sizeHint() const;
	void updateData(VTMControlModel::EventList* eventList, VTMControlModel::Model* model);
	void changeParameterSelection(unsigned int paramIndex, bool selected);
	double xZoomMin() const { return 0.1; }
	double xZoomMax() const { return 10.0; }
	double yZoomMin() const { return 0.1; }
	double yZoomMax() const { return 10.0; }
	void changeXZoom(double zoom);
	void changeYZoom(double zoom);
signals:
	void mouseMoved(double time, double value);
	void zoomReset();
protected:
	virtual void paintEvent(QPaintEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void mouseDoubleClickEvent(QMouseEvent *event);
private:
	enum {
		NUM_PARAM = 16 // hardcoded
	};

	const VTMControlModel::EventList* eventList_;
	const VTMControlModel::Model* model_;
	double timeScale_;
	double graphHeight_;
	bool modelUpdated_;
	double textAscent_;
	double textYOffset_;
	double labelWidth_;
	unsigned int maxLabelSize_;
	int totalWidth_;
	int totalHeight_;
	std::vector<unsigned int> selectedParamList_;
	std::vector<int> postureTimeList_;
};

} // namespace GS

#endif // EVENT_WIDGET_H

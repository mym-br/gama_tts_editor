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

#ifndef EVENT_WIDGET_H
#define EVENT_WIDGET_H

#include <vector>

#include <QWidget>



namespace GS {

namespace TRMControlModel {
class Model;
class EventList;
}

class EventWidget : public QWidget {
	Q_OBJECT
public:
	explicit EventWidget(QWidget* parent=0);

	virtual QSize sizeHint() const;
	void updateData(TRMControlModel::EventList* eventList, TRMControlModel::Model* model);
	void clearParameterSelection();
	void changeParameterSelection(unsigned int paramIndex, bool special, bool selected);
protected:
	virtual void paintEvent(QPaintEvent*);
private:
	enum {
		NUM_PARAM = 16
	};

	const TRMControlModel::EventList* eventList_;
	const TRMControlModel::Model* model_;
	double timeScale_;
	bool modelUpdated_;
	double textAscent_;
	double textYOffset_;
	double labelWidth_;
	int totalWidth_;
	int totalHeight_;
	std::vector<unsigned int> selectedParamList_;
	std::vector<int> postureTimeList_;
};

} // namespace GS

#endif // EVENT_WIDGET_H

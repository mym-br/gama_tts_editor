/***************************************************************************
 *  Copyright 2017 Marcelo Y. Matuda                                       *
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

#ifndef PARAMETER_MODIFICATION_WINDOW_H
#define PARAMETER_MODIFICATION_WINDOW_H

#include <memory>

#include <QTimer>
#include <QVector>
#include <QWidget>

namespace Ui {
class ParameterModificationWindow;
}

namespace GS {

struct Synthesis;
namespace VTMControlModel {
class Model;
}

class ParameterModificationWindow : public QWidget {
	Q_OBJECT
public:
	explicit ParameterModificationWindow(QWidget* parent=nullptr);
	~ParameterModificationWindow();

	void clear();
	void setup(VTMControlModel::Model* model, Synthesis* synthesis);
public slots:
	void resetData();
private slots:
	void on_resetParameterButton_clicked();
	void on_synthesizeButton_clicked();
	void on_synthesizeToFileButton_clicked();
	void on_parameterComboBox_currentIndexChanged(int index);
	void on_addRadioButton_toggled(bool checked);
	void handleModificationStarted();
	void handleOffsetChanged(double offset);
	void sendModificationValue();
private:
	enum {
		MODIF_TIMER_INTERVAL_MS = 2
	};
	enum class State {
		stopped,
		running
	};

	void showModifiedParameterData();

	std::unique_ptr<Ui::ParameterModificationWindow> ui_;
	VTMControlModel::Model* model_;
	Synthesis* synthesis_;
	double prevAmplitude_;
	State state_;
	double modificationValue_;
	QTimer modificationTimer_;
	QVector<double> paramY_;
	QVector<double> modifParamX_;
	QVector<double> modifParamY_;
};

} // namespace GS

#endif // PARAMETER_MODIFICATION_WINDOW_H

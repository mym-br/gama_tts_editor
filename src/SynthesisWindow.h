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

#ifndef SYNTHESIS_WINDOW_H
#define SYNTHESIS_WINDOW_H

#include <memory>

#include <QString>
#include <QThread>
#include <QWidget>



namespace Ui {
class SynthesisWindow;
}

namespace GS {

struct Synthesis;
namespace VTMControlModel {
class Model;
}
class AudioWorker;

class SynthesisWindow : public QWidget {
	Q_OBJECT
public:
	explicit SynthesisWindow(QWidget* parent=0);
	~SynthesisWindow();

	void clear();
	void setup(VTMControlModel::Model* model, Synthesis* synthesis);
signals:
	void textSynthesized();
	void playAudioRequested(double sampleRate, int outputDeviceIndex);
	void updateAudioDeviceComboBoxRequested();
	void audioFinished();
	void audioStarted();
	void synthesisFinished();
public slots:
	void setupParameterTable();
	void synthesizeWithManualIntonation();
	void synthesizeToFileWithManualIntonation(QString filePath);
private slots:
	void on_parseButton_clicked();
	void on_synthesizeButton_clicked();
	void on_synthesizeToFileButton_clicked();
	void on_parameterTableWidget_cellChanged(int row, int column);
	void on_xZoomSpinBox_valueChanged(double d);
	void on_yZoomSpinBox_valueChanged(double d);
	void updateMouseTracking(double time, double value);
	void handleAudioError(QString msg);
	void handleAudioFinished();
	void updateAudioDeviceComboBox(QStringList deviceNameList, int defaultDeviceIndex);
	void resetZoom();
private:
	enum {
		NUM_PARAM = 16 // hardcoded
	};

	void enableProcessingButtons();
	void disableProcessingButtons();

	std::unique_ptr<Ui::SynthesisWindow> ui_;
	VTMControlModel::Model* model_;
	Synthesis* synthesis_;
	QThread audioThread_;
	AudioWorker* audioWorker_;
};

} // namespace GS

#endif // SYNTHESIS_WINDOW_H

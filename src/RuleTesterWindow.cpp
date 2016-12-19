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

#include "RuleTesterWindow.h"

#include <QMessageBox>

#include "Model.h"
#include "ui_RuleTesterWindow.h"



namespace GS {

RuleTesterWindow::RuleTesterWindow(QWidget* parent)
		: QWidget(parent)
		, ui_(new Ui::RuleTesterWindow)
		, model_(nullptr)
{
	ui_->setupUi(this);
}

RuleTesterWindow::~RuleTesterWindow()
{
}

void
RuleTesterWindow::resetModel(VTMControlModel::Model* model)
{
	model_ = model;
}

void
RuleTesterWindow::on_shiftButton_clicked()
{
	ui_->posture1LineEdit->setText(ui_->posture2LineEdit->text());
	ui_->posture2LineEdit->setText(ui_->posture3LineEdit->text());
	ui_->posture3LineEdit->setText(ui_->posture4LineEdit->text());
	ui_->posture4LineEdit->clear();
}

void
RuleTesterWindow::on_testButton_clicked()
{
	if (model_ == nullptr) return;

	std::vector<const VTMControlModel::Posture*> postureSequence;

	QString posture1Text = ui_->posture1LineEdit->text().trimmed();
	if (!posture1Text.isEmpty()) {
		const VTMControlModel::Posture* posture = model_->postureList().find(posture1Text.toStdString());
		if (posture == nullptr) {
			clearResults();
			QMessageBox::warning(this, tr("Warning"), tr("Posture 1 not found."));
			return;
		} else {
			postureSequence.push_back(posture);
		}
	} else {
		return;
	}

	QString posture2Text = ui_->posture2LineEdit->text().trimmed();
	if (!posture2Text.isEmpty()) {
		const VTMControlModel::Posture* posture = model_->postureList().find(posture2Text.toStdString());
		if (posture == nullptr) {
			clearResults();
			QMessageBox::warning(this, tr("Warning"), tr("Posture 2 not found."));
			return;
		} else {
			postureSequence.push_back(posture);
		}
	} else {
		return;
	}

	QString posture3Text = ui_->posture3LineEdit->text().trimmed();
	if (!posture3Text.isEmpty()) {
		const VTMControlModel::Posture* posture = model_->postureList().find(posture3Text.toStdString());
		if (posture == nullptr) {
			clearResults();
			QMessageBox::warning(this, tr("Warning"), tr("Posture 3 not found."));
			return;
		} else {
			postureSequence.push_back(posture);
		}

		QString posture4Text = ui_->posture4LineEdit->text().trimmed();
		if (!posture4Text.isEmpty()) {
			const VTMControlModel::Posture* posture4 = model_->postureList().find(posture4Text.toStdString());
			if (posture4 == nullptr) {
				clearResults();
				QMessageBox::warning(this, tr("Warning"), tr("Posture 4 not found."));
				return;
			} else {
				postureSequence.push_back(posture4);
			}
		}
	}

	unsigned int ruleIndex;
	const VTMControlModel::Rule* rule = model_->findFirstMatchingRule(postureSequence, ruleIndex);
	if (rule == nullptr) {
		clearResults();
		QMessageBox::critical(this, tr("Error"), tr("Could not find a matching rule."));
		return;
	}

	QString ruleText = QString("%1. ").arg(ruleIndex + 1U);
	for (unsigned int i = 0, size = rule->booleanExpressionList().size(); i < size; ++i) {
		if (i > 0) {
			ruleText += " >> ";
		}
		ruleText += rule->booleanExpressionList()[i].c_str();
	}
	ui_->ruleLineEdit->setText(ruleText);

	ui_->consumedTokensLineEdit->setText(QString::number(rule->numberOfExpressions()));

	const double tempos[] = {1.0, 1.0, 1.0, 1.0};
	double ruleSymbols[5]; // {rd, beat, mark1, mark2, mark3}
	rule->evaluateExpressionSymbols(tempos, postureSequence, *model_, ruleSymbols);

	ui_->durationLineEdit->setText(QString::number(ruleSymbols[0]));
	ui_->beatLineEdit->setText(    QString::number(ruleSymbols[1]));
	ui_->mark1LineEdit->setText(   QString::number(ruleSymbols[2]));
	ui_->mark2LineEdit->setText(   QString::number(ruleSymbols[3]));
	ui_->mark3LineEdit->setText(   QString::number(ruleSymbols[4]));
}

void
RuleTesterWindow::clearResults()
{
	ui_->ruleLineEdit->clear();
	ui_->consumedTokensLineEdit->clear();
	ui_->durationLineEdit->clear();
	ui_->beatLineEdit->clear();
	ui_->mark1LineEdit->clear();
	ui_->mark2LineEdit->clear();
	ui_->mark3LineEdit->clear();
}

} // namespace GS

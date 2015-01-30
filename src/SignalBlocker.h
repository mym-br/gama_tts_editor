// This file is in the public domain.

#ifndef SIGNAL_BLOCKER_H
#define SIGNAL_BLOCKER_H

#include <QObject>



class SignalBlocker {
public:
	SignalBlocker(QObject* obj) : obj_(obj), previousSignalsBlocked_(obj->blockSignals(true)) { }
	~SignalBlocker() { obj_->blockSignals(previousSignalsBlocked_); }

private:
	SignalBlocker(const SignalBlocker&) = delete;
	SignalBlocker& operator=(const SignalBlocker&) = delete;

	QObject* obj_;
	bool previousSignalsBlocked_;
};

#endif // SIGNAL_BLOCKER_H

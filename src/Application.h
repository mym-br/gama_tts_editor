// This file is in the public domain.

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QString>



class Application : public QApplication {
	Q_OBJECT
public:
	Application(int& argc, char** argv);

	virtual bool notify(QObject* receiver, QEvent* e);
};

#endif // APPLICATION_H

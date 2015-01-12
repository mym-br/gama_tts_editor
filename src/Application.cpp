// This file is in the public domain.

#include "Application.h"

#include <cstdio>
#include <exception>
#include <new> /* bad_alloc */



Application::Application(int& argc, char** argv)
		: QApplication(argc, argv)
{
}

bool
Application::notify(QObject* receiver, QEvent* e)
{
	try {
		return QApplication::notify(receiver, e);
	} catch (const std::bad_alloc&) {
		fprintf(stderr, "Out of memory during execution of event handler.\n");
		std::terminate();
	} catch (const std::exception& exc) {
		fprintf(stderr, "Exception thrown in event handler: %s\n", exc.what());
		std::terminate();
	} catch (...) {
		fprintf(stderr, "Unknown exception thrown in event handler.\n");
		std::terminate();
	}
}

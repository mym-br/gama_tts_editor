// This file is in the public domain.
//
// Modified version of the code in:
// http://www.qtforum.org/article/39768/redirecting-std-cout-std-cerf-qdebug-to-qtextedit.html

#ifndef LOG_STREAM_BUFFER_H
#define LOG_STREAM_BUFFER_H

#include <cstdio>
#include <exception> /* terminate */
#include <iostream>
#include <streambuf>

#include <QPlainTextEdit>
#include <QString>

#include "Log.h"



namespace GS {

class LogStreamBuffer : public std::streambuf {
public:
	LogStreamBuffer(std::ostream& stream, QPlainTextEdit* textEdit)
			: stream_(stream)
			, logWidget_(textEdit)
			, origBuf_(stream_.rdbuf(this))
			, streamIsStderr_(stream == std::cerr)
	{
	}

	~LogStreamBuffer() {
		stream_.rdbuf(origBuf_);
	}

	static void registerQDebugMessageHandler() {
		qInstallMsgHandler(logQDebugMessageHandler);
	}

private:
	static void logQDebugMessageHandler(QtMsgType type, const char* msg) {
		switch (type) {
		case QtDebugMsg:
			std::cout << msg << std::endl;
			break;
		case QtWarningMsg:
			std::cerr << "Warning: " << msg << std::endl;
			break;
		case QtCriticalMsg:
			std::cerr << "Critical error: " << msg << std::endl;
			break;
		case QtFatalMsg:
			fprintf(stderr, "Fatal error: %s\n", msg);
			std::terminate();
		}
	}

protected:
	virtual int_type overflow(int_type ch) {
		if (ch == '\n') {
			logWidget_->appendPlainText("");
		}
		return ch;
	}

	virtual std::streamsize xsputn(const char* p, std::streamsize n) {
		QString str = QString::fromUtf8(p, n); // p does not always point to a C string
		logWidget_->moveCursor(QTextCursor::End);

		QTextCharFormat tcf;
		if (streamIsStderr_) {
			tcf = logWidget_->currentCharFormat();
			tcf.setForeground(QBrush(Qt::red));
			logWidget_->setCurrentCharFormat(tcf);
		}

		if (str.contains('\n')){
			QStringList strList = str.split('\n');
			logWidget_->insertPlainText(strList[0]); // index 0 is still on the same old line
			for (int i = 1; i < strList.size(); i++) {
				logWidget_->appendPlainText(strList[i]);
			}
		} else {
			logWidget_->insertPlainText(str);
		}

		if (streamIsStderr_) {
			tcf.setForeground(QBrush());
			logWidget_->setCurrentCharFormat(tcf);
		}

		return n;
	}

private:
	std::ostream& stream_;
	QPlainTextEdit* logWidget_;
	std::streambuf* origBuf_;
	bool streamIsStderr_;
};

} // namespace GS

#endif // LOG_STREAM_BUFFER_H

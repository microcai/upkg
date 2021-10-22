#ifdef _WIN32
#define QT_QPA_PLATFORM_WINDOWS
#endif

#ifdef QT_STATICPLUGIN
#include <QtPlugin>
Q_IMPORT_PLUGIN(QICOPlugin);

#if defined(QT_QPA_PLATFORM_XCB)
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin);
#elif defined(QT_QPA_PLATFORM_WINDOWS)
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
Q_IMPORT_PLUGIN(QWindowsVistaStylePlugin);
#elif defined(QT_QPA_PLATFORM_COCOA)
Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin);
#endif

#endif

#include <QApplication>
#include <QtWidgets>
#include <QCommandLineParser>

#include <QCoreApplication>
#include <QFile>

#include <thread>

#include <boost/regex.hpp>
#include <boost/asio/io_context.hpp>

#include "upkg/upkg.hpp"


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	upkg w;
	w.show();

	return a.exec();
}

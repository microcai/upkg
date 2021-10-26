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

QFont* global_default_font = nullptr;

int main(int argc, char *argv[])
{
	QCoreApplication::setOrganizationName(QString("Daimage"));
	QCoreApplication::setApplicationName(QString("Upkg"));

	QApplication a(argc, argv);

#if defined(WIN32)
 	std::unique_ptr<QFont> default_font_ptr(new QFont("Consolas", 12));
 	global_default_font = default_font_ptr.get();
	a.setFont(*default_font_ptr);
#else
	std::unique_ptr<QFont> default_font_ptr(new QFont());
	default_font_ptr->setFamily(default_font_ptr->defaultFamily());
	a.setFont(*default_font_ptr);
	global_default_font = default_font_ptr.get();
#endif

	upkg w;
	w.show();

	return a.exec();
}

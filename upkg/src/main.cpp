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

#include "upkg/upkg.hpp"

QFont* globalDefaultFont = nullptr;
upkg* mainWindow = nullptr;

int main(int argc, char *argv[])
{
	QCoreApplication::setOrganizationName(QStringLiteral("Daimage"));
	QCoreApplication::setApplicationName(QStringLiteral("Upkg"));

	QApplication a(argc, argv);

    QApplication::setWindowIcon(QIcon(":///upkg.ico"));

#if defined(WIN32)
 	std::unique_ptr<QFont> default_font_ptr(new QFont("Consolas", 12));
 	globalDefaultFont = default_font_ptr.get();
	a.setFont(*default_font_ptr);
#else
	std::unique_ptr<QFont> default_font_ptr(new QFont());
	default_font_ptr->setFamily(default_font_ptr->defaultFamily());
	a.setFont(*default_font_ptr);
	globalDefaultFont = default_font_ptr.get();
#endif

	upkg w;
	w.show();

	return a.exec();
}

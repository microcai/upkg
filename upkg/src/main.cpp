#ifdef _WIN32
#define QT_QPA_PLATFORM_WINDOWS
#endif

#ifdef QT_STATICPLUGIN
#include <QtPlugin>
Q_IMPORT_PLUGIN(QICOPlugin);

// Q_IMPORT_PLUGIN(QDirectFbIntegrationPlugin);
// Q_IMPORT_PLUGIN(QLinuxFbIntegrationPlugin);

#if defined(QT_QPA_PLATFORM_XCB)
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin);
#elif defined(QT_QPA_PLATFORM_WINDOWS)
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
Q_IMPORT_PLUGIN(QModernWindowsStylePlugin);
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

	QPalette darkPalette;
	darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
	darkPalette.setColor(QPalette::WindowText, Qt::white);
	darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
	darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
	darkPalette.setColor(QPalette::ToolTipBase, QColor(53, 53, 53));
	darkPalette.setColor(QPalette::ToolTipText, Qt::white);
	darkPalette.setColor(QPalette::Text, Qt::white);
	darkPalette.setColor(QPalette::PlaceholderText, QColor(127, 127, 127));
	darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
	darkPalette.setColor(QPalette::ButtonText, Qt::white);
	darkPalette.setColor(QPalette::BrightText, Qt::red);
	darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
	darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
	darkPalette.setColor(QPalette::HighlightedText, Qt::black);
	darkPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(164, 166, 168));
	darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(164, 166, 168));
	darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(164, 166, 168));
	darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(164, 166, 168));
	darkPalette.setColor(QPalette::Disabled, QPalette::Base, QColor(68, 68, 68));
	darkPalette.setColor(QPalette::Disabled, QPalette::Window, QColor(68, 68, 68));
	darkPalette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(68, 68, 68));

	a.setPalette(darkPalette);
	a.setStyle("Fusion");

#if defined(WIN32)
 	std::unique_ptr<QFont> default_font_ptr(new QFont("Consolas", 12));
 	globalDefaultFont = default_font_ptr.get();
	a.setFont(*default_font_ptr);
#else
	std::unique_ptr<QFont> default_font_ptr(new QFont("wqy-microhei", 12));
	default_font_ptr->setFamily(default_font_ptr->defaultFamily());
	a.setFont(*default_font_ptr);
	globalDefaultFont = default_font_ptr.get();
#endif

	upkg w;
	w.show();

	return a.exec();
}

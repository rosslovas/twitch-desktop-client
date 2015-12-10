#include "MainWindow.hpp"

#include <QApplication>
#include <QtQml>

#include <locale>

#include "MPVQuickItem.hpp"
#include "NetworkAccessManager.hpp"

int main(int argc, char * argv[])
{
	QApplication application{ argc, argv };
	application.setApplicationName("twitch-desktop-client");
	application.setApplicationVersion("0.1");
	application.setApplicationDisplayName(
	      application.applicationName() + " " + application.applicationVersion());

	// Destroy the global NetworkAccessManager before the QApplication to avoid errors.
	QObject::connect(&application, &QApplication::aboutToQuit, &NetworkAccessManager::Destroy);

	// LC_NUMERIC = C is required for mpv.
	std::setlocale(LC_NUMERIC, "C");

	// Register MPVQuickItem to make it accessible in QML.
	qmlRegisterType<MPVQuickItem>("twitch_desktop_client", 1, 0, "MPVItem");

	MainWindow mainWindow;
	mainWindow.show();

	return application.exec();
}

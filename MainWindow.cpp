#include "MainWindow.hpp"
#include "ui_MainWindow.h"

#include <QDir>
#include <QWebFrame>

#include "MPVQuickItem.hpp"
#include "StreamsDialog.hpp"
#include "TwitchAPI.hpp"

MainWindow::MainWindow(QWidget * parent /* = nullptr */)
    : QMainWindow(parent), ui(new Ui::MainWindow), streams{ new StreamsDialog{ this } }
{
	ui->setupUi(this);

	// Give the chat the same background as the BTTV dark theme.
	setStyleSheet("QWebView#chat { background-color: #1E1E1E }");

	auto webSettings = QWebSettings::globalSettings();
	webSettings->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
	webSettings->setAttribute(QWebSettings::JavascriptCanOpenWindows, true);
	webSettings->enablePersistentStorage(QDir("./storage").absolutePath());

	// Inject BetterTTV addon into the chat whenever it becomes ready.
	QObject::connect(ui->chat->page()->mainFrame(), &QWebFrame::javaScriptWindowObjectCleared, this,
	      &MainWindow::InjectBTTV);

	streams->setModal(true);
	streams->show();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::LoadChannel(const QString & channel)
{
	// Load the video,
	MPVQuickItem * mpv = qobject_cast<MPVQuickItem *>(ui->video->rootObject());
	mpv->LoadChannel(channel);

	// and the chat.
	if (channel.compare("destiny", Qt::CaseInsensitive) == 0) {
		ui->chat->load(QUrl("http://www.destiny.gg/embed/chat"));
	}
	else {
		ui->chat->load(
		      QUrl("http://twitch.tv/" + QUrl::toPercentEncoding(channel) + "/chat?popout=true"));
	}
}

void MainWindow::InjectBTTV()
{
	qDebug() << "javaScriptWindowObjectCleared" << ui->chat->url();
	if (ui->chat->url().host().compare("www.twitch.tv", Qt::CaseInsensitive) == 0) {
		qDebug() << "Injecting BTTV";
		ui->chat->page()->mainFrame()->evaluateJavaScript(
		      "window.onload = function() {"
		      "  var script = document.createElement('script');"
		      "  script.type = 'text/javascript';"
		      "  script.src = 'https://cdn.betterttv.net/betterttv.js?' + Math.random();"
		      "  var head = document.getElementsByTagName('head')[0];"
		      "  if (head != undefined) {"
		      "    head.appendChild(script);"
		      "  }"
		      "}");
	}
}

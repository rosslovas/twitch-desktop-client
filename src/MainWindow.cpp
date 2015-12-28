#include "MainWindow.hpp"
#include "ui_MainWindow.h"

#include <QDir>
#include <QKeySequence>
#include <QShortcut>
#include <QWebFrame>

#include "MPVQuickItem.hpp"
#include "StreamPreview.hpp"
#include "StreamsScrollArea.hpp"
#include "TwitchAPI.hpp"

MainWindow::MainWindow(QWidget * parent /* = nullptr */)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->scrollArea->SetStreamsGrid(ui->streamsGrid);

	setStyleSheet("StreamsScrollArea { background: #1C1C1C }"
	              "QWidget#scrollAreaWidgetContents { background: #1C1C1C }"
	              "QWidget#controls { background: #282828 }"
	              "QQuickWidget { background: #ff0000 }"
	              "QLabel {"
	              "	color: #D3D3D3;"
	              "	font-size: 13px;"
	              "}"
	              "QLineEdit {"
	              "	color: #D3D3D3;"
	              "	border: 1px solid #333333;"
	              "	border-radius: 2px;"
	              "	background: #1C1C1C;"
	              "	font-size: 13px;"
	              "}"
	              "QPushButton {"
	              "	color: #AAAAAA;"
	              "	background: #1C1C1C;"
	              "	font-size: 13px;"
	              "}"
	              "QWebView#chat { background-color: #1E1E1E }");

	// Hide the loading indicator as there's nothing loading (yet).
	ui->busyIndicator->hide();

	// Inject BetterTTV addon into the chat whenever it becomes ready.
	QObject::connect(ui->chat->page()->mainFrame(), &QWebFrame::javaScriptWindowObjectCleared, this,
	      &MainWindow::InjectBTTV);

	auto webSettings = QWebSettings::globalSettings();
	webSettings->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
	webSettings->setAttribute(QWebSettings::JavascriptCanOpenWindows, true);
	webSettings->enablePersistentStorage(QDir("./storage").absolutePath());

	// Handle the video player requesting a return to the streams grid.
	QObject::connect(qobject_cast<MPVQuickItem *>(ui->video->rootObject()),
	      &MPVQuickItem::gotoStreams, [this]() {

		      qDebug() << "Returning to the streams grid";
		      ui->stackedWidget->setCurrentWidget(ui->streams);

		      // Load an "empty" page in order to unload the chat, avoiding background bandwidth use.
		      ui->chat->setContent(QByteArray());
		   });

	// Add keyboard shortcuts.
	shortcuts.emplace_back(new QShortcut{ QKeySequence{ "F5" }, this });
	QObject::connect(shortcuts.back(), &QShortcut::activated, [this]() {
		if (ui->stackedWidget->currentWidget() == ui->streams) {
			ui->refreshButton->animateClick();
		}
	});
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

void MainWindow::UpdateStreams()
{
	// Show the loading indicator.
	ui->busyIndicator->show();

	auto username = ui->usernameEdit->text();
	qDebug() << "Updating streams" << username;

	TwitchAPI::GetLiveFollowedChannelsForUsername(
	      username, [this](const QList<QJsonObject> & liveStreams) {

		      // Clear the existing streams grid.
		      QLayoutItem * streamPreviewLayoutItem = nullptr;
		      while ((streamPreviewLayoutItem = ui->streamsGrid->takeAt(0)) != nullptr) {
			      delete streamPreviewLayoutItem->widget();
			      delete streamPreviewLayoutItem;
		      }

		      // Hide the loading indicator.
		      ui->busyIndicator->hide();

		      // For each of the live followed channels,
		      for (int streamIndex = 0; streamIndex < liveStreams.size(); ++streamIndex) {

			      const auto & stream = liveStreams[streamIndex];

			      // Get some of their important properties,
			      auto channel = stream["channel"].toObject();
			      auto channelName = channel["name"].toString();
			      auto streamerName = channel["display_name"].toString();
			      auto status = channel["status"].toString();
			      auto viewers = stream["viewers"].toInt();
			      auto previewUrl = stream["preview"].toObject()["medium"].toString();
			      auto game = stream["game"].toString();
			      auto gameUrl = "http://static-cdn.jtvnw.net/ttv-boxart/"
			            + QUrl::toPercentEncoding(game) + "-52x72.jpg";

			      // Generate a clickable preview for the stream,
			      StreamPreview * streamPreview = new StreamPreview{};
			      streamPreview->LoadPreviewImage(previewUrl);
			      streamPreview->LoadGameImage(gameUrl);
			      streamPreview->SetText("<div align=\"center\" style=\"font-weight:bold\">"
			            + QString::number(viewers) + " viewers on " + streamerName.toHtmlEscaped()
			            + "</div><div align=\"left\">" + status.toHtmlEscaped() + "</div>");

			      // Set up switching to the stream when it's clicked,
			      QObject::connect(
			            streamPreview, &StreamPreview::Clicked, [this, streamPreview, channelName]() {
				            qDebug() << "Clicked" << channelName;
				            LoadChannel(channelName);
				            ui->stackedWidget->setCurrentWidget(ui->player);
				         });

			      // And add it to the grid.
			      ui->streamsGrid->addWidget(streamPreview, streamIndex, 0, 1, 1);
		      }

		      // Lay out the grid properly.
		      ui->scrollArea->RearrangeGrid();
		   });
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

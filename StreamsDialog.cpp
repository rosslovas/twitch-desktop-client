#include "StreamsDialog.hpp"
#include "ui_StreamsDialog.h"

#include <QtCore>
#include <QPushButton>

#include "MainWindow.hpp"
#include "StreamPreview.hpp"
#include "StreamsScrollArea.hpp"
#include "TwitchAPI.hpp"

StreamsDialog::StreamsDialog(QWidget * parent /* = nullptr */)
    : QDialog(parent), ui(new Ui::StreamsDialog)
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
	              "}");

	// Increase the width to compensate for a possible scrollbar, plus 6 for other weird stuff.
	setGeometry(
	      x(), y(), width() + qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent) + 6, height());

	// Hide the loading indicator as there's nothing loading (yet).
	ui->busyIndicator->hide();
}

StreamsDialog::~StreamsDialog()
{
	delete ui;
}

void StreamsDialog::Update()
{
	auto mainWindow = qobject_cast<MainWindow *>(parent());

	// Show the loading indicator.
	ui->busyIndicator->show();

	auto username = ui->usernameEdit->text();
	qDebug() << "Updating streams" << username;

	TwitchAPI::GetLiveFollowedChannelsForUsername(
	      username, [this, mainWindow](const QList<QJsonObject> & liveStreams) {

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
			      QObject::connect(streamPreview, &StreamPreview::Clicked,
			            [this, streamPreview, channelName, mainWindow]() {
				            qDebug() << "Clicked" << channelName;
				            mainWindow->LoadChannel(channelName);
				            this->hide();
				         });

			      // And add it to the grid.
			      ui->streamsGrid->addWidget(streamPreview, streamIndex, 0, 1, 1);
		      }

		      // Lay out the grid properly.
		      ui->scrollArea->RearrangeGrid();
		   });
}

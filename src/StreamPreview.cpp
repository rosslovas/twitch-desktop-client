#include "StreamPreview.hpp"
#include "ui_StreamPreview.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>

#include "NetworkAccessManager.hpp"

StreamPreview::StreamPreview(QWidget * parent /* = nullptr */)
    : QWidget(parent), ui(new Ui::StreamPreview)
{
	ui->setupUi(this);
}

StreamPreview::~StreamPreview()
{
	delete ui;
}

void StreamPreview::LoadPreviewImage(const QString & url)
{
	auto networkAccessManager = NetworkAccessManager::Get();
	auto reply = networkAccessManager->get(QNetworkRequest{ QUrl{ url } });
	QObject::connect(reply, &QNetworkReply::finished, this, &StreamPreview::PreviewImageLoaded);
}

void StreamPreview::LoadGameImage(const QString & url)
{
	auto networkAccessManager = NetworkAccessManager::Get();
	auto reply = networkAccessManager->get(QNetworkRequest{ QUrl{ url } });
	QObject::connect(reply, &QNetworkReply::finished, this, &StreamPreview::GameImageLoaded);
}

void StreamPreview::SetText(const QString & text)
{
	ui->text->setText(text);
}

void StreamPreview::mousePressEvent(QMouseEvent * event)
{
	(void)event; // Suppress unused parameter warning.
	emit Clicked();
}

void StreamPreview::PreviewImageLoaded()
{
	// Get the image data and add it to the preview pixmap.
	QNetworkReply * reply = qobject_cast<QNetworkReply *>(sender());
	if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200) {
		auto imageData = reply->readAll();
		QPixmap imagePixmap{};
		imagePixmap.loadFromData(imageData);
		ui->preview->setPixmap(imagePixmap);
	}
}

void StreamPreview::GameImageLoaded()
{
	// Get the image data and add it to the game pixmap.
	QNetworkReply * reply = qobject_cast<QNetworkReply *>(sender());
	if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200) {
		auto imageData = reply->readAll();
		QPixmap imagePixmap{};
		imagePixmap.loadFromData(imageData);
		ui->game->setPixmap(imagePixmap);
	}
}

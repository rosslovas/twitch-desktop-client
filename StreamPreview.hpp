#ifndef STREAMPREVIEW_HPP
#define STREAMPREVIEW_HPP

#include <QWidget>

namespace Ui {
class StreamPreview;
}

class QNetworkReply;

class StreamPreview : public QWidget {
	Q_OBJECT

public:
	explicit StreamPreview(QWidget * parent = nullptr);
	~StreamPreview();

	void LoadPreviewImage(const QString & url);
	void LoadGameImage(const QString & url);
	void SetText(const QString & text);

signals:
	void Clicked();

private:
	Ui::StreamPreview * ui;
	virtual void mousePressEvent(QMouseEvent * event) override;

private slots:
	void PreviewImageLoaded();
	void GameImageLoaded();
};

#endif // STREAMPREVIEW_HPP

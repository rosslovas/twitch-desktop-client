#ifndef MPVQUICKITEM_HPP
#define MPVQUICKITEM_HPP

#include <memory>

#include <QQuickItem>
#include <QStateMachine>
#include <QVariant>

#include <mpv/qthelper.hpp>

#include "MPVQuickRenderer.hpp"

struct mpv_opengl_cb_context;

class MPVQuickItem : public QQuickItem {
	Q_OBJECT
public:
	MPVQuickItem();
	virtual bool event(QEvent * e) override;

	void SetOption(const QString & name, const QVariant & value);
	void SetProperty(const QString & name, const QVariant & value);
	void Command(const QVariantList & value);

	void LoadChannel(const QString & channel);

signals:
	// Signals for QML to call.
	void setVolume(double value);
	void setQuality(const QString & quality);
	void play();
	void stop();
	void gotoStreams();

	// Signals for the state machine operations.
	void ChannelLoading();
	void FileLoading();
	void FileLoaded();
	void Play();
	void Stop();
	void CoreWentIdle();
	void CoreNoLongerIdle();
	void Pause();
	void Resume();

public slots:
	void Sync();
	// void Cleanup();

private:
	QStateMachine stateMachine;
	std::unique_ptr<MPVQuickRenderer> renderer;
	mpv::qt::Handle mpv;

	QString channelToLoad;
	QString fileToLoad;
	QString desiredQuality;
};

#endif // MPVQUICKITEM_HPP

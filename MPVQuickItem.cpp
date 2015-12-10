#include "MPVQuickItem.hpp"

#include <QEvent>
#include <QQuickWindow>
#include <QSignalTransition>

#include <mpv/client.h>

#include "TwitchAPI.hpp"

namespace {
namespace local {
	void mpvWakeup(void * context)
	{
		auto mpvQuickItem = reinterpret_cast<MPVQuickItem *>(context);
		QCoreApplication::postEvent(mpvQuickItem, new QEvent(QEvent::User));
	}
}
}

MPVQuickItem::MPVQuickItem()
    : renderer{ nullptr }, channelToLoad{ "" }, fileToLoad{ "" }, desiredQuality{ "" }
{
	// Set up the state machine.
	auto idle = new QState();
	auto active = new QState(QState::ParallelStates);
	auto pausedOrNot = new QState(active);
	auto unpaused = new QState(pausedOrNot);
	auto paused = new QState(pausedOrNot);
	pausedOrNot->setInitialState(unpaused);
	auto pausableStates = new QState(active);
	auto loading = new QState(pausableStates);
	auto playing = new QState(pausableStates);
	auto buffering = new QState(pausableStates);
	pausableStates->setInitialState(loading);

	idle->addTransition(this, &MPVQuickItem::ChannelLoading, active);
	idle->addTransition(this, &MPVQuickItem::FileLoaded, active);
	active->addTransition(this, &MPVQuickItem::Stop, idle);
	unpaused->addTransition(this, &MPVQuickItem::Pause, paused);
	paused->addTransition(this, &MPVQuickItem::Resume, unpaused);
	loading->addTransition(this, &MPVQuickItem::Play, playing);
	playing->addTransition(this, &MPVQuickItem::CoreWentIdle, buffering);
	buffering->addTransition(this, &MPVQuickItem::CoreNoLongerIdle, playing);
	pausableStates->addTransition(this, &MPVQuickItem::ChannelLoading, loading);

	QObject::connect(idle, &QState::entered, [this]() {
		QMetaObject::invokeMethod(this, "changePlayStopState", Q_ARG(QVariant, "stopped"));
	});
	QObject::connect(idle, &QState::exited, [this]() {
		QMetaObject::invokeMethod(this, "changePlayStopState", Q_ARG(QVariant, "playing"));
	});
	QObject::connect(loading, &QState::entered, [this]() {
		QMetaObject::invokeMethod(this, "setBufferingIndicatorVisible", Q_ARG(QVariant, true));
	});
	QObject::connect(loading, &QState::exited, [this]() {
		QMetaObject::invokeMethod(this, "setBufferingIndicatorVisible", Q_ARG(QVariant, false));
	});
	QObject::connect(buffering, &QState::entered, [this]() {
		QMetaObject::invokeMethod(this, "setBufferingIndicatorVisible", Q_ARG(QVariant, true));
	});
	QObject::connect(buffering, &QState::exited, [this]() {
		QMetaObject::invokeMethod(this, "setBufferingIndicatorVisible", Q_ARG(QVariant, false));
	});

	stateMachine.addState(idle);
	stateMachine.addState(active);
	stateMachine.setInitialState(idle);
	// Wait until mpv is fully initialised before starting the state machine.

	// Create the mpv handle.
	mpv = mpv::qt::Handle::FromRawHandle(mpv_create());
	if (mpv == nullptr) {
		throw std::runtime_error("could not create mpv context");
	}

	// Enable console output from mpv for debugging.
	// SetOption("terminal", "yes");
	// SetOption("msg-level", "all=debug");

	// Initialise mpv.
	if (mpv_initialize(mpv) < 0) {
		throw std::runtime_error("could not initialize mpv context");
	}

	// Request hardware decoding.
	SetOption("hwdec", "auto");

	// Hook up render initialisation and cleanup to the appropriate signals.
	QObject::connect(this, &QQuickItem::windowChanged, [this](QQuickWindow * window) {
		if (window) {
			QObject::connect(window, &QQuickWindow::beforeSynchronizing, this, &MPVQuickItem::Sync,
			      Qt::DirectConnection);
			QObject::connect(window, &QQuickWindow::sceneGraphInvalidated, this,
			      &MPVQuickItem::Cleanup, Qt::DirectConnection);
			window->setClearBeforeRendering(false);
		}
	});

	// Make the ChannelLoading signal load the available qualities and pick a stream file.
	QObject::connect(this, &MPVQuickItem::ChannelLoading, [this]() {
		TwitchAPI::GetStreamPlaylist(
		      channelToLoad, [this](const QHash<QString, QByteArray> & qualities) {
			      // Set the current file then emit a signal that will tell mpv to load it.
			      fileToLoad = qualities.value(desiredQuality);
			      emit FileLoading();
			   });
	});

	// Make the FileLoading signal tell mpv to load the current file.
	QObject::connect(this, &MPVQuickItem::FileLoading,
	      [this]() { Command(QVariantList() << "loadfile" << fileToLoad); });

	// Handle the QML play button.
	QObject::connect(this, &MPVQuickItem::play, [this]() {
		qDebug() << "Play button hit";
		emit FileLoading();
	});

	// Handle the QML stop button.
	QObject::connect(this, &MPVQuickItem::stop, [this]() {
		qDebug() << "Stop button hit";
		Command(QVariantList() << QString{ "stop" });
		emit Stop();
	});

	// Handle the QML volume slider.
	QObject::connect(this, &MPVQuickItem::setVolume,
	      [this](double volume) { SetProperty("volume", volume * 100.0); });

	// Handle the QML quality options.
	QObject::connect(this, &MPVQuickItem::setQuality, [this](const QString & quality) {
		qDebug() << "Quality set to" << quality;
		desiredQuality = quality;
		if (!channelToLoad.isEmpty()) {
			emit ChannelLoading();
		}
	});
}

bool MPVQuickItem::event(QEvent * e)
{
	if (e->type() == QEvent::User) {
		while (mpv != nullptr) {

			// Get the event object.
			mpv_event * mpvEvent = mpv_wait_event(mpv, 0.0);

			// If it's a null or none event, ignore it.
			if (mpvEvent == nullptr || mpvEvent->event_id == MPV_EVENT_NONE) {
				break;
			}
			// If it's an error, print it out.
			if (mpvEvent->error < 0) {
				qWarning() << mpv_error_string(mpvEvent->error);
				break;
			}

			switch (mpvEvent->event_id) {
			// MPV_EVENT_START_FILE occurs upon mpv receiving the 'loadfile' command.
			case MPV_EVENT_START_FILE:
				qDebug() << "MPV_EVENT_START_FILE";
				emit FileLoaded();
				break;

			// MPV_EVENT_FILE_LOADED occurs upon the stream actually starting to play.
			case MPV_EVENT_FILE_LOADED:
				qDebug() << "MPV_EVENT_FILE_LOADED";
				emit Play();
				break;

			// MPV_EVENT_END_FILE occurs when the stream ends.
			case MPV_EVENT_END_FILE:
				qDebug() << "MPV_EVENT_END_FILE";
				emit Stop();
				break;

			// Watch for changes in certain properties.
			case MPV_EVENT_PROPERTY_CHANGE: {
				mpv_event_property * prop = (mpv_event_property *)mpvEvent->data;

				// The 'pause' property (yes/no string).
				if (std::strcmp(prop->name, "pause") == 0 && prop->format == MPV_FORMAT_STRING) {
					bool paused = std::strcmp(*reinterpret_cast<char **>(prop->data), "yes") == 0;
					qDebug() << "paused:" << paused;
					if (paused) {
						emit Pause();
					}
					else {
						emit Resume();
					}
				}
				// The 'core-idle' property (yes/no string).
				else if (std::strcmp(prop->name, "core-idle") == 0
				      && prop->format == MPV_FORMAT_STRING) {
					bool idle = std::strcmp(*reinterpret_cast<char **>(prop->data), "yes") == 0;
					qDebug() << "core-idle:" << idle;
					if (idle) {
						emit CoreWentIdle();
					}
					else {
						emit CoreNoLongerIdle();
					}
				}
				// The 'volume' property ([0.0, 100.0] double).
				else if (std::strcmp(prop->name, "volume") == 0 && prop->format == MPV_FORMAT_DOUBLE) {
					double newVolume = *reinterpret_cast<double *>(prop->data);

					// Update the volume slider to represent the changed volume.
					QMetaObject::invokeMethod(this, "updateVolume", Q_ARG(QVariant, newVolume / 100.0));
					qDebug() << "volume:" << newVolume;
				}
			} break;
			default:
				qDebug() << "Unhandled mpvEvent:" << mpvEvent->event_id;
				break;
			}
		}
		return true;
	}
	return QQuickItem::event(e);
}

void MPVQuickItem::SetOption(const QString & name, const QVariant & value)
{
	mpv::qt::set_option_variant(mpv, name, value);
}

void MPVQuickItem::SetProperty(const QString & name, const QVariant & value)
{
	mpv::qt::set_property_variant(mpv, name, value);
}

void MPVQuickItem::Command(const QVariantList & arguments)
{
	mpv::qt::command_variant(mpv, arguments);
}

void MPVQuickItem::LoadChannel(const QString & channel)
{
	// Set the desired stream then emit a signal that will load it.
	channelToLoad = channel;
	emit ChannelLoading();
}

void MPVQuickItem::Sync()
{
	if (renderer == nullptr && mpv != nullptr) {

		// Create the mpv OpenGL renderer.
		renderer = std::move(std::make_unique<MPVQuickRenderer>(mpv, window(), this));

		QObject::connect(window(), &QQuickWindow::beforeRendering, renderer.get(),
		      &MPVQuickRenderer::Paint, Qt::DirectConnection);
		QObject::connect(window(), &QQuickWindow::frameSwapped, renderer.get(),
		      &MPVQuickRenderer::Swapped, Qt::DirectConnection);

		// Set the wakeup callback and the properties to observe.
		// mpv_observe_property(mpv, 0, "duration", MPV_FORMAT_DOUBLE);
		// mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
		mpv_observe_property(mpv, 0, "pause", MPV_FORMAT_STRING);
		mpv_observe_property(mpv, 0, "core-idle", MPV_FORMAT_STRING);
		mpv_observe_property(mpv, 0, "volume", MPV_FORMAT_DOUBLE);
		mpv_set_wakeup_callback(mpv, local::mpvWakeup, this);

		// Now that mpv is ready, start the state machine.
		stateMachine.start();
	}
}

void MPVQuickItem::Cleanup()
{
	renderer.reset();
}

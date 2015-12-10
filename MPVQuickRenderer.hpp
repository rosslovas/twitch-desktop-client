#ifndef MPVQUICKRENDERER_HPP
#define MPVQUICKRENDERER_HPP

#include <QObject>

class QQuickWindow;

struct mpv_handle;
struct mpv_opengl_cb_context;

class MPVQuickItem;

class MPVQuickRenderer : public QObject {
	Q_OBJECT
public:
	MPVQuickRenderer(mpv_handle * mpv, QQuickWindow * window, MPVQuickItem * parent = nullptr);
	~MPVQuickRenderer();

public slots:
	void Paint();
	void Swapped();

private:
	mpv_handle * mpv;
	mpv_opengl_cb_context * mpvGL;

	QQuickWindow * window;
};

#endif // MPVQUICKRENDERER_HPP

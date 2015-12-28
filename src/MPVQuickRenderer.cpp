#include "MPVQuickRenderer.hpp"

#include <QDebug>
#include <QQuickWindow>
#include <QtGui/QOpenGLContext>
#include <QOpenGLFramebufferObject>

#include <mpv/client.h>
#include <mpv/opengl_cb.h>

#include "MPVQuickItem.hpp"

namespace {
namespace local {
	void * glGetProcAddress(void *, const char * name)
	{
		const auto openGLContext = QOpenGLContext::currentContext();
		if (!openGLContext) {
			return nullptr;
		}
		return reinterpret_cast<void *>(openGLContext->getProcAddress(QByteArray(name)));
	}
	void update(void * context)
	{
		QMetaObject::invokeMethod(
		      reinterpret_cast<QQuickWindow *>(context), "update", Qt::QueuedConnection);
	}
}
}

MPVQuickRenderer::MPVQuickRenderer(
      mpv_handle * mpv, QQuickWindow * window, MPVQuickItem * parent /* = nullptr */)
    : QObject(parent), mpv{ mpv }, window{ window }
{

	// Make use of the MPV_SUB_API_OPENGL_CB API.
	mpv_set_option_string(mpv, "vo", "opengl-cb");

	mpvGL = (mpv_opengl_cb_context *)mpv_get_sub_api(mpv, MPV_SUB_API_OPENGL_CB);
	if (!mpvGL) {
		throw std::runtime_error("OpenGL not compiled in");
	}
	mpv_opengl_cb_set_update_callback(mpvGL, &local::update, reinterpret_cast<void *>(window));

	int r = mpv_opengl_cb_init_gl(mpvGL, nullptr, &local::glGetProcAddress, nullptr);
	if (r < 0) {
		throw std::runtime_error("could not initialize OpenGL");
	}
}

MPVQuickRenderer::~MPVQuickRenderer()
{
	if (mpvGL) {
		mpv_opengl_cb_set_update_callback(mpvGL, nullptr, nullptr);
	}
	mpv_opengl_cb_uninit_gl(mpvGL);
}

void MPVQuickRenderer::Paint()
{
	const auto frameBufferObject = window->renderTarget();
	window->resetOpenGLState();
	mpv_opengl_cb_draw(mpvGL, frameBufferObject->handle(), frameBufferObject->width(),
	      frameBufferObject->height() * -1);
	window->resetOpenGLState();
}

void MPVQuickRenderer::Swapped()
{
	mpv_opengl_cb_report_flip(mpvGL, 0);
}

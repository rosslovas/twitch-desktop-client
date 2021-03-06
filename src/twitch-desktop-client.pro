#-------------------------------------------------
#
# Project created by QtCreator 2015-11-12T17:40:03
#
#-------------------------------------------------

QT += core gui webkitwidgets quickwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = twitch-desktop-client
TEMPLATE = app

CONFIG += c++14 link_pkgconfig
PKGCONFIG += mpv

SOURCES += \
    main.cpp \
    MainWindow.cpp \
    TwitchAPI.cpp \
    StreamPreview.cpp \
    StreamsScrollArea.cpp \
    NetworkAccessManager.cpp \
    MPVQuickItem.cpp \
    MPVQuickRenderer.cpp

HEADERS += \
    MainWindow.hpp \
    TwitchAPI.hpp \
    StreamPreview.hpp \
    StreamsScrollArea.hpp \
    NetworkAccessManager.hpp \
    MPVQuickItem.hpp \
    MPVQuickRenderer.hpp

FORMS += \
    ui/MainWindow.ui \
    ui/StreamPreview.ui

RESOURCES += \
    qml/QML.qrc \
    images/Images.qrc

DISTFILES +=

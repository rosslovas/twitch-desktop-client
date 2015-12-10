import QtQuick 2.5

Item {

    id: slider;

    property color backgroundColour: hoverArea.containsMouse ? '#999999' : '#888888';
    property color foregroundColour: hoverArea.containsMouse ? '#eeeeee' : '#dddddd';
    property real volume: 0.5;

    signal volumeChangedBySlider(double value);

    Rectangle {
        id: background;
        color: slider.backgroundColour;
        anchors.fill: parent;
    }

    Rectangle {
        id: foreground;
        color: slider.foregroundColour;
        anchors.left: parent.left;
        anchors.top: parent.top;
        anchors.bottom: parent.bottom;
        width: parent.width * volume;
    }

    MouseArea {
        id: hoverArea;
        anchors.fill: parent;
        hoverEnabled: true;
    }

    MouseArea {
        id: mouseArea;
        anchors.fill: parent;
        onPressed: onPositionChanged(mouse);
        onPositionChanged: {
            slider.volume = Math.max(0.0, Math.min(1.0, mouse.x / width));
            slider.volumeChangedBySlider(volume);
        }
    }

    Component.onCompleted: {
        volumeChangedBySlider(volume);
    }
}

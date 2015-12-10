import QtQuick 2.5
import QtGraphicalEffects 1.0

Item {

    id: button;

    property color colour: mouseArea.containsMouse ? '#ffffff' : '#eeeeee';
    property real morph: 1.0;

    signal playButtonClicked;
    signal stopButtonClicked;

    state: 'stopped';

    states: [
        State {
            name: 'stopped';
            PropertyChanges {
                target: button;
                morph: 1.0;
            }
        },
        State {
            name: 'playing'
            PropertyChanges {
                target: button;
                morph: 0.0;
            }
        }
    ]

    transitions: [
        Transition {
            NumberAnimation {
                property: 'morph'
                duration: 100
            }
        }
    ]

    onColourChanged: canvas.requestPaint();
    onMorphChanged: canvas.requestPaint();

    Canvas {

        id: canvas;

        anchors.fill: parent;

        renderTarget: Canvas.Image;
        renderStrategy: Canvas.Cooperative;

        function lerp(first, second, ratio) {
            return first + (second - first) * ratio;
        }

        onPaint: {
            var context = getContext('2d');
            context.clearRect(0, 0, width, height);

            context.fillStyle = button.colour;

            context.beginPath();
            context.moveTo(0, 0);
            context.lineTo(width, lerp(0, height * 0.5, button.morph));
            context.lineTo(width, lerp(height, height * 0.5, button.morph));
            context.lineTo(0, height);
            context.closePath();
            context.fill();
        }

    }

    MouseArea {
        id: mouseArea;
        anchors.fill: parent;
        hoverEnabled: true;
        onClicked: {
            if (button.state == 'stopped') {
                button.playButtonClicked();
            } else {
                button.stopButtonClicked();
            }
        }
    }

}


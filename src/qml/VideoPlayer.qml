import QtQuick 2.5
import QtGraphicalEffects 1.0
import QtQuick.Controls 1.4
import twitch_desktop_client 1.0

MPVItem {

    id: mpv;

    function updateVolume(value) {
        // console.debug('Updated volume slider to ' + value);
        volumeSlider.volume = value;
    }

    function changePlayStopState(newState) {
        console.debug(playStopButton + ' ' + playStopButton.state + ' -> ' + newState);
        playStopButton.state = newState;
    }

    function setBufferingIndicatorVisible(visibility) {
        console.debug(bufferingIndicator + ' visible: ' + bufferingIndicator.visible + ' -> ' + visibility);
        bufferingIndicator.visible = visibility;
    }

    function setControlsVisible(value) {
        controls.visible = value;
    }

    MouseArea {
        id: fullscreenDoubleClickArea;
        anchors.left: parent.left;
        anchors.right: parent.right;
        anchors.top: parent.top;
        anchors.topMargin: 32;
        anchors.bottom: parent.bottom;
        anchors.bottomMargin: 32;
    }

    Item {
        id: controls;
        anchors.fill: parent;

        // Top gradient
        LinearGradient {
            anchors.fill: parent;
            start: Qt.point(0, 40);
            end: Qt.point(0, 0);
            gradient: Gradient {
                GradientStop {
                    position: 0.0;
                    color: '#00000000';
                }
                GradientStop {
                    position: 1.0;
                    color: '#EE000000';
                }
            }
        }

        // Bottom gradient
        LinearGradient {
            anchors.fill: parent;
            start: Qt.point(0, height - 40);
            end: Qt.point(0, height);
            gradient: Gradient {
                GradientStop {
                    position: 0.0;
                    color: '#00000000';
                }
                GradientStop {
                    position: 1.0;
                    color: '#EE000000';
                }
            }
        }

        PlayStopButton {
            id: playStopButton;
            width: 18;
            height: 18;
            anchors.left: parent.left;
            anchors.leftMargin: 8;
            anchors.bottom: parent.bottom;
            anchors.bottomMargin: 8;
            opacity: 0.9;
        }

        VolumeSlider {
            id: volumeSlider;
            width: 90;
            height: 10;
            anchors.left: parent.left;
            anchors.leftMargin: 38; // 8 + 18 + 12
            anchors.bottom: parent.bottom;
            anchors.bottomMargin: 12;
            opacity: 0.9;
        }

        BusyIndicator {
            id: bufferingIndicator;
            width: 40;
            height: 40;
            anchors.centerIn: parent;
            visible: false;
        }

        QualityOptions {
            id: qualityOptions;
            anchors.right: parent.right;
            anchors.rightMargin: 8;
            anchors.bottom: parent.bottom;
            anchors.bottomMargin: 8;
        }

        Button {
            id: backButton;
            anchors.left: parent.left;
            anchors.leftMargin: 8;
            anchors.top: parent.top;
            anchors.topMargin: 8;
            text: 'Back';
        }
    }

    Component.onCompleted: {
        console.debug('MPVItem startup');
        playStopButton.playButtonClicked.connect(function () {
            mpv.play();
        });
        playStopButton.stopButtonClicked.connect(function () {
            mpv.stop();
        });
        volumeSlider.volumeChangedBySlider.connect(function (volume) {
            mpv.setVolume(volume);
        });
        qualityOptions.setQuality.connect(function (quality) {
            mpv.setQuality(quality);
        });
        backButton.clicked.connect(function () {
            mpv.stop();
            mpv.gotoStreams();
        });
        fullscreenDoubleClickArea.onDoubleClicked.connect(function () {
            console.debug('Fullscreen toggle requested');
            mpv.toggleFullscreen();
        });
    }

}

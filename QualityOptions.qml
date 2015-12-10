import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

Row {

    id: qualityOptions;

    readonly property color checkedColour: '#BB333333';
    readonly property color uncheckedColour: '#BB111111';

    readonly property var qualities: ['Mobile', 'Low', 'Medium', 'High', 'Source'];

    signal setQuality(string quality);

    spacing: 0;

    ExclusiveGroup {
        id: optionsGroup;
    }

    Repeater {
        id: repeater;
        model: 5;
        Item {
            width: text.width + 10;
            height: text.height + 8;
            Action {
                id: action;
                checkable: true;
                exclusiveGroup: optionsGroup;
                onTriggered: {
                    qualityOptions.setQuality(qualities[index]);
                }
            }
            Rectangle {
                color: action.checked ? checkedColour : uncheckedColour;
                anchors.fill: parent;
                Text {
                    id: text;
                    anchors.centerIn: parent;
                    color: '#FFFFFF';
                    text: qualities[index];
                }
                MouseArea {
                    anchors.fill: parent;
                    onClicked: {
                        action.trigger();
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        var defaultQualityAction = repeater.itemAt(0).resources[0];
        defaultQualityAction.checked = true;
        defaultQualityAction.trigger();
    }
}


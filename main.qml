import QtQml 2.0
import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.12

import Controller 1.0

Window {
    visible: true
    width: 800
    height: 600
    title: "Gamepad Utility"

    Controller {
        id: controller;
        onGamepadKeyPressed: {
            console.log("Получена строка: ", buttonName);
            var item = repeater.itemAt(controller.focus);
            var textField = item.children[1];
            textField.text = buttonName;
            textField.focus = false;
        }
        onCommandMappingChanged: {
            var item = repeater.itemAt(index);
            var textField = item.children[1];
            textField.text = buttonName;
        }
    }

    Rectangle {
        id: root
        anchors.fill: parent
        border.color: "black"
        border.width: 1

        Timer {
            id: mainTimer;
            interval: 100;
            repeat: true;
            running: false;
            onTriggered: {
                controller.handleTimerTick();
            }
        }

        ColumnLayout {
            Repeater {
                id: repeater
                model: controller.commandCount
                delegate: Row {
                    Label {
                        text: controller.getCommandName(index)
                    }
                    TextField {
                        id: textField
                        text: controller.getButtonName(index)
                        readOnly: true
                        focus: false
                        onActiveFocusChanged: {
                            if (textField.activeFocus) {
                                console.log("focus on: " + index);
                                controller.clearInput();
                                controller.focus = index
                                mainTimer.start();
                            } else {
                                console.log("focus off: " + index);
                                mainTimer.stop();
                            }
                        }

                        MouseArea {
                            id: textFieldArea
                            anchors.fill: parent
                            hoverEnabled: true
                        }
                    }
                }
            }
        }



        MouseArea {
            id: mainArea
            anchors.fill: parent
            onClicked: {
                for (var i = 0; i < repeater.count; i++) {
                    var item = repeater.itemAt(i);
                    var textField = item.children[1];
                    var mouseArea = textField.children[1];
                    if (mouseArea.containsMouse) {
                        textField.focus = true;
                        break;
                    } else {
                        textField.focus = false;
                    }
                }
            }
        }
    }
    Rectangle {
        anchors.topMargin: 300
        Button {
            id:saveButton;
            text: "Save"
            onClicked: {
                controller.saveConfig();
            }
        }
    }
}

#pragma once

#include <QObject>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include <vector>

#include "gamepad.h"
#include "command.h"


class Controller : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int focus READ focus WRITE setFocus NOTIFY focusChanged)
    Q_PROPERTY(int commandCount READ commandCount)
public:
    Controller(QObject* parent = nullptr) : QObject(parent) {
        if (SDL_Init(SDL_INIT_GAMECONTROLLER) != 0) {
            std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
            exit(1);
        }
        gamepad = new Gamepad();
        gamepad->Open(0);

        loadConfigJson("C:\\Users\\mikha\\Desktop\\diploma-utility\\gamepad-utility\\debug\\Message--GamepadMapping.json");
    }

    int focus() const {
        return mFocus;
    }

    int commandCount() const {
        return commands.size();
    }

    void setFocus(int focus) {
        mFocus = focus;
        emit focusChanged();
    }

    ~Controller() {
        gamepad->Close();
        delete gamepad;
        gamepad = nullptr;
    }


public slots:

    QString getCommandName(int index) {
        return commands[index].commandName;
    }

    QString getButtonName(int index) {
        return commands[index].buttonMapping;
    }

    void handleTimerTick() {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_CONTROLLERDEVICEADDED:
                    OnJoystickConnected(event.cdevice.which);
                    break;
                case SDL_CONTROLLERBUTTONDOWN:
                    gamepad->SetButtonState(SDL_GameControllerButton(event.cbutton.button), true);
                    break;
                case SDL_CONTROLLERBUTTONUP:
                    gamepad->SetButtonState(SDL_GameControllerButton(event.cbutton.button), false);
                    break;
            }
        }
        gamepad->ProcessPendingKeyEvents();
        for (int i = 0; i < Gamepad::ButtonCount; i++) {
            if (gamepad->IsKeyPressed(i)) {

                for (int j = 0; j < commands.size(); j++) {
                    if (commands[j].buttonMapping == SDL_GameControllerGetStringForButton((SDL_GameControllerButton)i)) {
                        commands[j].buttonMapping = "";
                        emit commandMappingChanged(j, "");
                    }
                }

                commands[mFocus].buttonMapping = SDL_GameControllerGetStringForButton((SDL_GameControllerButton)i);

                emit gamepadKeyPressed(SDL_GameControllerGetStringForButton((SDL_GameControllerButton)i));
                break;
            }
        }
        gamepad->ClearKeyState();
    }

    void saveConfig(){
        saveConfigure();
    }

    void clearInput() {
        gamepad->ClearKeyState();
        while (SDL_PollEvent(&event)) {}
    }

signals:
    void gamepadKeyPressed(const QString& buttonName);
    void commandMappingChanged(int index, const QString& buttonName);
    void focusChanged();

private:
    //QTimer* timer;
    Gamepad* gamepad;
    SDL_Event event;
    int mFocus;
    std::vector<Command> commands;

    void OnJoystickConnected(int deviceIndex)
    {
        if (!SDL_IsGameController(deviceIndex)) {
            return;
        }
        gamepad->Open(deviceIndex);
        qDebug() << "Gamepad connected." << endl;
    }

    void loadConfigJson(const QString& filename) {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qDebug() << "Не удалось открыть файл.";
            return;
        }

        QByteArray jsonData = file.readAll();

        // Создание JSON-документа из JSON-данных
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);

        // Проверка на успешное чтение JSON-документа
        if (jsonDoc.isNull())
        {
            qDebug() << "Не удалось прочитать JSON-документ.";
            return;
        }

        // Получение корневого JSON-объекта из JSON-документа
        QJsonObject jsonObject = jsonDoc.object();

        // Чтение значений из JSON-объекта
        QString startControl = jsonObject["start_control"].toString();
        QString stopControl = jsonObject["stop_control"].toString();
        QString setMinSpeed = jsonObject["set_min_speed"].toString();
        QString setSlowSpeed = jsonObject["set_slow_speed"].toString();
        QString setMaxSpeed = jsonObject["set_max_speed"].toString();
        QString setZeroSpeed = jsonObject["set_zero_speed"].toString();
        QString moveForward = jsonObject["move_forward"].toString();
        QString moveRight = jsonObject["move_right"].toString();

        file.close();

        for (const auto& key : jsonObject.keys()) {
            commands.push_back({key, jsonObject[key].toString()});
        }
    }

    void saveConfigure() {
        QFile file("C:\\Users\\mikha\\Desktop\\diploma-utility\\gamepad-utility\\debug\\Message--GamepadMapping.json");
        if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
        {
            qDebug() << "Не удалось открыть файл.";
            return;
        }

        QByteArray jsonData = file.readAll();

        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);


        QJsonObject jsonObject = jsonDoc.object();

        for (const auto& command : commands) {
            jsonObject[command.commandName] = command.buttonMapping;

        }

        jsonDoc.setObject(jsonObject);
        QByteArray newJsonData = jsonDoc.toJson();
        file.seek(0);
        file.write(newJsonData);
        file.resize(file.pos());
        file.close();
    }
};

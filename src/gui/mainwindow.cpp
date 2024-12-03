// #include <windows.h>

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QUrl>

using namespace std;

MainWindow *MainWindow::instance = nullptr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    MainWindow::instance = this;
    this->showMaximized();
    setButtonIndication(Tab::Home);
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(chat_listener()));
    timer->start(1500);
    QIcon micIcon("mic-on.png");
    ui->micButton->setIcon(micIcon);
    QIcon speakerIcon("speaker-on.png");
    ui->speakerButton->setIcon(speakerIcon);
    connect(ui->testButton, SIGNAL(clicked()), SLOT(on_keyboard_function_assaign()));
    readAllJsonFile();
    setupFunctions();
    setupCommands();
    setupGlobals();
    foreach(const QString& key, global_json.getJsonObject().keys()) {
        if (QString::compare(key.trimmed(), QString("speaker"), Qt::CaseInsensitive) == 0) {
            if (QString::compare(global_json.getJsonObject().value(key).toString().trimmed(), QString("False"), Qt::CaseInsensitive) == 0) {
                on_speakerButton_clicked();
            }
        }
    }
    on_refreshChatButton_clicked();
    ui->chatParentLayout->setSpacing(40);
    ui->testButton->hide();
}

void MainWindow::removeFunction(CFunctionPane *function)
{
    for (int i = functions.size() - 1; i >= 0; i--) {
        if (functions.at(i) == function) {
            functions.removeAt(i);
        }
    }
    delete function;
}

MainWindow::~MainWindow()
{
    delete ui;

    delete process;
    delete listener_process;
    delete storage_process;

    for (auto &f: functions) {
        delete f;
    }
    functions.clear();

    for (auto &f: variables) {
        delete f;
    }
    variables.clear();

    for (auto &f: chatList) {
        delete f;
    }
    chatList.clear();

    delete trigger;
    delete listener;
    delete transcriber;
    delete sender;
    delete reader;
    delete actuator;
    delete storage;

    if (instance == this) {
        instance = nullptr;
    }
}

void MainWindow::on_homeButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
    setButtonIndication(Tab::Home);
}

void MainWindow::on_globalVariablesButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    setButtonIndication(Tab::Variables);
}

void MainWindow::startProcess()
{
    delete storage_process;
    storage_process = new QProcess(this);
    if (!storage_process->isOpen()) {
        if (!commands_json.isValid() || !global_json.isValid()){
            ui->statusBar->showMessage(QString("WARNING: modules.json or globals.json does not correctly set up, cannot start listener process"), timeout);
            return;
        }
        QString globalizedCommand = replaceGlobalKeys(commands_json.getJsonObject().value("storage").toObject().value("command").toString(), global_json.getJsonObject());
        QStringList args = globalizedCommand.split(" ");
        storage_process->start(args.at(0), args.mid(1));
    }
    delete process;
    process = new QProcess(this);
    if (!process->isOpen()) {
        if (!commands_json.isValid() || !global_json.isValid()){
            ui->statusBar->showMessage(QString("WARNING: modules.json or globals.json does not correctly set up, cannot start process"), timeout);
            return;
        }
        QString globalizedCommand = replaceGlobalKeys(commands_json.getJsonObject().value("trigger").toObject().value("command").toString(), global_json.getJsonObject());
        QStringList args = globalizedCommand.split(" ");

        process->start(args.at(0), args.mid(1));
        ui->statusBar->showMessage(QString("Started assistant..."), timeout);
    }
    delete listener_process;
    listener_process = new QProcess(this);
    if (!listener_process->isOpen()) {
        if (!commands_json.isValid() || !global_json.isValid()){
            ui->statusBar->showMessage(QString("WARNING: modules.json or globals.json does not correctly set up, cannot start listener process"), timeout);
            return;
        }
        QString globalizedCommand = replaceGlobalKeys(commands_json.getJsonObject().value("listener").toObject().value("command").toString(), global_json.getJsonObject());
        QStringList args = globalizedCommand.split(" ");
        if (isMicOn) {
            listener_process->start(args.at(0), args.mid(1));
        }
    }
}

void MainWindow::stopProcess()
{
    auto stopQProcess = [](QProcess *process, const QString &name) {
        if (!process) {
            qDebug() << QString("Cannot stop %1 without initialization!").arg(name);
            return;
        }
        if (process->isOpen()) {
            process->terminate();
            if (!process->waitForFinished(0)) { // Wait up to 5 seconds
                if (process->state() != QProcess::NotRunning) {

                    qint64 pid = process->processId();

                    QProcess taskkillProcess;
                    QStringList arguments;

                    arguments << QString::number(pid);

                    taskkillProcess.start("killer.exe", arguments);
                    qDebug() << "killing process";
                    taskkillProcess.waitForFinished();


                }
            }
        }
    };

    stopQProcess(storage_process, "storage_process");
    stopQProcess(process, "process");
    stopQProcess(listener_process, "listener_process");
}

void MainWindow::readAllStandardOutput()
{
    if (process) {
        if (process->isOpen()) {
            QString output = process->readAllStandardOutput();
            QString outputErr = process->readAllStandardError();
            // do something with output
            return;
        }
    }
}

void MainWindow::on_modulesButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    setButtonIndication(Tab::Modules);
}


void MainWindow::on_functionsButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(3);
    setButtonIndication(Tab::Functions);
}


void MainWindow::on_helpButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(4);
    setButtonIndication(Tab::Help);
}


void MainWindow::on_startButton_clicked()
{
    // start CORE
    readAllJsonFile();
    bool shouldStart = isAllReplacementOK();
    if ( !shouldStart ) {
        return;
    }
    QString current = ui->startButton->text();
    if ( current == QString("START") ) {
        ui->startButton->setText(QString("STOP"));
        ui->startButton->setStyleSheet("QPushButton{"
                                       "background-color: #90009f;"
                                       "border: none;"
                                       "font: bold 14pt;"
                                       "}");
        isRunning = true;
        ui->micButton->setDisabled(true);
        ui->micButton->setToolTip(QString("Only adjustable when assistant is stopped"));
        ui->speakerButton->setDisabled(true);
        ui->speakerButton->setToolTip(QString("Only adjustable when assistant is stopped"));
        startProcess();
        on_homeButton_clicked();
    } else {
        ui->startButton->setText(QString("START"));
        ui->startButton->setStyleSheet("QPushButton{"
                                       "font: bold 14pt;"
                                       "}");
        isRunning = false;
        ui->micButton->setDisabled(false);
        ui->micButton->setToolTip(QString());
        ui->speakerButton->setDisabled(false);
        ui->speakerButton->setToolTip(QString());
        stopProcess();
    }
}

void MainWindow::readJson(const string &file, CJsonObj& obj)
{
    QFile qfile = QFile(QString(file.c_str()));
    if (qfile.open(QIODevice::ReadOnly)) {

        QByteArray bytes = qfile.readAll();
        qfile.close();

        QJsonParseError jsonError;
        QJsonDocument document = QJsonDocument::fromJson(bytes, &jsonError);
        if (jsonError.error != QJsonParseError::NoError) {
            // set status bar text to error
            const string message = "Error occured when parsing JSON file: " + file;
            ui->statusBar->showMessage(QString(message.c_str()), timeout);
            obj.setInvalid();
            return;
        }

        if (document.isObject()) {
            obj.setJsonObject(QJsonObject(document.object()));
            return;
        }
    }
    obj.setInvalid();
}

void MainWindow::writeJson(const string &file, const QJsonObject &jsonObj) const
{
    QFile qfile = QFile(QString(file.c_str()));
    if (qfile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        QJsonDocument document = QJsonDocument();
        document.setObject(jsonObj);
        qfile.write(document.toJson());
        const string message = "Saved JSON file: " + file;
        ui->statusBar->showMessage(QString(message.c_str()), timeout);
    } else {
        const string message = "Could not open file for writing: " + file;
        ui->statusBar->showMessage(QString(message.c_str()), timeout);
    }
}

void MainWindow::setupFunctions()
{

    if (map_json.isValid()) {
        foreach(const QString& key, map_json.getJsonObject().keys()) {
            CFunctionPane* newPane = new CFunctionPane(this);
            newPane->lEditName->setText(QString(key));
            newPane->lEditDesc->setText(map_json.getJsonObject().value(key).toObject().value("description").toString());
            newPane->lEditCommand->setText(map_json.getJsonObject().value(key).toObject().value("command").toString());
            newPane->cboxIsDestructive->setChecked(map_json.getJsonObject().value(key).toObject().value("destructive").toBool());

            QJsonArray requiredArray = map_json.getJsonObject().value(key).toObject().value("required").toArray();
            foreach (const QString& parkey, map_json.getJsonObject().value(key).toObject().value("parameters").toObject().keys()) {
                QString parDesc = map_json.getJsonObject().value(key).toObject().value("parameters").toObject().value(parkey).toObject().value("description").toString();
                QString parType = map_json.getJsonObject().value(key).toObject().value("parameters").toObject().value(parkey).toObject().value("type").toString();
                bool isOptional = true;
                for (const QJsonValue &value : requiredArray){
                    if (value == parkey) {
                        isOptional = false;
                        break;
                    }
                }
                newPane->addFullParam(parkey, parDesc, parType, isOptional);
            }

            ui->functionParentLayout->addWidget(newPane, 1);


            functions.push_back(newPane);
        }
        ui->functionParentLayout->setAlignment(Qt::AlignTop);
    }
    else {
        ui->statusBar->showMessage(QString("Could not open file"), timeout );
    }
}

void MainWindow::setupCommands()
{
    if (commands_json.isValid()) {
        if (trigger) {
            delete trigger;
        }
        if (listener) {
            delete listener;
        }
        if (transcriber) {
            delete transcriber;
        }
        if (sender) {
            delete sender;
        }
        if (reader) {
            delete reader;
        }
        if (actuator) {
            delete actuator;
        }
        if (storage) {
            delete storage;
        }
        trigger = new CCommandPane(QString("trigger"));
        listener = new CCommandPane(QString("listener"));
        transcriber = new CCommandPane(QString("transcriber"));
        sender = new CCommandPane(QString("sender"));
        reader = new CCommandPane(QString("reader"));
        actuator = new CCommandPane(QString("actuator"));
        storage = new CCommandPane(QString("storage"));

        trigger->setCommand(commands_json.getJsonObject().value("trigger").toObject().value("command").toString());
        listener->setCommand(commands_json.getJsonObject().value("listener").toObject().value("command").toString());
        transcriber->setCommand(commands_json.getJsonObject().value("transcriber").toObject().value("command").toString());
        sender->setCommand(commands_json.getJsonObject().value("sender").toObject().value("command").toString());
        reader->setCommand(commands_json.getJsonObject().value("reader").toObject().value("command").toString());
        actuator->setCommand(commands_json.getJsonObject().value("actuator").toObject().value("command").toString());
        storage->setCommand(commands_json.getJsonObject().value("storage").toObject().value("command").toString());

        trigger->setTrigger(commands_json.getJsonObject().value("trigger").toObject().value("trigger_folder").toString());
        listener->setTrigger(commands_json.getJsonObject().value("listener").toObject().value("trigger_folder").toString());
        transcriber->setTrigger(commands_json.getJsonObject().value("transcriber").toObject().value("trigger_folder").toString());
        sender->setTrigger(commands_json.getJsonObject().value("sender").toObject().value("trigger_folder").toString());
        reader->setTrigger(commands_json.getJsonObject().value("reader").toObject().value("trigger_folder").toString());
        actuator->setTrigger(commands_json.getJsonObject().value("actuator").toObject().value("trigger_folder").toString());
        storage->setTrigger(commands_json.getJsonObject().value("storage").toObject().value("trigger_folder").toString());

        ui->modulesLayout->addWidget(trigger, 1);
        ui->modulesLayout->addWidget(listener, 1);
        ui->modulesLayout->addWidget(transcriber, 1);
        ui->modulesLayout->addWidget(sender, 1);
        ui->modulesLayout->addWidget(reader, 1);
        ui->modulesLayout->addWidget(actuator, 1);
        ui->modulesLayout->addWidget(storage, 1);
        ui->modulesLayout->setAlignment(Qt::AlignTop);
    }
    else {
        ui->statusBar->showMessage(QString("Could not open file"), timeout );
    }
}

void MainWindow::readAllJsonFile()
{
    readJson(commandsFile, commands_json);
    readJson(globalVarsFile, global_json);
    readJson(mapFile, map_json);

}

void MainWindow::addEmptyFunction()
{
    CFunctionPane* newPane = new CFunctionPane(this);

    ui->functionParentLayout->addWidget(newPane, 1);
    functions.push_back(newPane);
}

void MainWindow::writeToMap()
{
    if (map_json.isValid()) {
        QJsonObject tmp = QJsonObject();
        for ( auto& func: functions ){
            tmp.insert(func->getName(), func->getJsonRepresentation());
        }
        writeJson(mapFile, tmp);
    }
}

void MainWindow::readChatJson()
{
    // read json
    QFile qfile = QFile(QString(chatFile.c_str()));
    if (qfile.open(QIODevice::ReadOnly)) {

        QByteArray bytes = qfile.readAll();
        qfile.close();

        QJsonParseError jsonError;
        QJsonDocument document = QJsonDocument::fromJson(bytes, &jsonError);
        if (jsonError.error != QJsonParseError::NoError) {
            // set status bar text to error
            const string message = "Error occured when parsing JSON file: " + chatFile;
            ui->statusBar->showMessage(QString(message.c_str()), timeout);
            return;
        }

        if (document.isArray()) {
            chat_json = QJsonArray(document.array());
        }
    } else {
        const string message = "Could not open file for reading: " + chatFile;
        ui->statusBar->showMessage(QString(message.c_str()), timeout);
        return;
    }

    for (QJsonArray::const_iterator iter = chat_json.begin(); iter != chat_json.end(); iter++ ) {
        QString roleString = (*iter).toObject().value("role").toString();
        if ( roleString == RoleString::userString ) {
            addMessage((*iter).toObject().value("content").toString(), Role::User);
        } else if ( roleString == RoleString::assistantString ) {
            addMessage(formatMarkdownWithNewlines(replaceMarkdownBold((*iter).toObject().value("content").toString())), Role::Assistant);
        } else if ( roleString == RoleString::functionString ) {
            addMessage((*iter).toObject().value("name").toString(), Role::Function);
        }
    }

}

void MainWindow::addMessage(QString msg, Role role)
{
    auto label = new Message(msg, role);
    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    chatList.push_back(label);
    if (role == Role::User) {
        ui->chatParentLayout->addWidget(label, chatRowCount++, Qt::AlignRight);
    } else if (role == Role::Assistant){
        ui->chatParentLayout->addWidget(label, chatRowCount++, Qt::AlignLeft);
    } else if (role == Role::Function) {
        for (auto it = chatList.size()-1; it>=0; it--){
            if ( chatList[it]->getType() == Role::Assistant ) {
                chatList[it]->setString(chatList[it]->getString() + QString(": ") + msg);
                break;
            }
        }
    } else if (role == Role::Unknown) {
        ui->chatParentLayout->addWidget(label, chatRowCount++, Qt::AlignRight);
    }
    label->getSmartWidth();
}

void MainWindow::setButtonIndication(Tab tab)
{
    QString unchecked = QString("QPushButton{background-color: #000000}QPushButton::hover{background-color: #505050;color: #ffffff;border-style: solid;border-width: 1px;border-radius: 3px;border-color: #051a39;padding: 5px;}");
    QString checked = QString("QPushButton{background-color: #607cff}QPushButton::hover{background-color: #8399ff;color: #ffffff;border-style: solid;border-width: 1px;border-radius: 3px;border-color: #051a39;padding: 5px;}");
    ui->globalVariablesButton->setStyleSheet(unchecked);
    ui->homeButton->setStyleSheet(unchecked);
    ui->functionsButton->setStyleSheet(unchecked);
    ui->modulesButton->setStyleSheet(unchecked);
    ui->helpButton->setStyleSheet(unchecked);

    switch (tab){
        case Tab::Home:
            ui->homeButton->setStyleSheet(checked);
            break;
        case Tab::Variables:
            ui->globalVariablesButton->setStyleSheet(checked);
            break;
        case Tab::Functions:
            ui->functionsButton->setStyleSheet(checked);
            break;
        case Tab::Modules:
            ui->modulesButton->setStyleSheet(checked);
            break;
        case Tab::Help:
            ui->helpButton->setStyleSheet(checked);
            break;
    }
}

void MainWindow::setupGlobals()
{
    if (global_json.isValid()) {
        foreach(const QString& key, global_json.getJsonObject().keys()) {

            CDictPair* newPair = new CDictPair(this);
            newPair->setValues(key, global_json.getJsonObject().value(key).toString());
            ui->variablesParentLayout->addWidget(newPair, 1);

            variables.push_back(newPair);
        }
        ui->variablesParentLayout->setAlignment(Qt::AlignTop);
    }
    else {
        ui->statusBar->showMessage(QString("Could not open file"), timeout );
    }
}

void MainWindow::stateWarning() const
{
    if (isRunning) {
        ui->statusBar->showMessage(QString("Warning: saving this file does not take affect while running the assistant! Consider restarting."), timeout );
    } else {
        ui->statusBar->showMessage(QString("Warning: you did not start the assistant, your message was not sent!"), timeout );
    }
}

bool MainWindow::isAllReplacementOK()
{
    try {
        replaceGlobalKeys(commands_json.getJsonObject().value("trigger").toObject().value("command").toString(), global_json.getJsonObject());
        replaceGlobalKeys(commands_json.getJsonObject().value("trigger").toObject().value("trigger_folder").toString(), global_json.getJsonObject());
        replaceGlobalKeys(commands_json.getJsonObject().value("transcriber").toObject().value("command").toString(), global_json.getJsonObject());
        replaceGlobalKeys(commands_json.getJsonObject().value("transcriber").toObject().value("trigger_folder").toString(), global_json.getJsonObject());
        replaceGlobalKeys(commands_json.getJsonObject().value("sender").toObject().value("command").toString(), global_json.getJsonObject());
        replaceGlobalKeys(commands_json.getJsonObject().value("sender").toObject().value("trigger_folder").toString(), global_json.getJsonObject());
        replaceGlobalKeys(commands_json.getJsonObject().value("reader").toObject().value("command").toString(), global_json.getJsonObject());
        replaceGlobalKeys(commands_json.getJsonObject().value("reader").toObject().value("trigger_folder").toString(), global_json.getJsonObject());
        replaceGlobalKeys(commands_json.getJsonObject().value("listener").toObject().value("command").toString(), global_json.getJsonObject());
        replaceGlobalKeys(commands_json.getJsonObject().value("listener").toObject().value("trigger_folder").toString(), global_json.getJsonObject());
        replaceGlobalKeys(commands_json.getJsonObject().value("actuator").toObject().value("command").toString(), global_json.getJsonObject());
        replaceGlobalKeys(commands_json.getJsonObject().value("actuator").toObject().value("trigger_folder").toString(), global_json.getJsonObject());

    } catch(string error) {
        bool autoFix = warnMessageForMissingGlobal(error);
        if ( autoFix ) {
            CDictPair* newPair = new CDictPair(this);
            newPair->setValues(QString::fromStdString(error), QString(""));
            newPair->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
            ui->variablesParentLayout->addWidget(newPair, 1);
            variables.push_back(newPair);
            on_globalVariablesButton_clicked();
        }
        return false;
    } catch(...) {
        QMessageBox::warning(this, "Warning", "Something went wrong in global variables replacement. Cannot start assistant.");
        return false;
    }
    return true;
}

bool MainWindow::warnMessageForMissingGlobal(const string &key)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowTitle("Warning");
    msgBox.setText(QString::fromStdString(string("A missing global variable is detected: ") + key));
    msgBox.setInformativeText("Do you want to add this to the global variables?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);

    int ret = msgBox.exec();
    if (ret == QMessageBox::Yes) {
        // User chose Yes
        return true;
    } else {
        // User chose No or closed the dialog
        return false;
    }
}

QString MainWindow::replaceMarkdownBold(const QString input)
{
    QRegularExpression re("\\*\\*(.*?)\\*\\*");

    QString output = input;
    output.replace(re, "<b>\\1</b>");

    return output;
}

QString MainWindow::formatMarkdownWithNewlines(const QString &input)
{
    QString output = input;

    QRegularExpression boldPattern("\\*\\*(.*?)\\*\\*");
    output.replace(boldPattern, "<b>\\1</b>");

    output.replace("\n", "<br>");

    return output;
}

void MainWindow::removeGlobal(CDictPair *pair)
{
    for (int i = variables.size() - 1; i >= 0; i--) {
        if (variables.at(i) == pair) {
            variables.removeAt(i);
        }
    }
    delete pair;
}


void MainWindow::on_addFunctionButton_clicked()
{
    addEmptyFunction();
}


void MainWindow::on_revertFunctionsButton_clicked()
{
    try {
        for (auto &f: functions) {
            delete f;
        }
        functions.clear();
        readAllJsonFile();
        setupFunctions();
    } catch(std::exception ex){
        qDebug() << ex.what();
    }
}


void MainWindow::on_saveFunctionsButton_clicked()
{
    writeToMap();
}


void MainWindow::on_buttonBox_accepted()
{
    if (ui->plainTextEdit->toPlainText() == "") {
        return;
    }
    if (!isRunning) {
        stateWarning();
    }
    addMessage(ui->plainTextEdit->toPlainText(), Role::User);
    string fName = "tmp_transcribed\\temp" + to_string(messageCount) + ".txt";
    if (DEBUG) {
        QFile file(QString(fName.c_str()));
        if (file.open(QIODevice::WriteOnly)) {
            QTextStream stream (&file);
            stream.reset();
            stream << ui->plainTextEdit->toPlainText();
            stream.flush();
            file.close();
        }
    } else {
        sendText(QString::fromStdString(fName), ui->plainTextEdit->toPlainText());
    }
    messageCount++;
    messageCount = 200 + messageCount%100;
    ui->plainTextEdit->clear();
    ui->home_scroll->verticalScrollBar()->setValue(ui->home_scroll->verticalScrollBar()->maximum());
}


void MainWindow::on_buttonBox_rejected()
{
    ui->plainTextEdit->clear();
}


void MainWindow::on_clearChatButton_clicked()
{
    QFile qfile = QFile(QString(chatFile.c_str()));
    qfile.remove();
    on_refreshChatButton_clicked();
}


void MainWindow::on_refreshChatButton_clicked()
{
    if (!isRunning) {
        for (auto& msg : chatList) {
            delete msg;
        }
        chatList.clear();
        readChatJson();
    } else {
        QList<QString> unsaved = QList<QString>();
        bool shouldBeSaved = false;
        for (int it = chatList.size()-1; it >= 0; it--) {
            if (!shouldBeSaved && (chatList[it]->getType() == Role::User || chatList[it]->getType() == Role::Unknown)){
                unsaved.push_back(chatList[it]->getString());
            } else if ( !shouldBeSaved ) {
                shouldBeSaved = true;
            }
            delete chatList[it];
        }
        chatList.clear();
        readChatJson();
        for (int id = unsaved.size()-1; id >= 0; id-- ) {
            bool found = false;
            for (int it = chatList.size()-1; it >= 0; it--) {
                if (QString::compare(chatList[it]->getString().trimmed(), unsaved[id].trimmed(), Qt::CaseInsensitive) == 0){
                    found = true;
                    break;
                }
            }
            if (!found) {
                addMessage(unsaved[id], Role::Unknown);
            }
        }
    }
    readAllStandardOutput();
}


void MainWindow::on_revertModulesButton_clicked()
{
    readAllJsonFile();
    setupCommands();
}


void MainWindow::on_saveModulesButton_clicked()
{
    if (isRunning) {
        stateWarning();
    }
    QJsonObject jsonObj(commands_json.getJsonObject());


    QJsonObject triggerObj = jsonObj["trigger"].toObject();
    QJsonObject listenerObj = jsonObj["listener"].toObject();
    QJsonObject transcriberObj = jsonObj["transcriber"].toObject();
    QJsonObject senderObj = jsonObj["sender"].toObject();
    QJsonObject readerObj = jsonObj["reader"].toObject();
    QJsonObject actuatorObj = jsonObj["actuator"].toObject();
    QJsonObject storageObj = jsonObj["storage"].toObject();


    triggerObj["command"] = trigger->getCommand();
    triggerObj["trigger_folder"] = trigger->getTrigger();

    listenerObj["command"] = listener->getCommand();
    listenerObj["trigger_folder"] = listener->getTrigger();

    transcriberObj["command"] = transcriber->getCommand();
    transcriberObj["trigger_folder"] = transcriber->getTrigger();

    senderObj["command"] = sender->getCommand();
    senderObj["trigger_folder"] = sender->getTrigger();

    readerObj["command"] = reader->getCommand();
    readerObj["trigger_folder"] = reader->getTrigger();

    actuatorObj["command"] = actuator->getCommand();
    actuatorObj["trigger_folder"] = actuator->getTrigger();

    storageObj["command"] = storage->getCommand();
    storageObj["trigger_folder"] = storage->getTrigger();

    jsonObj["trigger"] = triggerObj;
    jsonObj["listener"] = listenerObj;
    jsonObj["transcriber"] = transcriberObj;
    jsonObj["sender"] = senderObj;
    jsonObj["reader"] = readerObj;
    jsonObj["actuator"] = actuatorObj;
    jsonObj["storage"] = storageObj;

    writeJson(commandsFile, jsonObj);
}


void MainWindow::on_revertVariablesButton_clicked()
{
    for (auto& var : variables) {
        delete var;
    }
    variables.clear();
    readAllJsonFile();
    setupGlobals();
}


void MainWindow::on_saveVariablesButton_clicked()
{
    if (isRunning) {
        stateWarning();
    }
    QJsonObject jsonObj = QJsonObject();
    for (auto& var: variables) {
        jsonObj.insert(var->getKey(), QJsonValue::fromVariant(var->getValue()));
    }
    writeJson(globalVarsFile, jsonObj);
    readAllJsonFile();
}


void MainWindow::on_addVariableButton_clicked()
{
    CDictPair* newPair = new CDictPair(this);
    newPair->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    ui->variablesParentLayout->addWidget(newPair, 1);

    variables.push_back(newPair);
}

void MainWindow::chat_listener()
{
    on_refreshChatButton_clicked();
}

void MainWindow::on_keyboard_function_assaign()
{
    KeyboardWindow* newWindow = new KeyboardWindow(QString("typer.exe PRESS{cmd}RELEASE{cmd}SLEEP{1}TYPE{{0}aa{{}}{a{{}}{alma}}}SLEEP{1}PRESS{enter}RELEASE{enter}"), nullptr);

    connect(newWindow, &KeyboardWindow::returnString, this, &MainWindow::onStringReturned);
    newWindow->setStyleSheet(this->styleSheet());
    newWindow->setWindowTitle("Keyboard function");
    newWindow->setAttribute(Qt::WA_DeleteOnClose);
    newWindow->show();
}

void MainWindow::onStringReturned(const QString &returnedString)
{
    qDebug() << "returned: " << returnedString;
}


void MainWindow::on_speakerButton_clicked()
{
    if (isSpeakerOn) {
        QIcon speakerIcon("speaker-muted.png");
        ui->speakerButton->setIcon(speakerIcon);
        if (global_json.getJsonObject().contains("speaker")) {
            for (auto& var: variables){
                if (QString::compare(var->getKey().trimmed(), QString("speaker")) == 0) {
                    var->setValues(var->getKey(), QString("False"));
                    break;
                }
            }
        } else {
            CDictPair* newPair = new CDictPair(this);
            newPair->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
            newPair->setValues(QString("speaker"), QString("False"));
            ui->variablesParentLayout->addWidget(newPair, 1);
            variables.push_back(newPair);
        }
    } else {
        QIcon speakerIcon("speaker-on.png");
        ui->speakerButton->setIcon(speakerIcon);
        if (global_json.getJsonObject().contains("speaker")) {
            for (auto& var: variables){
                if (QString::compare(var->getKey().trimmed(), QString("speaker")) == 0) {
                    var->setValues(var->getKey(), QString("True"));
                    break;
                }
            }
        } else {
            CDictPair* newPair = new CDictPair(this);
            newPair->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
            newPair->setValues(QString("speaker"), QString("True"));
            ui->variablesParentLayout->addWidget(newPair, 1);
            variables.push_back(newPair);
        }
    }
    on_saveVariablesButton_clicked();
    isSpeakerOn = !isSpeakerOn;
}


void MainWindow::on_micButton_clicked()
{
    if (isMicOn) {
        QIcon micIcon("mic-muted.png");
        ui->micButton->setIcon(micIcon);
    } else {
        QIcon micIcon("mic-on.png");
        ui->micButton->setIcon(micIcon);
    }
    isMicOn = !isMicOn;
}

QString MainWindow::replaceGlobalKeys(const QString &input, const QJsonObject &jsonObject) const {
    QString output = input;

    QRegularExpression re("\\{GLOBAL:(\\w+)\\}");
    QRegularExpressionMatchIterator it = re.globalMatch(input);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString key = match.captured(1);

        if (jsonObject.contains(key)) {
            QString replacement = jsonObject.value(key).toString();

            // Replace {GLOBAL:key} with the corresponding value from QJsonObject
            output.replace(match.captured(0), replacement);
        } else {
            throw key.toStdString();
        }
    }

    return output;
}

void MainWindow::sendText(const QString &filename, const QString &text)
{

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QUrl serverUrl("http://127.0.0.1:5000/storefile");

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart filenamePart;
    filenamePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"filename\""));
    filenamePart.setBody(filename.toUtf8());
    multiPart->append(filenamePart);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"" + filename + "\""));
    filePart.setBody(text.toUtf8());
    multiPart->append(filePart);

    QNetworkRequest request(serverUrl);

    QNetworkReply *reply = manager->post(request, multiPart);
    multiPart->setParent(reply);

    QObject::connect(reply, &QNetworkReply::finished, [reply, manager]() {
        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "Upload successful:" << reply->readAll();
        } else {
            qDebug() << "Error occurred:" << reply->errorString();
        }

        // cleanup
        reply->deleteLater();
        manager->deleteLater();
    });
}

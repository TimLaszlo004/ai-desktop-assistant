#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <string>
#include "cfunctionpane.h"
#include "cjsonobj.h"
#include "ccommandpane.h"
#include "roles.h"
#include "cdictpair.h"
#include "keyboardwindow.h"
#include "message.h"

using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void removeFunction(CFunctionPane* function);
    void removeGlobal(CDictPair* pair);
    ~MainWindow();
    static MainWindow* instance;

private slots:

    void on_homeButton_clicked();

    void on_modulesButton_clicked();

    void on_functionsButton_clicked();

    void on_helpButton_clicked();

    void on_startButton_clicked();

    void on_globalVariablesButton_clicked();

    void startProcess();

    void stopProcess();

    void readAllStandardOutput();


    void on_addFunctionButton_clicked();

    void on_revertFunctionsButton_clicked();

    void on_saveFunctionsButton_clicked();

    void on_buttonBox_accepted();

    void on_buttonBox_rejected();


    void on_clearChatButton_clicked();

    void on_refreshChatButton_clicked();

    void on_revertModulesButton_clicked();

    void on_saveModulesButton_clicked();

    void on_revertVariablesButton_clicked();

    void on_saveVariablesButton_clicked();

    void on_addVariableButton_clicked();

    void chat_listener();

    void on_keyboard_function_assaign();
    void onStringReturned(const QString& returnedString);

    void on_speakerButton_clicked();

    void on_micButton_clicked();

private:
    Ui::MainWindow *ui;

    const int timeout = 5000;

    void readJson(const string& file, CJsonObj& obj);

    void writeJson(const string& file, const QJsonObject& jsonObj) const;

    void setupFunctions();

    void setupCommands();

    void readAllJsonFile();

    void addEmptyFunction();

    void writeToMap();

    void readChatJson();

    void addMessage(QString msg, Role role);

    void setButtonIndication(Tab tab);

    void setupGlobals();

    void stateWarning()const;

    bool isAllReplacementOK();

    bool warnMessageForMissingGlobal(const string& key);

    QString replaceMarkdownBold(const QString input);

    QString formatMarkdownWithNewlines(const QString& input);

    QString replaceGlobalKeys(const QString &input, const QJsonObject &jsonObject) const;

    void sendText(const QString &filename, const QString &text);

    int messageCount = 200;
    int chatRowCount = 0;
    QList<Message*> chatList = QList<Message*>();

    QProcess* process = nullptr;
    QProcess* listener_process = nullptr;
    QProcess* storage_process = nullptr;

    string mapFile = "map.json";
    CJsonObj map_json = CJsonObj();
    string commandsFile = "commands.json";
    CJsonObj commands_json = CJsonObj();

    string globalVarsFile = "global.json";
    CJsonObj global_json = CJsonObj();

    string chatFile = "tmp_chat\\output.json";
    QJsonArray chat_json = QJsonArray();

    CCommandPane* trigger = nullptr;
    CCommandPane* listener = nullptr;
    CCommandPane* transcriber = nullptr;
    CCommandPane* sender = nullptr;
    CCommandPane* reader = nullptr;
    CCommandPane* actuator = nullptr;
    CCommandPane* storage = nullptr;

    QList<CFunctionPane*> functions = QList<CFunctionPane*>();
    QList<CDictPair*> variables = QList<CDictPair*>();
    bool isRunning = false;
    bool isSpeakerOn = true;
    bool isMicOn = true;
    bool DEBUG = false;
};
#endif // MAINWINDOW_H

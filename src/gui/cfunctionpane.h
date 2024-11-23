#ifndef CFUNCTIONPANE_H
#define CFUNCTIONPANE_H

#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <list>
#include "cfunctionparam.h"

class MainWindow;

using namespace std;

class CFunctionPane : public QWidget
{
    Q_OBJECT
    int rowCounter = 0;
    bool minimized = false;
    MainWindow* holder = nullptr;
protected:
    void paintEvent(QPaintEvent *event) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        QColor backgroundColor(45, 45, 45);

        QBrush brush(backgroundColor);
        painter.setBrush(brush);
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(rect(), 10, 10);
    }
public:
    explicit CFunctionPane(MainWindow* holder, QWidget *parent = nullptr);
    ~CFunctionPane();
    QGridLayout* layout = nullptr;
    QPushButton* lblName = nullptr;
    QLineEdit* lEditName = nullptr;
    QLabel* lblDesc = nullptr;
    QLineEdit* lEditDesc = nullptr;
    QLabel* lblCommand = nullptr;
    QLineEdit* lEditCommand = nullptr;
    QCheckBox* cboxIsDestructive = nullptr;
    QPushButton* btnPlus = nullptr;
    QPushButton* btnMinus = nullptr;
    QLabel* lblParams = nullptr;
    QPushButton* smartFuncBtn = nullptr;

    QList<CFunctionParam*> params = QList<CFunctionParam*>();

    QJsonObject getJsonRepresentation()const;
    QString getName()const;


    void addEmptyParam();
    void addFullParam(QString name, QString description, QString type, bool optional);
    void removeParam(CFunctionParam* param);

private slots:
    void on_addParam_clicked();
    void on_remove();
    void on_minimize();
    void on_keyboard_function_assaign();
    void onStringReturned(const QString& returnedString);
};

#endif // CFUNCTIONPANE_H

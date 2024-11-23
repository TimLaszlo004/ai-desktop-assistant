#ifndef CFUNCTIONPARAM_H
#define CFUNCTIONPARAM_H

#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include "orientablepushbutton.h"

class CFunctionPane;

class CFunctionParam : public QWidget
{
    Q_OBJECT
    QLabel* lblName = nullptr;
    QLineEdit* lEditName = nullptr;
    QLabel* lblDesc = nullptr;
    QLineEdit* lEditDesc = nullptr;
    QLabel* lblType = nullptr;
    QLineEdit* lEditType = nullptr;
    QCheckBox* cboxIsOptional = nullptr;

    OrientablePushButton* btnRemove = nullptr;

    QGridLayout* layout = nullptr;

    CFunctionPane* parentFunction = nullptr;
protected:
    void paintEvent(QPaintEvent *event) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        QColor backgroundColor(60, 60, 60);

        QBrush brush(backgroundColor);
        painter.setBrush(brush);
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(rect(), 10, 10);
    }
public:
    explicit CFunctionParam(CFunctionPane* functionParent, QWidget *parent = nullptr);
    ~CFunctionParam();

    QLineEdit *getLEditName();
    QLineEdit *getLEditDesc();
    QLineEdit *getLEditType();
    QCheckBox *getCBoxIsOptional();

    QJsonObject getJsonRepresentation()const;
    QString getName()const;

    bool isRequired()const;

    void setDescText(QString text);
private slots:
    void onRemove();
};

#endif // CFUNCTIONPARAM_H

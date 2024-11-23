#ifndef MESSAGE_H
#define MESSAGE_H

#include <QWidget>
#include <QtWidgets>
#include "roles.h"


class Message : public QWidget
{
    Q_OBJECT
    QLabel* label = nullptr;
    QHBoxLayout* layout = nullptr;
    Role type;
    QColor color;
protected:
    void paintEvent(QPaintEvent *event) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        QBrush brush(color);
        painter.setBrush(brush);
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(rect(), 10, 10);
    }
public:
    explicit Message(QString lbl, Role msgType = Role::Unknown,  QWidget *parent = nullptr);
    ~Message();

    Role getType() const;
    void setString(QString lbl);
    QString getString()const;
    void setWordWrap(bool on);
    int getSmartWidth();

signals:
};

#endif // MESSAGE_H

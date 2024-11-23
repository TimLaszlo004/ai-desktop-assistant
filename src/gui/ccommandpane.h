#ifndef CCOMMANDPANE_H
#define CCOMMANDPANE_H

#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <string>
#include <QPainter>

using namespace std;

class CCommandPane : public QWidget
{
    Q_OBJECT
    QGridLayout* layout = nullptr;

    QLabel* lblName = nullptr;
    QLabel* lblCommand = nullptr;
    QLabel* lblTrigger = nullptr;

    QLineEdit* lEditCommand = nullptr;
    QLineEdit* lEditTrigger = nullptr;

    QPushButton* btnFileDialog = nullptr;

    QFrame* endLine = nullptr;
protected:
    // Override paintEvent to draw rounded corners
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
    explicit CCommandPane (const QString& name, QWidget* parent = nullptr);
    void setCommand(const QString& command);
    void setTrigger(const QString& trigger);
    QString getCommand() const;
    QString getTrigger() const;
    virtual ~CCommandPane ();
private slots:
    void getDirectory();
};

#endif // CCOMMANDPANE_H

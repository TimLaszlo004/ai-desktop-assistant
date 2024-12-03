#ifndef CDICTPAIR_H
#define CDICTPAIR_H

#include <QWidget>
#include <QtWidgets>

class MainWindow;

class CDictPair : public QWidget
{
    Q_OBJECT

    QLineEdit* key = nullptr;
    QLineEdit* value = nullptr;
    QPushButton* removeBtn = nullptr;
    MainWindow* holder = nullptr;

    QGridLayout* layout = nullptr;
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
    explicit CDictPair(MainWindow* holder, QWidget *parent = nullptr);
    void setValues(QString _key, QString _value);
    QString getKey() const;
    QString getValue() const;
    ~CDictPair();

private slots:
    void onRemove();
};

#endif // CDICTPAIR_H

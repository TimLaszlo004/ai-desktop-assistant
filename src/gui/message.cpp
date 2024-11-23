#include "message.h"


Role Message::getType() const
{
    return type;
}

void Message::setString(QString lbl)
{
    if (label) {
        label->setText(lbl);
    }
}

QString Message::getString() const
{
    if (label) {
        return label->text();
    }
    return QString("");
}

void Message::setWordWrap(bool on)
{
    label->setWordWrap(on);
}

int Message::getSmartWidth()
{
    label->setWordWrap(false);
    QFontMetrics fontMetrics(label->font());
    int textWidth = fontMetrics.horizontalAdvance(label->text());

    const int minWidth = 600;
    const int maxWidth = 1000;

    if (textWidth > maxWidth) {
        label->setWordWrap(true);
        QFontMetrics fontMetricsAgain(label->font());
        textWidth = fontMetricsAgain.horizontalAdvance(label->text());
    } else {
        return -1;
    }

    int idealWidth = qMin(textWidth + 20, maxWidth);
    idealWidth = qMax(idealWidth, minWidth);

    label->setMaximumWidth(idealWidth);
    label->setMinimumWidth(minWidth);
    return -1;
}

Message::Message(QString lbl, Role msgType, QWidget *parent)
: QWidget{parent}
{
    switch (msgType){
        case Role::User:
            color = QColor(40, 40, 80);
            break;
        case Role::Assistant:
            color = QColor(40, 40, 40);
            break;
        case Role::Unknown:
            color = QColor(80, 40, 60);
            break;
        case Role::Function:
            color = QColor(40, 40, 40);
            break;
    }
    type = msgType;
    label = new QLabel(lbl, this);
    label->setTextFormat(Qt::RichText);
    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    layout = new QHBoxLayout(this);
    layout->addWidget(label);
    setLayout(layout);
    this->setStyleSheet(QString("QLabel{background-color:rgb(" + QString(std::to_string(color.red()).c_str()) + ", "
                                                               + QString(std::to_string(color.green()).c_str()) + ", "
                                                               + QString(std::to_string(color.blue()).c_str()) + ")}"));
}

Message::~Message()
{
    delete label;
    delete layout;
}

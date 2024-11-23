#include "cfunctionparam.h"
#include "cfunctionpane.h"

QLineEdit *CFunctionParam::getLEditName()
{
    return lEditName;
}

QLineEdit *CFunctionParam::getLEditDesc()
{
    return lEditDesc;
}

QLineEdit *CFunctionParam::getLEditType()
{
    return lEditType;
}

QCheckBox *CFunctionParam::getCBoxIsOptional()
{
    return cboxIsOptional;
}

QJsonObject CFunctionParam::getJsonRepresentation() const
{
    QJsonObject jsonizedLittle = QJsonObject();
    if (lEditDesc) {
        jsonizedLittle.insert(QString("description"), lEditDesc->text());
    }
    if (lEditType) {
        jsonizedLittle.insert(QString("type"), lEditType->text());
    }

    return jsonizedLittle;
}

QString CFunctionParam::getName() const
{
    if (lEditName) {
        return lEditName->text();
    }
    return QString("unknown");
}

bool CFunctionParam::isRequired() const
{
    return !(cboxIsOptional->isChecked());
}

void CFunctionParam::setDescText(QString text)
{
    lEditDesc->setText(text);
}

void CFunctionParam::onRemove()
{
    parentFunction->removeParam(this);
}

CFunctionParam::CFunctionParam(CFunctionPane* functionParent, QWidget *parent)
    : QWidget{parent}
{
    parentFunction = functionParent;
    auto horizontal = QSizePolicy::Preferred;
    auto vertical = QSizePolicy::Preferred;
    lblName = new QLabel(QString("Name"), this);
    lEditName = new QLineEdit(this);
    lblDesc = new QLabel(QString("Description"), this);
    lEditDesc = new QLineEdit(this);
    lblType = new QLabel(QString("Type"), this);
    lEditType = new QLineEdit(this);
    cboxIsOptional = new QCheckBox(QString("Optional"), this);

    lblName->setSizePolicy(horizontal, vertical);
    lEditName->setSizePolicy(horizontal, vertical);
    lblDesc->setSizePolicy(horizontal, vertical);
    lEditDesc->setSizePolicy(horizontal, vertical);
    lblType->setSizePolicy(horizontal, vertical);
    lEditType->setSizePolicy(horizontal, vertical);
    cboxIsOptional->setSizePolicy(horizontal, vertical);

    btnRemove = new OrientablePushButton("Remove", this);
    btnRemove->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    btnRemove->setOrientation(OrientablePushButton::VerticalBottomToTop);

    connect(btnRemove, SIGNAL(clicked()), SLOT(onRemove()));

    layout = new QGridLayout(this);
    int rowCounter = 0;

    layout->addWidget(btnRemove, rowCounter, 0, 4, 1);
    layout->addWidget(lblName, rowCounter, 1);
    layout->addWidget(lEditName, rowCounter++, 2, 1, 2);
    layout->addWidget(lblDesc, rowCounter, 1);
    layout->addWidget(lEditDesc, rowCounter++, 2, 1, 2);
    layout->addWidget(lblType, rowCounter, 1);
    layout->addWidget(lEditType, rowCounter++, 2, 1, 2);
    layout->addWidget(cboxIsOptional, rowCounter++, 1);

    setLayout(layout);
    QString customStyle = QString(QString("QLabel{"
                                          "font-size:12pt;"
                                          "font:bold;"
                                          "}"
                                          "QFrame{"
                                          "background-color: #353535;"
                                          "}"
                                          "QCheckBox{"
                                          "background-color: #353535;"
                                          "}"));
    this->setStyleSheet(customStyle);
}

CFunctionParam::~CFunctionParam()
{
    delete lblName;
    delete lEditName;
    delete lblDesc;
    delete lEditDesc;
    delete lblType;
    delete lEditType;
    delete cboxIsOptional;
    delete btnRemove;
    delete layout;
}

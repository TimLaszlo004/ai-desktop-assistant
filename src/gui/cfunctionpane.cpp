#include "cfunctionpane.h"
#include "cfunctionparam.h"
#include "mainwindow.h"

#include <QPainter>

CFunctionPane::CFunctionPane(MainWindow* holder, QWidget *parent)
    : QWidget{parent},
    holder(holder)
{

    auto horizontal = QSizePolicy::Preferred;
    auto vertical = QSizePolicy::Preferred;
    lblName = new QPushButton(QString("Name"), this);
    lblName->setStyleSheet(QString("background-color: rgb(45, 45, 45); text-align:left; font-size:14pt;font:bold;"));
    lEditName = new QLineEdit(this);
    lblDesc = new QLabel(QString("Description"), this);
    lEditDesc = new QLineEdit(this);
    lblCommand = new QLabel(QString("Command"), this);
    lEditCommand = new QLineEdit(this);
    lblName->setSizePolicy(horizontal, vertical);
    lEditName->setSizePolicy(horizontal, vertical);
    lblDesc->setSizePolicy(horizontal, vertical);
    lEditDesc->setSizePolicy(horizontal, vertical);
    lblCommand->setSizePolicy(horizontal, vertical);
    lEditCommand->setSizePolicy(horizontal, vertical);

    cboxIsDestructive = new QCheckBox(QString("Destructive"), this);
    cboxIsDestructive->setSizePolicy(horizontal, vertical);
    lblParams = new QLabel(QString("Parameters"), this);
    lblParams->setSizePolicy(horizontal, vertical);
    btnMinus = new QPushButton(QString("Remove"), this);
    btnMinus->setSizePolicy(horizontal, vertical);
    btnPlus = new QPushButton(QString("Add parameter"), this);
    btnPlus->setSizePolicy(horizontal, vertical);
    smartFuncBtn = new QPushButton(QString("Edit keyfunc"), this);
    smartFuncBtn->setSizePolicy(horizontal, vertical);
    QObject::connect(btnPlus, SIGNAL(clicked()), SLOT(on_addParam_clicked()));
    QObject::connect(btnMinus, SIGNAL(clicked()), SLOT(on_remove()));
    QObject::connect(lblName, SIGNAL(clicked()), SLOT(on_minimize()));
    QObject::connect(smartFuncBtn, SIGNAL(clicked()), SLOT(on_keyboard_function_assaign()));


    layout = new QGridLayout(this);
    rowCounter = 0;
    layout->addWidget(lblName, rowCounter, 0, 1, 4);
    layout->addWidget(btnMinus, rowCounter++, 4);
    layout->addWidget(lEditName, rowCounter++, 0, 1, 5);
    layout->addWidget(lblDesc, rowCounter++, 0, 1, 5);
    layout->addWidget(lEditDesc, rowCounter++, 0, 1, 5);
    layout->addWidget(lblCommand, rowCounter++, 0, 1, 5);
    layout->addWidget(lEditCommand, rowCounter, 0, 1, 4);
    layout->addWidget(smartFuncBtn, rowCounter++, 4, 1, 1);
    layout->addWidget(cboxIsDestructive, rowCounter++, 0);
    layout->addWidget(lblParams, rowCounter, 0);
    layout->addWidget(btnPlus, rowCounter++, 4);
    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    setLayout(layout);
    QString customStyle = QString(QString("QLabel{"
                                         "font-size:14pt;"
                                         "font:bold;"
                                         "}"
                                         "QCheckBox{"
                                         "background-color: rgb(45, 45, 45);"
                                         "}"));
    this->setStyleSheet(customStyle);
}

CFunctionPane::~CFunctionPane()
{
    for (auto& p : params) {
        delete p;
    }
    params.clear();
    delete lblName;
    delete lEditName;
    delete lblDesc;
    delete lEditDesc;
    delete lblCommand;
    delete lEditCommand;
    delete btnPlus;
    delete btnMinus;
    delete lblParams;
    delete cboxIsDestructive;
    delete layout;
}



QJsonObject CFunctionPane::getJsonRepresentation() const
{
    QJsonObject little = QJsonObject();
    little.insert(QString("description"), lEditDesc->text());
    little.insert(QString("command"), lEditCommand->text());
    little.insert(QString("destructive"), cboxIsDestructive->isChecked());

    // add params and required
    QJsonObject paramsObject = QJsonObject();
    QStringList required = QStringList();
    for (auto& p: params) {
        paramsObject.insert(p->getName(), p->getJsonRepresentation());
        if (p->isRequired()) {
            required.push_back(p->getName());
        }
    }
    little.insert(QString("parameters"), paramsObject);
    little.insert(QString("required"), QJsonValue::fromVariant(required));


    return little;
}

QString CFunctionPane::getName() const
{
    return lEditName->text();
}

void CFunctionPane::addEmptyParam()
{
    CFunctionParam* par1 = new CFunctionParam(this);
    params.push_back(par1);

    layout->addWidget(par1, rowCounter, 0, 4, 5);
    rowCounter += 4;
}

void CFunctionPane::addFullParam(QString name, QString description, QString type, bool optional)
{
    CFunctionParam* par1 = new CFunctionParam(this);
    par1->getLEditName()->setText(name);
    par1->getLEditDesc()->setText(description);
    par1->getLEditType()->setText(type);
    par1->getCBoxIsOptional()->setChecked(optional);
    params.push_back(par1);

    layout->addWidget(par1, rowCounter, 0, 4, 5);
    rowCounter += 4;
}

void CFunctionPane::removeParam(CFunctionParam *param)
{
    for (int i = params.size() - 1; i >= 0; i--) {
        if (params.at(i) == param) {
            params.removeAt(i);
        }
    }
    delete param;
}

void CFunctionPane::on_addParam_clicked()
{
    addEmptyParam();
}

void CFunctionPane::on_remove()
{
    holder->removeFunction(this);
}

void CFunctionPane::on_minimize()
{
    if (!minimized){
        lEditName->hide();
        lblDesc->hide();
        lEditDesc->hide();
        lblCommand->hide();
        lEditCommand->hide();
        cboxIsDestructive->hide();
        lblParams->hide();
        btnPlus->hide();
        smartFuncBtn->hide();
        for (auto& p: params){
            p->hide();
        }
        lblName->setText(QString("Name: ") + lEditName->text());
    } else {
        lEditName->show();
        lblDesc->show();
        lEditDesc->show();
        lblCommand->show();
        lEditCommand->show();
        cboxIsDestructive->show();
        lblParams->show();
        btnPlus->show();
        smartFuncBtn->show();
        for (auto& p: params){
            p->show();
        }
        lblName->setText(QString("Name"));
    }
    minimized = ! minimized;
}

void CFunctionPane::on_keyboard_function_assaign()
{
    KeyboardWindow* newWindow = new KeyboardWindow(lEditCommand->text(), nullptr);

    connect(newWindow, &KeyboardWindow::returnString, this, &CFunctionPane::onStringReturned);
    newWindow->setStyleSheet((MainWindow::instance)->styleSheet());
    newWindow->setWindowTitle(QString("Keyboard function for ") + lEditName->text());
    newWindow->setAttribute(Qt::WA_DeleteOnClose);
    newWindow->resize(800, 600);
    newWindow->show();
}

void CFunctionPane::onStringReturned(const QString &returnedString)
{
    qDebug() << "returned: " << returnedString;
    lEditCommand->setText(returnedString);
}


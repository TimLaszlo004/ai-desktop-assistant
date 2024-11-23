#include "ccommandpane.h"
#include <QPalette>


CCommandPane::CCommandPane(const QString &name, QWidget *parent): QWidget(parent)
{
    setObjectName("customWidget");
    auto horizontal = QSizePolicy::Preferred;
    auto vertical = QSizePolicy::Preferred;

    layout = new QGridLayout(this);

    lblName = new QLabel(name, this);
    lblCommand = new QLabel(QString("Command"), this);
    lblTrigger = new QLabel(QString("Trigger Folder"), this);

    lEditCommand = new QLineEdit(this);
    lEditTrigger = new QLineEdit(this);

    btnFileDialog = new QPushButton("Change", this);
    connect(btnFileDialog, SIGNAL(clicked()), SLOT(getDirectory()));

    endLine = new QFrame();
    endLine->setFrameShape(QFrame::HLine);
    endLine->setStyleSheet(QString("background-color:#999999;"));

    lblName->setSizePolicy(horizontal, vertical);
    lblCommand->setSizePolicy(horizontal, vertical);
    lblTrigger->setSizePolicy(horizontal, vertical);
    lEditCommand->setSizePolicy(horizontal, vertical);
    lEditTrigger->setSizePolicy(horizontal, vertical);

    int rowCounter = 0;
    layout->addWidget(lblName, rowCounter++, 0);
    layout->addWidget(lblCommand, rowCounter, 0);
    layout->addWidget(lEditCommand, rowCounter++, 1, 1, 5);
    layout->addWidget(lblTrigger, rowCounter, 0);
    layout->addWidget(lEditTrigger, rowCounter, 1, 1, 4);
    layout->addWidget(btnFileDialog, rowCounter, 5);

    setLayout(layout);
    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QString customStyle = QString(QString("#customWidget{"
                                          "font-size:12pt;"
                                          "font:bold;"
                                          "background-color: rgb(230, 230, 230);"
                                          "border-radius: 15px;"
                                          "}"
        ));

}

void CCommandPane::setCommand(const QString &command)
{
    lEditCommand->setText(command);
}

void CCommandPane::setTrigger(const QString &trigger)
{
    lEditTrigger->setText(trigger);
}

QString CCommandPane::getCommand() const
{
    return lEditCommand->text();
}

QString CCommandPane::getTrigger() const
{
    return lEditTrigger->text();
}

CCommandPane::~CCommandPane()
{
    delete lblName;
    delete lblCommand;
    delete lblTrigger;
    delete lEditCommand;
    delete lEditTrigger;
    delete btnFileDialog;
    delete endLine;
    delete layout;
}

void CCommandPane::getDirectory()
{
    QString dirName = QFileDialog::getExistingDirectory();
    lEditTrigger->setText(dirName);
}

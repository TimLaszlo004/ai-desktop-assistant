#include "keyboardwindow.h"


KeyboardWindow::~KeyboardWindow()
{
    delete listWidget;
    delete typeBtn;
    delete pressBtn;
    delete releaseBtn;
    delete sleepBtn;
    delete acceptBtn;
    delete layout;
}

KeyboardWindow::KeyboardWindow(const QString& startState, QWidget *parent)
    : QWidget{parent}
{
    typeBtn = new QPushButton(QString("TYPE"), this);
    typeBtn->setMinimumWidth(100);
    typeBtn->setToolTip(QString("A sequence of characters"));
    pressBtn = new QPushButton(QString("PRESS"), this);
    pressBtn->setMinimumWidth(100);
    pressBtn->setToolTip(QString("Press a special key"));
    releaseBtn = new QPushButton(QString("RELEASE"), this);
    releaseBtn->setMinimumWidth(100);
    releaseBtn->setToolTip(QString("Release a special key"));
    sleepBtn = new QPushButton(QString("SLEEP"), this);
    sleepBtn->setMinimumWidth(100);
    sleepBtn->setToolTip(QString("Waiting time to continue execution"));
    acceptBtn = new QPushButton(QString("Save and exit"), this);

    listWidget = new QListWidget(this);

    listWidget->setDragEnabled(true);
    listWidget->setAcceptDrops(true);
    listWidget->setDropIndicatorShown(true);
    listWidget->setDefaultDropAction(Qt::MoveAction);
    listWidget->setDragDropMode(QAbstractItemView::InternalMove);
    listWidget->setSpacing(10);

    layout = new QGridLayout(this);
    layout->addWidget(typeBtn, 0, 0, 1, 1);
    layout->addWidget(pressBtn, 0, 1, 1, 1);
    layout->addWidget(releaseBtn, 0, 2, 1, 1);
    layout->addWidget(sleepBtn, 0, 3, 1, 1);
    layout->addWidget(listWidget, 1, 0, 1, 4);
    layout->addWidget(acceptBtn, 2, 3, 1, 1);
    connect(acceptBtn, SIGNAL(clicked()), SLOT(onButtonPressed()));
    connect(typeBtn, SIGNAL(clicked()), SLOT(onAddType()));
    connect(pressBtn, SIGNAL(clicked()), SLOT(onAddPress()));
    connect(releaseBtn, SIGNAL(clicked()), SLOT(onAddRelease()));
    connect(sleepBtn, SIGNAL(clicked()), SLOT(onAddSleep()));
    this->setLayout(layout);

    parseString(startState);
}

void KeyboardWindow::removeElement(CustomListElement *element)
{
    for (int i = 0; i < listWidget->count(); ++i) {
        QListWidgetItem* item = listWidget->item(i);
        QWidget* widget = listWidget->itemWidget(item);

        if (widget == element) {
            listWidget->removeItemWidget(item);
            delete widget;
            delete listWidget->takeItem(i);
            break;
        }
    }
}

void KeyboardWindow::onAddSleep()
{
    addCustomListItem(listWidget, "SLEEP", NumberType);
}

void KeyboardWindow::onAddPress()
{
    addCustomListItem(listWidget, "PRESS", ComboBoxType);
}

void KeyboardWindow::onAddRelease()
{
    addCustomListItem(listWidget, "RELEASE", ComboBoxType);
}

void KeyboardWindow::onAddType()
{
    addCustomListItem(listWidget, "TYPE", LineEditType);
}

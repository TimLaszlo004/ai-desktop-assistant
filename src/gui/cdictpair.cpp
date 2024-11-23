#include "cdictpair.h"
#include "mainwindow.h"

CDictPair::CDictPair(MainWindow* holder, QWidget *parent)
    : QWidget{parent},
    holder(holder)
{
    key = new QLineEdit(this);
    value = new QLineEdit(this);

    removeBtn = new QPushButton(QString("Remove"), this);
    connect(removeBtn, SIGNAL(clicked()), SLOT(onRemove()));

    layout = new QGridLayout();

    layout->addWidget(key, 0, 0, 1, 2);
    layout->addWidget(value, 0, 2, 1, 1);
    layout->addWidget(removeBtn, 0, 4, 1, 1);
    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    setLayout(layout);
}

void CDictPair::setValues(QString _key, QString _value)
{
    key->setText(_key);
    value->setText(_value);
}

QString CDictPair::getKey() const
{
    return key->text();
}

QString CDictPair::getValue() const
{
    return value->text();
}

CDictPair::~CDictPair()
{
    delete key;
    delete value;
    delete removeBtn;
    delete layout;
}

void CDictPair::onRemove(){
    holder->removeGlobal(this);
}

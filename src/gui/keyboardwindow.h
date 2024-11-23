#ifndef KEYBOARDWINDOW_H
#define KEYBOARDWINDOW_H

#include <QWidget>
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <string>
#include "customlistelement.h"

class KeyboardWindow : public QWidget
{
    Q_OBJECT
    QListWidget *listWidget = nullptr;
    QPushButton* typeBtn = nullptr;
    QPushButton* pressBtn = nullptr;
    QPushButton* releaseBtn = nullptr;
    QPushButton* sleepBtn = nullptr;
    QPushButton* acceptBtn = nullptr;
    QGridLayout *layout = nullptr;

public:
    ~KeyboardWindow();
    explicit KeyboardWindow(const QString& startState = QString(""), QWidget *parent = nullptr);
    void addCustomListItem(QListWidget *listWidget, const QString &labelText, InputType inputType) {
        QListWidgetItem *item = new QListWidgetItem();
        CustomListElement *customWidget = new CustomListElement(labelText, inputType, this);
        item->setSizeHint(QSize(customWidget->minimumSizeHint().width(), 60));
        listWidget->addItem(item);
        listWidget->setItemWidget(item, customWidget);
        item->setData(Qt::UserRole, QVariant::fromValue(customWidget));
    }

    void addListItemWithValue(QListWidget *listWidget, const QString &labelText, InputType inputType, QString value){
        QListWidgetItem *item = new QListWidgetItem();
        CustomListElement *customWidget = new CustomListElement(labelText, inputType, this);
        customWidget->addValue(value);
        item->setSizeHint(QSize(customWidget->minimumSizeHint().width(), 60));
        listWidget->addItem(item);
        listWidget->setItemWidget(item, customWidget);
        item->setData(Qt::UserRole, QVariant::fromValue(customWidget));
    }

    void removeElement(CustomListElement *element);
    void press(const QString& arg) {
        addListItemWithValue(listWidget, "PRESS", ComboBoxType, arg);

    }

    void release(const QString& arg) {
        addListItemWithValue(listWidget, "RELEASE", ComboBoxType, arg);
    }

    void sleep(const QString& arg) {
        addListItemWithValue(listWidget, "SLEEP", NumberType, arg);
    }

    void type(const QString& arg) {
        addListItemWithValue(listWidget, "TYPE", LineEditType, arg);
    }

    void parseString(const QString& input) {
        if (input == QString("")) {
            return;
        }
        QString newInput = input;
        QString prefix = "typer.exe ";

        if (newInput.startsWith(prefix)) {
            newInput.remove(0, prefix.length());

            QStringList result = newInput.split(QRegularExpression("[}]"));
            QString pressString = QString("PRESS{");
            QString releaseString = QString("RELEASE{");
            QString sleepString = QString("SLEEP{");
            QString typeString = QString("TYPE{");

            auto getArg = [&result, &pressString, &releaseString, &sleepString, &typeString](int i) -> QString {
                QString arg;
                bool added = false;
                for (int a = i + 1; a < result.size(); a++) {
                    if (result[a].startsWith(pressString) || result[a].startsWith(releaseString) ||
                        result[a].startsWith(sleepString) || result[a].startsWith(typeString)) {
                        arg.remove(arg.length()-1, arg.length());
                        break;
                    } else if ( a != result.size()-1 ){
                        if (!added) {
                            arg += QString("}");
                            added = true;
                        }
                        arg += result[a];
                        arg += QString("}");
                    }
                }
                return arg;
            };
            try {
                for (int i = 0; i < result.size(); i++) {
                    if (result[i].startsWith(pressString)){
                        result[i].remove(0, pressString.length());
                        press(result[i] + getArg(i));
                    }
                    else if (result[i].startsWith(releaseString)){
                        result[i].remove(0, releaseString.length());
                        release(result[i] + getArg(i));
                    }
                    else if (result[i].startsWith(sleepString)){
                        result[i].remove(0, sleepString.length());
                        sleep(result[i] + getArg(i));
                    }
                    else if (result[i].startsWith(typeString)){
                        result[i].remove(0, typeString.length());
                        type(result[i] + getArg(i));
                    }
                }
            } catch (std::string error) {
                QMessageBox::warning(this, "Warning", "Cannot parse arguments correctly. Stopped parsing.");
            } catch (...) {
                QMessageBox::warning(this, "Warning", "Something went wrong in parsing.");
            }
        } else {
            QMessageBox::warning(this, "Warning", "Incorrect prefix: executable name is not correct");
        }
    }

    QString getCommand(bool throwError = true) {
        QString command = QString("");
        QList<QString> pressAndRelease = QList<QString>();
        try {
            for (int i = 0; i < listWidget->count(); ++i) {
                QListWidgetItem *item = listWidget->item(i);

                QVariant data = item->data(Qt::UserRole);

                CustomListElement *object = data.value<CustomListElement*>();

                if (object) {
                    command += object->getValue();
                    try {
                        QString value = object->getValue();
                        if ( value.contains(QString("PRESS{")) ) {
                            value.remove(0, 6);
                            value.remove(value.length()-1, value.length());
                            qDebug() << "check PRESSED: " << value;
                            pressAndRelease.append(value);
                        } else if ( value.contains(QString("RELEASE{")) ) {
                            value.remove(0, 8);
                            value.remove(value.length()-1, value.length());
                            qDebug() << "check RELEASED: " << value;
                            pressAndRelease.removeAll(value);
                        }
                    } catch(...) {
                        qDebug() << "error in command check";
                    }
                }
            }
        } catch(...) {
            qDebug() << "Error";
        }
        if (pressAndRelease.size() > 0 && throwError) {
            throw pressAndRelease[0];
        }
        return command;
    }

signals:
    void returnString(const QString&);

private slots:
    void onButtonPressed() {
        QString retstring("typer.exe ");
        try {
            QString command = getCommand();
            retstring += command;
        } catch (QString key){
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setWindowTitle("Warning");
            msgBox.setText(QString("Missing RELEASE key for: ") + key);
            msgBox.setInformativeText("If a key is pressed, but not released the key will stay pressed. This is dangerous.\nDo you want to add a release element?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::Yes);

            int ret = msgBox.exec();
            if (ret == QMessageBox::Yes) {
                release(key);
                return;
            } else {
                retstring += getCommand(false);
            }
        }
        emit returnString(retstring);
        close();
    }

    void onAddSleep();
    void onAddPress();
    void onAddRelease();
    void onAddType();
};

#endif // KEYBOARDWINDOW_H

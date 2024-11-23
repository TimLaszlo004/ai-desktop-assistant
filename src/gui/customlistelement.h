#ifndef CUSTOMLISTELEMENT_H
#define CUSTOMLISTELEMENT_H

#include <QListView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QHBoxLayout>
#include <QPainter>
#include <QPushButton>
#include <QEvent>
#include <QKeyEvent>

enum InputType { ComboBoxType, LineEditType, NumberType };

class KeyboardWindow;
class CustomListElement : public QWidget {
    Q_OBJECT
    QLabel* lbl = nullptr;
    QLineEdit *edit = nullptr;
    QComboBox *combo = nullptr;
    QGridLayout *layout = nullptr;
    KeyboardWindow* holder = nullptr;
    QPushButton* remove = nullptr;
    QDoubleValidator *doubleValidator = nullptr;
    InputType type;
    QStringList options = {"alt", "alt_gr", "alt_l", "alt_r", "backspace", "caps_lock", "cmd", "cmd_l", "cmd_r",
                           "ctrl", "ctrl_l", "ctrl_r", "delete", "down", "end", "enter", "esc", "f1", "f10", "f11",
                           "f12", "f13", "f14", "f15", "f16", "f17", "f18", "f19", "f2", "f20", "f21", "f22", "f23",
                           "f24", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "home", "insert", "left", "media_next",
                           "media_play_pause", "media_previous", "media_volume_down", "media_volume_up",
                           "media_volume_mute", "menu", "num_lock", "page_down", "page_up", "pause", "print_screen",
                           "right", "scroll_lock", "shift", "shift_l", "shift_r", "space", "tab", "up"};
    bool eventFilter(QObject *obj, QEvent *event)
    {
        if (obj == edit && event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

            if (keyEvent->text() == ",") {
                edit->insert(".");
                return true;
            }
        }
        return QObject::eventFilter(obj, event);
    }

public:

    ~CustomListElement();

    CustomListElement(const QString &labelText, InputType inputType, KeyboardWindow *window, QWidget *parent = nullptr)
        : QWidget(parent) {
        type = inputType;
        holder = window;
        setMinimumHeight(60);
        layout = new QGridLayout(this);

        lbl = new QLabel(labelText, this);
        layout->addWidget(lbl, 0, 0, 1, 1);

        // Add either a QComboBox or QLineEdit based on the inputType parameter
        if (inputType == ComboBoxType) {
            combo = new QComboBox(this);
            combo->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            combo->addItems(options);
            layout->addWidget(combo, 0, 1, 1, 1);
        } else if (inputType == LineEditType) {
            edit = new QLineEdit(this);
            edit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            layout->addWidget(edit, 0, 1, 1, 1);
        } else {
            edit = new QLineEdit(this);
            edit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            doubleValidator = new QDoubleValidator(0.0, 100.0, 2, this);
            doubleValidator->setNotation(QDoubleValidator::StandardNotation);
            doubleValidator->setLocale(QLocale::C);
            edit->setValidator(doubleValidator);
            edit->installEventFilter(this);
            layout->addWidget(edit, 0, 1, 1, 1);
        }
        remove = new QPushButton(QString("Remove"), this);
        remove->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        remove->setMaximumWidth(150);
        lbl->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        connect(remove, SIGNAL(clicked()), SLOT(on_remove()));
        layout->addWidget(remove, 0, 3, 1, 1);

        layout->setContentsMargins(10, 10, 10, 10);
        setLayout(layout);

        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    }

    void addValue(QString value) {
        if (type == ComboBoxType)
        {
            int index = combo->findText(value);
            if (index != -1) {
                combo->setCurrentIndex(index);
            } else {
                throw std::string("Invalid key");
            }
        }
        else if (type == LineEditType) {
            edit->setText(value);
        }
        else {
            edit->setText(value);
        }
    }

    QString getValue(){
        QString value = lbl->text() + QString("{");
        if (type == LineEditType || type == NumberType) {
            if (edit) {
                value += edit->text();
            } else {
                qDebug() << "edit is null";
            }
        } else {
            if (combo) {
                value += combo->currentText();
            } else {
                qDebug() << "combo is null";
            }
        }
        value += QString("}");
        return value;
    }
private slots:
    void on_remove();
};

#endif // CUSTOMLISTELEMENT_H

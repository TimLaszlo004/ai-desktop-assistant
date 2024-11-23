#ifndef CJSONOBJ_H
#define CJSONOBJ_H

#include <QJsonObject>

class CJsonObj
{
private:
    QJsonObject jsonObj = QJsonObject();
    bool valid = false;

public:
    CJsonObj();
    CJsonObj(QJsonObject newObj);
    void setJsonObject(QJsonObject newObj);
    QJsonObject getJsonObject() const;
    bool isValid() const;
    void setInvalid();
};

#endif // CJSONOBJ_H

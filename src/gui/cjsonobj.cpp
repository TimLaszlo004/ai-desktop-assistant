#include "cjsonobj.h"



CJsonObj::CJsonObj()
{

}

CJsonObj::CJsonObj(QJsonObject newObj):
    jsonObj(newObj),
    valid(true)
{
}

void CJsonObj::setJsonObject(QJsonObject newObj)
{
    jsonObj = newObj;
    valid = true;
}

QJsonObject CJsonObj::getJsonObject() const
{
    return jsonObj;
}

bool CJsonObj::isValid() const
{
    return valid;
}

void CJsonObj::setInvalid()
{
    valid = false;
}

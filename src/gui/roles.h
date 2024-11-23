#ifndef ROLES_H
#define ROLES_H

#include <QString>

enum Role{
    User = 0,
    Assistant = 1,
    Function = 2,
    Unknown = 404
};

namespace RoleString {
    const static QString userString = QString("user");
    const static QString assistantString = QString("assistant");
    const static QString functionString = QString("function");
}

enum Tab{
    Home,
    Variables,
    Modules,
    Functions,
    Help

};

#endif // ROLES_H

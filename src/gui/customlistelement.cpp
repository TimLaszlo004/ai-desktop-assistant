#include "customlistelement.h"
#include "keyboardwindow.h"



CustomListElement::~CustomListElement()
{
    delete lbl;
    delete edit;
    delete combo;
    delete layout;
    delete remove;
    delete doubleValidator;
}

void CustomListElement::on_remove()
{
    if (holder) {
        holder->removeElement(this);
    }
}

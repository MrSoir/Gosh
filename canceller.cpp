#include "canceller.h"

Canceller::Canceller(QObject *parent)
    : QObject(parent)
{}

void Canceller::cancel()
{
    emit cancelled();
}

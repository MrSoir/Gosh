#ifndef TESTQOBJDER_H
#define TESTQOBJDER_H

#include <QObject>

class TestQObjDer : public QObject
{
    Q_OBJECT
public:
    explicit TestQObjDer(QObject *parent = nullptr);
signals:

public slots:
};

#endif // TESTQOBJDER_H

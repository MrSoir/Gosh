#ifndef CANCELLER_H
#define CANCELLER_H

#include <QObject>

class Canceller : public QObject
{
    Q_OBJECT
public:
    explicit Canceller(QObject* parent = nullptr);
signals:
    void cancelled();
public slots:
    void cancel();
};

#endif // CANCELLER_H

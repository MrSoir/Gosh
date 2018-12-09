#ifndef SCRPNEBD_H
#define SCRPNEBD_H

#include <QScrollArea>
#include <QObject>
#include <QWidget>

class ScrPneBD : public QScrollArea
{
    Q_OBJECT
public:
    explicit ScrPneBD(QWidget* parent = nullptr);
    virtual ~ScrPneBD();

signals:
    void resized_BD();
};

#endif // SCRPNEBD_H

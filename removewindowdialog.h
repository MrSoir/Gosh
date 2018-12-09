#ifndef REMOVEWINDOWDIALOG_H
#define REMOVEWINDOWDIALOG_H

#include <QObject>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QPaintEvent>
#include <QPainter>
#include <QDialog>
#include <QString>
#include <QFile>
#include <QDataStream>

#include <memory>
#include <functional>
#include <vector>
#include <sstream>

#include "orientation.h"
#include "filescoordinator.h"
#include "staticfunctions.h"
#include "windowcoordinator.h"

class RemoveWindowDialog : public QDialog
{
public:
    RemoveWindowDialog(int anzRects,
               const std::function<void(int)>& caller,
               Orientation::ORIENTATION orientation,
               QSize size,
               QPoint position,
               QWidget* parent = nullptr);
    ~RemoveWindowDialog();
    QRectF boundingRect();

    void setSize(const QSize &size);
    void setPosition(const QPoint& position);
protected:
    void paintEvent(QPaintEvent *event);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
private:
    void setRectsPosition(QRect* rect, const QPoint& pos);

    bool m_hover = false;

    bool m_horizontal;

    int m_anzRects = 2;

    bool isPressed = false;
    qint64 lastTmePrsd = Q_INT64_C(0);

    QVector<QRect*> rects;
    int mouseRectId = -1;

    std::function<void(int)> caller;
};

#endif // REMOVEWINDOWDIALOG_H

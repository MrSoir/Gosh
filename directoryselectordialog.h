#ifndef DIRECTORYSELECTORDIALOG_H
#define DIRECTORYSELECTORDIALOG_H

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDebug>
#include <functional>
#include <memory>
#include "directoryselectordialogviewer.h"
#include "staticfunctions.h"

class DirectorySelectorDialogViewer;

class DirectorySelectorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DirectorySelectorDialog(QString curDir,
                                     std::function<void(QString)> caller,
                                     QSizeF size = QSizeF(400,600),
                                     QPointF pos = QPointF(0,0),
                                     QWidget *parent = nullptr);

    QRectF boundingRect();

protected:
    void paintEvent(QPaintEvent *event);

private:
    QSizeF m_size;
    QPointF m_pos;

    QString m_curDir;

    DirectorySelectorDialogViewer* m_dirSelDlgVwr;

    std::function<void(QString)> m_caller;
};

#endif // DIRECTORYSELECTORDIALOG_H

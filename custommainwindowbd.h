#ifndef CUSTOMMAINWINDOW_H
#define CUSTOMMAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QStatusBar>
#include <QLayout>

class CustomMainWindowBD : public QMainWindow
{
    Q_OBJECT
public:
    explicit CustomMainWindowBD(QWidget *parent = nullptr);
    void makeTight();
    void setLayout(QLayout* layout);
};

#endif // CUSTOMMAINWINDOW_H

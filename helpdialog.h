#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include <QObject>
#include <QGridLayout>
#include <QWidget>
#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDebug>
#include <QScrollArea>
#include <QLabel>
#include <QLayout>

#include <functional>
#include <memory>

#include "staticfunctions.h"


class HelpDialog : public QDialog
{
    Q_OBJECT
public:
    explicit HelpDialog(QSize size = QSize(800,600),
                        QPoint pos = QPoint(500,100),
                        QWidget *parent = nullptr);
    ~HelpDialog();

public slots:
signals:
private:
    void addWidgetToGridPane(QWidget* widget,
                             int row, int col,
                             QGridLayout* layout);

    void addLabelToGridPane(const QString& txt,
                          int row, int col,
                          QGridLayout* lay,
                          int rowSpan=1, int colSpan=1);

    QFrame* genSeparator(int horizontal = true);

    void setMarginsOfLayout(QLayout* layout, int padding = 0);

    QScrollArea m_scrollPane;
    QGridLayout* m_mainLayout;
    QPushButton* m_closeBtn;

    QSize m_iconBtnSize = QSize(50,50);
};
#endif // HELPDIALOG_H

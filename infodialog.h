#ifndef INFODIALOG_H
#define INFODIALOG_H

#include <QObject>
#include <QGridLayout>
#include <QWidget>
#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDebug>
#include <QScrollArea>
#include <QLabel>

#include <functional>
#include <memory>

#include "staticfunctions.h"


class InfoDialog : public QDialog
{
    Q_OBJECT
public:
    explicit InfoDialog(QSize size = QSize(800,600),
                        QPoint pos = QPoint(500,100),
                        QWidget *parent = nullptr);
    ~InfoDialog();

public slots:
signals:
private:
    void addPixmapToGridPane(const QString& pictureFileName,
                                 int row, int col,
                                 QGridLayout* layout);
    void setLicenseTextToButton(const QString& artist,
                                int row, int col,
                                QGridLayout* layout);
    void addWidgetToGridPane(QWidget* widget,
                             int row, int col,
                             QGridLayout* layout);

    QString getGoshLicenseText();
    QString getIconsLicenseText();

    QFrame* genSeparator(int horizontal);

    void setMarginsOfLayout(QLayout* layout, int padding = 0);

    QScrollArea m_scrollPane;
    QGridLayout* m_mainLayout;
    QPushButton* m_closeBtn;

    QSize m_iconBtnSize = QSize(50,50);
};

#endif // INFODIALOG_H

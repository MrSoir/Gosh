#ifndef OPENWITHDIALOG_H
#define OPENWITHDIALOG_H

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
#include "programmimeassociation.h"
#include "filehandler.h"
#include "openwithgraphicsview.h"

class OpenWithDialog : public QDialog
{
    Q_OBJECT
public:
    explicit OpenWithDialog(QFileInfo fileToOpen,
                            QSize size = QSize(800,600),
                        QPoint pos = QPoint(500,100),
                        QWidget *parent = nullptr);
    ~OpenWithDialog();

public slots:
    void onOkClicked();
    void selected(int id);
signals:
private:
    QFrame* genSeparator(int horizontal);

    void setMarginsOfLayout(QLayout* layout, int padding = 0);

    QFileInfo m_fileToOpen;

    std::vector<ProgramMimeAssociation> m_programHandlers;

    int m_selectedProgramId = 0;
};

#endif // OPENWITHDIALOG_H

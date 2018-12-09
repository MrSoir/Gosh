#ifndef SEARCHFILEDIALOG_H
#define SEARCHFILEDIALOG_H

#include <QWidget>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QErrorMessage>
#include <QKeyEvent>
#include <QDebug>
#include <QTimer>

//class QCheckBox;
//class QLabel;
//class QErrorMessage;

//class DialogOptionsWidget;

class SearchFileDialog : public QWidget
{
    Q_OBJECT
public:
    explicit SearchFileDialog(QWidget *parent = nullptr);
signals:
    void okClicked(QString searchKeyword, bool deepSearch);
    void cancelled();
public slots:
protected:
    void keyPressEvent(QKeyEvent *event) override;
private:
    void onOkClicked();
    QLineEdit* m_searchKeyword;
    QCheckBox* m_deepSearch;
};

#endif // SEARCHFILEDIALOG_H

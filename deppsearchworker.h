#ifndef DEPPSEARCHWORKER_H
#define DEPPSEARCHWORKER_H

#include <QObject>
#include <QString>
#include <QThread>
#include <QDebug>
#include <QVector>
#include <QDir>
#include <QMutex>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <functional>

#include "deepsearch.h"
#include "fileinfobd.h"

namespace DeepSearchWorker{
    class DeepSearchDialog: public QDialog
    {
        Q_OBJECT
    public:
        explicit DeepSearchDialog(QWidget *parent = nullptr)
            : QDialog(parent)
        {

            auto* mainLyt = new QVBoxLayout();

            QPushButton* cnclBtn = new QPushButton("cancel");
            connect(cnclBtn, &QPushButton::clicked, [=](){
                qDebug() << "DeepSearchDialog: emitting cacelled";
                emit cancelled();

//                this->close();
            });
            auto* btnsLyt = new QHBoxLayout();
            btnsLyt->addWidget(cnclBtn);

            mainLyt->addLayout(btnsLyt);

            this->setLayout(mainLyt);

            setFixedSize(200,150);
            setModal(true);
        }
    signals:
        void cancelled();
    public slots:
    private:
    };
}


class DeppSearchWorker : public QObject
{
    Q_OBJECT
public:
    explicit DeppSearchWorker(std::shared_ptr<FileInfoBD> rootFold,
                              QString keyword, int searchWordsLimit = 1000,
                              bool includeHiddenFiles = false);
    ~DeppSearchWorker();
signals:
    void finishedSearch(bool maxSearchWordsLimitReached );
    void updateView();
    void cancelDeepSearcher();
    void canceled();
public slots:
    void folderElapsed(QFileInfo fi);
    void process();
    void cancel();
private:
    std::shared_ptr<FileInfoBD> m_rootFold;
    QString m_keyword;
    std::unique_ptr<DeepSearch> m_deepSearcher;

    bool m_cancelled = false;
    int m_searchWordsLimit;

    bool m_includeHiddenFiles;
};

#endif // DEPPSEARCHWORKER_H

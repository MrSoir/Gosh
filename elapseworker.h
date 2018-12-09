#ifndef ELAPSEWORKER_H
#define ELAPSEWORKER_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QString>
#include <QVector>
#include <QDir>
#include <QMutex>
#include <functional>
#include <atomic>
#include "fileinfobd.h"
#include "elapsevalidator.h"


class ElapseWorkerRecursively : public QObject
{
    Q_OBJECT
public:
    explicit ElapseWorkerRecursively( QString path,
                                      int elapseDepth = 0,
                                      bool rootWorker = true,
                                      bool includeHiddenFiles = false);
    explicit ElapseWorkerRecursively(QString path,
                                     int elapseDepth,
                                     std::shared_ptr<ElapseValidator> validator,
                                     bool includeHiddenFiles = false);

    ~ElapseWorkerRecursively();

    void setElapseRecursively(bool elapseRecursively);
public slots:
    void process();
    void receiveSubFolderFromSubThread(FileInfoBD* subFoldToAdd);
    void cancel();
signals:
    void sendingFiBdToReceiverThread(FileInfoBD* subFoldToAdd, bool limitReached);
    void finished();
    void cancelled();
private:
    QString getFileInfoName();

    void lastJobToDoBeforeTermination();

    bool checkIfCancelled();

    FileInfoBD* m_fiBD = nullptr;
    QString m_rootPath;
    int m_elapseDepth;
    int m_maxElapseDepth = 2;
    int m_threadCounter = 0;

    bool m_rootWorker;

    bool m_elapseRecursively = true;
//    std::unique_ptr<bool> m_cancelled;
    std::shared_ptr<ElapseValidator> m_validator;

    bool m_includeHiddenFiles;
};

#endif // ELAPSEWORKER_H

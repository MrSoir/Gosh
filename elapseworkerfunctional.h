#ifndef ELAPSEWORKERFUNCTIONAL_H
#define ELAPSEWORKERFUNCTIONAL_H

#include <QObject>
#include <QString>
#include <QThread>
#include <QTimer>
#include <QDebug>
#include <QProgressDialog>
#include <functional>
#include <memory>
#include "staticfunctions.h"

class WorkerHelper : public QObject
{
    Q_OBJECT
public:
    WorkerHelper(std::shared_ptr<bool> cancelled,
             std::function<void(std::shared_ptr<bool>, std::function<void(double)>)> caller,
             QObject* parent = nullptr)
        : m_cancelled(cancelled),
          m_caller(caller),
          QObject(parent)
    {
    }
    ~WorkerHelper()
    {
        m_cancelled.reset();
        m_caller = nullptr;
    }
public slots:
    void cancel()
    {
        if(m_cancelled)
            *m_cancelled = true;
    }
    void start()
    {
        if(m_caller)
            m_caller(m_cancelled, [=](double prgrs){
//                qDebug() << "progress: " << std::min(100, int(prgrs*100.0));
                emit progress( std::min(100, int(prgrs*100.0)) );
            });

        emit finished();
    }
signals:
    void progress(int prgrs);
    void finished();
private:
    std::shared_ptr<bool> m_cancelled;
    std::function<void(std::shared_ptr<bool>, std::function<void(double)>)> m_caller;
};

class WorkerBDFunctional : public QObject
{
    Q_OBJECT
public:
    explicit WorkerBDFunctional(
            std::function<void (std::shared_ptr<bool>, std::function<void(double)>)> caller,
            bool startThreadImmediately = true,
            QObject *parent = nullptr);
    ~WorkerBDFunctional();
signals:
    void finished();
    void cancelled();
    void progress(int prgs);
public slots:
    void prepare();
    void start();
//    void process();
    void cancel();
private:
    WorkerHelper* m_caller;
    QString m_infoMessageStr;

    bool m_isPrepared = false;
    QThread* m_thread = nullptr;
    std::shared_ptr<bool> m_cancelled;
};

#endif // ELAPSEWORKERFUNCTIONAL_H

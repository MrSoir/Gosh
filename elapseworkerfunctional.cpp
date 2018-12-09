#include "elapseworkerfunctional.h"

WorkerBDFunctional::WorkerBDFunctional(std::function<void (std::shared_ptr<bool>, std::function<void(double)>)> caller,
                                               bool startThreadImmediately,
                                               QObject *parent)
    : m_cancelled(std::make_shared<bool>(false)),
      QObject(parent)
{
    m_caller = new WorkerHelper(m_cancelled, caller, this);

    if(startThreadImmediately){
        start();
    }
}

WorkerBDFunctional::~WorkerBDFunctional()
{
//    qDebug() << "in ElapseWorkerFunctional-DEstructor";

    m_cancelled.reset();
    m_thread = nullptr;
}

void WorkerBDFunctional::prepare()
{
    m_thread = new QThread;

    this->moveToThread(m_thread);
    connect(this, &WorkerBDFunctional::finished,        m_thread, &QThread::quit, Qt::QueuedConnection);
    connect(this, &WorkerBDFunctional::finished,        this, &WorkerBDFunctional::deleteLater, Qt::QueuedConnection);
    connect(m_thread, &QThread::finished,               m_thread, &QThread::deleteLater, Qt::QueuedConnection);
    connect(this, &WorkerBDFunctional::cancelled,       this, &WorkerBDFunctional::deleteLater, Qt::QueuedConnection);
    connect(this, &WorkerBDFunctional::cancelled,       m_thread, &QThread::quit, Qt::QueuedConnection);

    connect(m_thread, &QThread::started,                m_caller, &WorkerHelper::start);
    connect(m_caller, &WorkerHelper::finished,          this, &WorkerBDFunctional::finished, Qt::QueuedConnection);
    connect(m_caller, &WorkerHelper::progress,          this, &WorkerBDFunctional::progress, Qt::DirectConnection);
    connect(this, &WorkerBDFunctional::cancelled,       m_caller, &WorkerHelper::cancel, Qt::DirectConnection);
    connect(this, &WorkerBDFunctional::cancelled,       m_caller, &WorkerHelper::deleteLater, Qt::QueuedConnection);
    connect(this, &WorkerBDFunctional::finished,        m_caller, &WorkerHelper::deleteLater, Qt::QueuedConnection);

    m_isPrepared = true;
}

void WorkerBDFunctional::start(){
    if( !m_isPrepared )
        prepare();

    m_thread->start();
}


void WorkerBDFunctional::cancel()
{
    if(m_cancelled)
        *m_cancelled = true;

    emit cancelled();
}

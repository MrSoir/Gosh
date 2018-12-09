#include "workerhelper.h"

WorkerHelper::WorkerHelper(std::shared_ptr<bool> cancelled,
         std::function<void(std::shared_ptr<bool>, std::function<void(double)>)> caller,
         QObject* parent = nullptr)
    : m_cancelled(cancelled),
      m_caller(caller),
      QObject(parent)
{
}
WorkerHelper::~WorkerHelper()
{
    m_cancelled.reset();
    m_caller = nullptr;
}
void WorkerHelper::cancel()
{
    if(m_cancelled)
        *m_cancelled = true;
}
void WorkerHelper::start()
{
    if(m_caller)
        m_caller(m_cancelled, [=](double prgrs){
            emit progress( std::max(100, int(prgrs*100.0)) );
        });

    emit finished();
}

#include "deppsearchworker.h"

DeppSearchWorker::DeppSearchWorker(std::shared_ptr<FileInfoBD> rootFold,
                                   QString keyword, int searchWordsLimit,
                                   bool includeHiddenFiles)
    : m_keyword(keyword),
      m_rootFold(rootFold),
      m_searchWordsLimit(searchWordsLimit),
      m_includeHiddenFiles(includeHiddenFiles)
{
//    qDebug() << "in DeppSearchWorker.CONstructor";
}

DeppSearchWorker::~DeppSearchWorker()
{
//    qDebug() << "in DeppSearchWorker.DEstructor";
    m_rootFold.reset();
    m_deepSearcher.reset();
}

void DeppSearchWorker::folderElapsed(QFileInfo fi)
{
    Q_UNUSED(fi);
    emit updateView();
}

void DeppSearchWorker::process()
{
//    qDebug() << "in DeppSearchWorker-process";

    m_rootFold->disableSignals(true);
    m_deepSearcher = std::make_unique<DeepSearch>(m_rootFold, m_keyword, m_searchWordsLimit);
    m_deepSearcher->launchSearch();
    m_deepSearcher->elapseRoot();
    m_rootFold->disableSignals(false);

    m_rootFold.reset();

    if( !m_cancelled )
        emit finishedSearch( m_deepSearcher->maxSearchWordsLimitReached() );
}

void DeppSearchWorker::cancel()
{
//    qDebug() << "in DeppSearchWorker::cancel";
    m_cancelled = true;

    if(m_deepSearcher)
        m_deepSearcher->cancel();

    emit canceled();
}

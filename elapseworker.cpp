#include "elapseworker.h"



ElapseWorkerRecursively::ElapseWorkerRecursively(  QString path,
                                                   int elapseDepth,
                                                   bool rootWorker,
                                                   bool includeHiddenFiles)
    : m_rootPath(path),
      m_elapseDepth(elapseDepth),
      m_rootWorker( rootWorker ),
      m_includeHiddenFiles(includeHiddenFiles)
{
    if(m_rootWorker)
    {
       m_validator = std::make_shared<ElapseValidator>();
    }
}

ElapseWorkerRecursively::ElapseWorkerRecursively(QString path,
                                                 int elapseDepth,
                                                 std::shared_ptr<ElapseValidator> validator,
                                                 bool includeHiddenFiles)
    : m_rootPath(path),
      m_elapseDepth(elapseDepth),
      m_rootWorker(false),
      m_includeHiddenFiles(includeHiddenFiles)
{
    if(validator)
        m_validator = validator;
}

ElapseWorkerRecursively::~ElapseWorkerRecursively()
{
//    if(m_rootWorker)
//        qDebug() << "in elapseworker-DEstructor -> m_rootWorker: " << getFileInfoName() << "    m_validator - "
//                 << "   cancelled: " << m_validator->cancelled()
//                 << "   limitReached: " << m_validator->limitReached()
//                 << "   refCounter: " << m_validator.use_count();
//    else
//        qDebug() << "in elapseworker-DEstructor -> child: " << getFileInfoName() << "    m_validator - "
//                 << "   cancelled: " << m_validator->cancelled()
//                 << "   limitReached: " << m_validator->limitReached()
//                 << "   refCounter: " << m_validator.use_count();

    if( !m_validator || m_validator->cancelled() )
        delete m_fiBD;
    else
        m_fiBD = nullptr;

    m_validator.reset();
}

void ElapseWorkerRecursively::setElapseRecursively(bool elapseRecursively)
{
    m_elapseRecursively = elapseRecursively;
}

void ElapseWorkerRecursively::process()
{
    if( m_validator && m_validator->cancelled() )
    {
        return;
    }

    m_fiBD = new FileInfoBD(m_rootPath);
    m_fiBD->disableSignals(true);

    if(m_elapseDepth < m_maxElapseDepth && m_elapseRecursively){
        QDir rootDir(m_rootPath);
        if(rootDir.exists()){

            if( checkIfCancelled() )
            {
//                lastJobToDoBeforeTermination();
                emit cancelled();
                return;
            }

            QFileInfoList subFolders = StaticFunctions::getFoldersInDirectory(rootDir, m_includeHiddenFiles);//rootDir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);

            QFileInfoList filesLst   = StaticFunctions::getFilesInDirectory  (rootDir, m_includeHiddenFiles);//rootDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
            m_fiBD->addFiles(filesLst);

            QVector<QThread*> m_threads;

            if(subFolders.size() > 0 && !(m_validator && m_validator->cancelled()) ){
//                qDebug() << "thread: subFolders.size: " << subFolders.size();

                for(int i=0; i < subFolders.size(); i++){
                    if( checkIfCancelled() )
                    {
                        emit cancelled();
//                        lastJobToDoBeforeTermination();
                        return;
                    }

                    ++m_threadCounter;
//                    qDebug() << "about to create thread " << i;

                    QThread* thread = new QThread();
                    ElapseWorkerRecursively* worker = new ElapseWorkerRecursively(subFolders[i].absoluteFilePath(),
                                                            m_elapseDepth+1,
                                                            m_validator);
                    worker->moveToThread(thread);
                    connect(thread, &QThread::started,       worker, &ElapseWorkerRecursively::process);
                    connect(worker, &ElapseWorkerRecursively::finished, thread, &QThread::quit);
                    connect(worker, &ElapseWorkerRecursively::finished, worker, &ElapseWorkerRecursively::deleteLater);
                    connect(thread, &QThread::finished,      thread, &QThread::deleteLater);
                    connect(worker, &ElapseWorkerRecursively::sendingFiBdToReceiverThread,
                            this,   &ElapseWorkerRecursively::receiveSubFolderFromSubThread
                            , Qt::QueuedConnection);
                    connect(this, &ElapseWorkerRecursively::cancelled, worker, &ElapseWorkerRecursively::cancel, Qt::DirectConnection);

                    m_threads.append(thread);
                }
                for(int i=0; i < m_threads.size(); i++){
                    m_threads[i]->start();
                }
                m_threads.clear();
            }else{
                if( checkIfCancelled() )
                {
                    emit cancelled();
                }else
                {
                    m_fiBD->elapseAll( std::static_pointer_cast<AbstrElapseValidator>(m_validator) );

                    if(checkIfCancelled())
                        emit cancelled();
                    else
                        lastJobToDoBeforeTermination();
                }
            }
        }
    }
    else{
//        qDebug() << "about to finish thread: << " << getFileInfoName();
        if( checkIfCancelled() )
        {
            emit cancelled();
        }else
        {
            if(m_elapseRecursively)
                m_fiBD->elapseAll( std::static_pointer_cast<AbstrElapseValidator>(m_validator) );
            else
                m_fiBD->setElapsed(true);

            if(checkIfCancelled())
                emit cancelled();
            else
                lastJobToDoBeforeTermination();
        }
    }
}

void ElapseWorkerRecursively::receiveSubFolderFromSubThread(FileInfoBD *subFoldToAdd)
{
//    if(m_elapseDepth == 0){
//        qDebug() << "in addSubFiBDToRootFiBD - m_threadCounter: " << m_threadCounter
//                 << "   [" << getFileInfoName() << "] jklasdfhjhasldfhasdlf";
//    }

    if( m_validator && m_validator->cancelled() )
    {
        emit cancelled();
    }else
    {
        m_fiBD->replaceSubFolder(subFoldToAdd);
        m_fiBD->setElapsedFlag(true);
        m_fiBD->setLoadedFlag(true);

        if(--m_threadCounter < 1){
    //        qDebug() << "m_threadCounter == 0 -> finishing thread " << getFileInfoName();
            lastJobToDoBeforeTermination();
        }
    }
}

void ElapseWorkerRecursively::cancel()
{
//    if(m_rootWorker)
//        qDebug() << "in ElapseWorker-cancel: " << getFileInfoName();

    if(m_validator)
        m_validator->setCancelled();

    emit cancelled();
}

QString ElapseWorkerRecursively::getFileInfoName()
{
    if(m_fiBD){
        return m_fiBD->getFileInfo().fileName();
    }else{
        return QString("nullptr");
    }
}

void ElapseWorkerRecursively::lastJobToDoBeforeTermination()
{
    bool limitReached = m_validator ? m_validator->limitReached() : false;
    emit sendingFiBdToReceiverThread(m_fiBD, limitReached);
    emit finished();
}

bool ElapseWorkerRecursively::checkIfCancelled()
{
    return !m_validator || m_validator->cancelled();
}

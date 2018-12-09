#include "filescoordinator.h"
#include "filescoordinator.h"

std::chrono::milliseconds getCurrentTime()
{
    return duration_cast< milliseconds >(
            system_clock::now().time_since_epoch()
        );
}

FilesCoordinator::FilesCoordinator(QObject* parent)
    : QObject(parent),
      m_root(std::shared_ptr<FileInfoBD>()),
      m_self(std::weak_ptr<FilesCoordinator>()),
      elapsingThreadCounter(0)
{
    qDebug() << "in FilesCoordinator-Constructor";

    m_watcher = std::make_shared<QFileSystemWatcher>();
    *m_watcherConn = connect(m_watcher.get(),   &QFileSystemWatcher::directoryChanged,
                             this,        &FilesCoordinator::directoryChanged);

    m_lastKeyPressed = getCurrentTime();
}

FilesCoordinator::~FilesCoordinator()
{
    qDebug() << "in filesCoordinator-Destructor!";

    emit killCurrentBlockingAction();

    if(m_root){
        m_root->close();
        m_root.reset();
    }

    if(m_watcherConn){
        disconnect(*m_watcherConn);
        m_watcherConn.reset();
    }
    if(m_watcher){
        m_watcher->removePaths(m_watcher->directories());
        m_watcher.reset();
    }

    m_focusCaller = nullptr;
    m_tellParentRootHasChanged = nullptr;
    m_onOpenFoldersInNewTab= nullptr;
    m_onCloseCurrentTab = nullptr;

    resetWidgets();
}

void FilesCoordinator::setFocusCaller(std::function<void (std::weak_ptr<FilesCoordinator>)> focusCaller)
{
    m_focusCaller = focusCaller;
}
void FilesCoordinator::setRootChangedCaller(std::function<void (QDir)> rootChangedCaller)
{
    m_tellParentRootHasChanged = rootChangedCaller;
}

void FilesCoordinator::setOnOpenFoldersInNewTab(std::function<void (QVector<QDir>)> onOpenFoldersInNewTab)
{
    m_onOpenFoldersInNewTab = onOpenFoldersInNewTab;
}
void FilesCoordinator::setOnCloseTab(std::function<void ()> onCloseTab)
{
    m_onCloseCurrentTab = onCloseTab;
}

void FilesCoordinator::resetToDefault(bool closeRoot)
{
    if(revalidatingIsBlocked())
        return;

    if(m_root && closeRoot)
        m_root->close();

    if(m_watcher){
        m_watcher->removePaths(m_watcher->directories());
    }

    clearSelection();
    clearSearchResults();
}
void FilesCoordinator::replaceRoot(std::shared_ptr<FileInfoBD> newRoot)
{
    bool closeRoot = !(newRoot && m_root && newRoot->getFileInfo().absoluteFilePath() ==
                                            m_root->getFileInfo().absoluteFilePath());
    resetToDefault( closeRoot );

    m_root = newRoot;
    m_root->setSelf(m_root);
    m_root->setIncludeHiddenFilesCaller([=](){return m_includeHiddenFiles;});

    m_lastSelection = m_root->getFileInfo().absoluteFilePath();

    revalidateOneDimnFolderOrder();

    for(int i=0; i < m_depthIdsElapsed.size(); i++){
        m_depthIdsElapsed[i] = (i==0); // der initialeordner wird automatisch von anfang an auf 'elapsed' gesetzt, alle danach vorerst nicht
    }

    if(m_self.expired()){
        throw QString("FilesCordinator.m_self is nut set or expired!!! -> must not add listener to root!!!");
    }else{
        m_root->addListener( m_self );
    }
    m_root->setElapsed(true);

    QDir newDir;
    if(m_root)
        newDir = QDir( m_root->getFileInfo().absoluteFilePath() );
    else
        newDir = QDir("");

    emit rootChanged( newDir );

    if(m_tellParentRootHasChanged)
        m_tellParentRootHasChanged( newDir );
}
void FilesCoordinator::setRootFolder(QDir dir)
{
    if(revalidatingIsBlocked())
        return;

    m_dirStack.push(dir.absolutePath());

    replaceRoot( std::make_shared<FileInfoBD>(dir.absolutePath()) );
}

int FilesCoordinator::evaluateDepth(std::weak_ptr<FileInfoBD> filInfo, int* curRow, int curDepth){
    if(auto locked = filInfo.lock()){
        if(*curRow < m_oneDimnFolderOrder.size()){
            int max = 0;
            m_oneDimnFolderOrder[*curRow].m_depth = curDepth;
            ++(*curRow);
            foreach(auto subFold, locked->getSubFolders()){
                int depth = evaluateDepth(subFold, curRow, curDepth+1);
                if(max < depth)
                    max = depth;
            }
            for(int i=0; i < locked->getFileCount(); i++){
                m_oneDimnFolderOrder[(*curRow)++].m_depth = curDepth;
            }
            return 1+max;
        }
    }
}

int FilesCoordinator::getDepth(){
    return m_maxDepth;
}

void FilesCoordinator::sort(std::weak_ptr<FileInfoBD> fiBD, ORDER_BY order)
{
    if(auto locked = fiBD.lock()){
//        QTimer::singleShot(0, [=](){
            locked->sortBy(order, true);
            repaintFolderViewer();
//        });
    }
}

void FilesCoordinator::sortAllFolders(ORDER_BY order)
{
    qDebug() << "in  FilesCoordinator::sortAllFolders: order: " << order;
    if( m_root){
        QTimer::singleShot(0, [=](){
            m_root->sortByRecursivelyWithoutNotification(order);
            revalidateOneDimnFolderOrder();
            repaintFolderViewer();
        });
    }
}

void FilesCoordinator::openSelectionByDefaultPrograms()
{
    QTimer::singleShot(0,[=](){
        for(auto it = m_slctdFiles.cbegin(); it != m_slctdFiles.cend(); ++it)
        {
            const QString& absFilePath = *it;
            if( !absFilePath.isEmpty() ){
                QDesktopServices::openUrl(QUrl(QString("file:%1").arg( absFilePath )));
            }
        }
        for(auto it = m_slctdFolds.cbegin(); it != m_slctdFolds.cend(); ++it)
        {
            const QString& absFilePath = *it;
            if( !absFilePath.isEmpty() ){
                QDesktopServices::openUrl(QUrl(QString("file:%1").arg( absFilePath )));
            }
        }
    });
}

void FilesCoordinator::openSelection()
{
    if(singleFolderSelected() && singleContentSelected()){
        // set folder to root
        const QString& absPath = *m_slctdFolds.cbegin();
        QFileInfo fileToOpen(absPath);
        if(fileToOpen.isDir())
            this->setRootFolder( QDir(absPath) );
        else
            openSelectionByDefaultPrograms();
    }else{
        openSelectionByDefaultPrograms();
    }
}

void FilesCoordinator::setParentToRoot()
{
    if(m_root){
        QDir rootPath(m_root->getFileInfo().absoluteFilePath());
        if(rootPath.cdUp()){ // QDor.cdUp() holt sich den parent-root-path, aber nur, wenn dieser auch existiert!
            this->setRootFolder(rootPath);
        }
    }
}

FilInfoForOneDim FilesCoordinator::getFileAt(int id)
{
    if( id < m_oneDimnFolderOrder.size() && id >= 0 ){
        return m_oneDimnFolderOrder[id];
    }
    std::stringstream ss;
    ss << "in FilesCoordinator::getFileAt: id == " << id << "   m_oneDimnFolderOrder.size: " << m_oneDimnFolderOrder.size();
    throw ss.str();
}

FilInfoForOneDim FilesCoordinator::getDisplayedFileAt(int id)
{
    if( id < m_oneDimnFolderOrderDisplayed.size() ){
        return m_oneDimnFolderOrderDisplayed[id];
    }
    std::stringstream ss;
    ss << "in FilesCoordinator::getDisplayedFileAt: id == " << id << "   m_oneDimnFolderOrderDisplayed.size: " << m_oneDimnFolderOrderDisplayed.size();
    throw ss.str();
}

int FilesCoordinator::getFileCount()
{
    return m_oneDimnFolderOrder.size();
}

int FilesCoordinator::getDisplayedFileCount()
{
    return m_oneDimnFolderOrderDisplayed.size();
}

void FilesCoordinator::revalidateOneDimnFolderOrder(bool revalAnyway){
    if(!revalAnyway && revalidatingIsBlocked())
        return;

    m_oneDimnFolderOrder.clear();
    m_oneDimnFolderOrderDisplayed.clear();
    if(m_root){
        m_root->traverse(
            [=](std::weak_ptr<FileInfoBD> weakFiBD, int depth){
                FilInfoForOneDim fiOneDim(weakFiBD, true, QString(""), depth);
                m_oneDimnFolderOrder.push_back( fiOneDim );
            },
            [=](std::weak_ptr<FileInfoBD> weakFiBD, int depth){
                if(auto locked = weakFiBD.lock()){
                    if(locked->fileCount() > 0){
                        const QVector<QFileInfo> files = locked->getFiles();
                        foreach(QFileInfo fi, files){
                            FilInfoForOneDim fiOneDim(weakFiBD, false, fi.fileName(), depth);
                            m_oneDimnFolderOrder.push_back( fiOneDim );
                        }
                    }
                }
            },
            false,
            0
        );
        m_root->traverse(
            [=](std::weak_ptr<FileInfoBD> weakFiBD, int depth){
                FilInfoForOneDim fiOneDim(weakFiBD, true, QString(""), depth);
                m_oneDimnFolderOrderDisplayed.push_back( fiOneDim );
            },
            [=](std::weak_ptr<FileInfoBD> weakFiBD, int depth){
                if(auto locked = weakFiBD.lock()){
                    if(locked->fileCount() > 0){
                        const QVector<QFileInfo> files = locked->getFiles();
                        foreach(QFileInfo fi, files){
                            FilInfoForOneDim fiOneDim(weakFiBD, false, fi.fileName(), depth);
                            m_oneDimnFolderOrderDisplayed.push_back( fiOneDim );
                        }
                    }
                }
            },
            true,
            0
        );

//        qDebug() << "\nafter revalidation:\n"
//                    "   m_oneDimnFolderOrder:           " << m_oneDimnFolderOrder.size()
//                 << "   m_oneDimnFolderOrderDisplayed:  " << m_oneDimnFolderOrderDisplayed.size()
//                 << "\n\n";

        m_maxDepth = 0;
        for(int i=0; i < m_oneDimnFolderOrder.size(); i++){
            if(m_maxDepth < m_oneDimnFolderOrder[i].m_depth)
                m_maxDepth = m_oneDimnFolderOrder[i].m_depth;
        }
        for(int i=m_depthIdsElapsed.size(); i <= m_maxDepth; i++){
            m_depthIdsElapsed.append(false);
        }

        QList<QString> slctdFiles = m_slctdFiles.toList();
        int id = 0;
        while(id < slctdFiles.size()){
            const QString& absFilePath = slctdFiles[id];
            if( absFilePath.isEmpty() || !QFileInfo(absFilePath).exists()){
                slctdFiles.removeAt(id);
            }else{
                ++id;
            }
        }
        m_slctdFiles = slctdFiles.toSet();

        QList<QString> slctdFolds = m_slctdFolds.toList();
        id = 0;
        while(id < slctdFolds.size()){
            const QString& absFilePath = slctdFolds[id];
            if( absFilePath.isEmpty() || !QFileInfo(absFilePath).exists()){
                slctdFolds.removeAt(id);
            }else{
                ++id;
            }
        }
        m_slctdFolds = slctdFolds.toSet();
    }

    revalidateSearchIds();

//    qDebug() << "revalidateOneDimnFolderOrder.size: " << m_oneDimnFolderOrder.size();
//    for(int i=0; i < m_oneDimnFolderOrder.size(); i++){
//        qDebug() << "   m_oneDimnFolderOrder[" << i << "]: " << m_oneDimnFolderOrder[i].m_fileInfoBD.lock()->fileName()
//                 << "   " << m_oneDimnFolderOrder[i].m_isFolder
//                 << "   " << m_oneDimnFolderOrder[i].m_fileName
//                 << "   depth: " << m_oneDimnFolderOrder[i].m_depth;
//    }
}

//std::shared_ptr<FilInfoForOneDim> FilesCoordinator::iterateBackwards(const FilInfoForOneDim& fileInfoOneDim, int offs,
//                                                                     std::function<void(FilInfoForOneDim)> func)
//{
//    return iterateForwBackwHelper(fileInfoOneDim, offs, false,func);
//}
//std::shared_ptr<FilInfoForOneDim> FilesCoordinator::iterateForwards(const FilInfoForOneDim& fileInfoOneDim, int offs,
//                                                                    std::function<void(FilInfoForOneDim)> func)
//{
//    return iterateForwBackwHelper(fileInfoOneDim, offs, true, func);
//}

//std::shared_ptr<FilInfoForOneDim> FilesCoordinator::iterateForwBackwHelper(const FilInfoForOneDim& fileInfoOneDim, int offs,
//                                                                           bool iterateForwards,
//                                                                           std::function<void(FilInfoForOneDim)> func)
//{
//     int targetId = getIdOf(fileInfoOneDim);
//     if(targetId > -1){

//         targetId = iterateForwards ? targetId+1 : targetId-1;
//         if(targetId < 0)
//             targetId = m_oneDimnFolderOrderDisplayed.size()-1;
//         else if(targetId >= m_oneDimnFolderOrderDisplayed.size())
//             targetId = 0;

//         if(func){
//             FilInfoForOneDim retVal;
//             bool found = false;
//             while(offs > 0){
//                 found = true;
//                 retVal = m_oneDimnFolderOrderDisplayed[targetId];
//                 func(retVal);
//                 targetId = iterateForwards ? ++targetId : --targetId;
//                 if(targetId < 0)
//                     targetId = m_oneDimnFolderOrderDisplayed.size()-1;
//                 else if(targetId >= m_oneDimnFolderOrderDisplayed.size())
//                     targetId = 0;
//                 --offs;
//             }
//             if(found)
//                 return std::make_shared<FilInfoForOneDim>(retVal);
//             return std::shared_ptr<FilInfoForOneDim>(nullptr);
//         }else{
//             --offs; // muss gemacht werden, da sonst eins zu viel uebersprungen wird
//             targetId = iterateForwards ? targetId + offs : targetId - offs;
//             if(targetId < 0)
//                 targetId = m_oneDimnFolderOrderDisplayed.size() - targetId;
//             else if (targetId >= m_oneDimnFolderOrderDisplayed.size())
//                 targetId = targetId - m_oneDimnFolderOrderDisplayed.size();
//             return std::make_shared<FilInfoForOneDim>(m_oneDimnFolderOrderDisplayed[targetId]);
//         }
//     }else{
//         return std::shared_ptr<FilInfoForOneDim>(nullptr);
//     }
//}

void FilesCoordinator::setSelectionToRoot()
{
    if(revalidatingIsBlocked())
        return;

    if(singleFolderSelected()){
        setRootFolder( QDir( *m_slctdFolds.cbegin() ) );
    }
}

void FilesCoordinator::setLastPathToRoot()
{
    if(revalidatingIsBlocked())
        return;

    if(m_dirStack.size() > 1){
        QString path("");
        while( !m_dirStack.isEmpty() && (path = m_dirStack.pop()) == m_root->getFileInfo().absoluteFilePath())
            ;
        if( !path.isEmpty() && QFileInfo(path).exists() ){
            this->setRootFolder(QDir(path));
        }
    }
}

QLayout *FilesCoordinator::getLayout()
{
    if(m_mainGrid == nullptr)
    {
        createWidgets();
    }
    return m_mainGrid;
}

void FilesCoordinator::openTerminal() const
{
    QString sourceDir;
    if(singleFolderSelected()){
        sourceDir = QString(*m_slctdFolds.cbegin());
    }else if (singleFileSelected()){
        sourceDir = QString(*m_slctdFiles.cbegin());
    }else if (m_root){
        sourceDir = m_root->getFileInfo().absoluteFilePath();
    }
    StaticFunctions::openTerminal(sourceDir);
}

void FilesCoordinator::revalidate()
{
    revalidateOneDimnFolderOrder();
    repaintFolderViewer();
}

//----------------------------------------------------------------------------------------------

void FilesCoordinator::collapseFolderRecursively(std::shared_ptr<FileInfoBD> fiBD)
{
    auto caller = [=](std::shared_ptr<bool> cancelled, std::function<void(double)> progress){
        Q_UNUSED(cancelled);
        Q_UNUSED(progress);
        if(fiBD)
        {
            fiBD->disableSignals(true);
            fiBD->collapseAll();
            fiBD->disableSignals(false);
        }
    };

    launchWorkerFunctional(caller);
}

void FilesCoordinator::collapseFolderRecursively(std::weak_ptr<FileInfoBD> fiBD)
{
    collapseFolderRecursively(fiBD.lock());
}
void FilesCoordinator::collapseFolderRecursively(FilInfoForOneDim &fold)
{
    collapseFolderRecursively(fold.m_fileInfoBD);
}

void FilesCoordinator::collapseFoldersRecursively(const QList<FilInfoForOneDim>& folds)
{
    blockRevalidating(true);

    QList<FilInfoForOneDim> foldersToCollapse = validateFoldersToElapse(folds);

    collapseFoldersRecursivelyHelper<FilInfoForOneDim>(foldersToCollapse, [](FilInfoForOneDim& folderToCollapse){return folderToCollapse.m_fileInfoBD;});

}
void FilesCoordinator::collapseFoldersRecursively(QList<QString> paths)
{
    blockRevalidating(true);

    QList<std::shared_ptr<FileInfoBD>> foldersToCollapse = getFileInfoBDsFromPaths(paths);

    collapseFoldersRecursivelyHelper<std::shared_ptr<FileInfoBD>>(foldersToCollapse, [](std::shared_ptr<FileInfoBD> folderToCollapse){return folderToCollapse;});

}
template<class T>
void FilesCoordinator::collapseFoldersRecursivelyHelper(QList<T>& foldersToCollapse, std::function<std::weak_ptr<FileInfoBD> (T&)> getFileInfoBDFromT)
{
    if(foldersToCollapse.isEmpty())
    {
        blockRevalidating(false);
        return;
    }

    blockRevalidating(true);

    for(int i=0; i < foldersToCollapse.size(); ++i)
    {
        collapseFolderRecursively( getFileInfoBDFromT(foldersToCollapse[i]));
    }
}
template<class T>
void FilesCoordinator::collapseFoldersHelper(QList<T>& foldersToCollapse, std::function<std::weak_ptr<FileInfoBD> (T&)> getFileInfoBDFromT)
{
    if(foldersToCollapse.isEmpty())
    {
        blockRevalidating(false);
        return;
    }

    blockRevalidating(true);

    for(int i=0; i < foldersToCollapse.size(); ++i)
    {
        collapseFolder( getFileInfoBDFromT(foldersToCollapse[i]) );
    }
}
void FilesCoordinator::collapseSelectedFoldersRecursively()
{
   collapseFoldersRecursively(m_slctdFolds.toList());
}

QList<std::shared_ptr<FileInfoBD>> FilesCoordinator::getFileInfoBDsFromPaths(QList<QString>& paths)
{
    paths = validateFoldersToElapse(paths);

    QList<std::shared_ptr<FileInfoBD>> foldersToCollapse;
    foreach(const auto& path, paths)
    {
        std::shared_ptr<FileInfoBD> foldToCollapse = getFileInfoBDFromPath(path);
        if(foldToCollapse)
            foldersToCollapse.push_back( foldToCollapse );
    }
    return foldersToCollapse;
}
std::shared_ptr<FileInfoBD> FilesCoordinator::getFileInfoBDFromPath(const QString& path)
{
     if(m_root->getFileInfo().absoluteFilePath() == path)
         return m_root;

     std::shared_ptr<FileInfoBD> targetFiBD;

     m_root->traverseOverSubFolders([&](std::shared_ptr<FileInfoBD> subFold, int depth){
         Q_UNUSED(depth);

         if(subFold->getFileInfo().absoluteFilePath() == path)
         {
             targetFiBD = subFold;
             return true;
         }
         return false;
     }, true, 0);

     return targetFiBD;
}

//----------------------------------------------------------------------------------------------

void FilesCoordinator::collapseFolder(std::weak_ptr<FileInfoBD> fiBD)
{
    auto caller = [=](std::shared_ptr<bool> cancelled, std::function<void(double)> progress){
        Q_UNUSED(cancelled);
        Q_UNUSED(progress);
        if(auto locked = fiBD.lock())
        {
            locked->disableSignals(true);
            locked->collapse();
            locked->disableSignals(false);
        }
    };

    launchWorkerFunctional(caller);
}
void FilesCoordinator::collapseFolder(FilInfoForOneDim &fold)
{
    collapseFolder(fold.m_fileInfoBD);
}

void FilesCoordinator::collapseFolders(const QList<FilInfoForOneDim>& folds)
{
    QList<FilInfoForOneDim> foldsToCollapse = validateFoldersToElapse(folds);

    collapseFoldersHelper<FilInfoForOneDim>(foldsToCollapse, [](FilInfoForOneDim& folderToCollapse){return folderToCollapse.m_fileInfoBD;});



}
void FilesCoordinator::collapseFolders(QList<QString> paths)
{
    QList<std::shared_ptr<FileInfoBD>> foldersToCollapse = getFileInfoBDsFromPaths(paths);

    collapseFoldersHelper<std::shared_ptr<FileInfoBD>>(foldersToCollapse, [](std::shared_ptr<FileInfoBD> folderToCollapse){return folderToCollapse;});
}
void FilesCoordinator::collapseSelectedFolders()
{
   collapseFolders(m_slctdFolds.toList());
}

//----------------------------------------------------------------------------------------------


void FilesCoordinator::elapseOrCollapseFolderDependingOnCurrentState(std::weak_ptr<FileInfoBD> fiBD)
{
    bool elapse;
    bool fiIsValid = false;
    if(auto locked = fiBD.lock())
    {
        fiIsValid = true;
        elapse = !locked->elapsed();
    }
    if(fiIsValid)
    {
        if(elapse)
            elapseFolder(fiBD);
        else
            collapseFolder(fiBD);
    }
}

void FilesCoordinator::elapseAll()
{
    elapseFolderRecursiveHelper(m_root);
}

void FilesCoordinator::elapseFolder(std::weak_ptr<FileInfoBD> fiBD){
    auto caller = [=](std::shared_ptr<bool> cancelled, std::function<void(double)> progress){
        Q_UNUSED(cancelled);
        Q_UNUSED(progress);
        if(auto locked = fiBD.lock())
        {
            locked->disableSignals(true);
            locked->setElapsed(true);
            locked->disableSignals(false);
        }
    };

    launchWorkerFunctional(caller);
}

void FilesCoordinator::elapseFolderRecursiveHelper(std::weak_ptr<FileInfoBD> fiBD,
                                             bool addElapseThreadCounter)
{
    if(auto locked = fiBD.lock()){
        blockRevalidating(true);

        if(elapsingThreadCounter.load() <= 0)
            startWaitingAnimation();

        QString pathToElapse = locked->getFileInfo().absoluteFilePath();

        QElapsedTimer timer;
        timer.start();

        if(addElapseThreadCounter)
            elapsingThreadCounter.fetch_add(1);

        QThread* thread = new QThread;
        ElapseWorkerRecursively* worker = new ElapseWorkerRecursively(pathToElapse,
                                                                      0,
                                                                      true,
                                                                      m_includeHiddenFiles);

        worker->setElapseRecursively(true);

        worker->moveToThread(thread);
        connect(thread, &QThread::started,       worker, &ElapseWorkerRecursively::process);
        connect(worker, &ElapseWorkerRecursively::finished, thread, &QThread::quit);
        connect(worker, &ElapseWorkerRecursively::finished, worker, &ElapseWorkerRecursively::deleteLater);
        connect(thread, &QThread::finished,      thread, &QThread::deleteLater);
        connect(worker, &ElapseWorkerRecursively::cancelled, thread, &QThread::quit);
        connect(worker, &ElapseWorkerRecursively::cancelled, worker, &ElapseWorkerRecursively::deleteLater);
        connect(worker, &ElapseWorkerRecursively::sendingFiBdToReceiverThread,
                this, [=](FileInfoBD* fiBDElapsed, bool limitReached){
                    bool displayLimitReachedMessage = false;
                    if(auto locked = fiBD.lock()){
                        std::shared_ptr<FileInfoBD> parentFiBD = locked->getParentFiBD();
                        if(parentFiBD){
                            parentFiBD->replaceSubFolder(fiBDElapsed);
                            displayLimitReachedMessage = true;
                        }else if (m_root->getFileInfo().absoluteFilePath() == locked->getFileInfo().absoluteFilePath()){
                            m_root->replaceSubFolders(fiBDElapsed);
                            displayLimitReachedMessage = true;
                        }else{
                            qDebug() << "in filescoordinator: parentFiBD == nullptr!!!";
                        }
                    }
                    if(limitReached && displayLimitReachedMessage)
                    {
                        const QString msg = QObject::tr("Limit reached: The folder contains too many elements to display\n"
                                                        "-> not all entries will be displayed!");
//                                    qDebug() << msg;
                        QTimer::singleShot(0, [=](){
                            StaticFunctions::showInfoDialog(msg, QObject::tr("Memory Overflow"));
                        });
                    }

                    blockingThreadFinished();
                }
                , Qt::QueuedConnection
        );
        connect(this, &FilesCoordinator::killCurrentBlockingAction, worker, &ElapseWorkerRecursively::cancel, Qt::DirectConnection);
        connect(worker, &ElapseWorkerRecursively::cancelled, this, &FilesCoordinator::blockingThreadFinished, Qt::DirectConnection);
        thread->start();
    }
}

void FilesCoordinator::blockingThreadFinished()
{
    int curElapseThrdCounter = elapsingThreadCounter.fetch_add(-1) -1;
//                    qDebug() << "\n\ncurElapseThrdCounter: " << curElapseThrdCounter << "\n\n";
    if(curElapseThrdCounter <= 0)
    {
        elapsingThreadCounter.store(0);
        blockRevalidating(false);
        killWaitingAnimation();
        revalidate();
    }
}

void FilesCoordinator::elapseFoldersRecursively(const QList<FilInfoForOneDim>& folds)
{
    QList<FilInfoForOneDim> foldersToElapse = validateFoldersToElapse(folds);

    elapseFoldersRecursivelyHelper<FilInfoForOneDim>(foldersToElapse,
                                                     [](FilInfoForOneDim& filInfoOneDim){return filInfoOneDim.m_fileInfoBD;});
}
void FilesCoordinator::elapseFoldersRecursively(QList<QString> paths)
{
    QList<std::shared_ptr<FileInfoBD>> foldersToElapse = getFileInfoBDsFromPaths(paths);

    elapseFoldersRecursivelyHelper<std::shared_ptr<FileInfoBD>>(foldersToElapse,
                                                                [](std::shared_ptr<FileInfoBD> fiBD_shared){return fiBD_shared;});
}

template<class T>
void FilesCoordinator::elapseFoldersRecursivelyHelper(QList<T>&foldersToElapse,
                                                      std::function<std::weak_ptr<FileInfoBD> (T&)> getFileInfBDFromT)
{
    if(foldersToElapse.isEmpty())
        return;

    elapsingThreadCounter.fetch_add(foldersToElapse.size());

    blockRevalidating(true);

    startWaitingAnimation();

    for(int i=0; i < foldersToElapse.size(); ++i)
    {
        elapseFolderRecursiveHelper( getFileInfBDFromT(foldersToElapse[i]), false );
    }
}
void FilesCoordinator::elapseFolders(const QList<FilInfoForOneDim>& folds)
{
    QList<FilInfoForOneDim> foldsToElapse = validateFoldersToElapse(folds);

    elapseFoldersHelper<FilInfoForOneDim>(foldsToElapse, [](FilInfoForOneDim& foldToElpse){return foldToElpse.m_fileInfoBD;});
}
void FilesCoordinator::elapseFolders(QList<QString> paths)
{
    QList<std::shared_ptr<FileInfoBD>> foldsToElapse = getFileInfoBDsFromPaths(paths);

    elapseFoldersHelper<std::shared_ptr<FileInfoBD>>(foldsToElapse, [](std::shared_ptr<FileInfoBD> foldToElpse){return foldToElpse;});
}
template<class T>
void FilesCoordinator::elapseFoldersHelper(QList<T>& foldersToElapse,
                                          std::function<std::weak_ptr<FileInfoBD> (T&)> getFileInfBDFromT)
{
    if(foldersToElapse.isEmpty())
        return;

    blockRevalidating(true);

    for(int i=0; i < foldersToElapse.size(); ++i)
    {
        elapseFolder( getFileInfBDFromT( foldersToElapse[i] ) );
    }
}

void FilesCoordinator::elapseSelectedFolders()
{
    elapseFolders(m_slctdFolds.toList());
}
void FilesCoordinator::elapseSelectedFoldersRecursively()
{
    elapseFoldersRecursively(m_slctdFolds.toList());
}

void FilesCoordinator::openSelectedFoldersInNewTab()
{
    if(m_slctdFolds.size() == 0)
        return;

    QVector<QDir> dirsToOpen;
    for(auto it = m_slctdFolds.cbegin(); it != m_slctdFolds.cend(); ++it)
    {
        dirsToOpen.append( QDir(*it) );
    }

    if(m_onOpenFoldersInNewTab)
        m_onOpenFoldersInNewTab(dirsToOpen);

    emit openFoldersInNewTabSGNL(dirsToOpen);
}

void FilesCoordinator::closeCurrentTab()
{
    if(m_onCloseCurrentTab)
        m_onCloseCurrentTab();

    emit closeCurrentTabSNGL();
}

//------------------------------------------------------------------------------------------------

template <class T>
QList<T> FilesCoordinator::validateSelfContainingList(const QList<T>& refFolds, std::function<QString(const T&)> getAbsPathFromT) const
{
    QList<T> foldsToElapse;
    for(int i=0; i < refFolds.size(); ++i)
    {
        bool addFold = true;
        for(int j=0; j < foldsToElapse.size(); ++j)
        {
            if( getAbsPathFromT(refFolds[i]).isEmpty() ||
                StaticFunctions::isSubDirectory(getAbsPathFromT(refFolds[i]), getAbsPathFromT(foldsToElapse[j])) )
            {
                addFold = false;
                break;
            }
        }
        if(addFold)
        {
            foldsToElapse.append( refFolds[i] );
            int j=0;
            while(j < foldsToElapse.size()-1)
            {
                const int lastId = foldsToElapse.size()-1;

                if( StaticFunctions::isSubDirectory(getAbsPathFromT(foldsToElapse[j]), getAbsPathFromT(foldsToElapse[lastId])) )
                {
                    foldsToElapse.removeAt(j);
                }else
                {
                    ++j;
                }
            }
        }
    }
    return foldsToElapse;
}
QList<FilInfoForOneDim> FilesCoordinator::validateFoldersToElapse(const QList<FilInfoForOneDim>& refFolds) const
{
    return validateSelfContainingList<FilInfoForOneDim>(refFolds, [](const FilInfoForOneDim& fi){return fi.getAbsoluteFilePath();});
}
QList<QString>  FilesCoordinator::validateFoldersToElapse(const QList<QString>& refFolds) const
{
    return validateSelfContainingList<QString>(refFolds, [](const QString& fi){return fi;});
}
QList<FilInfoForOneDim> FilesCoordinator::validateFoldersToDelete(const QList<FilInfoForOneDim>& refFolds) const
{
    return validateFoldersToElapse(refFolds);
}
QList<QString> FilesCoordinator::validateFoldersToDelete(const QList<QString>& refFolds) const
{
    return validateSelfContainingList<QString>(refFolds, [](const QString& fi){return fi;});
}
QList<QFileInfo> FilesCoordinator::validateFoldersToCopy(const QList<QFileInfo>& refFolds) const
{
    return validateSelfContainingList<QFileInfo>(refFolds, [](const QFileInfo& fi){return fi.absoluteFilePath();});
}
QList<QString> FilesCoordinator::validateFoldersToCopy(const QList<QString>& refFolds) const
{
    return validateSelfContainingList<QString>(refFolds, [](const QString& fi){return fi;});
}


template <class T>
void FilesCoordinator::validateFilesToDelete(QList<T>& filesToDelete, const QList<T>& foldersToDelete, std::function<QString(const T&)> getAbsPathFromT) const
{
    if(foldersToDelete.size() == 0)
        return;

    int i=0;
    while( i < filesToDelete.size() )
    {
        bool containingFolderWillAlrdBeDeleted = false;
        for(int j=0; j < foldersToDelete.size(); j++)
        {
            if( StaticFunctions::isSubDirectory(getAbsPathFromT(filesToDelete[i]), getAbsPathFromT(foldersToDelete[j])) )
//          if( getAbsPathFromT(filesToDelete[i]).startsWith( getAbsPathFromT(foldersToDelete[j]) ) )
            {
                containingFolderWillAlrdBeDeleted = true;
                break;
            }
        }
        if( containingFolderWillAlrdBeDeleted )
        {
            filesToDelete.removeAt(i);
        }else
        {
            ++i;
        }
    }
}
void FilesCoordinator::validateFilesToDelete(QList<QFileInfo>& filesToDelete, const QList<QFileInfo>& foldersToDelete) const
{
    validateFilesToDelete<QFileInfo>(filesToDelete, foldersToDelete, [](const QFileInfo& fi){return fi.absoluteFilePath();});
}
void FilesCoordinator::validateFilesToDelete(QList<QString>& filesToDelete, const QList<QString>& foldersToDelete) const
{
    validateFilesToDelete<QString>(filesToDelete, foldersToDelete, [](const QString& entry){return entry;});
}

void FilesCoordinator::validateFilesToCopy(QList<QString>& filesToCopy, const QList<QString>& foldersToCopy) const
{
    validateFilesToDelete<QString>(filesToCopy, foldersToCopy, [](const QString& entry){return entry;});
}
void FilesCoordinator::validateFilesToCopy(QList<QFileInfo>& filesToCopy, const QList<QFileInfo>& foldersToCopy) const
{
    validateFilesToDelete<QFileInfo>(filesToCopy, foldersToCopy, [](const QFileInfo& entry){return entry.absoluteFilePath();});
}

void FilesCoordinator::folderChanged(std::weak_ptr<FileInfoBD> f)
{
    Q_UNUSED(f);

    if(revalidatingIsBlocked())
        return;

    revalidateOneDimnFolderOrder();
    repaintFolderViewer();
}

void FilesCoordinator::folderElapsed(std::weak_ptr<FileInfoBD> f)
{
    if(revalidatingIsBlocked())
        return;

    revalidateOneDimnFolderOrder();
    repaintFolderViewer();
}

void FilesCoordinator::sortingChanged(std::weak_ptr<FileInfoBD> f)
{
    if(revalidatingIsBlocked())
        return;

    if(auto refLock = f.lock()){

        int oneDimId = -1;
        int oneDispDimId = -1;

        for(int i=0; i < m_oneDimnFolderOrderDisplayed.size(); i++){
            if( !m_oneDimnFolderOrderDisplayed[i].m_isFolder ){
                if (auto vglLock = m_oneDimnFolderOrderDisplayed[i].m_fileInfoBD.lock()){
                    if(refLock == vglLock){
                        // first file of folder detected:
                        oneDispDimId = i;
                        break;
                    }
                }
            }
        }
        for(int i=0; i < m_oneDimnFolderOrder.size(); i++){
            if( !m_oneDimnFolderOrder[i].m_isFolder ){
                if (auto vglLock = m_oneDimnFolderOrder[i].m_fileInfoBD.lock()){
                    if(refLock == vglLock){
                        // first file of folder detected:
                        oneDimId = i;
                        break;
                    }
                }
            }
        }
        if(oneDimId > -1 || oneDispDimId > -1){
            int offs = 0;
            foreach(const auto& fi, refLock->getFiles()){
                if(oneDimId > -1 && oneDimId+offs < m_oneDimnFolderOrder.size()){
                    m_oneDimnFolderOrder[oneDimId+offs] = FilInfoForOneDim(f, false, fi.fileName(),
                                                                           m_oneDimnFolderOrder[oneDimId+offs].m_depth);
                }
                if(oneDispDimId > -1 && oneDispDimId+offs < m_oneDimnFolderOrderDisplayed.size()){
                    m_oneDimnFolderOrderDisplayed[oneDispDimId+offs] = FilInfoForOneDim(f, false, fi.fileName(),
                                                                            m_oneDimnFolderOrderDisplayed[oneDispDimId+offs].m_depth);
                }
                ++offs;
            }
        }
    }
    revalidateSearchIds();
}

void FilesCoordinator::addDirectoryToWatcher(QString directory)
{
    if(m_watcher){
        m_watcher->addPath(directory);
    }
}
void FilesCoordinator::removeDirectoryFromWatcher(QString directory)
{
    if(m_watcher){
        m_watcher->removePath(directory);
    }
}

void FilesCoordinator::directoryChanged(QString path)
{
    for(int i=0; i < m_oneDimnFolderOrder.size(); i++){
        QString refPath = m_oneDimnFolderOrder[i].getAbsoluteFilePath();
        if( m_oneDimnFolderOrder[i].m_isFolder &&
                !refPath.isEmpty() && refPath == path){
            if(auto locked = m_oneDimnFolderOrder[i].m_fileInfoBD.lock()){
                locked->directoryChanged(path);
            }
            return;
        }
    }
}

void FilesCoordinator::unmountDrive(QDir drive)
{
    StaticFunctions::unmountDrive(drive);

//    auto caller = [=](std::shared_ptr<bool> cancelled, std::function<void(double)> progress){
//        Q_UNUSED(cancelled);
//        Q_UNUSED(progress);
//        StaticFunctions::unmountDrive(drive);
//    };

//    launchElapseWorkerFunctional(caller, QString("copying files..."));
}

void FilesCoordinator::copySelectedContent()
{
    copyCutToClipboardHelper(false);
}

void FilesCoordinator::cutSelectedContent()
{
    copyCutToClipboardHelper(true);
}

void FilesCoordinator::copyCutToClipboardHelper(bool deleteSourceAfterCopying)
{
    QClipboard *clipboard = QApplication::clipboard();
    QList<QUrl> urls;

    if(deleteSourceAfterCopying){
        urls.append(QUrl("cut"));
    }

    foreach (const auto& foldPath, m_slctdFolds) {
        if( !foldPath.isEmpty() && QFileInfo(foldPath).exists() ){
            urls.append( QUrl::fromLocalFile(foldPath) );
        }
    }
    foreach (const auto& filePath, m_slctdFiles) {
        if( !filePath.isEmpty() && QFileInfo(filePath).exists() ){
            urls.append( QUrl::fromLocalFile(filePath) );
        }
    }
    if( urls.size() > 0 ){
        QMimeData *mimeData = new QMimeData;
        mimeData->setUrls(urls);
        clipboard->setMimeData(mimeData);
    }
}

void FilesCoordinator::duplicateSelectedContent()
{
    auto caller = [=](std::shared_ptr<bool> cancelled, std::function<void(double)> progress){
        double totalProgress = double(m_slctdFiles.size() + m_slctdFolds.size());
        double curProgress = 0.0;

        foreach(const auto& filePath, m_slctdFiles){
            StaticFunctions::duplicateFile( cancelled, QFileInfo(filePath) );
            curProgress += 1.0;
            if(progress)
            {
                progress(curProgress / totalProgress);
            }
        }
        foreach(const auto& folderPath, m_slctdFolds){
            StaticFunctions::duplicateFolder(cancelled, QFileInfo(folderPath));
            curProgress += 1.0;
            if(progress)
            {
                progress(curProgress / totalProgress);
            }
        }
    };
    launchWorkerFunctional(caller, QString("copying files..."));
}

void FilesCoordinator::copySelectedFilePathToClipboard()
{
    if(singleContentSelected())
    {
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText( selectedContent()[0] );
    }

}

void FilesCoordinator::pasteFromClipboard()
{
    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    if      (mimeData->hasUrls()) {
        pasteHelper( mimeData->urls() );
    }else if(mimeData->hasText()) {
        QString paths = mimeData->text();
        pasteHelper( paths );
    }
}

void FilesCoordinator::paste(QString paths, QString tarPath)
{
    pasteHelper(paths, tarPath);
}
void FilesCoordinator::pasteHelper(QString paths, QString tarPath){
    QTextStream stream(&paths);
    QString line;
    QVector<QString> pathsFromClpbrd;
    while (stream.readLineInto(&line)) {
        pathsFromClpbrd.append(line);
    }
    if(pathsFromClpbrd.size() > 0){
        pasteHelper(pathsFromClpbrd, tarPath);
    }
}

void FilesCoordinator::pasteHelper(QList<QUrl> urls, QString tarPath)
{
    QVector<QString> pathsFromClpbrd;
    foreach (const auto& url, urls) {
        if(url.toString() == QString("cut")){
            pathsFromClpbrd.append(QString("cut"));
        }else{
            QString pth = url.toLocalFile();
            if( !pth.isEmpty() ){
                pathsFromClpbrd.append( url.toLocalFile() );
            }
        }
    }
    if(pathsFromClpbrd.size() > 0){
        pasteHelper(pathsFromClpbrd, tarPath);
    }
}
void FilesCoordinator::pasteHelper(QVector<QString> pathsFromClpbrd, const QString tarPath_)
{
    auto caller = [=](std::shared_ptr<bool> cancelled, std::function<void(double)> progress){

        QString tarPath = tarPath_;
        bool moveInsteadOfCopy = false;
        if(pathsFromClpbrd.size() > 0){

            if(pathsFromClpbrd[0] == QString("cut"))
                moveInsteadOfCopy = true;

            double totalProgress = 0;
            double curProgress = 0.0;

            if(moveInsteadOfCopy)
            {
                totalProgress = pathsFromClpbrd.size()-1; // -1, da pathsFromClpbrd[0] == "cut"
            }else
            {
                int pathCountr = 0;
                foreach(const auto& sourcePath, pathsFromClpbrd)
                {
                    if( !(moveInsteadOfCopy && pathCountr==0) )
                        totalProgress += double( StaticFunctions::evaluateContentCount(sourcePath) );
                    ++pathCountr;
                }
            }

            QList<QFileInfo> foldersToCopy;
            QList<QFileInfo> filesToCopy;
            for(int i = moveInsteadOfCopy ? 1 : 0; i < pathsFromClpbrd.size(); ++i)
            {
                QFileInfo fiInfo( pathsFromClpbrd[i] );
                if(fiInfo.exists())
                {
                    if(fiInfo.isDir())
                    {
                        foldersToCopy.append(fiInfo);
                    }else
                    {
                        filesToCopy.append(fiInfo);
                    }
                }
            }

//            int foldersToCopyCount = foldersToCopy.size();
//            int filesToCopyCount = filesToCopy.size();

            foldersToCopy = this->validateFoldersToCopy(foldersToCopy);
            this->validateFilesToCopy(filesToCopy, foldersToCopy);

//            qDebug() << "in pasteHelper: moveInsteadOfCopy: " << moveInsteadOfCopy
//                     << "   foldersToCopy before evaluation: " << foldersToCopyCount
//                     << "   after: " << foldersToCopy.size()
//                     << "   filesToCopyCount before: " << filesToCopyCount
//                     << "   after: " << filesToCopy.size();

            if(!tarPath.isEmpty()){
                QFileInfo fi(tarPath);
                if(fi.exists()){
                    if(fi.isFile()){
                        tarPath = fi.absolutePath();
                    }else{
                        tarPath = fi.absoluteFilePath();
                    }
                }
            }
            if( (tarPath.isEmpty() || !QFileInfo(tarPath).exists())
                    && singleFolderSelected()){
                tarPath = QString(*m_slctdFolds.cbegin());
            }
            if( tarPath.isEmpty() || !QFileInfo(tarPath).exists()){
                if(m_root){
                    tarPath = m_root->getFileInfo().absoluteFilePath();
                }
            }

            double progressSinceLastUpdate = 0.0;

            std::function<void()> callAfterProgress = [&](){
                ++curProgress;
                double newProgress = (curProgress / totalProgress);
                if(newProgress - progressSinceLastUpdate >= 0.01) // erst wenn min. 1 prozent dazugekommen ist, updaten. Wenn z.b. 10.000 dateien kopiert werden, nicht nach jeder einzlenen kopierten datei updaten
                {
                    progress( newProgress );
                    progressSinceLastUpdate = newProgress;
                }
            };

            bool overrideIfAlreadyExists = false;

            if( !tarPath.isEmpty() && QFileInfo(tarPath).exists()){
                foreach (const auto& folderToCopy, foldersToCopy)
                {
                    QString targetName = folderToCopy.fileName();
                    if(moveInsteadOfCopy)
                    {
                        StaticFunctions::moveFolder(cancelled,
                                                    folderToCopy.absoluteFilePath(),
                                                    tarPath,
                                                    targetName,
                                                    overrideIfAlreadyExists,
                                                    callAfterProgress
                                                    );
                    }else
                    {
                        StaticFunctions::copyFolder(cancelled,
                                                    folderToCopy.absoluteFilePath(),
                                                    tarPath,
                                                    targetName,
                                                    false, // == deleteAfterCopying
                                                    callAfterProgress
                                                    );
                    }
                }
                foreach (const auto& fileToCopy, filesToCopy)
                {
                    QString targetName = fileToCopy.fileName();
                    if(moveInsteadOfCopy)
                    {
                        StaticFunctions::moveFile(  cancelled,
                                                    fileToCopy.absoluteFilePath(),
                                                    tarPath,
                                                    targetName,
                                                    overrideIfAlreadyExists,
                                                    callAfterProgress
                                                    );
                    }else
                    {
                        StaticFunctions::copyFile(cancelled,
                                                  fileToCopy.absoluteFilePath(),
                                                  tarPath,
                                                  targetName,
                                                  false, // == deleteAfterCopying,
                                                  callAfterProgress);
                    }
                }
            }
        }
    };

    launchWorkerFunctional(caller, QString("copying files..."));
}

void FilesCoordinator::launchWorkerFunctional(std::function<void (std::shared_ptr<bool>, std::function<void(double)>)> caller,
                                                                       QString infoMessageStr)
{
    WorkerBDFunctional* worker = new WorkerBDFunctional(caller, false);

    worker->prepare();

    // folgendes auskommentiert:
    // grund: wenn killCurrentBlockingAction ausgeloest wird, sollte wohl eher nicht der ElapseWorkerFunctional
    // terminiert werden. Z.B. sollte das kopieren von dateien nicht abgebrochen werden, nur weil der filescoordinator geschlossen wird.
    // ausserdem ist dafuer ja gerade der QProgressDialog fuer das abbrechen/canceln zustaendig
//    connect(this, &FilesCoordinator::killCurrentBlockingAction, worker, &ElapseWorkerFunctional::cancel);


    if( !infoMessageStr.isEmpty() ){
        QProgressDialog* progress = new QProgressDialog(infoMessageStr, QString("cancel"), 0, 100);

        connect(worker, &WorkerBDFunctional::progress, progress, &QProgressDialog::setValue);//, Qt::QueuedConnection);

        progress->setWindowModality(Qt::WindowModal);
        progress->show();

        StaticFunctions::setIconToWidget(progress);

        connect(progress, &QProgressDialog::canceled, worker, &WorkerBDFunctional::cancel, Qt::DirectConnection);

        connect(worker, &WorkerBDFunctional::finished,   progress, &QProgressDialog::deleteLater, Qt::QueuedConnection);
        connect(worker, &WorkerBDFunctional::cancelled,   progress, &QProgressDialog::deleteLater, Qt::QueuedConnection);
        connect(worker, &WorkerBDFunctional::finished,   progress, &QProgressDialog::close, Qt::QueuedConnection);
        connect(worker, &WorkerBDFunctional::cancelled,   progress, &QProgressDialog::close, Qt::QueuedConnection);
        // folgendes auskommentiert:
        // grund: wenn killCurrentBlockingAction ausgeloest wird, sollte wohl eher nicht der ElapseWorkerFunctional
        // terminiert werden. Z.B. sollte das kopieren von dateien nicht abgebrochen werden, nur weil der filescoordinator geschlossen wird.
        // ausserdem ist dafuer ja gerade der QProgressDialog fuer das abbrechen/canceln zustaendig
//        connect(this, &FilesCoordinator::killCurrentBlockingAction, progress, &QProgressDialog::close);
//        connect(this, &FilesCoordinator::killCurrentBlockingAction, progress, &QProgressDialog::deleteLater);
    }
    worker->start();
}

void FilesCoordinator::launchWorkerFunctional(std::function<void (std::shared_ptr<bool>, std::function<void(double)>)> caller,
                                                    bool revalidate)
{
    elapsingThreadCounter.fetch_add(1);

    startWaitingAnimation();

    WorkerBDFunctional* worker = new WorkerBDFunctional(caller, false);

    worker->prepare();
    if(revalidate)
    {
        connect(worker, &WorkerBDFunctional::finished,  this, &FilesCoordinator::blockingThreadFinished, Qt::QueuedConnection);
        connect(worker, &WorkerBDFunctional::cancelled, this, &FilesCoordinator::blockingThreadFinished, Qt::QueuedConnection);
    }
    connect(this, &FilesCoordinator::killCurrentBlockingAction, worker, &WorkerBDFunctional::cancel, Qt::DirectConnection);

    worker->start();
}

void FilesCoordinator::launchWorkerFunctional(std::function<void(std::function<void(double)>)> caller,
                                              Canceller* canceller)
{
//    elapsingThreadCounter.fetch_add(1);

//    startWaitingAnimation();

//    WorkerBDFunctional* worker = new WorkerBDFunctional(caller, false);

//    worker->prepare();
//    if(revalidate)
//    {
//        connect(worker, &WorkerBDFunctional::finished,  this, &FilesCoordinator::blockingThreadFinished, Qt::QueuedConnection);
//        connect(worker, &WorkerBDFunctional::cancelled, this, &FilesCoordinator::blockingThreadFinished, Qt::QueuedConnection);
//    }
//    connect(this, &FilesCoordinator::killCurrentBlockingAction, worker, &WorkerBDFunctional::cancel, Qt::DirectConnection);

//    worker->start();
}

void FilesCoordinator::deleteSelectedContent()
{
    if(contentSelected()){
        QMessageBox msgBox;
        msgBox.setText("Are you sure you want to delete the selected content?");
        msgBox.setInformativeText(QString("%1 files and %2 folders selected for deletion")
                                  .arg(m_slctdFiles.size())
                                  .arg(m_slctdFolds.size()));
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Ok);

        int ret = msgBox.exec();

        if(ret == QMessageBox::Ok){            
            // validateFoldersToDelete(m_slctdFolders) stellt sicher, dass ein sub-folder
            // nicht beruecksichtigt wird, wenn sein parent-folder ebenfalls zu loeschen ist:
            QList<QFileInfo> foldersToDelete;
            foreach(const auto& folder, validateFoldersToDelete(m_slctdFolds.toList())){
                foldersToDelete.append( QFileInfo(folder) );
            }

            QList<QFileInfo> filesToDelete;
            foreach(const auto& file, m_slctdFiles){
                 filesToDelete.append( QFileInfo(file) );
            }

            validateFilesToDelete(filesToDelete, foldersToDelete);

            if(foldersToDelete.size() + filesToDelete.size() == 0)
                return;

            auto caller = [=](std::shared_ptr<bool> cancelled, std::function<void(double)> progress){

                double totalProgress = double(filesToDelete.size() + foldersToDelete.size());
                double curProgress = 0.0;

                foreach(const QFileInfo file, filesToDelete)
                {
                    if(cancelled && *cancelled)
                        return;
                    StaticFunctions::deleteFile(cancelled, file);

                    curProgress += 1.0;
                    progress(curProgress / totalProgress);
                }
                foreach(auto folder, foldersToDelete)
                {
                    if(cancelled && *cancelled)
                        return;
                    StaticFunctions::deleteFolder(cancelled, folder);

                    curProgress += 1.0;
                    progress(curProgress / totalProgress);
                }
            };

            launchWorkerFunctional(caller, QString("deleting files..."));
        }
    }
}

void FilesCoordinator::openSelectedContent()
{
    QTimer::singleShot(0, nullptr, [=](){
        foreach(const auto& fi, m_slctdFiles){
            if(QFileInfo(fi).isExecutable()){
                QProcess *process = new QProcess(this);
                process->start( fi );
            }else{
                QDesktopServices::openUrl(QUrl(QString("file:%1").arg(fi)));
            }
        }
        foreach(const auto& fi, m_slctdFolds){
            QDesktopServices::openUrl(QUrl(QString("file:%1").arg(fi)));
        }
    });
}

void FilesCoordinator::openSelectedContentWith()
{
    if(singleContentSelected() && m_slctdFiles.size() == 1)
    {
        QFileInfo fileToOpen (*m_slctdFiles.cbegin() );
        QTimer::singleShot(0, this, [=](){
            new OpenWithDialog(fileToOpen, QSize(400,400));
        });
    }
}

void FilesCoordinator::showDetailsOfSelectedContent()
{
}

void FilesCoordinator::renameSelectedContent()
{
    if(m_slctdFiles.size() == 1){
        StaticFunctions::renameFolder(QFileInfo( *(m_slctdFiles.cbegin()) ));
    }else if (m_slctdFolds.size() == 1){
        StaticFunctions::renameFolder( *(m_slctdFolds.cbegin()) );
    }
}

void FilesCoordinator::zipSelectedContent()
{
    QList<QFileInfo> foldersToZip;
    QList<QFileInfo> filesToZip;
    QFileInfo tarZipFile;

    if(this->singleContentSelected())
    {
        QFileInfo entryToZip = QFileInfo(selectedContent()[0]);
        filesToZip.push_back( entryToZip );

        QString fileNameWithoutExtension = StaticFunctions::getFileNameWithoutFileType(entryToZip.fileName());

        QString zipFileName = StaticFunctions::getTextViaDialog(QString(fileNameWithoutExtension.append(".zip")),
                                                                QString("select a file name"));

        QString tarZipPath = StaticFunctions::getDir(entryToZip.absoluteFilePath());
        QString tarZipFilePath = QString("%1%2%3").arg(tarZipPath)
                                                  .arg(QDir::separator())
                                                  .arg(zipFileName);

        tarZipFile = QFileInfo(tarZipFilePath);
    }else
    {
        foreach(const auto& fi, m_slctdFiles){
            filesToZip.push_back( QFileInfo(fi) );
        }
        foreach(const auto& fi, m_slctdFolds){
            foldersToZip.push_back( QFileInfo(fi) );
        }

        QList<QFileInfo> entriesToZip = foldersToZip;
        entriesToZip.append(filesToZip);


        QString zipFileName = StaticFunctions::getTextViaDialog(QString("zippedFiles.zip"),
                                                                QString("select a file name"));

        QFileInfo commonDir = StaticFunctions::getCommonParentDir( entriesToZip.toVector() );
        if(!commonDir.exists() || commonDir.absoluteFilePath().isEmpty())
        {
            qDebug() << "in FilesCoordinator::zipSelectedContent -> commomDir does not exist || commonDir.absFilePath() is empty!!! -> aborting fileZipping!";
        }
        QString tarZipDir = commonDir.absoluteFilePath();

        QString tarZipFilePath = QString("%1%2%3").arg(tarZipDir)
                                                  .arg(QDir::separator())
                                                  .arg(zipFileName);

        tarZipFile = QFileInfo(tarZipFilePath);
    }

    foldersToZip = validateFoldersToCopy(foldersToZip);
    validateFilesToCopy(filesToZip, foldersToZip);
    filesToZip.append(foldersToZip);

//    qDebug() << "tarZipFile: " << tarZipFile.absoluteFilePath()
//             << "   selectedFiles.count: " << filesToZip.size();
    if(filesToZip.size() > 0)
    {
        auto caller = [=](std::shared_ptr<bool> cancelled, std::function<void(double)> progress){
            Q_UNUSED(progress);
            StaticFunctions::zipFile(cancelled, filesToZip, tarZipFile, true);
        };
        launchWorkerFunctional(caller, QString("zipping files..."));
    }
}

void unzipPotentialFile(std::shared_ptr<bool> cancelled, const QFileInfo& qfi, bool wait_for_command_to_finish = false)
{
    if( qfi.exists() && StaticFunctions::isZippedFile(qfi) )
    {
        StaticFunctions::unzipFile(cancelled, qfi, wait_for_command_to_finish);
    }
}
void FilesCoordinator::unzipSelectedContent()
{
    auto caller = [=](std::shared_ptr<bool> cancelled, std::function<void(double)> progress){
        double totalProgress = double(m_slctdFiles.size() + m_slctdFolds.size());
        double curProgress = 0.0;

        foreach(const auto& fi, m_slctdFiles){
            unzipPotentialFile( cancelled, QFileInfo(fi), true );
            curProgress += 1.0;
            progress(curProgress / totalProgress);
        }
        foreach(const auto& fi, m_slctdFolds){
            unzipPotentialFile( cancelled, QFileInfo(fi), true );
            curProgress += 1.0;
            progress(curProgress / totalProgress);
        }
    };
    launchWorkerFunctional(caller, QString("unzipping file..."));
}

void FilesCoordinator::createNewFolder()
{
    addNewFileFolderHelper(false);
}

void FilesCoordinator::createNewFile()
{
    addNewFileFolderHelper(true);
}

void FilesCoordinator::addNewFileFolderHelper(bool createFile)
{
    bool ok;
    QString fileOrFoldName = QInputDialog::getText(nullptr, QString(""),
                                         QString("Choose a name for the new %1:")
                                                   .arg(createFile ?
                                                            tr("file") :
                                                            tr("folder")),
                                                   QLineEdit::Normal,
                                         createFile ?
                                         QString("new_File.txt") :
                                         QString("new_Folder"), &ok);
    if (ok && !fileOrFoldName.isEmpty()){
        QString path;
        if(m_slctdFolds.size() == 1){
            path = QString(*m_slctdFolds.begin());
            QFileInfo fi(path);
            if( !path.isEmpty() && fi.exists()){
                path = fi.absoluteFilePath();
            }
        }else if (m_slctdFiles.size() == 1){
            path = QString(*m_slctdFiles.begin());
            QFileInfo fi(path);
            if( !path.isEmpty() && fi.exists()){
                path = fi.absolutePath();
            }
        }else{
            if(m_root){
                path = m_root->getFileInfo().absoluteFilePath();
            }
        }
        if(!path.isEmpty()){
            if(createFile){
                StaticFunctions::createNewFile  (path, fileOrFoldName);
            }else{
                StaticFunctions::createNewFolder(path, fileOrFoldName);
            }
        }
    }
}

void FilesCoordinator::requestFocus()
{
    if(m_focusCaller)
        m_focusCaller(m_self);

    emit isNowFocused(m_self);

    emit focusFolderViewerSGNL();
}

void FilesCoordinator::initDragging(QString initiator)
{
    QList<QUrl> urls;
    urls.append(QUrl("cut"));

    foreach(const auto& path, m_slctdFolds)
    {
        if( !path.isEmpty() && QFileInfo(path).exists() ){
            urls.append( QUrl::fromLocalFile(path) );
        }
    }
    foreach(const auto& path, m_slctdFiles)
    {
        if( !path.isEmpty() && QFileInfo(path).exists() ){
            urls.append( QUrl::fromLocalFile(path) );
        }
    }

    if( !initiator.isEmpty() && QFileInfo(initiator).exists()){
        QUrl initatorUrl = QUrl::fromLocalFile(initiator);
        if( !urls.contains(initatorUrl) ){
            urls.append( initatorUrl );
        }
    }

    if(urls.size() > 0){
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;

        mimeData->setUrls( urls );
        drag->setMimeData(mimeData);
//        drag->setPixmap(iconPixmap);

        Qt::DropAction dropAction = drag->exec();
        Q_UNUSED( dropAction )
    }
}

QList<QString> FilesCoordinator::selectedFolders() const
{
    m_slctdFolds.toList();
}

QList<QString> FilesCoordinator::selectedFiles() const
{
    m_slctdFiles.toList();
}

QList<QString> FilesCoordinator::selectedContent() const
{
    QList<QString> entries = m_slctdFolds.toList();
    entries.append(m_slctdFiles.toList());
    return entries;
}

bool FilesCoordinator::isSelected(const QString &absFilePath) const
{
    return m_slctdFiles.contains(absFilePath) || m_slctdFolds.contains(absFilePath);
}

int FilesCoordinator::getIdOf(const FilInfoForOneDim& fileInfOneDim, bool searchInDisplayedContent) const
{
    const QList<FilInfoForOneDim>* vec;
    if(searchInDisplayedContent){
        vec = &m_oneDimnFolderOrderDisplayed;
    }else{
        vec = &m_oneDimnFolderOrder;
    }
    if(auto refLock = fileInfOneDim.m_fileInfoBD.lock()){
        for(int i=0; i < vec->size(); i++){
            if(vec->at(i).m_isFolder == fileInfOneDim.m_isFolder){
                if(auto curLock = vec->at(i).m_fileInfoBD.lock()){
                    if(curLock == refLock &&
                                   vec->at(i).getFileName() == fileInfOneDim.getFileName()){
                        return i;
                    }
                }
            }
        }
    }
    return -1;
}
int FilesCoordinator::getIdOf(std::shared_ptr<FilInfoForOneDim> fileInfOneDim, bool searchInDisplayedContent) const
{
    if(fileInfOneDim){
        const QList<FilInfoForOneDim>* vec;
        if(searchInDisplayedContent){
            vec = &m_oneDimnFolderOrderDisplayed;
        }else{
            vec = &m_oneDimnFolderOrder;
        }
        if(auto refLock = fileInfOneDim->m_fileInfoBD.lock()){
            for(int i=0; i < vec->size(); i++){
                if(vec->at(i).m_isFolder == fileInfOneDim->m_isFolder){
                    if(auto curLock = vec->at(i).m_fileInfoBD.lock()){
                        if(curLock == refLock &&
                                       vec->at(i).getFileName() == fileInfOneDim->getFileName()){
                            return i;
                        }
                    }
                }
            }
        }
    }
    return -1;
}
int FilesCoordinator::getIdOf(const QString& absEntryPath, bool searchInDisplayedContent) const
{
    if(absEntryPath.isEmpty())
        return -1;

    const QList<FilInfoForOneDim>* vec;
    if(searchInDisplayedContent){
        vec = &m_oneDimnFolderOrderDisplayed;
    }else{
        vec = &m_oneDimnFolderOrder;
    }
    for(int i=0; i < vec->size(); i++){
        if(vec->at(i).getAbsoluteFilePath() == absEntryPath)
            return i;
    }
    return -1;
}

const FilInfoForOneDim& FilesCoordinator::getFilInfoForOneDimFromPath(const QString absPath, bool searchInDisplayedContent)
{
    int id = getIdOf(absPath, searchInDisplayedContent);
    QList<FilInfoForOneDim>* vec = searchInDisplayedContent ? &m_oneDimnFolderOrderDisplayed :
                                                        &m_oneDimnFolderOrder;
    if(id < vec->size() && id >= 0)
    {
        return vec->at(id);
    }
    std::stringstream ss;
    ss << "Exception: FilesCoordinator::getFilInfoForOneDimFromPath: id: " << id << "   vec.size: " << vec->size();
    throw ss.str();
}

void FilesCoordinator::selectAllInRange(int lowerBound, int upperBound, bool selectInDisplayedContent)
{
    QList<FilInfoForOneDim>& vec = selectInDisplayedContent ? m_oneDimnFolderOrderDisplayed :
                                                  m_oneDimnFolderOrder;
    for(int i=lowerBound; i <= upperBound; ++i)
    {
        if(i >= 0 && i < vec.size())
        {
            if(vec[i].m_isFolder)
                m_slctdFolds.insert(vec[i].getAbsoluteFilePath());
            else
                m_slctdFiles.insert(vec[i].getAbsoluteFilePath());
        }
    }
}

void FilesCoordinator::selectContent(QString entry, bool isFolder,
                                     bool controlPrsd, bool shiftPrsd)
{
    if(!controlPrsd && !shiftPrsd){
        m_slctdFolds.clear();
        m_slctdFiles.clear();
    }
    if(shiftPrsd){
        int lastSelectiondId = getIdOf(m_lastSelection);
        int curId = getIdOf(entry);

        selectAllInRange(std::min(curId, lastSelectiondId), std::max(curId, lastSelectiondId));

        m_lastSelection = entry;
        repaintFolderViewer();
    }else{
        m_lastSelection = entry;
        if(isFolder){
            if(m_slctdFolds.contains(entry)){
                deselectContent(entry);
                return;
            }else{
                m_slctdFolds.insert(entry);
            }
            repaintFolderViewer();
        }else{
            if(m_slctdFiles.contains(entry)){
                deselectContent(entry);
                return;
            }else{
                m_slctdFiles.insert(entry);
            }
            repaintFolderViewer();
        }
    }
}

void FilesCoordinator::sortFromDisplayedContent(QFileInfo fi, ORDER_BY order)
{
    if( !fi.exists() )
        return;

    for(int i=0; i < m_oneDimnFolderOrderDisplayed.size(); ++i)
    {
        if(m_oneDimnFolderOrderDisplayed[i].getAbsoluteFilePath() == fi.absoluteFilePath())
        {
            sort(m_oneDimnFolderOrderDisplayed[i].m_fileInfoBD, order);
            return;
        }
    }
}

void FilesCoordinator::setZoomFactor(int zoomFactor)
{
    m_zoomFactor = zoomFactor;
}


void FilesCoordinator::selectEntireContent()
{
//    qDebug() << "in FilesCoordinator.selectEntireContent";
    m_slctdFolds.clear();
    m_slctdFiles.clear();

    auto func =
            [=](std::weak_ptr<FileInfoBD> fiBD, int depth){
        Q_UNUSED(depth);
        if(auto locked = fiBD.lock())
            m_slctdFolds.insert(locked->getFileInfo().absoluteFilePath());
    };
    auto fileFunc =
            [=](std::weak_ptr<FileInfoBD> fiBD, int depth){
        Q_UNUSED(depth);
        if(auto fiBDLock = fiBD.lock()){
            foreach(const QFileInfo& fi, fiBDLock->getFiles()){
                m_slctdFiles.insert( fi.absoluteFilePath() );
            }
        }
    };
    m_root->traverse(func, fileFunc, true, 0);

    if(m_slctdFolds.size() > 0){
        m_lastSelection = *m_slctdFolds.begin();
    }else if(m_slctdFiles.size() > 0){
        m_lastSelection = *m_slctdFiles.begin();
    }

    repaintFolderViewer();
}

void FilesCoordinator::deselectContent(const QString& entry, bool repaint)
{
    m_slctdFolds.remove(entry);
    m_slctdFiles.remove(entry);
    if(repaint)
        repaintFolderViewer();
}

bool FilesCoordinator::includeHiddenFiles() const
{
    return m_includeHiddenFiles;
}

void FilesCoordinator::setIncludeHiddenFiles(bool includeHiddenFiles)
{
    if(m_includeHiddenFiles == includeHiddenFiles)
        return;

    m_includeHiddenFiles = includeHiddenFiles;

    if(m_root)
    {
        m_root->disableSignals(true);
        m_root->includeHiddenFilesChanged();
        m_root->disableSignals(false);
        revalidate();
    }
}

void FilesCoordinator::clearSelection()
{
//    qDebug() << "in FilesCoordinator.clearSelection";
    m_slctdFiles.clear();
    m_slctdFolds.clear();

    repaintFolderViewer();
}

//std::function<void(FilInfoForOneDim)> FilesCoordinator::getSelectionFunction(){
//    return [=](FilInfoForOneDim fiForOneDim){
//        if(fiForOneDim.m_isFolder){
//            m_slctdFolds.insert( fiForOneDim.getAbsoluteFilePath() );
//        }else{
//            m_slctdFiles.insert( fiForOneDim.getAbsoluteFilePath() );
//        }
//        m_lastSelection = fiForOneDim.getAbsoluteFilePath();
//    };
//}
void FilesCoordinator::selectButtonUp(bool ctrl_prsd, bool shft_prsd)
{
    if( !(shft_prsd || ctrl_prsd) ){
        m_slctdFolds.clear();
        m_slctdFiles.clear();
    }
    int curId = getIdOf(m_lastSelection);
    if(curId == -1)
        return;

    int tarId = curId - 1;
    if(tarId < 0)
        tarId = m_oneDimnFolderOrderDisplayed.size() - 1;

    if(tarId > -1 && m_oneDimnFolderOrderDisplayed.size() > tarId)
    {
        QString path = m_oneDimnFolderOrderDisplayed[tarId].getAbsoluteFilePath();

        m_lastSelection = path;

        setSelected(path, m_oneDimnFolderOrderDisplayed[tarId].m_isFolder);

        focusFolderViewer(tarId);
    }
}


void FilesCoordinator::selectButtonDown(bool ctrl_prsd, bool shft_prsd)
{
    if( !(shft_prsd || ctrl_prsd) ){
        m_slctdFolds.clear();
        m_slctdFiles.clear();
    }
    int curId = getIdOf(m_lastSelection);
    if(curId == -1)
        return;

    int tarId = curId + 1;
    if(tarId > m_oneDimnFolderOrderDisplayed.size()-1)
        tarId = 0;

    if(tarId > -1 && m_oneDimnFolderOrderDisplayed.size() > tarId)
    {
        QString path = m_oneDimnFolderOrderDisplayed[tarId].getAbsoluteFilePath();

        m_lastSelection = path;

        setSelected(path, m_oneDimnFolderOrderDisplayed[tarId].m_isFolder);

        focusFolderViewer(tarId);
    }
}

bool FilesCoordinator::filesSelected() const
{
    return m_slctdFiles.size() != 0;
}

bool FilesCoordinator::foldersSelected() const
{
    return m_slctdFolds.size() != 0;
}

bool FilesCoordinator::contentSelected() const
{
    return filesSelected() || foldersSelected();
}

bool FilesCoordinator::singleFolderSelected() const
{
    return m_slctdFolds.size() == 1;
}

bool FilesCoordinator::singleFileSelected() const
{
    return m_slctdFiles.size() == 1;
}

bool FilesCoordinator::singleContentSelected() const
{
    return (m_slctdFolds.size() + m_slctdFiles.size()) == 1;
}

int FilesCoordinator::selectionCounter() const
{
    return m_slctdFolds.size() + m_slctdFiles.size();
}

void FilesCoordinator::resetSearchIds(QString keyword)
{
    revalidateSearchIds(keyword);

    qDebug() << "in FilesCoordinator::resetSearchIds: keyword: " << keyword
             << "   m_searchResults.size: " << m_searchResults.size();

    if(m_searchResults.size() > 0){
        m_searchIndex = 0;

//        emit repaintFolderViewer();
        focusSearchId();
    }else{
        clearSearchResults();

        QTimer::singleShot(0, [=](){
            QMessageBox msgBox;
            msgBox.setText(QString("no matches found for '%1'").arg(keyword));
            msgBox.exec();
        });
    }
}
void FilesCoordinator::revalidateSearchIds(QString keyword){
    if( !keyword.isEmpty() && !(m_curSearchKeyWord == keyword))
        m_curSearchKeyWord = keyword;

    m_searchResults.clear();
    m_searchIds.clear();

    for(int i=0; i < m_oneDimnFolderOrder.size();i++){
        QString fileOrFoldername = m_oneDimnFolderOrder[i].getFileName();
        if( !fileOrFoldername.isEmpty() && fileOrFoldername.toLower().contains(keyword) ){
            const FilInfoForOneDim& fiOneDim =  m_oneDimnFolderOrder.at(i);
            m_searchResults.push_back( fiOneDim );
        }
    }


    for(int i=0; i < m_searchResults.size(); i++){
        int id = m_oneDimnFolderOrderDisplayed.indexOf( m_searchResults[i] );
        if( id > -1 ){
            m_searchIds.append( id );
        }else{
            if(auto lockedRef = m_searchResults[i].m_fileInfoBD.lock()){
                std::weak_ptr<FileInfoBD> dispParentFold = lockedRef->getFirstDispParent();
                if(auto lockedParentFold = dispParentFold.lock()){
                    for(int j=0; j < m_oneDimnFolderOrderDisplayed.size(); j++){
                        if(m_oneDimnFolderOrderDisplayed[j].m_isFolder){
                            if(auto lockedVgl = m_oneDimnFolderOrderDisplayed[j].m_fileInfoBD.lock()){
                                if(lockedVgl == lockedParentFold){
                                    m_searchIds.append( j );
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void FilesCoordinator::searchForKeyWord(QString keyword, bool deepSearch)
{
    clearSearchResults();

    keyword = keyword.toLower();

    if(deepSearch)
    {
        blockRevalidating(true);
        startWaitingAnimation();

        int searchWordsLimit = 100;

        QThread* thread = new QThread;
        DeppSearchWorker* worker = new DeppSearchWorker(m_root, keyword, searchWordsLimit, m_includeHiddenFiles);
        worker->moveToThread(thread);

        connect(thread, &QThread::started,       worker, &DeppSearchWorker::process);
        connect(worker, &DeppSearchWorker::finishedSearch, thread, &QThread::quit);
        connect(worker, &DeppSearchWorker::finishedSearch, worker, &DeppSearchWorker::deleteLater);
        connect(worker, &DeppSearchWorker::canceled, thread, &QThread::quit);
        connect(worker, &DeppSearchWorker::canceled, worker, &DeppSearchWorker::deleteLater);
        connect(thread, &QThread::finished,      thread, &QThread::deleteLater);
        connect(this, &FilesCoordinator::killCurrentBlockingAction, worker, &DeppSearchWorker::cancel,
                Qt::DirectConnection);
        connect(worker, &DeppSearchWorker::canceled, this, [=](){
//                qDebug() << "\n\nin filescoordinator - DeepSearchWorker -> cancelled received";
                m_root->disableSignals(false);
                blockRevalidating(false);
                killWaitingAnimation();
                revalidateOneDimnFolderOrder();
                resetSearchIds(keyword);
            }
            , Qt::QueuedConnection
        );
        connect(worker, &DeppSearchWorker::finishedSearch,
                this, [=](bool reachedLimit){
//                    qDebug() << "\n\nin filescoordinator - DeepSearchWorker -> finishedSearch received: " << reachedLimit;
                    m_root->disableSignals(false);
                    blockRevalidating(false);
                    killWaitingAnimation();
                    revalidateOneDimnFolderOrder();
                    resetSearchIds(keyword);

                    if(reachedLimit)
                    {
                        QString infoMsg = QString("more than %1 matches found\n"
                                               "-> to prevent the program from crashing,"
                                                  " only the first %1 matches were loaded!")
                                       .arg(searchWordsLimit);
                        StaticFunctions::showInfoDialog(infoMsg, tr("too many matches"));
                    }
                }
                , Qt::QueuedConnection
        );

        thread->start();
    }else
    {
        resetSearchIds(keyword);
    }
}

void FilesCoordinator::blockRevalidating(bool block)
{
    m_blockRevalidating = block;

    emit blockToolBar(block);
}
bool FilesCoordinator::revalidatingIsBlocked()
{
    return m_blockRevalidating;
}

//void FilesCoordinator::searchForBeginsWith(QString startWith) const
//{
//    Q_UNUSED(startWith)
//}

bool FilesCoordinator::searchResultsEmpty() const
{
    return m_searchResults.size() == 0;
}

int FilesCoordinator::searchResultsFound() const
{
    return m_searchResults.size();
}

long FilesCoordinator::getSearchIndex() const
{
    return m_searchIndex;
}

void FilesCoordinator::nextSearchResult()
{
    if(m_searchResults.size() > 0){
        ++m_searchIndex;
        if(m_searchIndex >= m_searchResults.size()){
            m_searchIndex = 0;
        }
        if(m_searchIndex < m_searchIds.size()){
            focusSearchId();
        }
    }
}

void FilesCoordinator::previousSearchResult()
{
    if(m_searchResults.size() > 0){
        --m_searchIndex;
        if(m_searchIndex < 0){
            m_searchIndex = m_searchResults.size()-1;
        }
        if(m_searchIndex < m_searchIds.size()){
            focusSearchId();
        }
    }
}

void FilesCoordinator::focusSearchId()
{
    int id = m_searchIds[m_searchIndex];
    if(id > -1)
        focusFolderViewer(id, true);
}

long FilesCoordinator::getIndexOfCurrentSearchResult() const
{
    if(m_searchIds.size() > 0 && m_searchIndex > -1 &&
            m_searchIds.size() > m_searchIndex){
        return m_searchIds[m_searchIndex];
    }
    return -1;
}

long FilesCoordinator::getSearchResultsCount() const
{
    return m_searchIds.size();
}

bool FilesCoordinator::entryIsHidden(const FilInfoForOneDim& entry) const
{
    return !(m_oneDimnFolderOrderDisplayed.contains( entry ));
}
QString FilesCoordinator::getCurSearchResultStr()
{
    if(m_searchResults.size() > 0 && m_searchIndex > -1){
        bool isHidden = entryIsHidden(m_searchResults[m_searchIndex]);
        return (m_searchResults[m_searchIndex].getFileName()) +
               (isHidden ? QObject::tr(" (hidden)") : QObject::tr(""));
    }
    return QString("");
}

const FilInfoForOneDim& FilesCoordinator::getCurSearchResult() const
{
    if(m_searchResults.size() > 0 && m_searchIndex > -1){
        return m_searchResults.at(m_searchIndex);
    }
    std::stringstream ss;
    ss << "Exception: FilesCoordinator::getCurSearchResult(: m_searchIndex: " << m_searchIndex << " m_searchResults.size: " << m_searchResults.size();
    throw ss.str();
}

bool FilesCoordinator::isCurentSearchResult(const FilInfoForOneDim& fiForOneDim) const
{
    if(m_searchResults.size() > 0 &&
            m_searchIndex > -1 &&
            m_searchIds.size() > m_searchIndex &&
            m_oneDimnFolderOrderDisplayed.size() > m_searchIds[m_searchIndex]){

        return m_oneDimnFolderOrderDisplayed[m_searchIds[m_searchIndex]] == fiForOneDim;
    }
    return false;
}
bool FilesCoordinator::isCurentSearchResult(const QString& path) const
{
    if(m_searchResults.size() > 0 &&
            m_searchIndex > -1 &&
            m_searchIds.size() > m_searchIndex &&
            m_oneDimnFolderOrderDisplayed.size() > m_searchIds[m_searchIndex]){

        return m_oneDimnFolderOrderDisplayed[m_searchIds[m_searchIndex]].getAbsoluteFilePath() == path;
    }
    return false;
}

void FilesCoordinator::clearSearchResults()
{
    m_searchResults.clear();
    m_searchIds.clear();
    m_searchIndex = -1;
}

void FilesCoordinator::keyPressed(const char c)
{
    auto currentTime = getCurrentTime();
    auto timePassed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_lastKeyPressed).count();
    m_lastKeyPressed = currentTime;

    if(timePassed > 300)
    {
        m_keysPressed.clear();
    }
    m_keysPressed.append(c);

    focusEntryForKeyPressed();
}

void FilesCoordinator::setSelected(const QString &fi, bool isFolder)
{
    if(isFolder)
        m_slctdFolds.insert(fi);
    else
        m_slctdFiles.insert(fi);
}

void FilesCoordinator::focusEntryForKeyPressed()
{
    int focusId = -1;
    const QString keysPressed = m_keysPressed.toLower();
    for(unsigned int i=0; i < m_oneDimnFolderOrderDisplayed.size(); ++i)
    {
        if(m_oneDimnFolderOrderDisplayed[i].getFileName().toLower().contains(keysPressed))
        {
            clearSelection();
            setSelected(m_oneDimnFolderOrderDisplayed[i].getAbsoluteFilePath(), m_oneDimnFolderOrderDisplayed[i].m_isFolder);
            focusId = i;
            break;
        }
    }
    if(focusId > -1)
    {
        emit focusIdFolderViewerSGNL(focusId, true);
    }
}

int FilesCoordinator::getMaxDepth()
{
    return m_maxDepth;
}

void FilesCoordinator::elapseAllFoldersOfDepthId(int depthId)
{
    m_depthIdsElapsed[depthId] = !m_depthIdsElapsed[depthId];
    QList<FilInfoForOneDim> foldersToElapse;
    for(int i=0; i < m_oneDimnFolderOrder.size(); i++){
        if(m_oneDimnFolderOrder[i].m_isFolder &&
                depthId == m_oneDimnFolderOrder[i].m_depth){
            foldersToElapse.append(m_oneDimnFolderOrder[i]);
        }
    }
    if(foldersToElapse.size() > 0 ){
        if(m_depthIdsElapsed[depthId])
            elapseFolders(foldersToElapse);
        else
            collapseFolders(foldersToElapse);
    }
}


bool FilesCoordinator::depthIdElapsed(int depthId)
{
    if(depthId < m_depthIdsElapsed.size()){
        return m_depthIdsElapsed[depthId];
    }
    return false;
}

void FilesCoordinator::setSelf(std::shared_ptr<FilesCoordinator> self)
{
    m_self = self;
}

void FilesCoordinator::resetWidgets()
{
    m_mainGrid = nullptr;
}

void FilesCoordinator::createWidgets()
{
    QDir rootDir;
    if(m_root)
        rootDir = QDir(m_root->getFileInfo().absoluteFilePath());

    GraphicsView* m_folderViewer = new GraphicsView(m_graphicsViewVBarValueBackup,
                                                    m_graphicsViewHBarValueBackup,
                                                    m_zoomFactor);
    m_mainGrid = new QGridLayout();
    DirectorySelectionPane* m_toolBar = new DirectorySelectionPane(rootDir);

    m_folderViewer->setFilesCoordinator(m_self);
    connect(m_folderViewer, &GraphicsView::requestOpenFoldersInTab, this, &FilesCoordinator::openSelectedFoldersInNewTab);
    connect(m_folderViewer, &GraphicsView::requestCloseCurrentTab,  this, &FilesCoordinator::closeCurrentTab);

    connect(m_folderViewer, &GraphicsView::nextSearchResultSGNL, this, &FilesCoordinator::nextSearchResult, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::prevSearchResultSGNL, this, &FilesCoordinator::previousSearchResult, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::closeSearchMenuSGNL,  this, &FilesCoordinator::exitSearchMode, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::searchForKeyWord,     this, &FilesCoordinator::searchForKeyWord, Qt::QueuedConnection);

    connect(m_folderViewer, &GraphicsView::selectEntireContent,  this, &FilesCoordinator::selectEntireContent, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::clearSelection,  this, &FilesCoordinator::clearSelection, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::selectButtonUp,  this, &FilesCoordinator::selectButtonUp, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::selectButtonDown,  this, &FilesCoordinator::selectButtonDown, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::copySelectedContent,  this, &FilesCoordinator::copySelectedContent, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::cutSelectedContent,  this, &FilesCoordinator::cutSelectedContent, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::duplicateSelectedContent,  this, &FilesCoordinator::duplicateSelectedContent, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::openSelectedContent,  this, &FilesCoordinator::openSelectedContent, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::openSelectedContentWith,  this, &FilesCoordinator::openSelectedContentWith, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::renameSelectedContent,  this, &FilesCoordinator::renameSelectedContent, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::pasteFromClipboard,  this, &FilesCoordinator::pasteFromClipboard, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::paste,  this, &FilesCoordinator::paste, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::deleteSelectedContent,  this, &FilesCoordinator::deleteSelectedContent, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::zipSelectedContent,  this, &FilesCoordinator::zipSelectedContent, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::unzipSelectedContent,  this, &FilesCoordinator::unzipSelectedContent, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::createNewFolderSGNL,  this, &FilesCoordinator::createNewFolder, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::createNewFileSGNL,  this, &FilesCoordinator::createNewFile, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::elapseSelectedFoldersRecursively,  this, &FilesCoordinator::elapseSelectedFoldersRecursively, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::elapseSelectedFolders,  this, &FilesCoordinator::elapseSelectedFolders, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::collapseSelectedFoldersRecursively,  this, &FilesCoordinator::collapseSelectedFoldersRecursively, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::collapseSelectedFolders,  this, &FilesCoordinator::collapseSelectedFolders, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::copySelectedFilePathToClipboard,  this, &FilesCoordinator::copySelectedFilePathToClipboard, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::killCurrentBlockingAction,  this, &FilesCoordinator::killCurrentBlockingActionSLT, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::requestFocusSGNL,  this, &FilesCoordinator::requestFocus, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::initDragging,  this, &FilesCoordinator::initDragging, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::keyPressed,  this, &FilesCoordinator::keyPressed, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::setSelectionToRoot,  this, &FilesCoordinator::setSelectionToRoot, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::setRootFolder,  this, &FilesCoordinator::setRootFolder, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::sortAllFolders,  this, &FilesCoordinator::sortAllFolders, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::openTerminalSGNL,  this, &FilesCoordinator::openTerminal, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::openSelection,  this, &FilesCoordinator::openSelection, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::setParentToRoot,  this, &FilesCoordinator::setParentToRoot, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::selectContent,  this, &FilesCoordinator::selectContent, Qt::QueuedConnection);

    connect(m_folderViewer, &GraphicsView::sortFromDisplayedContent,  this, &FilesCoordinator::sortFromDisplayedContent, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::zoomFactorChanged,  this, &FilesCoordinator::setZoomFactor, Qt::QueuedConnection);
    connect(m_folderViewer, &GraphicsView::showHiddenFilesSGNL,  this, &FilesCoordinator::setIncludeHiddenFiles, Qt::QueuedConnection);

    connect(m_folderViewer, &GraphicsView::elapseAllFoldersOfDepthId,  this, &FilesCoordinator::elapseAllFoldersOfDepthId, Qt::QueuedConnection);

    connect(m_folderViewer->verticalScrollBar(),   &QScrollBar::valueChanged,  this, &FilesCoordinator::saveGraphicsViewVBarValue);
    connect(m_folderViewer->horizontalScrollBar(), &QScrollBar::valueChanged,  this, &FilesCoordinator::saveGraphicsViewHBarValue);

    connect(this, &FilesCoordinator::rootChanged,                 m_folderViewer, &GraphicsView::revalidate);
    connect(this, &FilesCoordinator::repaintFolderViewerSGNL,     m_folderViewer, &GraphicsView::revalidate);
    connect(this, &FilesCoordinator::focusFolderViewerSGNL,       m_folderViewer, &GraphicsView::requestFocus);
    connect(this, &FilesCoordinator::startWaitingAnimationSGNL,   m_folderViewer, &GraphicsView::startWaitingAnimation);
    connect(this, &FilesCoordinator::killWaitingAnimationSGNL,    m_folderViewer, &GraphicsView::killWaitingAnimation);
    connect(this, &FilesCoordinator::focusIdFolderViewerSGNL,     m_folderViewer, &GraphicsView::focusId);

    m_toolBar->undoEnabled( [=](){return m_dirStack.size() != 1;});
    connect(this, &FilesCoordinator::rootChanged,    m_toolBar, &DirectorySelectionPane::setFolder, Qt::QueuedConnection);
    connect(this, &FilesCoordinator::rootChanged,    m_toolBar, &DirectorySelectionPane::revalidate, Qt::QueuedConnection);
    connect(this, &FilesCoordinator::blockToolBar,   m_toolBar, &DirectorySelectionPane::blockButtons);
    connect(m_toolBar, &DirectorySelectionPane::undo,           this, &FilesCoordinator::setLastPathToRoot, Qt::QueuedConnection);
    connect(m_toolBar, &DirectorySelectionPane::buttonClicked,  this, &FilesCoordinator::setRootFolder, Qt::QueuedConnection);

    m_mainGrid->addWidget(m_toolBar, 0,0);

    m_mainGrid->addWidget(m_folderViewer, 1, 0);
    m_mainGrid->setContentsMargins(0, 4, 0, 4);
    m_mainGrid->setSpacing(0);
}

void FilesCoordinator::saveGraphicsViewVBarValue(int value)
{
    m_graphicsViewVBarValueBackup = value;
}

void FilesCoordinator::saveGraphicsViewHBarValue(int value)
{
    m_graphicsViewHBarValueBackup = value;
}

void FilesCoordinator::exitSearchMode()
{
    clearSearchResults();

    emit repaintFolderViewer();
}

void FilesCoordinator::forceRevalidation()
{
    revalidateOneDimnFolderOrder();
    repaintFolderViewer();
}

bool FilesCoordinator::selectionContainsZippedFile()
{
    foreach(const auto& fi, m_slctdFiles){
        if( StaticFunctions::isZippedFile(QFileInfo(fi)) )
            return true;
    }
    foreach(const auto& fi, m_slctdFolds){
        if( StaticFunctions::isZippedFile(QFileInfo(fi)) )
            return true;
    }
    return false;
}

QString FilesCoordinator::getCurRootPath()
{
    if(m_root)
        return m_root->getFileInfo().absoluteFilePath();
    else
        return QString("");
}

bool FilesCoordinator::inSearchMode()
{
    return m_searchIndex > -1;
}

void FilesCoordinator::printSelectedFolders()
{
    qDebug() << "   folders: size: " << m_slctdFolds.size();
    int cntr = 0;
    for(auto it = m_slctdFolds.cbegin(); it != m_slctdFolds.cend(); ++it)
    {
        qDebug() << "        " << "[" << cntr++ << "]: " << QFileInfo(*it).fileName();
    }
}

void FilesCoordinator::printSelectedFiles()
{
    qDebug() << "   files: size: " << m_slctdFiles.size();
    int cntr = 0;
    for(auto it = m_slctdFiles.cbegin(); it != m_slctdFiles.cend(); ++it)
    {
        qDebug() << "        " << "[" << cntr++ << "]: " << QFileInfo(*it).fileName();
    }
}


void FilesCoordinator::repaintFolderViewer()
{
    if( !revalidatingIsBlocked() )
        emit repaintFolderViewerSGNL();
}
void FilesCoordinator::focusFolderViewer(int id, bool repaintAnyway){
    if( !revalidatingIsBlocked() )
        emit focusIdFolderViewerSGNL(id, repaintAnyway);
}

void FilesCoordinator::startWaitingAnimation()
{
    emit startWaitingAnimationSGNL();
}
void FilesCoordinator::killWaitingAnimation()
{
    emit killWaitingAnimationSGNL();
}


void FilesCoordinator::killCurrentBlockingActionSLT()
{
    emit killCurrentBlockingAction();
}


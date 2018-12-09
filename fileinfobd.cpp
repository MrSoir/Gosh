#include "fileinfobd.h"

FileInfoBD::FileInfoBD(const QString &path,
                       std::weak_ptr<FileInfoBD> parentFiBD,
                       QObject *parent)
    : QObject(parent),
      m_order(ORDER_BY::NAME),
      fileInfo(QFileInfo(path)),
      m_parent(parentFiBD),
      m_self(std::weak_ptr<FileInfoBD>())
{
}

FileInfoBD::FileInfoBD(const QFileInfo &fileInfo,
                       std::weak_ptr<FileInfoBD> parentFiBD,
                       QObject *parent)
    : QObject(parent),
      m_order(ORDER_BY::NAME),
      fileInfo(fileInfo),
      m_parent(parentFiBD),
      m_self(std::weak_ptr<FileInfoBD>())
{
}

FileInfoBD::~FileInfoBD()
{
//    qDebug() << "in FileinfoBD-DEstructor: " << fileInfo.absoluteFilePath();

    disableSignals(true);

    m_includeHiddenFilesCaller = nullptr;

//    removeFromParent();

    for(int i=0; i < sub_folds.size(); ++i)
    {
        sub_folds[i].reset();
    }
    sub_folds.clear();

    for(int i=0; i < listeners.size(); ++i)
    {
        if(auto locked = listeners[i].lock())
            locked->removeDirectoryFromWatcher(fileInfo.absoluteFilePath());
        listeners[i].reset();
    }
    listeners.clear();

    sub_fold_names.clear();
    m_parent.reset();
    m_self.reset();
    files.clear();
}

void FileInfoBD::setElapsed()
{
    isElapsed = !isElapsed;
//    qDebug() << "setting elspse to: " << isElapsed << " of file: " << fileInfo.fileName();
    doElapsing();
}

void FileInfoBD::setElapsed(bool elpsd)
{
    isElapsed = elpsd;
    doElapsing();
}

void FileInfoBD::setElapsedFlag(bool elpsd)
{
    isElapsed = elpsd;
}

void FileInfoBD::setLoadedFlag(bool loaded)
{
    alrLoaded = loaded;
}

void FileInfoBD::elapseAll()
{
    std::shared_ptr<ElapseValidator> dummy_ptr(new ElapseValidator());
    elapseAll( std::static_pointer_cast<AbstrElapseValidator>(dummy_ptr) );
    dummy_ptr.reset();
}
void FileInfoBD::elapseAll(std::shared_ptr<AbstrElapseValidator> validator)//  std::shared_ptr<bool> cancelled)
{
    if( validator && validator->aborted() )
    {
//        qDebug() << "in fileInfoBD.elapseAll -> limitReached: use_count: " << validator.use_count() << "    " << fileInfo.absoluteFilePath();
        return;
    }
//    qDebug() << "in fileInfoBD.elapseAll ->               use_count: " << validator.use_count() << "    " << fileInfo.absoluteFilePath();

    this->setElapsed(true);
    validator->addElements(this->m_filesCount + m_subFoldsCount);

    for(int i=0; i < sub_folds.size(); i++)
    {
        if( validator && validator->aborted() )
        {
//            qDebug() << "in fileInfoBD -> cancelled II: " <<     fileInfo.absoluteFilePath();
            return;
        }
        sub_folds[i]->elapseAll(validator);
    }
}

void FileInfoBD::collapseAll()
{
    setElapsed(false);
    foreach(auto subFold, sub_folds){
        if(subFold){
             subFold->collapseAll();
        }
    }
}

void FileInfoBD::collapse()
{
    setElapsed(false);
}

bool FileInfoBD::elapsed() const
{
    return this->isElapsed;
}

bool FileInfoBD::isLoaded() const
{
    return alrLoaded;
}

std::function<bool(const QFileInfo&, const QFileInfo&)> FileInfoBD::getSortFunction(){
    std::function<bool (const QFileInfo&,const QFileInfo&)> sortFunc;
    if(       m_order == ORDER_BY::NAME){
        sortFunc = [](const auto& a, const auto& b){return a.fileName().toLower() < b.fileName().toLower();};
    }else if(       m_order == ORDER_BY::R_NAME){
        sortFunc = [](const auto& a, const auto& b){return a.fileName().toLower() > b.fileName().toLower();};
    }else if (m_order == ORDER_BY::MOD_DATE){
        sortFunc = [](const auto& a, const auto& b){return a.lastModified() < b.lastModified();};
    }else if (m_order == ORDER_BY::R_MOD_DATE){
        sortFunc = [&](const auto& a, const auto& b){return (a.lastModified() > b.lastModified());};
    }else if (m_order == ORDER_BY::SIZE){
        sortFunc = [](const auto& a, const auto& b){return a.size() < b.size();};
    }else if (m_order == ORDER_BY::R_SIZE){
        sortFunc = [](const auto& a, const auto& b){return a.size() > b.size();};
    }else if (m_order == ORDER_BY::TYPE){
        sortFunc = [](const auto& a, const auto& b){
            QString aFileName = a.fileName().toLower();
            QString bFileName = b.fileName().toLower();

            QString aType = StaticFunctions::getFileType(aFileName);
            QString bType = StaticFunctions::getFileType(bFileName);

            if(aType == bType){
                return aFileName < bFileName;
            }
            return aType < bType;
         };
    }else{
       sortFunc = [](const auto& a, const auto& b){
           QString aFileName = a.fileName().toLower();
           QString bFileName = b.fileName().toLower();

           QString aType = StaticFunctions::getFileType(aFileName);
           QString bType = StaticFunctions::getFileType(bFileName);

           if(aType == bType){
               return aFileName < bFileName;
           }
           return aType > bType;
       };
    }
    return sortFunc;
}

void FileInfoBD::sortBy(ORDER_BY ord, bool ifNewOrderEqualsOldOrderDoReverse, bool notificateListener)
{
    bool doSorting = false;
    if(ord != m_order){
        doSorting = true;
        this->m_order = ord;
    }else if(ifNewOrderEqualsOldOrderDoReverse){
        doSorting = true;
        this->m_order = StaticFunctions::getOppositeOfOrder(m_order);
    }
    if(doSorting){
        this->sort( getSortFunction() );
        if(notificateListener && !m_disableSignals){
            emit sortingHasChanged( m_self );
        }
    }
}

bool FileInfoBD::isEmpty()
{
    return getDispElmntCount() == 0;
}


int FileInfoBD::getDispElmntCount()
{
    return m_filesCount + m_subFoldsCount;
}
int FileInfoBD::getDispElmntCountRecurs(){
    int cntr = 1;
    if(isElapsed){
        for(int i=0; i < sub_folds.size(); i++){
            cntr += sub_folds.at(i)->getDispElmntCountRecurs();
        }
        cntr += m_filesCount;
    }
    return cntr;
}



const QVector<std::weak_ptr<FileInfoBD>> FileInfoBD::getSubFolders(int startId, int endId) const
{
    QVector<std::weak_ptr<FileInfoBD>> cpyVct;

    if(startId == -1)
        startId = 0;
    if(endId == -1 || endId >= sub_folds.size())
        endId = sub_folds.size()-1;

    for(int i=0; i <= endId; i++){
        cpyVct.append(sub_folds[i]);
    }
    return cpyVct;
}

QDir::SortFlags FileInfoBD::getSortFlags() const{
    QDir::SortFlags sortFlags;
    if(m_order == ORDER_BY::MOD_DATE){
        sortFlags = QDir::Time;
    }else if (m_order == ORDER_BY::R_MOD_DATE){
        sortFlags = QDir::Time | QDir::Reversed;
    }else if (m_order == ORDER_BY::NAME){
        sortFlags = QDir::Name;
    }else if (m_order == ORDER_BY::R_NAME){
        sortFlags = QDir::Name | QDir::Reversed;
    }else if (m_order == ORDER_BY::TYPE){
        sortFlags = QDir::Type;
    }else if (m_order == ORDER_BY::R_TYPE){
        sortFlags = QDir::Type | QDir::Reversed;
    }else if (m_order == ORDER_BY::SIZE){
        sortFlags = QDir::Size;
    }else{// if (ord == ORDER_BY::R_SIZE){
        sortFlags = QDir::Size | QDir::Reversed;
    }
    return sortFlags;
}

const QVector<QFileInfo> FileInfoBD::loadTooBigFiles(int startId, int endId) const{
    QVector<QFileInfo> cpyVct;

    QDir dir(fileInfo.absoluteFilePath());

    QDir::SortFlags sortFlags = this->getSortFlags();

    QFileInfoList filesLst = StaticFunctions::getFilesInDirectorySorted(dir, sortFlags, includeHiddenFiles());//dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, sortFlags);

    if(startId == -1)
        startId = 0;
    if(endId == -1 || endId >= filesLst.size())
        endId = filesLst.size()-1;
    for(int i=startId; i <= endId; i++){
        cpyVct.append(filesLst[i]);
    }

    // bereits sortiert beim laden, daher sortieren irrelevant
//    auto sortFunc = getSortFunction();
//    std::sort(cpyVct.begin(), cpyVct.end(),
//          [&](const QFileInfo& a, const QFileInfo& b){return sortFunc(a, b);}
//    );

    return cpyVct;
}

int FileInfoBD::getFileCount()
{
    return m_filesCount;
}

int FileInfoBD::getFolderCount()
{
    return m_subFoldsCount;
}

void FileInfoBD::disableSignals(bool disableSignals)
{
    m_disableSignals = disableSignals;
    for(int i=0; i < sub_folds.size(); i++){
        sub_folds[i]->disableSignals(disableSignals);
    }
}

const QFileInfo FileInfoBD::getFileAt(int id) const
{
    if(m_tooBigToSave){
        const QVector<QFileInfo> tempFiles = getFiles();
        if(id >= 0 && id < tempFiles.size()){
            QFileInfo fileInfo = tempFiles.at(id);
            return fileInfo;
        }
    }else{
        if(id >= 0 && id < files.size()){
            return files.at(id);
        }
    }
    qDebug() << "ine getFileAt -> could not find file -> MUST NOT HAPPEN!!";
    return QFileInfo();
}

int FileInfoBD::getFileId(const QString fileName) const
{
    const QVector<QFileInfo> files = getFiles();
    for(int i=0; i < files.size(); i++){
        if(files[i].fileName() == fileName)
            return i;
    }
    return -1;
}
const QVector<QFileInfo> FileInfoBD::getFiles(int startId, int endId) const
{
    // differenzieren: wenn files > 300 sind sie zu groß um direkt geladen zu werden. dann werden sie immer nur
    // auf anfrage geladen und auch nur so viele, wie gerade notwendig.
    if(m_tooBigToSave){
        return loadTooBigFiles(startId, endId);
    }else{
        if(startId == -1 && endId == -1){
            return files;
        }else{
            if (startId > files.size() && files.size() > 0){
                startId = files.size()-1;
            }else if(startId == -1)
                startId = 0;
            if(endId == -1 || endId >= files.size())
                endId = files.size()-1;
            return files.mid(startId, endId-startId+1);
        }
    }
}

int FileInfoBD::fileCount() const
{
    return m_filesCount;
}

const QFileInfo &FileInfoBD::getFileInfo() const
{
    return fileInfo;
}

bool FileInfoBD::isEmpty() const
{
    return (m_subFoldsCount+m_filesCount) == 0;
}

void FileInfoBD::setRoot(const QString &path)
{
    close();
    fileInfo = QFileInfo(path);
}
void FileInfoBD::close()
{
    try{
//        qDebug() << "in close";

//        if(conn)
//            disconnect(*conn);

//        closeWatcher();

        listeners.clear();
        for(int i=0; i < sub_folds.size(); i++){
            sub_folds[i]->close();
            sub_folds[i].reset();
        }
        sub_folds.clear();
    }catch (...) {
      qDebug() << "Exception occurred in FileInfoBD::close()";
    }
//    qDebug() << "leaving close";
}

void FileInfoBD::directoryChanged(const QString &path)
{
    Q_UNUSED(path);
//    qDebug() << "in directoryChanged: " << path
//             << "   m_disableSignals: " << m_disableSignals;

    revalFolderContent();

    if( !m_disableSignals ){
        emit folderContentHasChanged( m_self );

    }
//    qDebug() << "in direcotryChanged: " << this->fileName()
//             << "   elapsed: " << this->isElapsed
//             << "   folders: " << this->sub_folds.size()
//             << "   files: " << this->files.size();
}

void FileInfoBD::fileChanged(const QString &path)
{
    Q_UNUSED(path)
}

void FileInfoBD::sort(std::function<bool (const QFileInfo&, const QFileInfo&)> sortFunc)
{
    if(files.size() > 0){
        std::sort(files.begin(), files.end(),
              [&](const QFileInfo& a, const QFileInfo& b){return sortFunc(a, b);}
        );
    }
}

void FileInfoBD::sort()
{
    sortBy(m_order);
}

void FileInfoBD::print(int id)
{
    QString str("");
    for(int i=0; i < id; i++){
        str.append("\t");
    }
    qDebug() << str + fileInfo.fileName();
    str.append("\t");
    for(int i=0; i < files.size(); i++){
        qDebug() << str + files.at(i).fileName();
    }
    for(int i=0; i < sub_folds.size(); i++){
        sub_folds[i]->print(id+1);
    }
}

const QString FileInfoBD::fileName() const
{
    return fileInfo.fileName();
}

void FileInfoBD::addListener(std::weak_ptr<FolderListener> lstnr)
{
    if( !StaticFunctions::contains<FolderListener>(listeners, lstnr) ){
        listeners.append(lstnr);
        connect(this, &FileInfoBD::folderContentHasChanged,
                [=](){
            if(auto lockedLstnr = lstnr.lock()){
                    lockedLstnr->folderChanged(m_self);
        }});
        connect(this, &FileInfoBD::fileInfoBdElapsed,
                [=](){if(auto lockedLstnr = lstnr.lock()){
                        lockedLstnr->folderElapsed(m_self);
        }});
        connect(this, &FileInfoBD::sortingHasChanged,
                [=](){if(auto lockedLstnr = lstnr.lock()){
                        lockedLstnr->sortingChanged(m_self);
        }});

        for(int i=0; i < sub_folds.size(); i++){
            sub_folds[i]->addListener(lstnr);
        }
    }
}

void FileInfoBD::removeListener(std::weak_ptr<FolderListener> lstnr)
{
    if(StaticFunctions::contains<FolderListener>(listeners, lstnr)){
//    if(lstnr && this->listeners.contains(lstnr)){
        StaticFunctions::removeOne<FolderListener>(listeners, lstnr);
//        listeners.removeOne(lstnr);
        for(int i=0; i < sub_folds.size(); i++){
            sub_folds[i]->removeListener(lstnr);
        }
    }
}

void FileInfoBD::traverse(std::function<void(std::weak_ptr<FileInfoBD>, int)> folderFunc,
                          std::function<void(std::weak_ptr<FileInfoBD>, int)> fileFunc,
                          bool ignoreUnElapsedFolders,
                          int depth)
{
    if(folderFunc)
        folderFunc( m_self, depth );

    if( isElapsed ||
        (isLoaded() && !ignoreUnElapsedFolders) )
    {
        foreach(auto subFold, sub_folds){
            subFold->traverse(folderFunc, fileFunc, ignoreUnElapsedFolders, depth+1);
        }

        if(fileFunc)
            fileFunc( m_self, depth+1 );
    }
}
// traverseOverSubFolders funktioniert etwas anders als traverse:
// hier gibt es zusaetzlich bool als rueckgabewert: sowohl die traverseOverSubFolders-Funktion selbst als auch die
// folderFunc: wenn eine der beiden true zurueckgibt, wird der druchlauf ueber die subfolders sofort abgebrochen und
// nicht weiter ueber die subfolders traversiert -> grund: wenn die folderFunc das gefunden hat, was sie braucht,
// darf der durchlauf beendet werden.
bool FileInfoBD::traverseOverSubFolders(std::function<bool(std::shared_ptr<FileInfoBD>, int)> folderFunc,
                                        bool ignoreUnElapsedFolders,
                                        int depth)
{
    if(!folderFunc)
        return true;

    if( isElapsed ||
        (isLoaded() && !ignoreUnElapsedFolders) )
    {
        foreach(auto subFold, sub_folds){
            if(folderFunc(subFold, depth+1))
                return true;

            if(subFold->traverseOverSubFolders(folderFunc,ignoreUnElapsedFolders, depth+1))
                return true;
        }
    }
    return false;
}

ORDER_BY FileInfoBD::getSortinOrder() const
{
    return m_order;
}

bool FileInfoBD::isReversedSorted() const
{
    return m_order == ORDER_BY::R_NAME ||
           m_order == ORDER_BY::R_TYPE ||
           m_order == ORDER_BY::R_SIZE ||
            m_order == ORDER_BY::R_MOD_DATE;
}

bool FileInfoBD::isReversedSortedBy(ORDER_BY ord) const
{
    if(ord == ORDER_BY::NAME){
        return m_order == ORDER_BY::R_NAME;
    }else if(ord == ORDER_BY::SIZE){
        return m_order == ORDER_BY::R_SIZE;
    }else if(ord == ORDER_BY::MOD_DATE){
        return m_order == ORDER_BY::R_MOD_DATE;
    }else if(ord == ORDER_BY::TYPE){
        return m_order == ORDER_BY::R_TYPE;
    }
    return false;
}

bool FileInfoBD::isSortedBy(ORDER_BY ord) const
{
    return m_order == ord;
}

void FileInfoBD::sortByRecursivelyWithoutNotification(ORDER_BY ord)
{
    sortBy(ord, false, false);
    for(int i=0; i < sub_folds.size(); i++){
        sub_folds[i]->sortByRecursivelyWithoutNotification(ord);
    }
}


void FileInfoBD::replaceSubFolders(FileInfoBD* containsSubfoldersToReplace)
{
    for(int i=0; i < containsSubfoldersToReplace->sub_folds.size(); ++i)
    {
        replaceSubFolder( containsSubfoldersToReplace->sub_folds[i] );
    }
}
void FileInfoBD::replaceSubFolder(FileInfoBD* subFoldToReplace)
{
    replaceSubFolder( std::shared_ptr<FileInfoBD>(subFoldToReplace) );
}
void FileInfoBD::replaceSubFolder(std::shared_ptr<FileInfoBD> subFoldToReplace)
{
//    qDebug() << "in replacing subfolder: parent: " << fileInfo.absoluteFilePath()
//             << "   subFold: " << subFoldToReplace->fileInfo.fileName()
//             << "   subFolds.size: " << sub_folds.size();

    if( !subFoldToReplace )
        return;

    std::shared_ptr<FileInfoBD> replacedSubFold = std::shared_ptr<FileInfoBD>(nullptr);
    int replacementId = -1;

    bool replacementFound = false;
    for(int i=0; i < sub_folds.size(); i++){
        if(sub_folds[i]){
            if(sub_folds[i]->getFileInfo().absoluteFilePath() == subFoldToReplace->getFileInfo().absoluteFilePath()){
//                qDebug() << "subFolderToReplaceDetected!";

                replacementFound = true;

                sub_folds[i] = std::shared_ptr<FileInfoBD>(subFoldToReplace);

                replacementId = i;

                break;
            }
        }
    }
    if(!replacementFound){
        replacementFound = true;

        auto sharedPntr = std::shared_ptr<FileInfoBD>(subFoldToReplace);

        sub_folds.append(sharedPntr);
        sub_fold_names.append( subFoldToReplace->getFileInfo().fileName() );

        int i = sub_folds.size()-1;
        replacementId = i;
    }
    if(replacementId > -1 && replacementId < sub_folds.size())
    {
        int i = replacementId;
        sub_folds[i]->disableSignals(true);

        sub_folds[i]->m_self = sub_folds[i];
        sub_folds[i]->setParentFiBD(m_self);
        sub_folds[i]->setIncludeHiddenFilesCaller(m_includeHiddenFilesCaller, false);
        foreach (auto listener, listeners) {
             sub_folds[i]->addListener(listener);
        }
         sub_folds[i]->registerFolderToWatcherRec();

         sub_folds[i]->disableSignals(false);
    }

    sortSubFolders();

    m_subFoldsCount = sub_folds.size();
    m_contentCount = sub_folds.size() + files.size();
}

void FileInfoBD::addFiles(const QFileInfoList &files)
{
    foreach (auto file, files) {
        addFileIfNotAlrExstnd(file);
    }

    this->sort(this->getSortFunction());

    m_filesCount = files.size();
    m_contentCount = sub_folds.size() + files.size();
}

void FileInfoBD::registerFolderToWatcherRec()
{
    foreach (std::weak_ptr<FolderListener> lstnr, listeners) {
        if(auto lockedLstnr = lstnr.lock()){
            lockedLstnr->addDirectoryToWatcher(fileInfo.absoluteFilePath());
        }
    }
    foreach(auto sub_fold, sub_folds){
        if(sub_fold)
            sub_fold->registerFolderToWatcherRec();
    }
}

void FileInfoBD::setParentFiBD(std::weak_ptr<FileInfoBD> parentFiBD)
{
    m_parent = parentFiBD;
}

std::shared_ptr<FileInfoBD> FileInfoBD::getParentFiBD()
{
    return m_parent.lock();
}
void FileInfoBD::removeFromParent()
{
    if(auto locked = m_parent.lock())
    {
        bool successfullyRemoved = locked->removeFolder(fileInfo);
        Q_UNUSED(successfullyRemoved);
    }
}

bool FileInfoBD::isBeingDisplayed()
{
    if(auto locked = m_parent.lock()){
        return locked->isElapsed;
    }
    return true;
}

std::weak_ptr<FileInfoBD> FileInfoBD::getFirstDispParent()
{
    if(auto locked = m_parent.lock()){
        if(isBeingDisplayed()){
            return m_self;
        }else{
            locked->getFirstDispParent();
        }
    }
    return m_self;
}

void FileInfoBD::setSelf(std::weak_ptr<FileInfoBD> self)
{
    m_self = self;
}

void FileInfoBD::setIncludeHiddenFilesCaller(std::function<bool ()> includeHiddenFilesCaller,
                                             bool revalidate)
{
    m_includeHiddenFilesCaller = includeHiddenFilesCaller;
    for(int i=0; i < sub_folds.size(); ++i)
    {
        if(sub_folds[i])
            sub_folds[i]->setIncludeHiddenFilesCaller(includeHiddenFilesCaller, revalidate);
    }
    if(isElapsed && revalidate)
        revalFolderContent();
}

void FileInfoBD::includeHiddenFilesChanged()
{
    for(int i=0; i < sub_folds.size(); ++i)
    {
        if(sub_folds[i])
            sub_folds[i]->includeHiddenFilesChanged();
    }
    if(isElapsed)
        revalFolderContent();
}


void FileInfoBD::doElapsing()
{
    if(isElapsed && !alrLoaded){
        alrLoaded = true;

        foreach (std::weak_ptr<FolderListener> lstnr, listeners) {
            if(auto lockedLstnr = lstnr.lock()){
                lockedLstnr->addDirectoryToWatcher(fileInfo.absoluteFilePath());
            }
        }

        revalFolderContent();
    }
//    directoryChanged(this->fileInfo.absoluteFilePath());
    if(!m_disableSignals)
        emit fileInfoBdElapsed(m_self);

    //    qDebug() << "fileInfoDB: elapsed -> emit folderContentHasChanged(" << fileName() << ")";
}

void FileInfoBD::revalFolderContent()
{
    QDir dir(fileInfo.absoluteFilePath());

    QFileInfoList filesLst = StaticFunctions::getFilesInDirectory(dir, includeHiddenFiles());//dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

    if(filesLst.size() > m_tooBigToSave_Limit)
    {
        m_tooBigToSave = true;

        files.clear();

        QFileInfoList foldersLst = StaticFunctions::getFoldersInDirectory(dir, includeHiddenFiles());//dir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);
//        QFileInfoList filesLst = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

        for(int i=0; i < sub_folds.size();){
            if(!foldersLst.contains(sub_folds[i]->getFileInfo())){
                sub_folds[i].reset();
                sub_folds.removeAt(i);
                sub_fold_names.removeAt(i);
            }else{
                i++;
            }
        }
        std::for_each(foldersLst.begin(), foldersLst.end(),
                      [&](auto fi){
            this->addFolderIfNotAlrExstnd(fi);
        });

        m_subFoldsCount = foldersLst.size();
        m_filesCount = filesLst.size();
    }else{
        m_tooBigToSave = false;

        QFileInfoList foldersLst = StaticFunctions::getFoldersInDirectory(dir, includeHiddenFiles());//dir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);

        for(int i=0; i < files.size();){
            if(!filesLst.contains(files[i])){
                files.removeAt(i);
            }else{
                i++;
            }
        }
        for(int i=0; i < sub_folds.size();){
            if(!foldersLst.contains(sub_folds[i]->getFileInfo())){
                sub_folds[i].reset();
                sub_folds.removeAt(i);
                sub_fold_names.removeAt(i);
            }else{
                i++;
            }
        }
        std::for_each(foldersLst.begin(), foldersLst.end(),
                      [&](auto fi){
            this->addFolderIfNotAlrExstnd(fi);
        });

        std::for_each(filesLst.begin(), filesLst.end(),
                      [&](auto fi){
            this->addFileIfNotAlrExstnd(fi);
        });
        m_subFoldsCount = sub_folds.size();
        m_filesCount = files.size();
        m_contentCount = m_subFoldsCount + m_filesCount;

        this->sort( getSortFunction() ); // hier die direkte methode, da keine notification an die listener herausgegeben werden soll (emit sortingChanged)
    }
    sortSubFolders();

//    qDebug() << "after evaluation: "
//             << "   files.size: " << files.size()
//             << "   folds.size: " << sub_folds.size()
    //             << "   getDispElmntCountRecurs: " << getDispElmntCountRecurs();
}

void FileInfoBD::sortSubFolders()
{
    qSort(sub_folds.begin(), sub_folds.end(), [=](const std::shared_ptr<FileInfoBD>& fi1,
                                                  const std::shared_ptr<FileInfoBD>& fi2){
                                                        if(fi1){
                                                            if(fi2){
                                                                return fi1->fileName().toLower() < fi2->fileName().toLower();
                                                            }else{
                                                                return false;
                                                            }
                                                        }else{
                                                            return false;
                                                        }
                                                  }
    );
    qSort(sub_fold_names.begin(), sub_fold_names.end(), [=](const QString& fi1,
                                                            const QString& fi2){
                                                                return fi1.toLower() < fi2.toLower();
                                                          }
    );
}

bool FileInfoBD::addFolderIfNotAlrExstnd(const QFileInfo& f)
{
    if(!sub_fold_names.contains(f.fileName())){
        auto fiBD = std::make_shared<FileInfoBD>(f);
        fiBD->m_self = fiBD;
        fiBD->m_parent = m_self;
        fiBD->m_includeHiddenFilesCaller = m_includeHiddenFilesCaller;
        sub_folds.append(fiBD);
        sub_fold_names.append(f.fileName());
        for(int i=0; i < listeners.size(); i++){
            fiBD->addListener(listeners[i]);
        }
        return true;
    }
    return false;
}

bool FileInfoBD::addFileIfNotAlrExstnd(const QFileInfo& f)
{
    if(!files.contains(f)){
        files.append(f);
        return true;
    }
    return false;
}

bool FileInfoBD::removeFolder(const QFileInfo& fi){
    int id = sub_fold_names.indexOf(fi.fileName());

    if( id > -1 ){
        sub_folds[id].reset();
        sub_folds.removeAt(id);
        sub_fold_names.removeAt(id);
        return true;
    }
    return false;
}
bool FileInfoBD::removeFolder(const QString& foldName){
    int id = sub_fold_names.indexOf(foldName);

    if( id > -1 ){
        sub_folds[id].reset();
        sub_folds.removeAt(id);
        sub_fold_names.removeAt(id);
        return true;
    }
    return false;
}

bool FileInfoBD::removeFile(const QFileInfo &fi)
{
    if(int id=files.indexOf(fi) > -1){
        files.removeAt(id);
        return true;
    }
    return false;
}


void FileInfoBD::printChildFolders(int tabcount)
{
    QString tabs("");
    for(int i=0; i < tabcount; ++i)
    {
        tabs.append("   ");
    }
    for(int i=0; i < sub_folds.size(); ++i)
    {
        qDebug() << tabs << "subFold[" << i << "]: " << sub_folds[i]->fileInfo.absoluteFilePath();
    }
}
void FileInfoBD::printChildFolderNames(int tabcount)
{
    QString tabs("");
    for(int i=0; i < tabcount; ++i)
    {
        tabs.append("   ");
    }
    for(int i=0; i < sub_fold_names.size(); ++i)
    {
        qDebug() << tabs << "subFold[" << i << "]: " << sub_fold_names[i];
    }
}

bool FileInfoBD::includeHiddenFiles() const
{
    if(m_includeHiddenFilesCaller)
        return m_includeHiddenFilesCaller();
    return false;
}

#include "deepsearch.h"

DeepSearch::DeepSearch(std::shared_ptr<FileInfoBD> rootToSearchFor,
                       const QString &keyword, long searchLimit,
                       bool includeHiddenFiles,
                       QObject *parent) :
    QObject(parent),
    m_rootFolder(rootToSearchFor),
    m_keyword(keyword),
    m_searchLimit(searchLimit),
    m_includeHiddenFiles(includeHiddenFiles)
{
//    qDebug() << "in deepsearch::CONstructor -> m_searchLimit: " << m_searchLimit;
}

DeepSearch::~DeepSearch()
{
//    qDebug() << "in deepsearch::DEstructor";
    m_rootFolder.reset();
}

void clearSelfContainedParentFolders(QList<QString>& treffer)
{
    int i=0;
    while( i < treffer.size()-1 )
    {
        if( StaticFunctions::isSubDirectory(treffer[i], treffer[i+1]) )
//      if( treffer[i].startsWith( treffer[i+1] ) )
        {
            treffer.removeAt(i+1);
        }else
        {
            ++i;
        }
    }
}
QDir getDir(QFileInfo fi)
{
    if(fi.isDir())
        return QDir(fi.absoluteFilePath());
    else
        return fi.dir();
}
const QList<QString> &DeepSearch::launchSearch()
{
//    qDebug() << "launching DeepSearch";
    m_searchResultCounter = 0;

    QList<QString> trefferPaths = searchFolder( m_rootFolder );

    if (m_cancelled)
        return trefferPaths;

    // sortiert die treffer so, dass direkt danach clearSelfContainedParentFolders ganz effizient unnoetige
    // trefferPaths rausschmeissen kann aus der liste, ohnehin schon aufgrund anderer treffer geladen werden:
    // (daher auch die umgekehrte sortierung dir_a.absolutePath() > dir_b.absolutePath(); (> sortiert in umgekehter reihenfolge)
    std::sort(trefferPaths.begin(), trefferPaths.end(),
          [&](const auto& a, const auto& b){
                QDir dir_a = getDir(a);
                QDir dir_b = getDir(b);
                return dir_a.absolutePath() > dir_b.absolutePath();
            }
    );

//    qDebug() << "\n\ntrefferPaths: size: " << trefferPaths.size();
//    int a=0;
//    foreach(const QString& trefferpth, trefferPaths)
//    {
//        qDebug() << "[" << a++ << "]: " << trefferpth;
//    }

    clearSelfContainedParentFolders(trefferPaths);

//    qDebug() << "\n\ntrefferPaths after clearance: size: " << trefferPaths.size();
//    a=0;
//    foreach(const QString& trefferpth, trefferPaths)
//    {
//        qDebug() << "[" << a++ << "]: " << trefferpth;
//    }
    m_trefferPaths = trefferPaths;

    return m_trefferPaths;
}

void DeepSearch::elapseRoot()
{
    if (m_cancelled)
        return;
    elapseFoldersContainingKeywords(m_trefferPaths);
}

bool DeepSearch::maxSearchWordsLimitReached()
{
    return matchLimitReached(0);
}

void DeepSearch::cancel()
{
    m_cancelled = true;
}

// checkt, ob potentailParentFolder der parent-folder EINES der trefferPaths ist, also z.B:
// potentailParentFolder:    "/home/bigdaddy/Documents"
// einer der treffer-paths:  "/home/bigdaddy/Documents/test_folder/test.txt"
// ~> return true
// denn: diese Funktion folderIsParentFolderOfAnyTrefferPath wird dazu verwendet, den potentailParentFolder
// zu "elapsen", damit sichergestellt wird, dass alle trefferPaths sichtbar sind in dem dargestellten
// datei-baum:
bool folderIsParentFolderOfAnyTrefferPath(const QFileInfo& potentailParentFolder, const QList<QString> &trefferPaths)
{
    const QString absPath = potentailParentFolder.absoluteFilePath();
    foreach (const QString& curTrefferPath, trefferPaths) {
        bool isParentFolder = curTrefferPath != absPath &&
                              StaticFunctions::isSubDirectory(curTrefferPath, absPath);
//                            curTrefferPath.startsWith(absPath) ;
        if(isParentFolder)
            return true;
    }
    return false;
}

void DeepSearch::elapseFoldersContainingKeywords(const QList<QString> &trefferPaths)
{
    if (m_cancelled)
    {
        m_rootFolder->disableSignals(false);
        return;
    }

    m_rootFolder->disableSignals(true);
    elapseFoldersContainingKeywordsHelper(m_rootFolder, trefferPaths);
    m_rootFolder->disableSignals(false);
}

void DeepSearch::elapseFoldersContainingKeywordsHelper(std::shared_ptr<FileInfoBD> curFolder, const QList<QString>& trefferPaths)
{
    if( !curFolder || m_cancelled )
        return;

//    qDebug() << "in eFCKH: " << curFolder->getFileInfo().absoluteFilePath();
//    qDebug() << "   contaisTrefferPath: " << folderContainsKeywordPath(curFolder->getFileInfo(), trefferPaths);
//    qDebug() << "   curFolder->elapsed: " << curFolder->elapsed();

    if( folderIsParentFolderOfAnyTrefferPath(curFolder->getFileInfo(), trefferPaths) )
    {
        if( !curFolder->elapsed() )
        {
            curFolder->setElapsed(true);
//            qDebug() << "emitting folderElapsed";
            if( !m_cancelled )
                emit folderElapsed(curFolder->getFileInfo());
        }
        const QVector<std::weak_ptr<FileInfoBD>> subFolders = curFolder->getSubFolders();
        for(int i=0; i < subFolders.size(); ++i)
        {
            elapseFoldersContainingKeywordsHelper( subFolders[i].lock(), trefferPaths);
        }
    }
}

QList<QString> DeepSearch::searchFolder(std::shared_ptr<FileInfoBD> folder)
{
//    qDebug() << "in DeepSearch::searchFolder(std::shared_ptr<FileInfoBD>)";
    QList<QString> searchResults;

    if( !folder || m_cancelled || matchLimitReached(0))
        return searchResults;

    bool filesContainMatch = false;

     auto files = folder->getFiles();
     for(unsigned int i=0; i < files.size(); i++)
     {
         QString fileName = files[i].fileName();
         QString filePth  = files[i].absoluteFilePath();
         if(fileName.toLower().contains(m_keyword))
         {
             searchResults.append( filePth );

             if( matchLimitReached() )
                 return searchResults;

             // break, da ein einziger dateiName das keyword im namen tragen muss. Hier gehts ja nur darum, den
             // parent-Folder zu "elapsen":
             filesContainMatch = true;
             break;
         }
     }
     auto subFolders = folder->getSubFolders();

     // schauen, ob einer der ordnerNamen das keyword enthaelt:
     if( !filesContainMatch )
     {
         for(unsigned int i=0; i < subFolders.size(); ++i)
         {
             auto locked = subFolders[i].lock();
             if(locked)
             {
                 QString subFoldName = locked->getFileInfo().fileName();
                 QString subFoldPath = locked->getFileInfo().absoluteFilePath();
                 if(subFoldName.toLower().contains(m_keyword))
                 {
                     searchResults.append( subFoldPath );

                     if( matchLimitReached() )
                         return searchResults;

                     // break, da ein einziger ordner das keyword im namen tragen muss. Hier gehts ja nur darum, den
                     // parent-Folder zu "elapsen":
                     break;
                 }
             }
         }
     }
     for(unsigned int i=0; i < subFolders.size(); ++i)
     {
         auto locked = subFolders[i].lock();
         if(locked)
         {
             if(locked->elapsed())
             {
                 // recursive: in already elapsed subfolder:
                 searchResults.append( searchFolder( locked ) );
             }else
             {
                 // recursive: in NOT elapsed subfolder:
                 searchResults.append( searchFolder(locked->getFileInfo()) );
             }
         }
     }
     return searchResults;
}

QList<QString> DeepSearch::searchFolder(QFileInfo folder)
{
//    qDebug() << "in DeepSearch::searchFolder(QFileInfo)";

    QList<QString> searchResults;
    QDir dir = getDir(folder);

    if(m_cancelled || matchLimitReached(0))
        return searchResults;

    QFileInfoList foldersLst = StaticFunctions::getFoldersInDirectory(dir, m_includeHiddenFiles);//dir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);
    QFileInfoList filesLst   = StaticFunctions::getFilesInDirectory  (dir, m_includeHiddenFiles);//dir.entryInfoList(QDir::Files   | QDir::NoDotAndDotDot);

    bool filesContainMatch = false;

    for(unsigned int i=0; i < filesLst.size(); ++i){
        const QString fileName = filesLst[i].fileName();
        const QString filePath = filesLst[i].absoluteFilePath();
        if(fileName.toLower().contains(m_keyword))
        {
            if( matchLimitReached() )
                return searchResults;

            searchResults.append(filePath);

            filesContainMatch = true;
            break;
        }
    }

    for(unsigned int i=0; i < foldersLst.size(); ++i){
        const QString foldName = foldersLst[i].fileName();
        const QString foldPath = foldersLst[i].absoluteFilePath();
        if( !filesContainMatch && foldName.toLower().contains(m_keyword))
        {
            if( matchLimitReached() )
                return searchResults;

            searchResults.append(foldPath);
        }

        // recursive: search for keywords in subfolder:
        searchResults.append( searchFolder(foldersLst[i]) );
    }

    return searchResults;
}

bool DeepSearch::matchLimitReached(int matchesToAdd)
{
    long counter = m_searchResultCounter.fetch_add(matchesToAdd, std::memory_order_relaxed) + matchesToAdd;
//    qDebug() << "   matchLimitReached - matchesToAdd: " << matchesToAdd;
    return counter > m_searchLimit;
}

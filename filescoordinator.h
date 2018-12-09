#ifndef FILESCOORDINATOR_H
#define FILESCOORDINATOR_H

#include <QObject>
#include <QVector>
#include <QDir>
#include <QFileInfo>
#include <QTimer>
#include <QPushButton>
#include <QMessageBox>
#include <QIcon>
#include <QPixmap>
#include <QStringList>
#include <QFontMetrics>
#include <QPalette>
#include <QStack>
#include <QElapsedTimer>
#include <QProcess>
#include <QScrollArea>
#include <chrono>

#include <memory>
#include <functional>

#include "graphicsview.h"
#include "fileinfobd.h"
#include "folderlistener.h"
#include "staticfunctions.h"
#include "elapseworker.h"
#include "elapseworkerfunctional.h"
#include "directoryselectionpane.h"
#include "deepsearch.h"
#include "deppsearchworker.h"
#include "openwithdialog.h"
#include "canceller.h"

class GraphicsView;
class FileInfoBD;

using namespace std::chrono;

struct FilInfoForOneDim{
    FilInfoForOneDim(std::shared_ptr<FileInfoBD> fileInfoBD,
                     bool isFolder = true,
                     QString fileName = QString(""),
                     int depth = 0)
        : m_fileInfoBD(fileInfoBD), m_isFolder(isFolder), m_fileName(fileName), m_depth(depth)
    {
        m_absolutPath = getAbsoluteFilePathHelper();
        m_fileName = getFileNameHelper();
    }

    FilInfoForOneDim(std::weak_ptr<FileInfoBD> fileInfoBD,
                     bool isFolder = true,
                     QString fileName = QString(""),
                     int depth = 0)
        : m_fileInfoBD(fileInfoBD), m_isFolder(isFolder), m_fileName(fileName), m_depth(depth)
    {
        m_absolutPath = getAbsoluteFilePathHelper();
        m_fileName = getFileNameHelper();
    }
    FilInfoForOneDim()
        : m_fileInfoBD(std::weak_ptr<FileInfoBD>()), m_fileName(QString("")), m_isFolder(false), m_depth(0)
    {
    }
    FilInfoForOneDim(std::shared_ptr<FilInfoForOneDim> sharedPtr)
        : m_fileInfoBD(sharedPtr->m_fileInfoBD),
          m_isFolder(sharedPtr->m_isFolder),
          m_fileName(sharedPtr->m_fileName),
          m_depth(sharedPtr->m_depth),
          m_absolutPath(sharedPtr->m_absolutPath)
    {
    }
    FilInfoForOneDim(FilInfoForOneDim& filForOneDim)
        : m_fileInfoBD(filForOneDim.m_fileInfoBD),
          m_isFolder(filForOneDim.m_isFolder),
          m_fileName(filForOneDim.m_fileName),
          m_depth(filForOneDim.m_depth),
          m_absolutPath(filForOneDim.m_absolutPath)
    {
    }
    FilInfoForOneDim(const FilInfoForOneDim& filForOneDim)
        : m_fileInfoBD(filForOneDim.m_fileInfoBD),
          m_isFolder(filForOneDim.m_isFolder),
          m_fileName(filForOneDim.m_fileName),
          m_depth(filForOneDim.m_depth),
          m_absolutPath(filForOneDim.m_absolutPath)
    {
    }

    ~FilInfoForOneDim(){
        m_fileInfoBD.reset();
    }

    bool operator==(const FilInfoForOneDim& vgl) const {
        if(vgl.m_fileName == this->m_fileName &&
           vgl.m_isFolder == this->m_isFolder){
            if(auto locked1 = vgl.m_fileInfoBD.lock()){
                if(auto locked2 = this->m_fileInfoBD.lock()){
                    return locked1 == locked2;
                }else{
                    return false;
                }
            }else{
                return false;
            }
        }else{
            return false;
        }
    }
    bool operator!=(const FilInfoForOneDim& vgl) const {
        if(vgl.m_fileName == this->m_fileName &&
           vgl.m_isFolder == this->m_isFolder){
            if(auto locked1 = vgl.m_fileInfoBD.lock()){
                if(auto locked2 = this->m_fileInfoBD.lock()){
                    return !(locked1 == locked2);
                }else{
                    return true;
                }
            }else{
                return true;
            }
        }else{
            return true;
        }
    }

    const QString& getAbsoluteFilePath() const{
        return m_absolutPath;
    }
    const QString& getFileName() const{
        return m_fileName;
    }

    std::weak_ptr<FileInfoBD> m_fileInfoBD;
    bool m_isFolder = false;
    int m_depth;
private:
    QString getAbsoluteFilePathHelper() const
    {
        if (auto locked = m_fileInfoBD.lock()){
            if(m_isFolder){
                return locked->getFileInfo().absoluteFilePath();
            }else{
                return QString("%1%2%3")
                        .arg(locked->getFileInfo().absoluteFilePath())
                        .arg(QDir::separator())
                        .arg(m_fileName);
            }
        }
        return QString("");
    }
    QString getFileNameHelper() const
    {
        if(m_isFolder){
            if(auto locked = m_fileInfoBD.lock()){
                return locked->getFileInfo().fileName();
            }else{
                return QString("");
            }
        }else{
            return m_fileName;
        }
    }

    QString m_absolutPath;
    QString m_fileName;
};

class FilesCoordinator
        : public QObject, public FolderListener
{
    Q_OBJECT
public:
    FilesCoordinator(QObject* parent = nullptr);
    ~FilesCoordinator();

    void setFocusCaller(std::function<void (std::weak_ptr<FilesCoordinator>)> focusCaller);
    void setRootChangedCaller(std::function<void(QDir)> rootChangedCaller);
    void setOnOpenFoldersInNewTab(std::function<void(QVector<QDir>)> onOpenFoldersInNewTab);
    void setOnCloseTab(std::function<void()> onCloseTab);

    void resetToDefault(bool closeRoot = true);

    QLayout* getLayout();
    void resetWidgets();

    void collapseFolderRecursively(std::shared_ptr<FileInfoBD> fiBD);
    void collapseFolderRecursively(std::weak_ptr<FileInfoBD> fiBD);
    void collapseFolderRecursively(FilInfoForOneDim& fold);
    void collapseFoldersRecursively(const QList<FilInfoForOneDim> &folds);
    void collapseFoldersRecursively(QList<QString> paths);

    QList<std::shared_ptr<FileInfoBD>> getFileInfoBDsFromPaths(QList<QString>& paths);
    std::shared_ptr<FileInfoBD> getFileInfoBDFromPath(const QString& path);

    void collapseFolder(std::weak_ptr<FileInfoBD> fiBD);
    void collapseFolder(FilInfoForOneDim& fold);
    void collapseFolders(const QList<FilInfoForOneDim> &folds);
    void collapseFolders(QList<QString> paths);

    void elapseAll();
    void elapseFolder(std::weak_ptr<FileInfoBD> fiBD);
    void elapseFoldersRecursively(const QList<FilInfoForOneDim>& folds);
    void elapseFoldersRecursively(QList<QString> folds);
    void elapseFolders(const QList<FilInfoForOneDim>& folds);
    void elapseFolders(QList<QString> paths);

    void elapseOrCollapseFolderDependingOnCurrentState(std::weak_ptr<FileInfoBD> fiBD);

    template <class T>
    QList<T> validateSelfContainingList(const QList<T>& refFolds, std::function<QString(const T&)> getStringFromT) const;
    QList<FilInfoForOneDim> validateFoldersToElapse(const QList<FilInfoForOneDim>& refFolds) const;
    QList<QString>          validateFoldersToElapse(const QList<QString>& refFolds) const;
    QList<FilInfoForOneDim> validateFoldersToDelete(const QList<FilInfoForOneDim>& refFolds) const;
    QList<QString>          validateFoldersToDelete(const QList<QString>&          refFolds) const;
    QList<QString>          validateFoldersToCopy  (const QList<QString>& refFolds) const;
    QList<QFileInfo>        validateFoldersToCopy  (const QList<QFileInfo>& refFolds) const;

    template <class T>
    void validateFilesToDelete(QList<T>& filesToDelete, const QList<T>& foldersToDelete, std::function<QString(const T&)> getAbsPathFromT) const;
    void validateFilesToDelete(QList<QFileInfo>& filesToDelete, const QList<QFileInfo>& foldersToDelete) const;
    void validateFilesToDelete(QList<QString>& filesToDelete, const QList<QString>& foldersToDelete) const;
    void validateFilesToCopy  (QList<QString>& filesToCopy, const QList<QString>& foldersToCopy) const;
    void validateFilesToCopy  (QList<QFileInfo>& filesToCopy, const QList<QFileInfo>& foldersToCopy) const;

    void pasteHelper(QString paths, QString tarPath = QString(""));
    void pasteHelper(QList<QUrl> urls, QString tarPath = QString(""));
    void pasteHelper(QVector<QString> paths, const QString tarPath = QString(""));

    QList<QString> selectedFolders() const;
    QList<QString> selectedFiles() const;
    QList<QString> selectedContent() const;

//    bool isSelected(FilInfoForOneDim fold);
    bool isSelected(const QString& absFilePath) const;

    bool filesSelected() const;
    bool foldersSelected() const;
    bool contentSelected() const;
    bool singleFolderSelected() const;
    bool singleFileSelected() const;
    bool singleContentSelected() const;

    int selectionCounter() const;

    void revalidateSearchIds(QString keyword = QObject::tr(""));
    void resetSearchIds(QString keyword);
    bool searchResultsEmpty() const;
    int  searchResultsFound() const;
    long getSearchIndex() const;
    void focusSearchId();
    long getIndexOfCurrentSearchResult() const;
    long getSearchResultsCount() const;

    bool entryIsHidden(const FilInfoForOneDim& entry) const;
    QString getCurSearchResultStr();
    const FilInfoForOneDim& getCurSearchResult() const;
    bool isCurentSearchResult(const FilInfoForOneDim& fiForOneDim) const;
    bool isCurentSearchResult(const QString& path) const;
    void clearSearchResults();

    void focusEntryForKeyPressed();

    int getMaxDepth();

    bool depthIdElapsed(int depthId);

    int getDepth();

    void sort(std::weak_ptr<FileInfoBD> fiBD, ORDER_BY order);
    bool isReversedSorted(std::weak_ptr<FileInfoBD> fiBD);

    void openSelectionByDefaultPrograms();

    FilInfoForOneDim getFileAt(int id);
    FilInfoForOneDim getDisplayedFileAt(int id);

    int getFileCount();
    int getDisplayedFileCount();

    void setSelf(std::shared_ptr<FilesCoordinator> self);

    void forceRevalidation();

    bool selectionContainsZippedFile();

    QString getCurRootPath();

    bool inSearchMode();    

    void selectAllInRange(int lowerBound, int upperBound, bool selectInDisplayedContent = true);
    void selectContent(QString cont, bool isFolder, bool controlPrsd = false, bool shiftPrsd = false);
    void deselectContent(const QString& entry, bool repaint=false);

    bool includeHiddenFiles() const;

public slots:
    void openSelectedFoldersInNewTab();
    void closeCurrentTab();

    void revalidate();

    void killCurrentBlockingActionSLT();

    void repaintFolderViewer();
    void startWaitingAnimation();
    void killWaitingAnimation();

    void folderChanged(std::weak_ptr<FileInfoBD> f) override;
    void folderElapsed(std::weak_ptr<FileInfoBD> f) override;
    void sortingChanged(std::weak_ptr<FileInfoBD> f) override;
    void addDirectoryToWatcher(QString directory) override;
    void removeDirectoryFromWatcher(QString directory) override;

    void directoryChanged(QString path);

    void saveGraphicsViewVBarValue(int value);
    void saveGraphicsViewHBarValue(int value);

    void exitSearchMode();
    void searchForKeyWord(QString keyword, bool deepSearch);

    void elapseAllFoldersOfDepthId(int depthId);

//    ---------------------------------

    void selectEntireContent();
    void clearSelection();
    void selectButtonUp(bool ctrl_prsd, bool shft_prsd);
    void selectButtonDown(bool ctrl_prsd, bool shft_prsd);

    void unmountDrive(QDir drive);

    void copySelectedContent();
    void cutSelectedContent();
    void duplicateSelectedContent();

    void deleteSelectedContent();
    void openSelectedContent();
    void openSelectedContentWith();

    void showDetailsOfSelectedContent();
    void renameSelectedContent();

    void zipSelectedContent();
    void unzipSelectedContent();

    void createNewFolder();
    void createNewFile();

    void elapseSelectedFolders();
    void elapseSelectedFoldersRecursively();
    void collapseSelectedFoldersRecursively();
    void collapseSelectedFolders();

    void copySelectedFilePathToClipboard();

    void requestFocus();

    void initDragging(QString initiator);

    void keyPressed(const char c);

    void setSelectionToRoot();
    void setLastPathToRoot();
    void setRootFolder(QDir dir);

    void pasteFromClipboard();
    void paste(QString paths, QString tarPath = QString(""));

    void sortAllFolders(ORDER_BY order);

    void openTerminal() const;

    void nextSearchResult();
    void previousSearchResult();

    void openSelection();
    void setParentToRoot();

//    void selectFromDisplayedContent(QFileInfo fi, bool controlPrsd = false, bool shiftPrsd = false);
    void sortFromDisplayedContent(QFileInfo fi, ORDER_BY order);

    void setZoomFactor(int zoomFactor);

    void setIncludeHiddenFiles(bool includeHiddenFiles);

    void blockingThreadFinished();
signals:
    void killCurrentBlockingAction();
    void rootChanged(QDir newRootPath);

    void isNowFocused(std::weak_ptr<FilesCoordinator> filesCoordRequestingFocus);

    void openFoldersInNewTabSGNL(QVector<QDir> dirsToOpen);
    void closeCurrentTabSNGL();

    void repaintFolderViewerSGNL();
    void focusIdFolderViewerSGNL(int id, bool repaintAnyway);
    void startWaitingAnimationSGNL();
    void killWaitingAnimationSGNL();
    void focusFolderViewerSGNL();
    void showSearchMenuSGNL();

    void blockToolBar(bool block);
private:

    void addNewFileFolderHelper(bool createFile);

    void copyCutToClipboardHelper(bool deleteSourceAfterCopying = false);

    void createWidgets();

    void elapseFolderRecursiveHelper(std::weak_ptr<FileInfoBD> fiBD,
                               bool addElapseThreadCounter = true);
    template<class T>
    void elapseFoldersRecursivelyHelper(QList<T>& folds, std::function<std::weak_ptr<FileInfoBD>(T&)> getFileInfBDFromT);
    template<class T>
    void elapseFoldersHelper(QList<T>& foldersToElapse, std::function<std::weak_ptr<FileInfoBD> (T&)> getFileInfBDFromT);
    template<class T>
    void collapseFoldersRecursivelyHelper(QList<T>& folderToElapse, std::function<std::weak_ptr<FileInfoBD> (T&)> getFileInfoBDFromT);
    template<class T>
    void collapseFoldersHelper(QList<T>& folderToElapse, std::function<std::weak_ptr<FileInfoBD> (T&)> getFileInfoBDFromT);

    void replaceRoot(std::shared_ptr<FileInfoBD> newRoot);

//    void setRootToGraphicsView();

    void launchWorkerFunctional(std::function<void (std::shared_ptr<bool>, std::function<void(double)>)> caller,
                                                         QString infoMessage);

    void launchWorkerFunctional(std::function<void (std::shared_ptr<bool>, std::function<void(double)>)> caller, bool revalidate = true);
    void launchWorkerFunctional(std::function<void(std::function<void(double)>)> caller,
                                Canceller* canceller = nullptr);

//    std::function<void(FilInfoForOneDim)> getSelectionFunction();
//    int getIdOf(FilInfoForOneDim fileInfOneDim) const;
//    int getIdOfDisp(FilInfoForOneDim fileInfOneDim) const;

    int getIdOf(const FilInfoForOneDim& fileInfOneDim, bool searchInDisplayedContent = true) const;
    int getIdOf(std::shared_ptr<FilInfoForOneDim> fileInfOneDim, bool searchInDisplayedContent = true) const;
    int getIdOf(const QString& absEntryPath, bool searchInDisplayedContent = true) const;

    const FilInfoForOneDim& getFilInfoForOneDimFromPath(const QString absPath, bool searchInDisplayedContent = true);

    void focusFolderViewer(int id, bool repaintAnyway = false);

    int evaluateDepth(std::weak_ptr<FileInfoBD> filInfo, int* curRow, int curDepth = 0);
    void revalidateOneDimnFolderOrder(bool revalAnyway = false);

//    std::shared_ptr<FilInfoForOneDim> iterateBackwards(const FilInfoForOneDim& fileInfoOneDim, int offs,
//                                                       std::function<void(FilInfoForOneDim)> func = nullptr);
//    std::shared_ptr<FilInfoForOneDim> iterateForwards(const FilInfoForOneDim& fileInfoOneDim, int offs,
//                                                      std::function<void(FilInfoForOneDim)> func = nullptr);
//    std::shared_ptr<FilInfoForOneDim> iterateForwBackwHelper(const FilInfoForOneDim& fileInfoOneDim, int offs,
//                                                             bool iterateForwards,
//                                                             std::function<void(FilInfoForOneDim)> func = nullptr);

    void setSelected(const QString &fi, bool isFolder);


    void printSelectedFolders();
    void printSelectedFiles();

//    void revalidateToolBar();

    void showInfoDialog(QString str, QString title = tr(""));

    void blockRevalidating(bool block);
    bool revalidatingIsBlocked();

//    GraphicsView* m_folderViewer      = nullptr;
    QGridLayout* m_mainGrid           = nullptr;
//    DirectorySelectionPane* m_toolBar = nullptr;

    std::shared_ptr<FileInfoBD> m_root;
    QList<FilInfoForOneDim> m_oneDimnFolderOrder;
    QList<FilInfoForOneDim> m_oneDimnFolderOrderDisplayed;

//    QList<FilInfoForOneDim> m_slctdFolds;
//    QList<FilInfoForOneDim> m_slctdFiles;
    QSet<QString> m_slctdFolds;
    QSet<QString> m_slctdFiles;
    QString m_lastSelection;

    QStack<QString> m_dirStack;

    QList<FilInfoForOneDim> m_searchResults;
    QList<int> m_searchIds;
    int m_searchIndex = -1;
    QString m_curSearchKeyWord;

    std::shared_ptr<QFileSystemWatcher> m_watcher = std::make_shared<QFileSystemWatcher>();
    std::shared_ptr<QMetaObject::Connection> m_watcherConn = std::make_shared<QMetaObject::Connection>();

    QVector<bool> m_depthIdsElapsed;

    bool m_blockRevalidating = false;
    QAtomicInt m_alreadyUpdating = QAtomicInt();

    // wird in/fuer elapseFolderRecursive-Funktion verwendet:
    std::atomic<int> elapsingThreadCounter;

    std::weak_ptr<FilesCoordinator> m_self;

    int m_graphicsViewVBarValueBackup = 0;
    int m_graphicsViewHBarValueBackup = 0;
    int m_zoomFactor = 9;

    int m_maxDepth = 0;


    std::chrono::milliseconds m_lastKeyPressed;
    QString m_keysPressed;

    std::function<void (std::weak_ptr<FilesCoordinator>)> m_focusCaller = nullptr;
    std::function<void (QDir)> m_tellParentRootHasChanged = nullptr;
    std::function<void (QVector<QDir>)> m_onOpenFoldersInNewTab = nullptr;
    std::function<void ()> m_onCloseCurrentTab = nullptr;

    bool m_includeHiddenFiles = false;
};

#endif // FILESCOORDINATOR_H

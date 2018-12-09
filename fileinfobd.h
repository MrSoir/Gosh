#ifndef FILEINFOBD_H
#define FILEINFOBD_H

#include <QObject>
#include <QVector>
#include <QFileInfo>
#include <QFileInfoList>
#include <QFileSystemWatcher>
#include <QDir>
#include <QString>
#include <QDateTime>
#include <QDebug>
#include <QTimer>
#include <memory>
#include "orderby.h"
#include "staticfunctions.h"
#include "folderlistener.h"
#include "fileinforef.h"
#include "elapsevalidator.h"

class FileInfoRef;

class FileInfoBD
        : public QObject
{
    Q_OBJECT
public:
    explicit FileInfoBD(const QString &path,
                        std::weak_ptr<FileInfoBD> parentFiBD = std::weak_ptr<FileInfoBD>(),
                        QObject *parent = nullptr);
    explicit FileInfoBD(const QFileInfo &fileInfo,
                        std::weak_ptr<FileInfoBD> parentFiBD = std::weak_ptr<FileInfoBD>(),
                        QObject *parent = nullptr);
    ~FileInfoBD();

    bool elapsed() const;
    bool isLoaded() const;

    bool isEmpty();

    int getDispElmntCount();
    int getDispElmntCountRecurs();

    const QVector<std::weak_ptr<FileInfoBD>> getSubFolders(int startId = -1, int endId = -1) const;
    const QVector<QFileInfo> getFiles(int startId = -1, int endId = -1) const;
    const QVector<QFileInfo> loadTooBigFiles(int startId = -1, int endId = -1) const;

    int getFileCount();
    int getFolderCount();

    void disableSignals(bool disableSignals);

    const QFileInfo getFileAt(int pos) const;
    int getFileId(const QString fileName) const;

    int fileCount() const;

    const QFileInfo& getFileInfo() const;

    bool isEmpty() const;

    void setRoot(const QString& path);

    void close();
    void print(int i=0);

    const QString fileName() const;

    void addListener(std::weak_ptr<FolderListener> lstnr);
    void removeListener(std::weak_ptr<FolderListener> lstnr);

    void traverse(std::function<void(std::weak_ptr<FileInfoBD>, int)> folderFunc,
                  std::function<void(std::weak_ptr<FileInfoBD>, int)> fileFunc,
                  bool ignoreUnElapsedFolders = false, int depth = 0);

    bool traverseOverSubFolders(std::function<bool(std::shared_ptr<FileInfoBD>, int)> folderFunc,
                                bool ignoreUnElapsedFolders = false, int depth=0);

    FileInfoRef iterateBack(int* howManyTimesBack, int fileId = -1);

    ORDER_BY getSortinOrder() const;
    bool isReversedSorted() const;
    bool isReversedSortedBy(ORDER_BY ord) const;
    bool isSortedBy(ORDER_BY ord) const;
    void sortByRecursivelyWithoutNotification(ORDER_BY ord);

    void replaceSubFolders(FileInfoBD* containsSubfoldersToReplace);
    void replaceSubFolder(std::shared_ptr<FileInfoBD> subFoldToReplace);
    void replaceSubFolder(FileInfoBD* subFoldToReplace);
    void addFiles(const QFileInfoList& files);

    void registerFolderToWatcherRec();

    void setParentFiBD(std::weak_ptr<FileInfoBD> parentFiBD);
    std::shared_ptr<FileInfoBD> getParentFiBD();

    bool isBeingDisplayed();
    std::weak_ptr<FileInfoBD> getFirstDispParent();

    void setSelf(std::weak_ptr<FileInfoBD> self);

    void setIncludeHiddenFilesCaller(std::function<bool()> includeHiddenFilesCaller, bool revalidate = true);
    void includeHiddenFilesChanged();

signals:
    void folderContentHasChanged(std::weak_ptr<FileInfoBD> fileInfoBD = std::weak_ptr<FileInfoBD>());
    void fileInfoBdElapsed(std::weak_ptr<FileInfoBD> fileInfoBD = std::weak_ptr<FileInfoBD>());

    void sortingHasChanged(std::weak_ptr<FileInfoBD> fileInfoBD = std::weak_ptr<FileInfoBD>());

    void addDirectoryToWatcher(QString directory);
public slots:
    void setElapsed();
    void setElapsed(bool elpsd);
    void setElapsedFlag(bool elpsd); // <- ganz gefaehrlich! setzt nur die isElapsed-flag ohne revalidating!!! (wird fuers threading gebraucht)
    void setLoadedFlag(bool loaded);
    void elapseAll();//std::make_unique<bool>(false));
    void elapseAll(std::shared_ptr<AbstrElapseValidator> validator );//std::shared_ptr<bool> cancelled);//std::make_unique<bool>(false));
    void collapseAll();
    void collapse();

    void directoryChanged(const QString& path);
    void fileChanged(const QString& path);

    void sortBy(ORDER_BY m_order, bool ifNewOrderEqualsOldOrderDoReverse = false, bool notificateListener = true);
private:
    void removeFromParent();

    void revalFolderContent();

    void sortSubFolders();

    QDir::SortFlags getSortFlags() const;
    std::function<bool(const QFileInfo&, const QFileInfo&)> getSortFunction();
    void sort(std::function<bool (const QFileInfo&,const QFileInfo&)> sortFunc);
    void sort();

    bool isElapsed = false;
    bool alrLoaded = false;

    void doElapsing();

    void closeWatcher();

    bool addFolderIfNotAlrExstnd(const QFileInfo& f);
    bool addFileIfNotAlrExstnd(const QFileInfo& f);
//    bool removeFolder(std::weak_ptr<FileInfoBD> fiBD);
    bool removeFolder(const QFileInfo& fi);
    bool removeFolder(const QString& foldName);
    bool removeFile(const QFileInfo& fi);

    void printChildFolders(int tabcount = 1);
    void printChildFolderNames(int tabcount = 1);

    QFileInfo fileInfo;

    QVector<QFileInfo> files;
    QVector<std::shared_ptr<FileInfoBD>> sub_folds;
    QVector<QString> sub_fold_names; // only the folder names, not the entire path -> needed for fast lookup
    int m_filesCount = 0;
    int m_subFoldsCount = 0;
    bool m_tooBigToSave = false;
    int m_tooBigToSave_Limit = 10000;

    QVector< std::weak_ptr<FolderListener> > listeners;

    ORDER_BY m_order;

    int m_contextId = 0;
    int m_contentCount = 0;

    bool m_disableSignals = false;

    std::weak_ptr<FileInfoBD> m_parent;

    std::weak_ptr<FileInfoBD> m_self;

    bool includeHiddenFiles() const;
    std::function<bool()> m_includeHiddenFilesCaller = nullptr;
};

#endif // FILEINFOBD_H

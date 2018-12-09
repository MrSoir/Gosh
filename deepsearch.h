#ifndef DEEPSEARCH_H
#define DEEPSEARCH_H

#include <QObject>
#include <QVector>
#include <QList>

#include <memory>
#include <iostream>
#include <functional>
#include <atomic>

#include "fileinfobd.h"

class DeepSearch : public QObject
{
    Q_OBJECT
public:
    explicit DeepSearch(std::shared_ptr<FileInfoBD> m_rootFolder,
                        const QString& keyword,
                        long searchLimit = 1000,
                        bool includeHiddenFiles = false,
                        QObject *parent = nullptr);
    ~DeepSearch();

    const QList<QString>& launchSearch();
    void elapseRoot();

    bool maxSearchWordsLimitReached();
signals:
    void folderElapsed(QFileInfo fi);
public slots:
    void cancel();
private:
    bool matchLimitReached(int matchesToAdd = 1);

    QList<QString> searchFolder(std::shared_ptr<FileInfoBD> folder);
    QList<QString> searchFolder(QFileInfo folder);

    void elapseFoldersContainingKeywords(const QList<QString>& trefferPaths);
    void elapseFoldersContainingKeywordsHelper(std::shared_ptr<FileInfoBD> curFolder, const QList<QString> &trefferPaths);

    std::shared_ptr<FileInfoBD> m_rootFolder;
    bool m_cancelled = false;
    const QString m_keyword;
    QList<QString> m_trefferPaths;

    long m_searchLimit;
    std::atomic<long> m_searchResultCounter;

    bool m_includeHiddenFiles;
};

#endif // DEEPSEARCH_H

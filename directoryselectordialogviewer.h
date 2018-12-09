#ifndef DIRECTORYSELECTORDIALOGVIEWER_H
#define DIRECTORYSELECTORDIALOGVIEWER_H

#include <QObject>
#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QFileInfo>
#include <QStorageInfo>
#include <QStandardPaths>
#include <functional>
#include "dynamicfunctioncaller.h"

#include "staticfunctions.h"
#include "customgraphicitems.h"

#include "graphicsfile.h"
#include "graphicsview.h"
#include "graphicsview.h"

//class GraphicsItemBD;
//class TextRect;

class DirectorySelectorDialogViewer : public QGraphicsView
{
public:
    DirectorySelectorDialogViewer(
            QString currentDirectory,
            std::function<void(QString)> caller,
            QSizeF size,
            QWidget *parent = nullptr);

    ~DirectorySelectorDialogViewer();
private:
    void rePaintCanvas();

    void showBookmarkDialog();
    void revalidateBookmarks();
    void loadBookmarks();

    void paintLabel(QString str, int rowId = 0, int colId = 0);
    void paintFileInfo(QFileInfo fi, int rowId, int colId = 1, bool drawAbsoluteFilePath = false);
    void paintBookmark(QFileInfo fi, int rowId, int colId = 1, bool drawAbsoluteFilePath = false);
    void paintMountedDrive(QFileInfo fi, int rowId, int colId = 1, bool drawAbsoluteFilePath = true);
    void paintStandardLocation(QStandardPaths::StandardLocation loc,
                               int* rowId);
    QGraphicsScene m_scene;

    QVector<QString> m_bookmarks;

    std::function<void(QString)> m_caller = nullptr;

    QString m_bookmarks_res_path = QString("%1%2%3").arg("resources").arg(QDir::separator()).arg("/bookmarks.res");

    QSizeF m_size;

    QString m_curDir;

    int m_rowHeight = 20;
    int m_colOffs = 25;

    GraphicItemsBD::TextRect* m_bookmarkAddBtn = nullptr;
};

#endif // DIRECTORYSELECTORDIALOGVIEWER_H

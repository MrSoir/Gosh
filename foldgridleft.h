#ifndef FOLDGRIDLEFT_H
#define FOLDGRIDLEFT_H

#include <QObject>
#include <QWidget>
#include <QStandardPaths>
#include <QFileInfoList>
#include <QFileInfo>
#include <QDebug>
#include <QPainter>

class FoldGridLeft : public QWidget
{
    Q_OBJECT
public:
    explicit FoldGridLeft(QWidget *parent = nullptr);

signals:
public slots:
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    void doPainting();
    void paintFolder(QPainter& painter, const QFileInfo& f, float* yOffs, int ascent);

    QStringList getStandardLocations(QStandardPaths::StandardLocation loc);
    QFileInfoList getDrives();
    void addFavourite_helper(QStandardPaths::StandardLocation loc);
    void addFavourite(QStandardPaths::StandardLocation loc);

    QVector<QFileInfo> favourites;
    QVector<QFileInfo> bookmarks;
    QVector<QFileInfo> drives;
};

#endif // FOLDGRIDLEFT_H

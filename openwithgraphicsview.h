#ifndef OPENWITHGRAPHICSVIEW_H
#define OPENWITHGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QWidget>
#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QObject>
#include <QLabel>

#include "staticfunctions.h"
#include "programmimeassociation.h"
#include "filehandler.h"
#include "customgraphicitems.h"

class OpenWithGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit OpenWithGraphicsView(const QFileInfo& fileToLoad,
                                  std::vector<ProgramMimeAssociation>& handlers,
                                  QWidget* parent = nullptr);
    ~OpenWithGraphicsView();

signals:
    void clicked(int id);
private:

    void repaint();

    QGraphicsRectItem* genSeparator(int yOffs);
    void paintHandler(int i, int yOffs);

    void deselectAll();

    QGraphicsScene m_scene;

    int m_selectedItem = 0;

    int m_eadingRowHeight = 60;
    int m_rowHeight = 40;

    std::vector<ProgramMimeAssociation>& m_programHandlers;
    QVector<QString> m_iconPaths;
    QVector<QString> m_programNames;

    QSize m_headinRectSize = QSize(60,60);
    QSize m_iconSize = QSize(m_rowHeight,m_rowHeight);

    QString m_defaultIconPath = QString("");

    const QFileInfo& m_fileToOpen;

    int m_separatorHeight = 3;

    QVector<GraphicItemsBD::IconAndLabelItem*> items;
};

#endif // OPENWITHGRAPHICSVIEW_H

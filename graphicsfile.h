#ifndef GRAPHICSFILE_H
#define GRAPHICSFILE_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsItemGroup>
#include <QStyleOptionGraphicsItem>
#include <QScrollBar>
#include <QRect>
#include <QColor>
#include <QFileInfo>
#include <QIcon>
#include <QGraphicsWidget>
#include <QDesktopServices>
#include <QMenu>
#include <QActionGroup>
#include <QAction>
#include <QApplication>
#include <QMimeData>
#include <QDrag>
#include <QClipboard>
#include <QUrl>

#include <functional>
#include <memory>
#include <atomic>

#include "staticfunctions.h"
#include "fileinfobd.h"
#include "dynamicfunctioncaller.h"
//#include "graphicsviewupdater.h"
#include "graphicsview.h"

class GraphicsFile : public QGraphicsItem
{
public:
    explicit GraphicsFile(const QFileInfo& m_fileInfo,
                          const QSize& size,
                          const int rowId=0,
                          const int rowHeight=0,
                          const int colId=0,
                          const int colOff=0,
                          std::shared_ptr<DynamicFunctionCaller<QString, std::function<bool()>>> caller = std::shared_ptr<DynamicFunctionCaller<QString, std::function<bool()>>>(),
                          std::shared_ptr<DynamicFunctionCaller<QString, std::function<bool(ORDER_BY)>>> sortCaller = std::shared_ptr<DynamicFunctionCaller<QString, std::function<bool(ORDER_BY)>>>(),
                          const int fontSize = 8,
                          QGraphicsItem *parent = nullptr);
    ~GraphicsFile();

    void setPosition(const QPoint& position);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void setTextColor(QColor textColor);
    void setDetailsTextColor(QColor detailsTextColor);
    void drawAbsoluteFilePath(bool drawAbsoluteFilePath);
    void openOnDoubleClick(bool openOnDoubleClick);
    void rightClickEnabled(bool rightClickEnabled);

    void setBackgroundColor(QColor background);

    void setInitDraggingFunction(std::function<void(QString)> func);
    void setDropFunction(std::function<void(QString)> func);
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

    void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent * event);

    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

    void keyPressEvent(QKeyEvent *event);

    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);
private:    
    void repaintBD();

    void paintRectangle(QPainter *painter, QRectF triangleRect, bool upside_down);

    void openContent();

    bool elapseRectPressed(QPointF mouP);

    QFileInfo m_fileInfo;

    QSize m_size;
    QPoint m_pos;
    bool m_hover = false;
    int m_rowId;
    int m_rowHeight;
    int m_colId;
    int m_colOffs;

    QRectF* m_elapseRect = nullptr;
    QVector<QRectF> m_sortRects;
    int m_sortRectHoverId = -1;

    QColor m_textColor;
    QColor m_detailsTextColor;
    bool m_drawAbsoluteFilePath;
    bool m_openOnDoubleClick;
    bool m_rightClickEnabled = false;

    QColor m_backgroundColor = QColor(255,255,255);

    int m_fontSize;

    bool m_dragEntered = false;

    std::function<void(QString)> m_initDraggingFunc = nullptr;
    std::function<void(QString)> m_dropFunc = nullptr;

    std::function<void()> m_dirFunc;
    std::shared_ptr<DynamicFunctionCaller<QString, std::function<bool()>>> m_caller;
    std::shared_ptr<DynamicFunctionCaller<QString, std::function<bool(ORDER_BY)>>> m_sortCaller;

    bool isPressed = false;
    QPointF m_initMousePrsdPos;
    qint64 lastTmePrsd = Q_INT64_C(0);
//    GraphicsViewUpdater* m_graphicsView;

    std::atomic<int> m_isCurrentlyRepainting;
};

#endif // GRAPHICSFILE_H

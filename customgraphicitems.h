#ifndef CUSTOMGRAPHICITEMS_H
#define CUSTOMGRAPHICITEMS_H

#include <QObject>
#include <QObject>
#include <QDebug>
#include <QString>
#include <QGraphicsItem>

#include <functional>
#include <memory>

#include "staticfunctions.h"

enum ORIENTATION{VERTICAL, HORIZONTAL};

namespace GraphicItemsBD
{
    class GraphicsItemBD : public QGraphicsItem{
    public:
        explicit GraphicsItemBD(const QSize& size = QSize(0,0),
                       const QPoint& pos = QPoint(0,0),
                       QGraphicsItem* parent = nullptr);
        ~GraphicsItemBD();

        void setSize(QSize size);
        void setPosition(QPoint position);
        QRectF boundingRect() const;

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    protected:
        QSize m_size;
        QPoint m_pos;

        bool revalidate = true;
    };

    class ButtonBD : public GraphicsItemBD{
    public:
        explicit ButtonBD(const QSize& size,
                 const QPoint& pos,
                 const QColor& gradCol1,
                 const QColor& gradCol2,
                 const QColor& selectionCol1 = QColor(255,255,255),
                 const QColor& selectionCol2 = QColor(255,255,255),
                 QGraphicsItem* parent = nullptr);
        ~ButtonBD();

        virtual void revalidateSize(float width, float height);

        void setCallFunction(std::function<void()> callFunc);

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        void hoverMoveEvent(QGraphicsSceneHoverEvent * event);
//    signals:
//        void clicked();
    private:
        QColor m_gradCol1;
        QColor m_gradCol2;
        QColor m_selectCol1;
        QColor m_selectCol2;

        bool mouIn = false;
        std::function<void()> m_callFunc;

        qint64 lastTmePrsd = QDateTime::currentMSecsSinceEpoch();
    };

    class TextRect : public ButtonBD{
    public:
        explicit TextRect(QString str,
                 const QSize& size,
                 const QPoint& pos,
                 const QColor& gradCol1,
                 const QColor& gradCol2,
                 const QColor& selectionCol1 = QColor(255,255,255),
                 const QColor& selectionCol2 = QColor(255,255,255),
                 const QColor& textCol = QColor(0,0,0),
                 QGraphicsItem* parent = nullptr);

        ~TextRect();

        void setFont(const QFont& font);

        void setText(QString str, int paddingX = 2, int paddingY = 2);

        void revalidateSize(float paddingX = 2, float paddingY = 2) override;

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    private:
        QString m_str;
        QColor m_textCol;
        QFont m_font;
    };

    class PixmapRect : public ButtonBD{
    public:
        explicit PixmapRect(QPixmap pixmap,
                 const QSize& size,
                 const QPoint& pos,
                 const QColor& selGradient1 = QColor(255,255,255, 255),
                 const QColor& selGradient2 = QColor(255,255,255, 255),
                 const QColor& selectionCol1 = QColor(255,255,255),
                 const QColor& selectionCol2 = QColor(255,255,255),
                 QGraphicsItem* parent = nullptr);
        ~PixmapRect();

        void setPixmap(QPixmap pixmap);

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    private:
        QPixmap m_pixmap;
    };

    class CustomShapeRect : public ButtonBD{
    public:
        explicit CustomShapeRect(std::function<void(QPainter*, const QRectF&)> paintFunction,
                 const QSize& size,
                 const QPoint& pos,
                 const QColor& selGradient1 = QColor(255,255,255, 255),
                 const QColor& selGradient2 = QColor(255,255,255, 255),
                 const QColor& selectionCol1 = QColor(255,255,255),
                 const QColor& selectionCol2 = QColor(255,255,255),
                 QGraphicsItem* parent = nullptr);
        ~CustomShapeRect();

        void setPaintFunction(std::function<void (QPainter*, const QRectF&)> paintFunction);

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    private:
        std::function<void(QPainter*, const QRectF&)> m_paintFunction;
    };

    class IconAndLabelItem : public GraphicsItemBD{
    public:
        explicit IconAndLabelItem(const QString& pixmapPath,
                                  bool loadAsFilIcon,
                                  const QString& labelTxt,
                                   const QSize& size = QSize(0,0),
                                   const QPoint& pos = QPoint(0,0),
                                   QGraphicsItem* parent = nullptr);
        ~IconAndLabelItem();

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
        void setSelected(bool selected);
        bool selected();
        void setOnClicked(std::function<void()> onClcked);
        void setSelectable(bool selectable);
        void setTextColor(QColor textCol);
    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        void hoverMoveEvent(QGraphicsSceneHoverEvent * event);
    private:
        QString m_pixmapPath;
        bool m_loadAsFileIcon;
        QString m_labelTxt;

        int m_paddingX = 10;
        int m_paddingY = 0;

        bool m_selectable = true;

        bool m_selected = false;
        bool m_hover = false;

        QColor m_textCol = QColor(0,0,0, 255);
        QPixmap m_pixmap;

        std::function<void()> m_onClicked = nullptr;
    };
}

#endif // CUSTOMGRAPHICITEMS_H

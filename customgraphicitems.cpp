#include "customgraphicitems.h"

GraphicItemsBD::GraphicsItemBD::GraphicsItemBD(const QSize& size,
               const QPoint& pos,
               QGraphicsItem* parent)
    : QGraphicsItem(parent),
      m_size(size),
      m_pos(pos)
{
    setAcceptHoverEvents(true);
}

GraphicItemsBD::GraphicsItemBD::~GraphicsItemBD()
{
//    qDebug() << "in GraphicsItemBD.DEstructor";
}

void GraphicItemsBD::GraphicsItemBD::setSize(QSize size){
    m_size = size;
    revalidate = true;
    update();
}
void GraphicItemsBD::GraphicsItemBD::setPosition(QPoint position){
    m_pos = position;
    revalidate = true;
    update();
}
QRectF GraphicItemsBD::GraphicsItemBD::boundingRect() const {
    return QRectF(m_pos, m_size);
}

void GraphicItemsBD::GraphicsItemBD::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
//    qDebug() << "paint-GraphicsItemBD";
//    painter->end();
}


GraphicItemsBD::ButtonBD::ButtonBD(const QSize& size,
         const QPoint& pos,
         const QColor& gradCol1,
         const QColor& gradCol2,
         const QColor& selectionCol1,
         const QColor& selectionCol2,
         QGraphicsItem* parent)
    : GraphicsItemBD(size, pos, parent),
      m_gradCol1(gradCol1),
      m_gradCol2(gradCol2),
      m_selectCol1(selectionCol1),
      m_selectCol2(selectionCol2)
{
    setAcceptHoverEvents(true);
//        revalidateSize();
}

GraphicItemsBD::ButtonBD::~ButtonBD()
{
//    qDebug() << "in ButtonBD.DEstructor";
    m_callFunc = nullptr;
}


void GraphicItemsBD::ButtonBD::revalidateSize(float width, float height)//float paddingX = 2, float paddingY = 2)
{
    setSize( QSize(width, height) );
    revalidate = true;
    update();
}

void GraphicItemsBD::ButtonBD::setCallFunction(std::function<void()> callFunc){
    m_callFunc = callFunc;
}

void GraphicItemsBD::ButtonBD::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    Q_UNUSED(widget)
    Q_UNUSED(option)

    GraphicItemsBD::GraphicsItemBD::paint(painter, option, widget);

//    qDebug() << "paint-ButtonBD";

    // gaaanz wichtig: erstmal painter an boundingRect clippen!!!:
    QRectF rct = boundingRect();

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setClipRect(rct);

    QColor grad1, grad2;
    if(mouIn){
        grad1 = m_selectCol1;
        grad2 = m_selectCol2;
    }else{
        grad1 = m_gradCol1;
        grad2 = m_gradCol2;
    }

    StaticFunctions::paintRect(painter, rct, grad1, grad2);

//    painter->end();
}
void GraphicItemsBD::ButtonBD::mousePressEvent(QGraphicsSceneMouseEvent *event){
    if(m_callFunc)
        m_callFunc();

//    emit clicked();

    QGraphicsItem::mousePressEvent(event);
}
void GraphicItemsBD::ButtonBD::hoverEnterEvent(QGraphicsSceneHoverEvent *event){
    Q_UNUSED(event)
    mouIn = true;
    update();
}
void GraphicItemsBD::ButtonBD::hoverLeaveEvent(QGraphicsSceneHoverEvent *event){
    Q_UNUSED(event)
    mouIn = false;
    update();
}
void GraphicItemsBD::ButtonBD::hoverMoveEvent(QGraphicsSceneHoverEvent * event){
    Q_UNUSED(event)
}


GraphicItemsBD::TextRect::TextRect(QString str,
         const QSize& size,
         const QPoint& pos,
         const QColor& gradCol1,
         const QColor& gradCol2,
         const QColor& selectionCol1,
         const QColor& selectionCol2,
         const QColor& textCol,
         QGraphicsItem* parent)
    : ButtonBD(size, pos, gradCol1, gradCol2, selectionCol1, selectionCol2, parent),
      m_str(str),
      m_textCol(textCol),
      m_font(StaticFunctions::getGoshFont())
{
//        setAcceptHoverEvents(true); // wird in base-class ButtonBD schon gemacht
    revalidateSize();
}
GraphicItemsBD::TextRect::~TextRect()
{
//    qDebug() << "in TextRect.DEstructor";
}

void GraphicItemsBD::TextRect::setFont(const QFont &font)
{
    m_font = font;
}

void GraphicItemsBD::TextRect::setText(QString str, int paddingX, int paddingY)
{
    m_str = str;
    revalidateSize(paddingX, paddingY);
}

void GraphicItemsBD::TextRect::revalidateSize(float paddingX, float paddingY)
{
    QFontMetrics fm(m_font);
    int width = fm.width(m_str) + paddingX;
    int height = fm.height() + paddingY;
    setSize( QSize(width, height) );
    revalidate = true;
    update();
}

void GraphicItemsBD::TextRect::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    Q_UNUSED(widget)
    Q_UNUSED(option)

//    qDebug() << "paint-TextRect";

    ButtonBD::paint(painter, option, widget);

    auto rct = boundingRect();

    StaticFunctions::paintTextRect(painter, m_str,
                  rct,
                  QColor(0,0,0, 0), QColor(0,0,0, 0),
                  m_textCol,
                  m_font);


//    painter->end();
}

GraphicItemsBD::PixmapRect::PixmapRect(QPixmap pixmap,
         const QSize& size,
         const QPoint& pos,
         const QColor& selGradient1,
         const QColor& selGradient2,
         const QColor& selectionCol1,
         const QColor& selectionCol2,
         QGraphicsItem* parent)
    : ButtonBD(size, pos, selGradient1, selGradient2, selectionCol1, selectionCol2, parent),
      m_pixmap(pixmap)
{}

GraphicItemsBD::PixmapRect::~PixmapRect()
{
//    qDebug() << "in PixmapRect.DEstructor";
}

void GraphicItemsBD::PixmapRect::setPixmap(QPixmap pixmap)
{
    m_pixmap = pixmap;
}

void GraphicItemsBD::PixmapRect::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    Q_UNUSED(widget)
    Q_UNUSED(option)

//    qDebug() << "paint-PixmapRect";

    ButtonBD::paint(painter, option, widget);

    auto frameBounds = boundingRect();

    StaticFunctions::paintPixmapTranspRect(painter, m_pixmap, frameBounds);

//    painter->end();
}
//}

GraphicItemsBD::CustomShapeRect::CustomShapeRect(std::function<void (QPainter*, const QRectF&)> paintFunction,
                                                 const QSize &size,
                                                 const QPoint &pos,
                                                 const QColor &selGradient1, const QColor &selGradient2,
                                                 const QColor &selectionCol1, const QColor &selectionCol2,
                                                 QGraphicsItem *parent)
    : ButtonBD(size, pos, selGradient1, selGradient2, selectionCol1, selectionCol2, parent),
      m_paintFunction(paintFunction)
{
}

GraphicItemsBD::CustomShapeRect::~CustomShapeRect()
{
    m_paintFunction = nullptr;
}

void GraphicItemsBD::CustomShapeRect::setPaintFunction(std::function<void (QPainter*, const QRectF&)> paintFunction)
{
    m_paintFunction = paintFunction;
}

void GraphicItemsBD::CustomShapeRect::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    ButtonBD::paint(painter, option, widget);

    auto frameBounds = boundingRect();

    if(m_paintFunction)
        m_paintFunction(painter, frameBounds);
}


GraphicItemsBD::IconAndLabelItem::IconAndLabelItem(const QString &pixmapPath, bool loadAsFilIcon,
                                                   const QString &labelTxt,
                                                   const QSize &size,
                                                   const QPoint &pos,
                                                   QGraphicsItem *parent)
    : GraphicsItemBD(size, pos, parent),
      m_pixmapPath(pixmapPath),
      m_labelTxt(labelTxt),
      m_loadAsFileIcon(loadAsFilIcon)
{
}

GraphicItemsBD::IconAndLabelItem::~IconAndLabelItem()
{
    m_onClicked = nullptr;
}

void GraphicItemsBD::IconAndLabelItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRectF rct = boundingRect();

    int padding = 10;

    int pixmapEdge = rct.height();


    QColor backgrCol(255,255,255, 255);
    if(m_selectable)
    {
        if(m_selected && m_hover)
        {
            backgrCol = QColor(0,255,255, 255);
        }
        else if(m_selected)
        {
            backgrCol = QColor(0,255,0, 255);
        }else if(m_hover)
        {
            backgrCol = QColor(0,200,255, 255);
        }
    }

    painter->setPen(QPen(QColor(0,0,0, 0)));
    painter->setFont(StaticFunctions::getGoshFont());
    QFontMetrics fm(StaticFunctions::getGoshFont());
    int fontTextHeight = fm.height();

    painter->setBrush(QBrush(backgrCol));
    painter->drawRect(rct);
    painter->setBrush(QBrush(QColor(255,255,255, 255)));

    QRectF pixmapRect(rct.x() + m_paddingX, rct.y() + m_paddingY,
                      pixmapEdge, pixmapEdge-2*m_paddingY);
    QRectF labelRect(pixmapRect.right() + padding,
                     rct.center().y() - fontTextHeight*0.5,
                     rct.width() -pixmapRect.width() - padding - 2*m_paddingX,
                     fontTextHeight);

    QPixmap pixmap;
    if(m_loadAsFileIcon)
        pixmap = StaticFunctions::getFilePixmap(m_pixmapPath, QSize(pixmapEdge,pixmapEdge));
    else
    {
        pixmap = QPixmap(m_pixmapPath);
    }

    pixmap = pixmap.scaled(QSize(pixmapEdge, pixmapEdge));
    painter->drawPixmap(pixmapRect,
                        pixmap,
                        pixmap.rect());
    painter->setPen(QPen(QColor(0,0,0, 255)));
    painter->setPen(QPen(m_textCol));
    painter->drawText(labelRect,
                      m_labelTxt);

}

void GraphicItemsBD::IconAndLabelItem::setSelected(bool selected)
{
    m_selected = selected;
    update();
}

bool GraphicItemsBD::IconAndLabelItem::selected()
{
    return m_selected;
}

void GraphicItemsBD::IconAndLabelItem::setOnClicked(std::function<void()> onClcked)
{
    m_onClicked = onClcked;
}

void GraphicItemsBD::IconAndLabelItem::setSelectable(bool selectable)
{
    m_selectable = selectable;
    update();
}

void GraphicItemsBD::IconAndLabelItem::setTextColor(QColor textCol)
{
    m_textCol = textCol;
    update();
}

void GraphicItemsBD::IconAndLabelItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);

    if(!m_selectable)
        return;

    if(m_onClicked)
        m_onClicked();
    m_selected = true;
}

void GraphicItemsBD::IconAndLabelItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);

    if(!m_selectable)
        return;

    m_hover = true;
    update();
}

void GraphicItemsBD::IconAndLabelItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);

    if(!m_selectable)
        return;

    m_hover = false;
    update();
}

void GraphicItemsBD::IconAndLabelItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
}

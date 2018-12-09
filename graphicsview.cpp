#include "graphicsview.h"

class SearchMenuBD : public GraphicItemsBD::GraphicsItemBD{
public:
    SearchMenuBD(const QSize& size = QSize(0,0),
                 const QPoint& pos = QPoint(0,0),
                 QGraphicsItem* parent = nullptr)
        : GraphicItemsBD::GraphicsItemBD(size, pos, parent)
    {}
    ~SearchMenuBD(){
//        qDebug() << "in SearchMenuBD.DEstructor";
        m_caller.reset();
    }
    void setCaller(std::shared_ptr<DynamicFunctionCaller<QString, std::function<QString()>>> caller){
        this->m_caller = caller;
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
        Q_UNUSED(widget)
        Q_UNUSED(option)

        // gaaanz wichtig: erstmal painter an boundingRect clippen!!!:
        QRectF rct = boundingRect();

        painter->setRenderHint(QPainter::Antialiasing, true);

        painter->setClipRect(rct);

        QColor gradCol1(255,255,255, 200);
        QColor gradCol2(255,255,255, 200);
        StaticFunctions::paintRect(painter, rct, gradCol1, gradCol2);

        if(revalidate){
            float prevBtnWidth = (float)qMin((float)(rct.width() * 0.2), (float)80.);
            float prevBtnHeight = rct.height() * 0.5;
            float offs = 20;
            float prevBtnX = rct.center().x() - prevBtnWidth - offs;
            float nextBtnX = rct.center().x() + offs;
            float upBtnY = 3 + rct.y();
            m_preBtn = QRectF(prevBtnX, upBtnY, prevBtnWidth, prevBtnHeight);
            m_nextBtn = QRectF(nextBtnX, upBtnY, prevBtnWidth, prevBtnHeight);

            float closeBtnWidth = (float)qMin((float)(rct.width() * 0.5), (float)40.);
            float closeBtnHeight = rct.height() * 0.4;
            offs = rct.width() * 0.05;
            float closeBtnX = rct.right()-closeBtnWidth-offs;
            float closeBtnY = rct.center().y()-(float)(closeBtnHeight*0.5);
            m_closeBtn = QRectF(closeBtnX,
                                closeBtnY,
                                closeBtnWidth,
                                closeBtnHeight);
        }
        QColor nextColor1 = QColor(255, 255, 255, 255),
               nextColor2 = QColor(255, 255, 255, 255),
               prevColor1 = QColor(255, 255, 255, 255),
               prevColor2 = QColor(255, 255, 255, 255),
               closColor1 = QColor(255, 255, 255, 255),
               closColor2 = QColor(255, 255, 255, 255);
        if(mouInNextBtn){
            nextColor1 = QColor(255, 255, 255, 255);
            nextColor2 = QColor(200, 200, 200, 255);
        }else if (mouInPrevBtn){
            prevColor1 = QColor(255, 255, 255, 255);
            prevColor2 = QColor(200, 200, 200, 255);
        }else if (mouInCloseBtn){
            closColor1 = QColor(255, 180, 180, 255);
            closColor2 = QColor(255, 0, 0, 255);
        }

        QString curSearchId = m_caller->getFunction("getCurrentSearchIndex")();
        QString searchCount = m_caller->getFunction("getSearchResultsCount")();
        QString indexStr = QString("%1/%2").arg(curSearchId).arg(searchCount);


        QFont font = StaticFunctions::getGoshFont(7, QFont::Normal);
        QFontMetrics fm(font);
        int indxWidth = fm.width(indexStr);
        int indxHeight = fm.height();
        int indxPadding = 2;

        int indxCentX = int((m_preBtn.center().x() + m_nextBtn.center().x()) * 0.5);
        int indxLeft = indxCentX - int(indxWidth*0.5);
        int indxY = m_preBtn.top() + indxPadding;

        QRect indxRct = QRect( indxLeft,
                               indxY,
                               indxWidth,
                               indxHeight );
        painter->setFont(font);
        painter->drawText(indxRct, indexStr);

        StaticFunctions::paintRect(painter, m_nextBtn,
                  nextColor1,
                  nextColor2);
        StaticFunctions::paintArrowDown(painter, m_nextBtn);
        StaticFunctions::paintRect(painter, m_preBtn,
                  prevColor1,
                  prevColor2);
        StaticFunctions::paintArrowUp(painter, m_preBtn);

        StaticFunctions::paintRect(painter, m_closeBtn,
                  closColor1,
                  closColor2);
        StaticFunctions::paintCrossRect(painter, m_closeBtn);

        if(m_caller){
            QString str = m_caller->getFunction("getSearchResult")();
            if(str.isEmpty()){
                str = QString("no match!");
            }
             str.prepend("--").append("--");

            QFont font = StaticFunctions::getGoshFont(12);
            painter->setFont(font);
            QFontMetrics fm(font);
            int pixelsWidth = fm.width(str);
            int pixelsHeight = fm.height();
            int offsTxt = 2;
            painter->setPen(QPen(QColor(0,0,0), 3, Qt::SolidLine));
            painter->drawText(QRect(rct.center().x()-pixelsWidth*0.5,
                                    rct.bottom() - pixelsHeight -offsTxt,
                                    pixelsWidth*2,
                                    pixelsHeight),
                                    str);
        }

        revalidate = false;

//        painter->end();
    }
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event){
        QPointF mouP = event->pos();
        if(       m_nextBtn.contains(mouP) && m_caller){
            m_caller->getFunction(QString("next"))();
        }else if (m_preBtn.contains(mouP) && m_caller){
            m_caller->getFunction(QString("previous"))();
        }else if (m_closeBtn.contains(mouP) && m_caller){
//            qDebug() << "SearchMenu close-Btn pressed!";
            m_caller->getFunction(QString("close"))();
        }
        qint64 curTime = QDateTime::currentMSecsSinceEpoch();
        if(curTime - lastTmePrsd < 300){

        }
        lastTmePrsd = curTime;
        update();
    }
    void hoverMoveEvent(QGraphicsSceneHoverEvent * event){
        QPointF mouP = event->pos();
        if(       m_nextBtn.contains(mouP)){
            if(!mouInNextBtn){
                mouInNextBtn = true;
                update();
                return;
            }
        }else if (m_preBtn.contains(mouP)){
            if(!mouInPrevBtn){
                mouInPrevBtn = true;
                update();
                return;
            }
        }else if (m_closeBtn.contains(mouP)){
            if(!mouInCloseBtn){
                mouInCloseBtn = true;
                update();
                return;
            }
        }else if(mouInNextBtn || mouInPrevBtn || mouInCloseBtn){
            mouInNextBtn = false;
            mouInPrevBtn = false;
            mouInCloseBtn = false;
            update();
            return;
        }
//        return QGraphicsItem::hoverEnterEvent(event);
    }
private:
    QRectF m_nextBtn;
    QRectF m_preBtn;
    QRectF m_closeBtn;

    bool mouInNextBtn = false;
    bool mouInPrevBtn = false;
    bool mouInCloseBtn = false;

    std::shared_ptr<DynamicFunctionCaller<QString, std::function<QString()>>> m_caller = std::shared_ptr<DynamicFunctionCaller<QString, std::function<QString()>>>();

    qint64 lastTmePrsd = Q_INT64_C(0);
};
class ElapseMenuBD : public GraphicItemsBD::GraphicsItemBD{
public:
    ElapseMenuBD(int buttonCount,
                 int columnWidth,
                 const QSize& size = QSize(0,0),
                 const QPoint& pos = QPoint(0,0),
                 QGraphicsItem* parent = nullptr)
        : m_buttonCount(buttonCount),
          m_colWidth(columnWidth),
          GraphicItemsBD::GraphicsItemBD(size, pos, parent)
    {}
    ~ElapseMenuBD(){
        m_caller.reset();
    }
    void setCaller(std::shared_ptr<DynamicFunctionCaller<QString, std::function<bool(int)>>> caller){
        this->m_caller = caller;
    }

    void setBackroundColor(QColor grad1, QColor grad2)
    {
        m_grad1 = grad1;
        m_grad2 = grad2;
        update();
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
        Q_UNUSED(widget)
        Q_UNUSED(option)

        // gaaanz wichtig: erstmal painter an boundingRect clippen!!!:
        QRectF rct = boundingRect();

        painter->setRenderHint(QPainter::Antialiasing, true);

        painter->setClipRect(rct);

        StaticFunctions::paintRect(painter, rct, m_grad1, m_grad2);

        if(revalidate){
            int xOffs = m_pos.x();
            int yOffs = m_pos.y();
            int buttonHeight = m_size.height();
            int buttonWidth = m_colWidth;
            for(int i=0; i < m_buttonCount; i++){
                QRectF rect(xOffs, yOffs, buttonWidth, buttonHeight);
                m_buttons.append(rect);
                xOffs += buttonWidth;
            }
        }
        QColor basicColor1 = QColor(255, 255, 255, 255),
               basicColor2 = QColor(200, 0, 0, 255),
               hoverColor1 = QColor(255, 0, 0, 255),
               hoverColor2 = QColor(150, 0, 0, 255),
               elapsedColor1 = QColor(255, 255, 255, 255),
               elapsedColor2 = QColor(0, 150, 0, 255),
               hoverElapsedColor1 = QColor(255, 255, 255, 255),
               hoverElapsedColor2 = QColor(0, 100, 0, 255);
        QColor col1, col2;
        int fontSize, fontWeight;
        for(int i=0; i < m_buttons.size(); i++){
            if(m_mouInButtonId == i){
                if(m_caller->containsFunction(QString("elapsed")) &&
                                     m_caller->getFunction(QString("elapsed"))(i)) {
                    col1 = hoverElapsedColor1;
                    col2 = hoverElapsedColor2;
                }else{
                    col1 = hoverColor1;
                    col2 = hoverColor2;
                }
                fontSize = 9;
                fontWeight = QFont::Bold;
            }else{
                if(m_caller->containsFunction(QString("elapsed")) &&
                     m_caller->getFunction(QString("elapsed"))(i)) {
                    col1 = elapsedColor1;
                    col2 = elapsedColor2;
                }else{
                    col1 = basicColor1;
                    col2 = basicColor2;
                }
                fontSize = 9;
                fontWeight = QFont::Normal;
            }
            StaticFunctions::paintTextRect(painter,
                                           QString("%1").arg(i+1),
                                           m_buttons[i], col1, col2,
                                           QColor(255,255,255,255),
                                           StaticFunctions::getGoshFont(fontSize, fontWeight));
        }

        revalidate = false;

//        painter->end();
    }
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event){
        QPointF mouP = event->pos();
        for(int i=0; i < m_buttons.size(); i++){
            if(m_buttons[i].contains(mouP) && m_caller->containsFunction(QString("call"))){
                m_caller->getFunction(QString("call"))(i);
            }
        }
        qint64 curTime = QDateTime::currentMSecsSinceEpoch();
        if(curTime - lastTmePrsd < 300){

        }
        lastTmePrsd = curTime;
        update();
    }
    void hoverMoveEvent(QGraphicsSceneHoverEvent * event){
        QPointF mouP = event->pos();
        int hoverId = -1;
        for(int i=0; i < m_buttons.size(); i++){
            if(m_buttons[i].contains(mouP)){
                hoverId = i;
                break;
            }
        }
        if(hoverId != m_mouInButtonId){
            m_mouInButtonId = hoverId;
            update();
        }
//        return QGraphicsItem::hoverEnterEvent(event);
    }
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event){
        m_mouInButtonId = -1;
        update();
    }
private:
    QVector<QRectF> m_buttons;
    int m_mouInButtonId = -1;
    int m_colWidth;
    int m_buttonCount;

    QColor m_grad1 = QColor(0,0,0, 255);
    QColor m_grad2 = QColor(0,0,0, 255);

    std::shared_ptr<DynamicFunctionCaller<QString, std::function<bool(int)>>> m_caller = std::shared_ptr<DynamicFunctionCaller<QString, std::function<bool(int)>>>();

    qint64 lastTmePrsd = Q_INT64_C(0);
};
class WindowSelector : public GraphicItemsBD::GraphicsItemBD{
public:
    WindowSelector(const QSize& size,
                   const QPoint& pos,
                   QGraphicsItem *parent = nullptr)
        : GraphicItemsBD::GraphicsItemBD(size, pos, parent)
    {
        rects.append(nullptr);
        rects.append(nullptr);
        rects.append(nullptr);
        rects.append(nullptr);
    }
    ~WindowSelector(){
        foreach(QRect* rct, rects){
            delete rct;
        }
        rects.clear();
    }


    void setCaller(std::function<void(int)> caller){
        this->caller = caller;
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
        Q_UNUSED(widget)
        Q_UNUSED(option)

        // gaaanz wichtig: erstmal painter an boundingRect clippen!!!:
        QRectF rct = boundingRect();

        painter->setRenderHint(QPainter::Antialiasing, true);

        painter->setClipRect(rct);

        QLinearGradient gradient(rct.topLeft(), rct.bottomRight());
        gradient.setColorAt(0, QColor(255,255,255, 200));
        gradient.setColorAt(1, QColor(220,220,255, 200));
        painter->fillRect(rct, gradient);

        if(m_anzRects == 1){
            if(!rects[0]){
                float fctr = 0.8;
                float wndwWdth = (float)rct.width()*fctr;
                float xStart = rct.x() + (rct.width()-wndwWdth)*0.5;
                float yStart = rct.y() + (rct.height()-wndwWdth)*0.5;
                QRect* wndwRct = new QRect(xStart,yStart, wndwWdth,wndwWdth);

                rects[0] = wndwRct;
            }
        }else if(m_anzRects == 2){
            float fctr = 0.9;
            float rctWidth = ((float)rct.width()) *0.4;
            float offs = ((float)rct.width())*(1.0-fctr)*0.5;

            if(horizontal){
                painter->drawLine(rct.left()+offs,
                                  rct.center().y(),
                                  rct.right()-offs,
                                  rct.center().y());
                if(!rects[0]){
                    QSize size(rctWidth, rctWidth);
                    QPoint p1, p2;
                    p1 = QPoint(rct.center().x()-rctWidth*0.5,
                              rct.top()+offs);
                    p2 = QPoint(rct.center().x()-rctWidth*0.5,
                              rct.bottom()-offs-rctWidth);
                    rects[0] = new QRect(p1, size);
                    rects[1] = new QRect(p2, size);
                }
            }else{
                painter->drawLine(rct.center().x(),
                                  rct.top()+offs,
                                  rct.center().x(),
                                  rct.bottom()-offs);

                if(!rects[0]){
                    QSize size(rctWidth, rctWidth);
                    QPoint p1, p2;
                    p1 = QPoint(rct.left()+offs,
                              rct.center().y()-rctWidth*0.5);
                    p2 = QPoint(rct.right()-offs-rctWidth,
                              rct.center().y()-rctWidth*0.5);
                    rects[0] = new QRect(p1, size);
                    rects[1] = new QRect(p2, size);
                }
            }  
        }else if(m_anzRects == 3){
            float fctr = 0.9;
            float rctWidth = ((float)rct.width()) *0.4;
            float offs = ((float)rct.width())*(1.0-fctr)*0.5;
            if(horizontal){
                painter->drawLine(rct.left()+offs,
                                  rct.center().y(),
                                  rct.right()-offs,
                                  rct.center().y());
                painter->drawLine(rct.center().x(),
                                  rct.center().y(),
                                  rct.center().x(),
                                  rct.bottom()-offs);

                if(!rects[0]){
                    QSize size(rctWidth, rctWidth);
                    QPoint p1, p2, p3;
                    p1 = QPoint(rct.center().x()-rctWidth*0.5,
                              rct.top()+offs);
                    p2 = QPoint(rct.left()+offs,
                              rct.bottom()-offs-rctWidth);
                    p3 = QPoint(rct.right()-offs-rctWidth,
                              rct.bottom()-offs-rctWidth);

                    rects[0] = new QRect(p1, size);
                    rects[1] = new QRect(p2, size);
                    rects[2] = new QRect(p3, size);
                }
            }else{
                painter->drawLine(rct.center().x(),
                                  rct.top()+offs,
                                  rct.center().x(),
                                  rct.bottom()-offs);
                painter->drawLine(rct.center().x(),
                                  rct.center().y(),
                                  rct.right()-offs,
                                  rct.center().y());
                if(!rects[0]){
                    QSize size(rctWidth, rctWidth);
                    QPoint p1, p2, p3;
                    p1 = QPoint(rct.left()+offs,
                              rct.center().y()-rctWidth*0.5);
                    p2 = QPoint(rct.right()-offs-rctWidth,
                              rct.top()+offs);
                    p3 = QPoint(rct.right()-offs-rctWidth,
                              rct.bottom()-offs-rctWidth);

                    rects[0] = new QRect(p1, size);
                    rects[1] = new QRect(p2, size);
                    rects[2] = new QRect(p3, size);
                }
            }
        }else{ // == 4
            float fctr = 0.9;
            float rctWidth = ((float)rct.width()) *0.4;
            float offs = ((float)rct.width())*(1.0-fctr)*0.5;

            painter->drawLine(rct.left()+offs,
                              rct.center().y(),
                              rct.right()-offs,
                              rct.center().y());
            painter->drawLine(rct.center().x(),
                              rct.top()+offs,
                              rct.center().x(),
                              rct.bottom()-offs);
            if(!rects[0]){
                    QSize size(rctWidth, rctWidth);
                    QPoint p1, p2, p3, p4;
                    p1 = QPoint(rct.left()+offs,
                              rct.top()+offs);
                    p2 = QPoint(rct.right()-offs-rctWidth,
                              rct.top()+offs);
                    p3 = QPoint(rct.left()+offs,
                              rct.bottom()-offs-rctWidth);
                    p4 = QPoint(rct.right()-offs-rctWidth,
                              rct.bottom()-offs-rctWidth);
                rects[0] = new QRect(p1, size);
                rects[1] = new QRect(p2, size);
                rects[2] = new QRect(p3, size);
                rects[3] = new QRect(p4, size);
            }
            painter->drawRect(*rects[0]);
            painter->drawRect(*rects[1]);
            painter->drawRect(*rects[2]);
            painter->drawRect(*rects[3]);
        }
        for(int i=0; i < m_anzRects; i++){
            StaticFunctions::paintRect(painter, *rects[i]);
        }
        if(mouseRectId > -1 && rects[mouseRectId]){
            StaticFunctions::paintCrossRect(painter, *rects[mouseRectId]);
        }

//        painter->end();
    }
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event){
        QPointF mouP = event->pos();
        for(int i=0; i < m_anzRects; i++){
            if(rects[i] && caller
                    && rects[i]->contains(QPoint(mouP.x(), mouP.y()))){
                caller(i);
            }
        }
        qint64 curTime = QDateTime::currentMSecsSinceEpoch();
        if(curTime - lastTmePrsd < 300){

        }
        lastTmePrsd = curTime;
        isPressed = true;
        update();
    }
    void hoverMoveEvent(QGraphicsSceneHoverEvent * event){
        m_hover = true;
        QPointF mouP = event->pos();
        mouseRectId = -1;
        for(int i=0; i < m_anzRects; i++){
            if(rects[i] && rects[i]->contains(QPoint(mouP.x(), mouP.y()))){
                mouseRectId = i;
                break;
            }
        }
        update();
        return QGraphicsItem::hoverEnterEvent(event);
    }
private:
    bool m_hover = false;

    bool horizontal = true;

    int m_anzRects = 2;

    bool isPressed = false;
    qint64 lastTmePrsd = Q_INT64_C(0);

    QVector<QRect*> rects;
    int mouseRectId = -1;

    std::function<void(int)> caller;
};

GraphicsView::GraphicsView(int hBarValue,
                           int vBarValue,
                           int zoomFactor,
                           QWidget* parent)
    : QGraphicsView(parent),
      m_isLoading(false),
      m_loadingId(0),
      m_animationTimer(new QTimer(this)),
      m_fontSize(zoomFactor)
{
    qDebug() << "in GraphcisView-Constructor";

    revalidateRowHeight();

    connect(m_animationTimer, &QTimer::timeout, [=](){
//        qDebug() << "running in timer: loadingId: " << loadingId;
        int id = m_loadingId;
        m_loadingId = (id+1) % m_loadingLength;

        QString cursor_pixmap_path = QString("%1%2%3")
                .arg("pics")
                .arg(QDir::separator())
                .arg("MrSoirIcon_cursor.png");
        QPixmap cursor_pixmap = QPixmap(cursor_pixmap_path);

        QTransform trans;
        qreal id_rl = (qreal)id;
        qreal pi = 3.14159265;
        qreal value = (id_rl / ((qreal)m_loadingLength)) * (2*pi);
        qreal scaleFactor = (sin(value) +1) * 0.3;

        trans.rotate(id * 4);
        trans.scale(1.0 + scaleFactor, 1.0 + scaleFactor);

        cursor_pixmap = cursor_pixmap.transformed(trans);
        setCursor(QCursor(cursor_pixmap,
                          (int)(-((qreal)cursor_pixmap.size().width())*0.5),
                          (int)(-((qreal)cursor_pixmap.size().height())*0.5)));

        rePaintCanvas();
    });


    // wichtig: damit die scene immer ganz oben beginnt!
    this->setAlignment(Qt::AlignTop);

//    this->setContentsMargins(0, 0, 0, 0);

    this->setScene(&m_scene);

    this->verticalScrollBar();
    connect(this->verticalScrollBar(),   &QScrollBar::valueChanged,  this, &GraphicsView::vScrollValueChanged);
    connect(this->horizontalScrollBar(), &QScrollBar::valueChanged,  this, &GraphicsView::hScrollValueChanged);

    setHBarValue(hBarValue);
    setVBarValue(vBarValue);
    rePaintCanvas();

//    qDebug() << "in GraphcisView-Constructor-finished";
}

GraphicsView::~GraphicsView()
{
    qDebug() << "in GrphicsView.DEstructor";
    if(m_mouseP != nullptr)
        delete m_mouseP;

    m_scene.clear();

    if(m_animationTimer != nullptr &&
            m_animationTimer->isActive())
        m_animationTimer->stop();
    m_animationTimer->deleteLater();

    delete m_upperRect;

//    qDebug() << "   -> GraphicsView.DEstructor finished";
}

void GraphicsView::revalidate(){
    rePaintCanvas();
}

int GraphicsView::getFirstToDispFi(){
    qreal yOffs = getViewportYOffset();

    int firstToDispFi = (yOffs / m_rowHeight) -m_filePuffer;
    if(firstToDispFi < 0)
        firstToDispFi = 0;
    return firstToDispFi;
}
int GraphicsView::getLastToDispFi(){
    qreal vwPrtHght = getDisplayableHeight();

    int lastToDispFi = getFirstToDispFi() + (vwPrtHght / m_rowHeight) + m_filePuffer*2.0;
    if(lastToDispFi >= m_fileCount)
        lastToDispFi = m_fileCount-1;
    return lastToDispFi;
}
bool GraphicsView::viewPortOutOfDisplayedRange(){
    int newFirstToDispFi = getFirstToDispFi();
    int newLastToDispFi = getLastToDispFi();
    int curFirst = m_firstDispFI;
    int curLast =  m_lastDispFI;

    if(curFirst < 0)
        curFirst = 0;
    if(curLast >= m_fileCount)
        curLast  = m_fileCount-1;

//    qDebug() << "newFirstToDispFi: " << newFirstToDispFi
//             << "    m_firstDispFI: " << m_firstDispFI
//             << "   newLastToDispFi: " << newLastToDispFi
//             << "   curLast: " << curLast
//             << "   viewPortOutOfDisplayedRange: " <<
//                (newFirstToDispFi < curFirst ||
//                 newLastToDispFi  > curLast);
    return newFirstToDispFi < curFirst ||
            newLastToDispFi  > curLast;
}

qreal GraphicsView::getDisplayableHeight()
{
    return this->viewport()->height()-m_elapseBarHeight;
}

void GraphicsView::vScrollValueChanged(int newValue)
{
//    qDebug() << "in vScrollValueChanged -> viewPortOutOfDisplayedRange: " << viewPortOutOfDisplayedRange();

    bool revalidate = false;
    if( viewPortOutOfDisplayedRange() ){
        revalidate = true;
    }
    if(!revalidate && m_graphicsGroup){
        m_graphicsGroup->setY(newValue);
    }
    if(revalidate)
        rePaintCanvas();
}

void GraphicsView::hScrollValueChanged(int newValue)
{
    if(m_graphicsGroup){
        m_graphicsGroup->setX(newValue);
    }
}

//void GraphicsView::setWaitingAnimation()
//{
//    if(m_animationTimer && m_animationTimer->isActive()){
//        killWaitingAnimation();
//    }else{
//        startWaitingAnimation();
//    }
//}
void GraphicsView::startWaitingAnimation()
{
    this->closeAllSubMenus();

    m_isLoading = true;
    m_animationTimer->start(m_loadingPeriodMS);
}
void GraphicsView::killWaitingAnimation()
{
    if(m_animationTimer && m_animationTimer->isActive()){
        m_isLoading = false;
//        qDebug() << "stopping timer";
        this->setCursor(QCursor(Qt::ArrowCursor));
        m_animationTimer->stop();
        update();
    }
}

void GraphicsView::closeAllSubMenus(){
    m_paintMenuBar = false;
    m_paintContBar = false;

    closeSearchMenu();
}

std::shared_ptr<FilesCoordinator> GraphicsView::lockFilesCoordinator()
{
    // derzeit ist lockFilesCoordinator() eig. noch eine sinnlose methode.
    // aber: da das locking von m_filesCoor.lock() hier und nur hier zentral ausgefuhert wird,
    // kann das locking einfacher ueberwacht werden und unter bestimmten szenarien zukuenftig leicht
    // unterbunden werden:

    // if(m_isLoading)... ist hier ausgeklammert, da das blocken der nicht-const-funktionsaufrufe
    // auf den m_filescoorinator in executeAction(function<void(shared_ptr<>)>) geblockt wird, wenn
    // z.B. gerade im Hintergrund etwas geladen wird:
//    if(m_isLoading)
//        return nullptr;

    return m_filesCoord.lock();
}



void GraphicsView::EnterPressedBD(QKeyEvent* event)
{
    Q_UNUSED(event);

    if( inSearchMode() )
        emit nextSearchResult();
    else
        emit openSelection();

//    executeFileAction([=](auto locked){
//        if(locked->inSearchMode()){
//            locked->nextSearchResult();
//        }else{
//            locked->openSelection();
//        }
//    });
}

void GraphicsView::keyPressEvent(QKeyEvent *event)
{

    bool contrlPrsd = StaticFunctions::controlPressed() || event->key() == Qt::Key_Control;
    bool shiftPrsd = StaticFunctions::shiftPressed() || event->key() == Qt::Key_Shift;
//    bool altPrsd = StaticFunctions::altPressed() || event->key() == Qt::Key_Alt || event->key() == Qt::Key_AltGr; // wird bisher nicht gebraucht

//    qDebug() << "in keyPressed: controlPressed: " << contrlPrsd << "  shiftPressed: " << shiftPrsd;

    if(event->key() == Qt::Key_Escape){
        if(inSearchMode() || m_paintMenuBar || m_paintContBar){
            closeAllSubMenus();
            update();
        }else{
            emit clearSelection();
//            executeFileAction([](auto locked){
//                locked->clearSelection();
//            });
        }
    }else if (event->key() == Qt::Key_Enter ||
              event->key() == Qt::Key_Return){
        EnterPressedBD(event);
    }else if(event->key() == Qt::Key_Backspace){
        emit setParentToRoot();
//        executeFileAction([=](auto locked){locked->setParentToRoot();});
    }else if(event->key() == Qt::Key_Up){
        emit selectButtonUp(contrlPrsd, shiftPrsd);
//        executeFileAction([=](auto locked){locked->selectButtonUp(contrlPrsd, shiftPrsd);});
    }else if(event->key() == Qt::Key_Down){
        emit selectButtonDown(contrlPrsd, shiftPrsd);
//        executeFileAction([=](auto locked){locked->selectButtonDown(contrlPrsd, shiftPrsd);});
    }else if(event->key() == Qt::Key_Control){

    }else if(event->key() == Qt::Key_Shift){

    }else if(event->key() == Qt::Key_Plus){
        if(contrlPrsd)
            zoomIn();
    }else if(event->key() == Qt::Key_Minus){
        if(contrlPrsd)
            zoomOut();
    }else if(event->key() == Qt::Key_F4){
        if(contrlPrsd){
            executeFileAction(FILE_ACTION::CLOSE_TAB);
        }
    }else if(event->key() == Qt::Key_F5){
//        executeFileAction([=](auto locked){
//            locked->forceRevalidation();
//        });
    }else if(event->key() == Qt::Key_Alt ||
             event->key() == Qt::Key_AltGr){

    }else if (event->key() == Qt::Key_Left)
    {
        executeFileAction(FILE_ACTION::COLLAPSE);
    }else if (event->key() == Qt::Key_Right)
    {
        executeFileAction(FILE_ACTION::ELAPSE);
    }else if(event->key() == Qt::Key_A){
        if(StaticFunctions::controlPressed()){
            emit selectEntireContent();
        }
    }else if(event->key() == Qt::Key_C){
        if(contrlPrsd){
            emit copySelectedContent();
        }
    }else if(event->key() == Qt::Key_F){
        if(contrlPrsd){
            this->launchSearchMode();
        }
    }else if(event->key() == Qt::Key_M){
        if(contrlPrsd){
            this->createNewFile();
        }
    }else if(event->key() == Qt::Key_N){
        if(contrlPrsd){
            createNewFolder();
        }
    }else if(event->key() == Qt::Key_R){
        if(contrlPrsd){
            emit renameSelectedContent();
        }
    }else if(event->key() == Qt::Key_T){
        if(contrlPrsd){
            openTerminal();
        }
    }else if(event->key() == Qt::Key_V){
        if(contrlPrsd){
            emit pasteFromClipboard();
        }
    }else if(event->key() == Qt::Key_X){
        if(contrlPrsd){
            emit cutSelectedContent();
        }
    }else if(event->key() == Qt::Key_W){
        if(contrlPrsd){
            executeFileAction(FILE_ACTION::TAB);
        }
    }else if(event->key() == Qt::Key_H){
        if(contrlPrsd){
            showHiddenFiles();
        }
    }else if(event->key() == Qt::Key_Delete){
        emit deleteSelectedContent();
    }

    if( !contrlPrsd && !shiftPrsd )
    {
        QString txt = event->text();
        bool startsWithBackslash = txt.startsWith("\\");
        if( !startsWithBackslash && !txt.isEmpty() )
            emit keyPressed(event->key());
    }

    this->rePaintCanvas();
}

void GraphicsView::keyReleaseEvent(QKeyEvent *event)
{
    Q_UNUSED(event);
//    if(event->key() == Qt::Key_Control){
//        m_ctrl_prsd = false;
//    }else if(event->key() == Qt::Key_Shift){
//        m_shft_prsd = false;
//    }else if(event->key() == Qt::Key_Alt ||
//             event->key() == Qt::Key_AltGr){
//        m_alt_prsd = false;
//    }
    this->rePaintCanvas();
}

void GraphicsView::resizeEvent(QResizeEvent *event)
{
    rePaintCanvas();
    return QGraphicsView::resizeEvent(event);
}

void GraphicsView::mousePressEvent(QMouseEvent *event)
{
//    qDebug() << "GraphicsView.mousePressed";
    return QGraphicsView::mousePressEvent(event);
}
//void GraphicsView::mouseReleaseEvent(QMouseEvent *event)
//{
//    qDebug() << "GraphicsView.mouseReleased";
//    return QGraphicsView::mouseReleaseEvent(event);
//}
void GraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    if(m_mouseP)
        delete m_mouseP;

    m_mouseP = new QPoint(event->pos());

    int upperRectBound = 70;

    QPoint scrAdjMouP = QPoint(m_mouseP->x() + this->horizontalScrollBar()->value(),
                               m_mouseP->y() + this->verticalScrollBar()->value());

    if(!m_paintMenuBar && !inSearchMode() && m_upperRect &&
            m_upperRect->contains(scrAdjMouP)){
        m_paintUpperRect = false;
        m_paintMenuBar = true;
        rePaintCanvas();
    }else if(!m_paintMenuBar){
        if (m_paintUpperRect && m_mouseP->y() > upperRectBound){
            m_paintUpperRect = false;
            rePaintCanvas();
        }else if (!m_paintUpperRect && m_mouseP->y() <= upperRectBound){
            m_paintUpperRect = true;
            rePaintCanvas();
        }
    }
    return QGraphicsView::mouseMoveEvent(event);
}

void GraphicsView::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    executeFileAction(FILE_ACTION::REQUEST_FOCUS); // <- keyboard-focus: damit sofort, wenn man mit der maus ueber das fenster geht, das fenster den fokus hat
}

void GraphicsView::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
}


void GraphicsView::updateGraphicsView()
{
    this->viewport()->update();
}

void GraphicsView::setFilesCoordinator(std::weak_ptr<FilesCoordinator> filesCoor)
{
    m_filesCoord = filesCoor;
    executeFileAction([=](auto locked){
        m_paintContBar = locked->contentSelected();
    });
}

int GraphicsView::getVScrollBarValue()
{
    QScrollBar* scrBr = verticalScrollBar();
    return this->verticalScrollBar()->value();
}

int GraphicsView::getHScrollBarValue()
{
    return this->horizontalScrollBar()->value();
}

void GraphicsView::requestFocus()
{
    this->setFocus();
}

void GraphicsView::focusId(int id, bool repaintAnyway)
{
    revalidate(); // das revalidate macht eigetnlich das repaintAnyway komplett zunichte. aber:
    // wenn beim suchergebnis bisher nur 10 dateien angezeigt wurden -> vScrollBar also nicht existent
    // und nun 1000 files dargestellt werden und man versucht vScrollBar->setValue(1000) zu setzen,
    // juckt das den vScrollBar nicht und sein wert bleibt bei 0.

    qreal potSearchMenuHeight = (inSearchMode() ? m_searchMenuHeight : 0);

    qreal yStart = this->verticalScrollBar()->value() + m_elapseBarHeight + potSearchMenuHeight;
    qreal paneHeight = this->viewport()->height() -m_elapseBarHeight - potSearchMenuHeight;

    int startId = yStart / m_rowHeight;
    int endId = startId + paneHeight / m_rowHeight-1;
    if(endId < startId)
        endId = startId;

    bool repaintingIsNecessary = false;
    if(startId > id){
        qreal newVScrBrVal = id * m_rowHeight - m_elapseBarHeight - potSearchMenuHeight + m_rowHeight - 10;
        if(newVScrBrVal < 0)
            newVScrBrVal = 0;
        this->verticalScrollBar()->setValue(newVScrBrVal);
        repaintingIsNecessary = true;
    }else if(endId < id){
        qreal newVScrBrVal = id * m_rowHeight - m_elapseBarHeight
                - (this->viewport()->height()-m_elapseBarHeight) + m_rowHeight*2;
        if(newVScrBrVal < 0)
            newVScrBrVal = 0;
        this->verticalScrollBar()->setValue(newVScrBrVal);

        repaintingIsNecessary = true;
    }

    if(repaintingIsNecessary || repaintAnyway){
        this->rePaintCanvas();
    }
}

void GraphicsView::setHBarValue(int hBarValue)
{
    this->horizontalScrollBar()->setValue(hBarValue);
}

void GraphicsView::setVBarValue(int vBarValue)
{
    this->verticalScrollBar()->setValue(vBarValue);
}

//void GraphicsView::folderChanged(std::weak_ptr<const FileInfoBD> f)
//{
//    Q_UNUSED(f);
//    rePaintCanvas();
//}

void GraphicsView::paintTopRectangle(const QPointF& center, const QSize& size){
    const QSizeF halfSize(((float)size.width())*0.5, ((float)size.height())*0.5);

    QPointF p1(-halfSize.width(), -halfSize.height());
    QPointF p2(+halfSize.width(), -halfSize.height());
    QPointF p3(0, +halfSize.height());
    p1 += center;
    p2 += center;
    p3 += center;

    if(m_upperRect != nullptr){
        delete m_upperRect;
    }

    m_upperRect = new QPainterPath();
    m_upperRect->moveTo(p1);
    m_upperRect->lineTo(p2);
    m_upperRect->lineTo(p3);
    m_upperRect->closeSubpath();

    QGraphicsPathItem* painterPthItm = new QGraphicsPathItem();
    painterPthItm->setBrush(QBrush(StaticFunctions::getGoshBlueColor()));
    painterPthItm->setPen(QPen(QColor(0,0,0,100), 1, Qt::SolidLine));
    painterPthItm->setPath(*m_upperRect);

    m_graphicsGroup->addToGroup(painterPthItm);
}

void GraphicsView::addMenuBar(){
    m_menuBar = new MenuBar(this->viewport()->width(),
                            QPointF(this->viewport()->width()*0.5, m_elapseBarHeight));
    m_menuBar->setPosition(QPoint(this->horizontalScrollBar()->value(),
                                   this->verticalScrollBar()->value()));

    auto menu_caller = std::make_shared<DynamicFunctionCaller<QString,std::function<void()>>>();
    auto searchModeFunc     = [=](){ launchSearchMode(); };
    auto zoomInFunc         = [=](){ zoomIn(); };
    auto zoomOutFunc        = [=](){ zoomOut(); };
    auto sortFunc           = [=](){ sortAllFoldersDialog(); };
    auto newFolderFunc      = [=](){ createNewFolder(); };
    auto newFileFunc        = [=](){ createNewFile(); };
    auto terminalFunc       = [=](){ openTerminal(); };
    auto incognitoFunc      = [=](){ showHiddenFiles(); };
    auto closeMenuBarFunc   = [=](){ closeMenuBar(); };

    menu_caller->setFunction(QString("buttonFunction0"), searchModeFunc);
    menu_caller->setFunction(QString("buttonFunction1"), zoomInFunc);
    menu_caller->setFunction(QString("buttonFunction2"), zoomOutFunc);
    menu_caller->setFunction(QString("buttonFunction3"), sortFunc);
    menu_caller->setFunction(QString("buttonFunction4"), newFolderFunc);
    menu_caller->setFunction(QString("buttonFunction5"), newFileFunc);
    menu_caller->setFunction(QString("buttonFunction6"), terminalFunc);
    menu_caller->setFunction(QString("buttonFunction7"), incognitoFunc);
    menu_caller->setFunction(QString("buttonFunction8"), closeMenuBarFunc);

    QString incognitoImageFileName("incognito.png");
    if(auto locked = m_filesCoord.lock())
        if(locked->includeHiddenFiles())
            incognitoImageFileName = QString("incognito_NOT.png");

    auto painting_caller = std::make_shared<DynamicFunctionCaller<QString,std::function<void(QPainter*,QRectF)>>>();
    auto paintSearchModeFunc     = [=](QPainter* painter, QRectF rct){ StaticFunctions::paintLoupe(painter, rct, StaticFunctions::SHAPE::NONE); };
    auto paintZoomInFunc         = [=](QPainter* painter, QRectF rct){ StaticFunctions::paintLoupe(painter, rct, StaticFunctions::SHAPE::PLUS); };
    auto paintZoomOutFunc        = [=](QPainter* painter, QRectF rct){ StaticFunctions::paintLoupe(painter, rct, StaticFunctions::SHAPE::MINUS); };
    auto paintSortFunc           = [=](QPainter* painter, QRectF rct){
        QString cursor_pixmap_path = QString("%1%2%3").arg("pics").arg(QDir::separator()).arg("sort_icon.png");
        StaticFunctions::paintPixmapRect(painter, QPixmap(cursor_pixmap_path), rct ,Qt::transparent,Qt::transparent,Qt::transparent); };
    auto paintNewFoldFunc           = [=](QPainter* painter, QRectF rct){
        QString cursor_pixmap_path = QString("%1%2%3").arg("pics").arg(QDir::separator()).arg("empty_fold_icon.png");
        StaticFunctions::paintPixmapRect(painter, QPixmap(cursor_pixmap_path), rct ,Qt::transparent,Qt::transparent,Qt::transparent); };
    auto paintNewFileFunc           = [=](QPainter* painter, QRectF rct){
        QString cursor_pixmap_path = QString("%1%2%3").arg("pics").arg(QDir::separator()).arg("empty_file_icon.png");
        StaticFunctions::paintPixmapRect(painter, QPixmap(cursor_pixmap_path), rct ,Qt::transparent,Qt::transparent,Qt::transparent); };
    auto paintTerminalFunc           = [=](QPainter* painter, QRectF rct){
        QString cursor_pixmap_path = QString("%1%2%3").arg("pics").arg(QDir::separator()).arg("terminal.png");
        StaticFunctions::paintPixmapRect(painter, QPixmap(cursor_pixmap_path), rct ,Qt::transparent,Qt::transparent,Qt::transparent); };
    auto paintIncognitoFunc           = [=](QPainter* painter, QRectF rct){
        QString cursor_pixmap_path = QString("%1%2%3").arg("pics").arg(QDir::separator()).arg(incognitoImageFileName);
        StaticFunctions::paintPixmapRect(painter, QPixmap(cursor_pixmap_path), rct ,Qt::transparent,Qt::transparent,Qt::transparent); };
    auto paintCloseMenuBarFunc   = [=](QPainter* painter, QRectF rct){ StaticFunctions::paintCrossRect(painter, rct); };

    painting_caller->setFunction(QString("paintingFunction0"), paintSearchModeFunc);
    painting_caller->setFunction(QString("paintingFunction1"), paintZoomInFunc);
    painting_caller->setFunction(QString("paintingFunction2"), paintZoomOutFunc);
    painting_caller->setFunction(QString("paintingFunction3"), paintSortFunc);
    painting_caller->setFunction(QString("paintingFunction4"), paintNewFoldFunc);
    painting_caller->setFunction(QString("paintingFunction5"), paintNewFileFunc);
    painting_caller->setFunction(QString("paintingFunction6"), paintTerminalFunc);
    painting_caller->setFunction(QString("paintingFunction7"), paintIncognitoFunc);
    painting_caller->setFunction(QString("paintingFunction8"), paintCloseMenuBarFunc);

    m_menuBar->setCaller(menu_caller, painting_caller);

    m_graphicsGroup->addToGroup(m_menuBar);
}

void GraphicsView::closeMenuBar()
{
    m_paintMenuBar = false;
    QTimer::singleShot(10,[=](){
        rePaintCanvas();
    });
}

void GraphicsView::addElapseBar()
{
    if(auto locked = lockFilesCoordinator()){
        ElapseMenuBD* elapseMenu = new ElapseMenuBD(locked->getMaxDepth()+1,
                                                   m_colOffs*2,
                                                   QSize(this->viewport()->width(),m_elapseBarHeight),
                                                   QPoint(0,0)
                                                 );
        elapseMenu->setBackroundColor(m_elapseCol1, m_elapseCol2);
        elapseMenu->setPosition(QPoint(this->horizontalScrollBar()->value(), this->verticalScrollBar()->value()));

        auto button_caller = std::make_shared<DynamicFunctionCaller<QString,std::function<bool(int)>>>();
        auto buttonClickFunc = [=](int i){
            emit elapseAllFoldersOfDepthId(i);
            return false;
        };
        auto elapsedFunc = [=](int depthId){
            if(auto locked = lockFilesCoordinator()){
                return locked->depthIdElapsed(depthId);
            }
            return false;
        };

        button_caller->setFunction(QString("call"), buttonClickFunc);
        button_caller->setFunction(QString("elapsed"), elapsedFunc);

        elapseMenu->setCaller(button_caller);

        m_graphicsGroup->addToGroup(elapseMenu);
    }
}

void GraphicsView::executeFileAction(FILE_ACTION action)
{
    switch(action)
    {
        case FILE_ACTION::COPY:
            emit copySelectedContent();
            break;
        case FILE_ACTION::CUT:
            emit cutSelectedContent();
            break;
        case FILE_ACTION::PASTE:
            emit pasteFromClipboard();
            break;
        case FILE_ACTION::DUPLICATE:
            emit duplicateSelectedContent();
            break;
        case FILE_ACTION::OPEN:
            emit openSelectedContent();
            break;
        case FILE_ACTION::OPEN_WITH:
            emit openSelectedContentWith();
            break;
        case FILE_ACTION::DELETE:
            emit deleteSelectedContent();
            break;
        case FILE_ACTION::ZIP:
            emit zipSelectedContent();
            break;
        case FILE_ACTION::UNZIP:
            emit unzipSelectedContent();
            break;
        case FILE_ACTION::RENAME:
            emit renameSelectedContent();
            break;
        case FILE_ACTION::TERMINAL:
            openTerminal();
            break;
        case FILE_ACTION::DETAILS:
            emit showDetailsOfSelectedContent();
            break;
        case FILE_ACTION::ELAPSE_REC:
            emit elapseSelectedFoldersRecursively();
            break;
        case FILE_ACTION::ELAPSE:
            emit elapseSelectedFolders();
            break;
        case FILE_ACTION::LOAD:
            emit setSelectionToRoot();
            break;
        case FILE_ACTION::COLLAPSE_REC:
            emit collapseSelectedFoldersRecursively();
            break;
        case FILE_ACTION::COLLAPSE:
            emit collapseSelectedFolders();
            break;
        case FILE_ACTION::PATH:
            emit copySelectedFilePathToClipboard();
            break;
        case CANCEL_CURRENT_ACTION:
            emit killCurrentBlockingAction();
            break;
        case REQUEST_FOCUS:
            emit requestFocusSGNL();
            break;
        case TAB:
            emit requestOpenFoldersInTab();
            break;
        case CLOSE_TAB:
            emit requestCloseCurrentTab();
            break;
    }
}
void GraphicsView::executeFileAction(std::function<void (std::shared_ptr<FilesCoordinator>)> fctn)
{
    if(m_isLoading)
        return;

    if(auto locked = lockFilesCoordinator())
    {
        fctn(locked);
    }
}

void GraphicsView::addContentBar()
{
    qreal fctr = 0.75;
    m_contBar = new MenuBar(getDisplayableHeight()*fctr,
                            QPointF(this->viewport()->width(),
                                    m_elapseBarHeight+getDisplayableHeight()*0.5),
                            ORIENTATION::VERTICAL,
                            true);
    m_contBar->setPosition(QPoint(this->horizontalScrollBar()->value(), this->verticalScrollBar()->value()));

    auto menu_caller = std::make_shared<DynamicFunctionCaller<QString,std::function<void()>>>();
    auto painting_caller = std::make_shared<DynamicFunctionCaller<QString,std::function<void(QPainter*,QRectF)>>>();
    std::map<int,int> groupingMap;

    int fontWeight = QFont::Bold;
    int fontSize = 6;

    int funcId = 0;
    if(auto lockedFilesCoord = lockFilesCoordinator()){

        bool rename = false;
        bool path = false;
        bool terminal = false;
        bool openWith = false;

        if(lockedFilesCoord->foldersSelected()){
            if(lockedFilesCoord->singleFolderSelected())
            {
            }

            // elapse folders:

            auto setElapseFunc = [=](){
                executeFileAction(FILE_ACTION::ELAPSE_REC);
            };
            menu_caller->setFunction(QString("buttonFunction%1").arg(funcId), setElapseFunc);
            auto paintElapseFunc = [=](QPainter* painter, QRectF rct){
                StaticFunctions::paintTextRect(painter, QString("elapse"), rct, Qt::transparent,Qt::transparent,
                                               QColor(0,0,0),StaticFunctions::getGoshFont(fontSize, fontWeight));
            };
            painting_caller->setFunction(QString("paintingFunction%1").arg(funcId), paintElapseFunc);
            groupingMap[funcId] = 0;
            ++funcId;

            // collapse folder - hier wird reursiv collapsed
            // -> d.h. der ausgewaehlte ordner + alle darin eventuell bereits "elapseten" sub-folders:

            auto setCollapseFunc = [=](){
                executeFileAction(FILE_ACTION::COLLAPSE_REC);
            };
            menu_caller->setFunction(QString("buttonFunction%1").arg(funcId), setCollapseFunc);
            auto paintCollapseFunc = [=](QPainter* painter, QRectF rct){
                StaticFunctions::paintTextRect(painter, QString("collapse"), rct, Qt::transparent,Qt::transparent,
                                               QColor(0,0,0),StaticFunctions::getGoshFont(fontSize, fontWeight));
            };
            painting_caller->setFunction(QString("paintingFunction%1").arg(funcId), paintCollapseFunc);
            groupingMap[funcId] = 0;
            ++funcId;

            // tab: open selected folders in new tab:

            auto setTabFunc = [=](){
                executeFileAction(FILE_ACTION::TAB);
            };
            menu_caller->setFunction(QString("buttonFunction%1").arg(funcId), setTabFunc);
            auto paintTabFunc = [=](QPainter* painter, QRectF rct){
                StaticFunctions::paintTextRect(painter, QString("tab"), rct, Qt::transparent,Qt::transparent,
                                               QColor(0,0,0),StaticFunctions::getGoshFont(fontSize, fontWeight));
            };
            painting_caller->setFunction(QString("paintingFunction%1").arg(funcId), paintTabFunc);
            groupingMap[funcId] = 0;
            ++funcId;
        }
        if(lockedFilesCoord->singleContentSelected()){

            rename = true;

            terminal = true;

            path = true;

            if(lockedFilesCoord->filesSelected())
                openWith = true;
        }
        if(lockedFilesCoord->selectionCounter() > 0){

            // open files/folders:

            auto openFunc = [=](){
                executeFileAction(FILE_ACTION::OPEN);
//                lockedFilesCoord->openSelectedContent();
            };
            menu_caller->setFunction(QString("buttonFunction%1").arg(funcId), openFunc);
            std::function<void(QPainter*,QRectF)> paintElapseFunc = [=](QPainter* painter, QRectF rct){
                StaticFunctions::paintTextRect(painter, QString("open"), rct, Qt::transparent,Qt::transparent,
                                               QColor(0,0,0),StaticFunctions::getGoshFont(fontSize, fontWeight));
            };
            painting_caller->setFunction(QString("paintingFunction%1").arg(funcId), paintElapseFunc);
            groupingMap[funcId] = 1;
            ++funcId;

            if(openWith)
            {
                auto openWithFunc = [=](){
                    executeFileAction(FILE_ACTION::OPEN_WITH);
                };
                menu_caller->setFunction(QString("buttonFunction%1").arg(funcId), openWithFunc);
                paintElapseFunc = [=](QPainter* painter, QRectF rct){
                    StaticFunctions::paintTextRect(painter, QString("open with"), rct, Qt::transparent,Qt::transparent,
                                                   QColor(0,0,0),StaticFunctions::getGoshFont(fontSize, fontWeight));
                };
                painting_caller->setFunction(QString("paintingFunction%1").arg(funcId), paintElapseFunc);
                groupingMap[funcId] = 1;
                ++funcId;
            }


            if(rename)
            {
                // rename file/folder:

                auto renameFunc = [=](){
                    executeFileAction(FILE_ACTION::RENAME);
                };
                menu_caller->setFunction(QString("buttonFunction%1").arg(funcId), renameFunc);
                auto paintRenameFunc = [=](QPainter* painter, QRectF rct){
                    StaticFunctions::paintTextRect(painter, QString("rename"), rct, Qt::transparent,Qt::transparent,
                                                   QColor(0,0,0),StaticFunctions::getGoshFont(fontSize, fontWeight));
                };
                painting_caller->setFunction(QString("paintingFunction%1").arg(funcId), paintRenameFunc);
                groupingMap[funcId] = 2;
                ++funcId;
            }

            // copy files/folders:

            std::function<void()> setFunc = [=](){
                executeFileAction(FILE_ACTION::COPY);
            };
            menu_caller->setFunction(QString("buttonFunction%1").arg(funcId), setFunc);
            paintElapseFunc = [=](QPainter* painter, QRectF rct){
                StaticFunctions::paintTextRect(painter, QString("copy"), rct, Qt::transparent,Qt::transparent,
                                               QColor(0,0,0),StaticFunctions::getGoshFont(fontSize, fontWeight));
            };
            painting_caller->setFunction(QString("paintingFunction%1").arg(funcId), paintElapseFunc);
            groupingMap[funcId] = 2;
            ++funcId;

            // duplicate files/folders:

            setFunc = [=](){
                executeFileAction(FILE_ACTION::DUPLICATE);
//                lockedFilesCoord->duplicateSelectedContent();
            };
            menu_caller->setFunction(QString("buttonFunction%1").arg(funcId), setFunc);
            paintElapseFunc = [=](QPainter* painter, QRectF rct){
                StaticFunctions::paintTextRect(painter, QString("duplicate"), rct, Qt::transparent,Qt::transparent,
                                               QColor(0,0,0),StaticFunctions::getGoshFont(fontSize, fontWeight));
            };
            painting_caller->setFunction(QString("paintingFunction%1").arg(funcId), paintElapseFunc);
            groupingMap[funcId] = 2;
            ++funcId;

            // cut files/folders:
            setFunc = [=](){
                executeFileAction(FILE_ACTION::CUT);
//                lockedFilesCoord->cutSelectedContent();
            };
            menu_caller->setFunction(QString("buttonFunction%1").arg(funcId), setFunc);
            paintElapseFunc = [=](QPainter* painter, QRectF rct){
                StaticFunctions::paintTextRect(painter, QString("cut"), rct, Qt::transparent,Qt::transparent,
                                               QColor(0,0,0),StaticFunctions::getGoshFont(fontSize, fontWeight));
            };
            painting_caller->setFunction(QString("paintingFunction%1").arg(funcId), paintElapseFunc);
            groupingMap[funcId] = 2;
            ++funcId;

            // paste files/folders:

            setFunc = [=](){
                executeFileAction(FILE_ACTION::PASTE);
//                lockedFilesCoord->pasteFromClipboard();
            };
            menu_caller->setFunction(QString("buttonFunction%1").arg(funcId), setFunc);
            paintElapseFunc = [=](QPainter* painter, QRectF rct){
                StaticFunctions::paintTextRect(painter, QString("paste"), rct, Qt::transparent,Qt::transparent,
                                               QColor(0,0,0),StaticFunctions::getGoshFont(fontSize, fontWeight));
            };
            painting_caller->setFunction(QString("paintingFunction%1").arg(funcId), paintElapseFunc);
            groupingMap[funcId] = 2;
            ++funcId;

            // zip files:

            auto zipFunc = [=](){
                executeFileAction(FILE_ACTION::ZIP);
            };
            menu_caller->setFunction(QString("buttonFunction%1").arg(funcId), zipFunc);
            auto paintZipFunc = [=](QPainter* painter, QRectF rct){
                StaticFunctions::paintTextRect(painter, QString("zip"), rct, Qt::transparent,Qt::transparent,
                                               QColor(0,0,0),StaticFunctions::getGoshFont(fontSize, fontWeight));
            };
            painting_caller->setFunction(QString("paintingFunction%1").arg(funcId), paintZipFunc);
            groupingMap[funcId] = 4;
            ++funcId;

            // unzip files:

            if( lockedFilesCoord->selectionContainsZippedFile() )
            {
                auto unzipFunc = [=](){
                    executeFileAction(FILE_ACTION::UNZIP);
                };
                menu_caller->setFunction(QString("buttonFunction%1").arg(funcId), unzipFunc);
                auto paintUnZipFunc = [=](QPainter* painter, QRectF rct){
                    StaticFunctions::paintTextRect(painter, QString("unzip"), rct, Qt::transparent,Qt::transparent,
                                                   QColor(0,0,0),StaticFunctions::getGoshFont(fontSize, fontWeight));
                };
                painting_caller->setFunction(QString("paintingFunction%1").arg(funcId), paintUnZipFunc);
                groupingMap[funcId] = 4;
                ++funcId;
            }

            if(path)
            {
                // path:

                auto copyPathToClipboardFunc = [=](){
                    executeFileAction(FILE_ACTION::PATH);
                };
                menu_caller->setFunction(QString("buttonFunction%1").arg(funcId), copyPathToClipboardFunc);
                auto paintPathToClipboardFunc = [=](QPainter* painter, QRectF rct){
                    StaticFunctions::paintTextRect(painter, QString("path"), rct, Qt::transparent,Qt::transparent,
                                                   QColor(0,0,0),StaticFunctions::getGoshFont(fontSize, fontWeight));
                };
                painting_caller->setFunction(QString("paintingFunction%1").arg(funcId), paintPathToClipboardFunc);
                groupingMap[funcId] = 3;
                ++funcId;
            }

            if(terminal)
            {
                // terminal:

                auto terminalFunc = [=](){
                    executeFileAction(FILE_ACTION::TERMINAL);
                };
                menu_caller->setFunction(QString("buttonFunction%1").arg(funcId), terminalFunc);
                auto paintTerminalFunc = [=](QPainter* painter, QRectF rct){
                    StaticFunctions::paintTextRect(painter, QString("terminal"), rct, Qt::transparent,Qt::transparent,
                                                   QColor(0,0,0),StaticFunctions::getGoshFont(fontSize, fontWeight));
                };
                painting_caller->setFunction(QString("paintingFunction%1").arg(funcId), paintTerminalFunc);
                groupingMap[funcId] = 3;
                ++funcId;
            }

            // delete files/folders:

            setFunc = [=](){
                executeFileAction(FILE_ACTION::DELETE);
            };
            menu_caller->setFunction(QString("buttonFunction%1").arg(funcId), setFunc);
            paintElapseFunc = [=](QPainter* painter, QRectF rct){
                StaticFunctions::paintTextRect(painter, QString("delete"), rct, Qt::transparent,Qt::transparent,
                                               QColor(0,0,0),StaticFunctions::getGoshFont(fontSize, fontWeight));
            };
            painting_caller->setFunction(QString("paintingFunction%1").arg(funcId), paintElapseFunc);
            groupingMap[funcId] = 5;
            ++funcId;
        }
    }

//    std::function<int(int)> groupingFunc = [&](int id){return groupingMap[id];};
//    m_contBar->setCaller(menu_caller, painting_caller, groupingFunc);
    m_contBar->setCaller(menu_caller, painting_caller);

    m_graphicsGroup->addToGroup(m_contBar);
}

void GraphicsView::closeContentBar()
{
    m_paintContBar = false;
    QTimer::singleShot(10,[=](){
        rePaintCanvas();
    });
}
void GraphicsView::addSearchMenu(){
    QSize menuSize(this->viewport()->width(), m_searchMenuHeight);

//    if(m_searchMenu == nullptr)
//        m_searchMenu = new SearchMenuBD(menuSize);
//    else
//        m_searchMenu->setSize(menuSize);

    m_searchMenu = new SearchMenuBD(menuSize);

    m_searchMenu->setPosition(QPoint(this->horizontalScrollBar()->value(),
                                     this->verticalScrollBar()->value() + m_elapseBarHeight));


    auto menu_caller = std::make_shared<DynamicFunctionCaller<QString,std::function<QString()>>>();
    auto nextPrevCaller = [=](){nextSearchResult();return QString("");};
    auto prevPrevCaller = [=](){prevSearchResult();return QString("");};
    auto closePrevCaller = [=](){closeSearchMenu();return QString("");};
    auto getNextSearchText = [=](){ return getCurrentSearchResult(); };
    auto getCurrentSearchIndex = [=](){ return QString("%1").arg( getSearchIndex()+1 ); };
    auto getSearRsltsCount = [=](){ return QString("%1").arg( getSearchResultsCount() ); };
    menu_caller->setFunction(QString("next"), nextPrevCaller);
    menu_caller->setFunction(QString("previous"), prevPrevCaller);
    menu_caller->setFunction(QString("close"), closePrevCaller);
    menu_caller->setFunction(QString("getSearchResult"), getNextSearchText);
    menu_caller->setFunction(QString("getCurrentSearchIndex"), getCurrentSearchIndex);
     menu_caller->setFunction(QString("getSearchResultsCount"), getSearRsltsCount);
    m_searchMenu->setCaller(menu_caller);

    m_graphicsGroup->addToGroup(m_searchMenu);
}


void GraphicsView::rePaintCanvas()
{
//    qDebug() << "repainting canvas";

    // darauf achten, dass, wenn inSearchMode()==true,
    // auch nach dem searchMode m_paintUpperRect und m_paintMenuBar auf false stehen
    // denke es ist am besten das hier in rePaintCanvas zu platzieren, da der filescoordinaotr nicht
    // mehr ein signal gibt, umd den searchMode zu starten:
    if(inSearchMode())
    {
        m_paintUpperRect = false;
        m_paintMenuBar = false;
    }

    if(m_isLoading)
    {
        if( !m_waitingBarIsAddedToScene )
        {
//            qDebug() << "   -> rePaintCanvas:isLoading";
            QGraphicsRectItem* blockingRect = new QGraphicsRectItem(QRectF(0,0, 1000000,1000000));
            blockingRect->setBrush(QBrush(QColor(0,255,0, 50)));
            m_graphicsGroup->addToGroup(blockingRect);

            m_waitingBar = new WaitingBarGraphicsItem(QSize(0,0), QPoint(0,0));
            m_waitingBar->startAnimation();

            m_graphicsGroup->addToGroup( m_waitingBar );
            m_waitingBarIsAddedToScene = true;

            QString cancelPixmapPath = StaticFunctions::getPictureResourceFromFileName("cancel.png");
            QPixmap cancelPixmap(cancelPixmapPath);
            cancelPixmap = cancelPixmap.scaled(QSize(300,300));

            m_cancelBtnSize = QSize(cancelPixmap.width(), cancelPixmap.height());
            m_cancelBtn = new GraphicItemsBD::PixmapRect(cancelPixmap,
                                                         m_cancelBtnSize,
                                                         QPoint((this->viewport()->width()-cancelPixmap.width()) * 0.5f, 50),
                                                         QColor(255,255,255, 255), QColor(240,240,255, 255),
                                                         QColor(255,150,150, 255), QColor(255,0,0, 255));
            m_cancelBtn->setCallFunction([=](){
                executeFileAction(FILE_ACTION::CANCEL_CURRENT_ACTION);
            });

            m_graphicsGroup->addToGroup(m_cancelBtn);

//            setWaitingBarSizeAndPos();
        }/*else{
            setWaitingBarSizeAndPos();            
        }*/
    }else
    {
        if(m_waitingBarIsAddedToScene)
        {
            m_waitingBarIsAddedToScene = false;
            m_waitingBar->stopAnimation();
        }

        m_scene.clear();

        m_graphicsGroup = new QGraphicsItemGroup();
        m_graphicsGroup->setX(getAbsoluteHorBarValue());
        m_graphicsGroup->setY(getAbsoluteVerticalBarValue());
        m_graphicsGroup->setHandlesChildEvents(false);

        if(auto fiCoordLocked = lockFilesCoordinator()){
            m_fileCount = fiCoordLocked->getDisplayedFileCount();

            int height = m_rowHeight*m_fileCount + m_elapseBarHeight;
            if(height < this->viewport()->height()){
                height = this->viewport()->height();
            }
            m_scene.setSceneRect(QRect(0,0,this->viewport()->width(), height));
        }else{
            m_scene.setSceneRect(QRect(0,0,this->viewport()->width(), this->viewport()->height()));
        }

//        auto lckdFiCoord = lockFilesCoordinator();

        if(m_fileCount < m_fileMaxCount){
            m_firstDispFI = 0;
            m_lastDispFI = m_fileCount-1;
            m_curDispFI = m_fileCount-1;
        }else{
            m_firstDispFI = getFirstToDispFi();
            m_lastDispFI = getLastToDispFi();
        }

        std::function<void(std::weak_ptr<FileInfoBD>, const QFileInfo&, int,int, bool, bool, bool, bool, bool, bool)> dirFunc
                = [=](std::weak_ptr<FileInfoBD> fiBD, const QFileInfo& fi, int a, int b,
                        bool isElapsed, bool isLoaded, bool isEmpty, bool containsFiles, bool isSelected, bool isSearched){

            auto elapseFunc = [=](){
                executeFileAction([=](auto locked){
                    locked->elapseOrCollapseFolderDependingOnCurrentState(fiBD);
                });
                return false;
            };
            auto isElapsedFunc = [=](){return isElapsed;};
            auto isLoadedFunc = [=](){return isLoaded;};
            auto isDirFunc = [=](){ return true;};
            auto isEmptyFunc = [=](){return isEmpty;};
            auto containsFilesFunc = [=](){return containsFiles;};
            auto selectFunc = [=](){
                m_paintContBar = true;
                emit selectContent(fi.absoluteFilePath(), true, StaticFunctions::controlPressed(), StaticFunctions::shiftPressed());
                return false;
            };
            auto isSelectedFunc = [=](){
                return isSelected;
            };
            auto setSelectedFoldToRootFunc = [=](){
                emit setSelectionToRoot();
                return false;
            };
            auto isSearchedFunc = [=](){
                return isSearched;
            };

            auto caller = std::make_shared<DynamicFunctionCaller<QString, std::function<bool()>>>();
            caller->setFunction(QString("elapse"), elapseFunc);
            caller->setFunction(QString("isElapsed"), isElapsedFunc);
            caller->setFunction(QString("isLoaded"), isLoadedFunc);
            caller->setFunction(QString("isDir"), isDirFunc);
            caller->setFunction(QString("isEmpty"), isEmptyFunc);
            caller->setFunction(QString("select"), selectFunc);
            caller->setFunction(QString("isSelected"), isSelectedFunc);
            caller->setFunction(QString("setAsRoot"), setSelectedFoldToRootFunc);
            caller->setFunction(QString("isSearched"), isSearchedFunc);
            caller->setFunction(QString("containsFiles"), containsFilesFunc);
//            caller->setFunction(QString("executeAction"), [=](FILE_ACTION action){this->executeFileAction(action);});

            auto sortFunc = [=](ORDER_BY order){
                emit sortFromDisplayedContent(fi, order);
                return false;
            };
            auto isRevSortFunc = [=](ORDER_BY order){
                if(auto locked = fiBD.lock()){
                    return locked->isReversedSortedBy(order);
                }
                return false;
            };
            auto isSortedByFunc = [=](ORDER_BY order){
                if(auto locked = fiBD.lock()){
                    return locked->isSortedBy(order);
                }
                return false;
            };

            auto sortCaller = std::make_shared<DynamicFunctionCaller<QString, std::function<bool(ORDER_BY)>>>();
            sortCaller->setFunction(QString("sortBy"), sortFunc);
            sortCaller->setFunction(QString("isReversedSortedBy"), isRevSortFunc);
            sortCaller->setFunction(QString("isSortedBy"), isSortedByFunc);


            paintFileInfo(fi, a,b, caller, sortCaller);
        };
        std::function<void(const QFileInfo&,int,int, bool, bool)> fileFunc =
                [=](const QFileInfo& fi, int a, int b, bool isSelected, bool isSearched){


            auto isDirFunc = [=](){ return false; };
            auto selectFunc = [=](){
                m_paintContBar = true;
                emit selectContent(fi.absoluteFilePath(), false, StaticFunctions::controlPressed(), StaticFunctions::shiftPressed());
                return false;
            };
            auto isSelectedFunc = [=](){
                return isSelected;
            };
            auto isSearchedFunc = [=](){
                return isSearched;
            };

            auto caller = std::make_shared<DynamicFunctionCaller<QString, std::function<bool()>>>();
            caller->setFunction(QString("isDir"), isDirFunc);
            caller->setFunction(QString("select"), selectFunc);
            caller->setFunction(QString("isSelected"), isSelectedFunc);
            caller->setFunction(QString("isSearched"), isSearchedFunc);

            paintFileInfo(fi, a,b, caller);
        };

        if(auto lockedFiCoord =  lockFilesCoordinator()){
            for(int i=m_firstDispFI; i <= m_lastDispFI; i++){
                FilInfoForOneDim curFileToDisp = lockedFiCoord->getDisplayedFileAt(i);
                QString absFilePath = curFileToDisp.getAbsoluteFilePath();

                bool isSelected = lockedFiCoord->isSelected(absFilePath);
                bool isSearched = lockedFiCoord->isCurentSearchResult(absFilePath);

                if(curFileToDisp.m_isFolder){
                    if(auto locked = curFileToDisp.m_fileInfoBD.lock())
                    {
                        const QFileInfo& fi = locked->getFileInfo();

                        bool isElapsed = locked->elapsed();
                        bool isLoaded = locked->isLoaded();
                        bool isEmpty = locked->isEmpty();
                        bool containsFiles = (locked->fileCount() > 0);

                        dirFunc(curFileToDisp.m_fileInfoBD, fi, i, curFileToDisp.m_depth, isElapsed, isLoaded, isEmpty, containsFiles, isSelected, isSearched);
                    }
                }else{
                    fileFunc(QFileInfo(absFilePath), i, curFileToDisp.m_depth, isSelected, isSearched);
                }
            }
        }

        m_scene.addItem(m_graphicsGroup);
        m_graphicsGroup->setY(this->verticalScrollBar()->value());

        if(!inSearchMode() && m_paintUpperRect){
            qreal trnglWidth = m_upperRectWidth;//qMin(trnglWidth, (qreal)20.);
            float trnglHeight = trnglWidth*0.5;
            QPointF center( ((float)this->viewport()->width())*0.5, trnglHeight*0.5+5. + getViewportYOffset()+m_elapseBarHeight);
            paintTopRectangle(center, QSize(trnglWidth,trnglHeight));
        }
        if(inSearchMode()){
            addSearchMenu();
        }else
        {
            m_searchMenu = nullptr;
        }

        if(m_paintMenuBar){
            addMenuBar();
        }
        if(m_paintContBar){
            addContentBar();
        }
        addElapseBar();

        if(m_shwRtSel){
            QString directory_pixmap_path = QString("%1%2%3")
                    .arg("pics")
                    .arg(QDir::separator())
                    .arg("root_scld.png");
            QPixmap directory_pixmap = QPixmap(directory_pixmap_path);
            GraphicItemsBD::PixmapRect* rotSlctr = new GraphicItemsBD::PixmapRect(directory_pixmap, QSize(50, 50),
                       QPoint(0,0),
                       QColor(255,255,255, 0),QColor(255,255,255, 0),
                       QColor(255,0,200, 255), QColor(255,200,200, 255));
            rotSlctr->setCallFunction([=](){showRootSelector();});
            rotSlctr->setPosition(QPoint(this->viewport()->width() - (rotSlctr->boundingRect().width() + 4),
                                           getViewportYOffset()));
            m_graphicsGroup->addToGroup(rotSlctr);
        }
    }
}

void GraphicsView::setWaitingBarSizeAndPos()
{
    if( !m_isLoading )
        return;

    m_waitingBar->setSize( QSize(this->viewport()->width(), m_waitingBarHeight) );
    m_waitingBar->setPosition( QPoint(0, 5) );//(this->viewport()->height()-m_waitingBarHeight) * 0.5f) );
    
//    m_cancelBtn->setSize(m_cancelBtnSize);
    m_cancelBtn->setPosition(QPoint((this->viewport()->width()-m_cancelBtnSize.width()) * 0.5f, 50));
}

void GraphicsView::paintFileInfo(const QFileInfo& fi, int rowId, int colId,
                                 std::shared_ptr<DynamicFunctionCaller<QString, std::function<bool()>>> caller,
                                 std::shared_ptr<DynamicFunctionCaller<QString, std::function<bool(ORDER_BY)>>> sortCaller){
    int colOffs = 30;
    GraphicsFile* gf = new GraphicsFile(fi, QSize(this->viewport()->width(),m_rowHeight),
                                        rowId, m_rowHeight,
                                        colId, colOffs,
                                        caller,
                                        sortCaller,
                                        m_fontSize);
    gf->setDetailsTextColor(QColor(0,0,0, 150));
    gf->setDropFunction([=](QString dropStr){
        emit paste(dropStr, fi.absoluteFilePath());
//        executeFileAction([=](auto locked){
//            locked->paste(dropStr, fi.absoluteFilePath());
//        });
//        if(auto locked = lockFilesCoordinator()){
//            locked->paste(dropStr, fi.absoluteFilePath());
//        }
    });
    gf->setInitDraggingFunction([=](QString draggingSource){
        emit initDragging(draggingSource);
//        executeFileAction([=](auto locked){
//            locked->initDragging(draggingSource);
//        });
//        if(auto locked = lockFilesCoordinator()){
//            locked->initDragging(draggingSource);
//        }
    });
    gf->setPosition(QPoint(0, rowId*m_rowHeight + m_elapseBarHeight));
    m_scene.addItem(gf);
}

qreal GraphicsView::getRelativeHorBarValue()
{
    qreal val = this->horizontalScrollBar()->value();
    qreal max = this->horizontalScrollBar()->maximum();
    qreal min = this->horizontalScrollBar()->minimum();
    qreal retVal = val / (max-min);
    return retVal;
}
qreal GraphicsView::getRelativeVerBarValue()
{

    qreal val = this->verticalScrollBar()->value();
    qreal max = this->verticalScrollBar()->maximum();
    qreal min = this->verticalScrollBar()->minimum();
    qreal retVal = val / (max-min);
    return retVal;
}

int GraphicsView::getAbsoluteHorBarValue()
{
    return this->horizontalScrollBar()->value();
}

int GraphicsView::getAbsoluteVerticalBarValue()
{
    return this->verticalScrollBar()->value();
}

int GraphicsView::getViewportYOffset()
{
    return getAbsoluteVerticalBarValue();
}

void GraphicsView::handleSearchKeyword(QString keyword, bool deepSearch)
{
    if(!keyword.isEmpty()){
        emit searchForKeyWord(keyword, deepSearch);
//        executeFileAction([=](auto locked){
//            locked->searchForKeyWord(keyword, deepSearch);
//        });
    }
}
void GraphicsView::launchSearchMode()
{
    SearchFileDialog* sfd = new SearchFileDialog();
    sfd->setFixedWidth(350);
    sfd->show();

    connect( sfd, &SearchFileDialog::okClicked, this, [=](QString keyword, bool deepSearch){
            handleSearchKeyword(keyword, deepSearch);
        }, Qt::QueuedConnection);
}

void GraphicsView::nextSearchResult()
{
    emit nextSearchResultSGNL();
}

void GraphicsView::prevSearchResult()
{
    emit prevSearchResultSGNL();
}

void GraphicsView::closeSearchMenu()
{
    emit closeSearchMenuSGNL();
}

QString GraphicsView::getCurrentSearchResult()
{
    if(auto locked = lockFilesCoordinator()){
        return locked->getCurSearchResultStr();
    }
    return QString("");
}
long GraphicsView::getIndexOfCurrentSearchResult()
{
    if(auto locked = lockFilesCoordinator()){
        return locked->getIndexOfCurrentSearchResult();
    }
    return 0;
}
long GraphicsView::getSearchIndex()
{
    if(auto locked = lockFilesCoordinator()){
        return locked->getSearchIndex();
    }
    return 0;
}

long GraphicsView::getSearchResultsCount()
{
    if(auto locked = lockFilesCoordinator()){
        return locked->getSearchResultsCount();
    }
    return 0;
}

void GraphicsView::zoomOut()
{
    if(m_fontSize > 5){
        --m_fontSize;
        revalidateRowHeight();
        QTimer::singleShot(0, [=](){rePaintCanvas();});
        emit zoomFactorChanged(m_fontSize);
    }
}

void GraphicsView::zoomIn()
{
    if(m_fontSize < 20){
        ++m_fontSize;
        revalidateRowHeight();
        QTimer::singleShot(0, [=](){rePaintCanvas();});
        emit zoomFactorChanged(m_fontSize);
    }
}

void GraphicsView::sortAllFoldersDialog()
{
//    if(auto locked = lockFilesCoordinator()){
//        clearControlButtons();

        QComboBox* comboBox = new QComboBox();
        comboBox->addItem(QString("Name"));
        comboBox->addItem(QString("Type"));
        comboBox->addItem(QString("Modification Date"));
        comboBox->addItem(QString("Size"));

        QCheckBox* revChckBx = new QCheckBox(QString("Reverse ordering"));
        QPushButton* okBtn = new QPushButton(QString("ok"));
        QPushButton* cancelBtn = new QPushButton(QString("cancel"));

        QHBoxLayout* hBox = new QHBoxLayout();
        hBox->addWidget(okBtn);
        hBox->addWidget(cancelBtn);

        QVBoxLayout* lay = new QVBoxLayout();
        lay->addWidget(new QLabel(QString("Sort by:")));
        lay->addWidget(comboBox);
        lay->addWidget(revChckBx);
        lay->addLayout(hBox);

        QDialog* dialog = new QDialog();
        dialog->setModal(true);
        dialog->setLayout(lay);
        dialog->setFixedSize(QSize(200,110));

        connect(okBtn, &QPushButton::clicked,
                [=](){
            ORDER_BY ord;
            if(comboBox->currentText() == QString("Name")){
                if(revChckBx->isChecked()){
                    ord = ORDER_BY::R_NAME;
                }else{
                    ord = ORDER_BY::NAME;
                }
            }else if(comboBox->currentText() == QString("Type")){
                if(revChckBx->isChecked()){
                    ord = ORDER_BY::R_TYPE;
                }else{
                    ord = ORDER_BY::TYPE;
                }            }else if(comboBox->currentText() == QString("Size")){
                if(revChckBx->isChecked()){
                    ord = ORDER_BY::R_SIZE;
                }else{
                    ord = ORDER_BY::SIZE;
                }            }else{// if(comboBox->currentText() == QString("Modification Date")){
                if(revChckBx->isChecked()){
                    ord = ORDER_BY::R_MOD_DATE;
                }else{
                    ord = ORDER_BY::MOD_DATE;
                }            }
            dialog->close();

            emit sortAllFolders(ord);

//            executeFileAction([=](auto locked){
//                locked->sortAllFolders(ord);
//            });
//            locked->sortAllFolders(ord);
        });
        connect(cancelBtn, &QPushButton::clicked,
                [=](){
            dialog->close();
        });

        dialog->exec();

//        int retVal = dialog->exec();
//        qDebug() << "retVal: " << retVal;
//    }
}

void GraphicsView::createNewFolder()
{
    emit createNewFolderSGNL();
}

void GraphicsView::createNewFile()
{
    emit createNewFileSGNL();
}

void GraphicsView::openTerminal()
{
    emit openTerminalSGNL();
}

void GraphicsView::showHiddenFiles()
{
    bool currently_showHiddenFiles = false;
    bool ok = false;
    if(auto locked = m_filesCoord.lock())
    {
        currently_showHiddenFiles = locked->includeHiddenFiles();
        ok = true;
    }

    if(ok)
        emit showHiddenFilesSGNL( !currently_showHiddenFiles );
}

void GraphicsView::showRootSelector()
{
    if(auto locked = lockFilesCoordinator()){
        QString curRootDir =locked->getCurRootPath();
        if( !curRootDir.isEmpty() ){
            DirectorySelectorDialog* dialog = new DirectorySelectorDialog(
                    curRootDir,
                    [=](QString filePath){
                        emit setRootFolder(QDir(filePath));
                    }
            );
            dialog->show();
        }
    }
}

void GraphicsView::revalidateRowHeight()
{
    QFont _font(StaticFunctions::getGoshFont(m_fontSize));
    QFontMetrics fm(_font);
    m_rowHeight = fm.height() +6;
}

bool GraphicsView::inSearchMode()
{
    bool inSearchMode = false;
    if(auto locked = m_filesCoord.lock())
        inSearchMode = locked->inSearchMode();
    return inSearchMode;
}

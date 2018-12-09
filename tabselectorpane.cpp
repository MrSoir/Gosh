#include "tabselectorpane.h"

TabSelectorPane::TabSelectorPane(int curActiveTabId,
                                 QVector<QDir> tabLabels,
                                 int scrollOffsetX,
                                 QWidget *parent)
    : QGraphicsView(parent),
      m_scene(new QGraphicsScene(nullptr)),
      m_curActiveTabId(curActiveTabId),
      m_scrollOffsetX(scrollOffsetX)
{
    m_tabLabels = tabLabels;

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    this->setScene(m_scene);
    setMaximumHeight(m_paneHeight);

    revalidate();
}

TabSelectorPane::~TabSelectorPane()
{
    m_scene->clear();
//    this->setScene(nullptr);

    m_tabLabels.clear();

    killScrollTimer();
}

void TabSelectorPane::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    revalidate();
}

void TabSelectorPane::setActiveTabId(int id)
{
    qDebug() << "in TabSelectorPane::setActiveTabId: " << id;
//    if(id == m_curActiveTabId)
//        return;

    m_curActiveTabId = id;

    revalidate();
}

int TabSelectorPane::getScrollOffsetX()
{
    return m_scrollOffsetX;
}

void TabSelectorPane::setTabs(QVector<QDir> tabLabels)
{
    m_tabLabels = tabLabels;

    revalidate();
}

void TabSelectorPane::mouseMoveEvent(QMouseEvent *event)
{
    checkForMouseScrolling( event->x() );

    QGraphicsView::mouseMoveEvent(event);
}


void TabSelectorPane::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    QGraphicsView::enterEvent(event);
}

void TabSelectorPane::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    killScrollTimer();
    QGraphicsView::leaveEvent(event);
}

void TabSelectorPane::checkForMouseScrolling(int mouseX)
{
    int usedWidth = determineUsedPaneWidth();
    int viewportWidth = this->viewport()->width();

    // schauen, ob die zu zeichnenden tabs nicht ins pane passen:
    if(usedWidth > viewportWidth)
    {
        if( mouseX > (viewportWidth-m_scrollAreaWidth) )
        {
            if(!m_isScrollingRight &&
                    (m_scrollOffsetX + usedWidth) > viewportWidth)
            {
                m_isScrollingRight = true;
                m_isScrollingLeft  = false;

                initScrolling();
            }
        }else if(mouseX < m_scrollAreaWidth)
        {
            if(!m_isScrollingLeft &&
                    m_scrollOffsetX < 0)
            {
                m_isScrollingRight = false;
                m_isScrollingLeft  = true;

                initScrolling();
            }
        }else //if( mouseX > m_scrollAreaWidth && mouseX < (viewportWidth-m_scrollAreaWidth))
        {
            if(m_isScrollingLeft || m_isScrollingRight)
                killScrollTimer();
        }
    }else
    {
        if(m_isScrollingLeft || m_isScrollingRight)
            killScrollTimer();
    }
}


//void TabSelectorPane::blockButtons(bool blockBtns)
//{
//    m_blockButtons = blockBtns;
//}
//bool TabSelectorPane::isBlocked()
//{
//    return m_blockButtons;
//}


void TabSelectorPane::revalidate(){
    rePaintCanvas();
}


void TabSelectorPane::rePaintCanvas()
{
    m_scene->clear();
    m_scene->setSceneRect(QRect(0,0, this->viewport()->width(), this->viewport()->height()));

    validateScrollOffset();

    int x = m_scrollOffsetX;
    for(int i=0; i < m_tabLabels.size(); ++i)
    {
        if(i != m_curActiveTabId)
        {
            createTab(x, i);

        }
        x += m_tabWidth + m_tabInsets;
    }
    createTab(m_scrollOffsetX + m_curActiveTabId * (m_tabWidth + m_tabInsets), m_curActiveTabId);

    x += std::abs(m_tabInsets);

//    QPixmap addPixmap(StaticFunctions::getPictureResourceFromFileName(QString("addBtn.png")));
//    int addBtnY = 0;//std::max(0, int(float(m_paneHeight-m_addBtnHeight)/2.0));
//    GraphicItemsBD::PixmapRect* addBtn = new GraphicItemsBD::PixmapRect(addPixmap,
//                                                                        QSize(m_addBtnHeight,m_addBtnHeight),
//                                                                        QPoint(x, addBtnY),
//                                                                        QColor(255,0,0, 255),
//                                                                        QColor(255,100,100, 255));
    GraphicItemsBD::CustomShapeRect* addBtn =
            new GraphicItemsBD::CustomShapeRect(
                [=](QPainter* painter, const QRectF& boundingRect){
                    int penWidth = 2;
                    int padding = 4;

                    QPen pen(QColor(0,0,0, 255));
                    pen.setWidth( penWidth );
                    painter->setPen(pen);

                    QPoint p1(boundingRect.left()+padding, boundingRect.center().y());
                    QPoint p2(boundingRect.right()-padding, boundingRect.center().y());
                    painter->drawLine(p1,p2);

                    p1 = QPoint(boundingRect.center().x(), boundingRect.top()+padding);
                    p2 = QPoint(boundingRect.center().x(), boundingRect.bottom()-padding);
                    painter->drawLine(p1,p2);
                },
                QSize(m_addBtnHeight,m_addBtnHeight),
                QPoint(x, 0),
                QColor(255,255,255, 255),
                QColor(255,255,255, 255),
                QColor(200,255,200, 255),
                QColor(0,255,0, 255));

    addBtn->setCallFunction([=](){
        emit addClicked();
    });
    m_scene->addItem(addBtn);
}





void TabSelectorPane::validateScrollOffset()
{
    int usedPaneWidth = determineUsedPaneWidth();
    int viewportWidth = this->viewport()->width();

    if(usedPaneWidth > viewportWidth)
    {
        if(m_scrollOffsetX + usedPaneWidth < viewportWidth)
        {
            m_scrollOffsetX = viewportWidth - usedPaneWidth;
            killScrollTimer();
        }
        else if(m_scrollOffsetX > 0)
        {
            m_scrollOffsetX = 0;
            killScrollTimer();
        }
    }else
    {
        if(m_scrollOffsetX != 0)
        {
            m_scrollOffsetX = 0;
            killScrollTimer();
        }
    }
}

void TabSelectorPane::initScrolling()
{
    if(m_isScrollingLeft || m_isScrollingRight)
    {
        const int scrollLeft = m_isScrollingLeft;

        m_scrollTimer = new QTimer(this);

        connect(m_scrollTimer, &QTimer::timeout, this, [=](){
            if(scrollLeft)
            {
                m_scrollOffsetX += m_scrollingOffset;
                if(m_scrollOffsetX > 0)
                {
                    m_scrollOffsetX = 0;
                    killScrollTimer();
                }
            }else
            {
                m_scrollOffsetX -= m_scrollingOffset;
                int usedPaneWidth = determineUsedPaneWidth();
                int viewportWidth = this->viewport()->width();
                if(m_scrollOffsetX + usedPaneWidth < viewportWidth)
                {
                    m_scrollOffsetX = viewportWidth - usedPaneWidth;
                    killScrollTimer();
                }
            }

            revalidate();
        });
        m_scrollTimer->start(30);
    }
}

void TabSelectorPane::killScrollTimer()
{
    if(m_scrollTimer != nullptr )
    {
        m_scrollTimer->stop();
        m_scrollTimer->deleteLater();
        m_scrollTimer = nullptr;

        m_isScrollingLeft = false;
        m_isScrollingRight = false;
    }
}

int TabSelectorPane::determineUsedPaneWidth()
{
    int usedWidth = int((m_tabWidth + m_tabInsets) * m_tabLabels.size() + std::abs(m_tabInsets) + m_addBtnHeight);
    return std::max(0, usedWidth);
}

void TabSelectorPane::createTab(int x, int id)
{
    if(id >= m_tabLabels.size())
        return;
    auto* tabRect = createTabRect(QPoint(x, 0),
                                  id,
                                  m_tabLabels[id].dirName());
    if(tabRect)
        m_scene->addItem( tabRect );
}

TabRect* TabSelectorPane::createTabRect(QPoint pos, const int id, QString tabLabel) const
{
    QFont font = StaticFunctions::getGoshFont(8);

    QColor col1    = (id == m_curActiveTabId) ? m_activeCol1 : m_tabCol1;
    QColor col2    = (id == m_curActiveTabId) ? m_activeCol2 : m_tabCol2;
    QColor txtCol1 = (id == m_curActiveTabId) ? m_textColActiv1 : m_textCol1;
    QColor txtCol2 = (id == m_curActiveTabId) ? m_textColActiv2 : m_textCol2;

    TabRect* tabRect = new TabRect(tabLabel,
                                   font,
                                   QSize(m_tabWidth, m_tabHeight),
                                   pos,
                                   col1,
                                   col2,
                                   m_selCol1,
                                   m_selCol2,
                                   txtCol1,
                                   txtCol2);

    tabRect->setCallbackFunctions([=](){emit tabClicked(id);},
                                  [=](){emit tabCloseClicked(id);}
                                 );
    return tabRect;
}



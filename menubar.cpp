#include "menubar.h"


MenuBar::MenuBar(qreal maxWidthOrHeight,
        QPointF centerPosition,
        ORIENTATION orientation,
        bool m_centerFromEnd,
        QGraphicsItem* parent)
    : GraphicItemsBD::GraphicsItemBD(QSize(0,0), QPoint(0,0), parent),
      m_orientation(orientation),
      m_centerFromEnd(m_centerFromEnd),
      m_centerOrientation(centerPosition),
      m_maxWidthOrHeight(maxWidthOrHeight)
{
    revalidateSize();
}

MenuBar::~MenuBar(){
//    qDebug() << "in MenuBar.DEstructor";

    m_mouInBtns.clear();
    m_groupingMap.clear();
    m_groupSeparators.clear();
    m_groupingMap.clear();

    m_btnFuncs.reset();
    m_paintingFuncs.reset();
    m_groupingFunc = nullptr;
}

void MenuBar::setPosition(QPoint position){
    m_centerOrientation += position;
}

QRectF MenuBar::boundingRect() const{
    QRectF rct;
    if(m_centerFromEnd){
        if(m_orientation == ORIENTATION::HORIZONTAL){
            rct = QRectF(m_centerOrientation.x() - m_size.width()*0.5,
                          m_centerOrientation.y() - m_size.height(),
                          m_size.width(),
                          m_size.height());
        }else{
            rct = QRectF(m_centerOrientation.x() - m_size.width(),
                          m_centerOrientation.y() - m_size.height()*0.5,
                          m_size.width(),
                          m_size.height());
        }
    }else{
        if(m_orientation == ORIENTATION::HORIZONTAL){
            rct = QRectF(m_centerOrientation.x() - m_size.width()*0.5,
                      m_centerOrientation.y(),
                      m_size.width(),
                      m_size.height());
        }else{
            rct = QRectF(m_centerOrientation.x(),
                          m_centerOrientation.y() - m_size.height()*0.5,
                          m_size.width(),
                          m_size.height());
        }
    }
    return rct;
}
void MenuBar::setOrientation(ORIENTATION orientation){
    m_orientation = orientation;
    revalidateSize();
    revalidate = true;
    update();
}
void MenuBar::revalidateSize(){
    if(orientation() == ORIENTATION::HORIZONTAL){
        m_size = QSize(m_maxWidthOrHeight, 0);
    }else{
        m_size = QSize(0,m_maxWidthOrHeight);
    }

    std::pair<int,int> dimensions = evalColumnAndRowDimensions();

    int anzRows    = dimensions.first;
    int anzColumns = dimensions.second;

    qreal borderOffsWidth  = orientation() == ORIENTATION::HORIZONTAL ? bordersToPaint() * (m_groupingBorder+m_groupingPadding) : 0;
    qreal borderOffsHeight = orientation() == ORIENTATION::VERTICAL   ? bordersToPaint() * (m_groupingBorder+m_groupingPadding) : 0;

    qreal paneWidth  = anzColumns * (m_btnEdge+m_offsets) + 2.0*m_padding + borderOffsWidth;
    qreal paneHeight = anzRows    * (m_btnEdge+m_offsets) + 2.0*m_padding + borderOffsHeight;
    m_size = QSize(paneWidth, paneHeight);

    m_anzColumns = anzColumns;
    m_anzRows = anzRows;
}

void MenuBar::setCaller(std::shared_ptr<DynamicFunctionCaller<QString, std::function<void()>>> btnFunctions,
               std::shared_ptr<DynamicFunctionCaller<QString, std::function<void(QPainter*, QRectF)>>> buttonPaintingFunctions,
               std::function<int(int)> groupingFunc){
    m_groupingFunc = groupingFunc;

    this->m_btnFuncs = btnFunctions;
    this->m_paintingFuncs = buttonPaintingFunctions;
    m_btnsCount = btnFunctions->formulaCount();
    m_buttons.clear();
    m_mouInBtns.clear();
    for(int i=0; i < m_btnsCount; i++){
        m_buttons.append(QRectF(0.,0., 0.,0.));
        m_mouInBtns.append(false);
    }

    revalGroupMap();

    revalidateSize();
}

void MenuBar::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    Q_UNUSED(widget)
    Q_UNUSED(option)

    // gaaanz wichtig: erstmal painter an boundingRect clippen!!!:
    QRectF rct = boundingRect();

    painter->setRenderHint(QPainter::Antialiasing, true);

    painter->setClipRect(rct);

    int alpha = 200;
    QColor gradCol1(255,255,255, alpha);
    QColor gradCol2(255,255,255, alpha);
    StaticFunctions::paintRect(painter, rct, gradCol1, gradCol2);

    int groupBordStrokeWidth = 2;
    int groupBorderPadding = 1;

    if(revalidate){
        qreal startX;
        qreal startY;
        if(orientation() == ORIENTATION::HORIZONTAL){
            startX = m_centerOrientation.x() - m_size.width()*0.5 + m_padding;
            if(m_centerFromEnd){
                startY = m_centerOrientation.y() - m_size.height() + m_padding;
            }else{
                startY = m_centerOrientation.y() + m_padding;
            }
        }else{
            startY = m_centerOrientation.y() - m_size.height()*0.5 + m_padding;
            if(m_centerFromEnd){
                startX = m_centerOrientation.x() - m_size.width() + m_padding;
            }else{
                startX = m_centerOrientation.x() + m_padding;
            }
        }

        int groupBordCount = 0;
        m_groupSeparators.clear();
        int curCol = 0;
        int curRow = 0;
        int curGroupId = 0;
        int maxColumns = m_anzColumns;
        auto it = m_groupingMap.begin();

        for(int i=0; i < m_buttons.size(); i++){
            if(it == m_groupingMap.end())
                break;

            int btnId = it->second[curGroupId];

            qreal curX = startX + curCol * (m_btnEdge + m_offsets);
            qreal curY = startY + curRow * (m_btnEdge + m_offsets);

            if(orientation() == ORIENTATION::VERTICAL)
                curY += groupBordCount * (groupBordStrokeWidth + 2);
            else
                curX += groupBordCount * (groupBordStrokeWidth + 2);

            m_buttons[btnId].setX( curX );
            m_buttons[btnId].setY( curY );

            m_buttons[btnId].setWidth (m_btnEdge);
            m_buttons[btnId].setHeight(m_btnEdge);

//            qDebug() << "painting: curGroupId: " << curGroupId << " curRow: " << curRow << "    curCol: " << curCol;

            ++curGroupId;

            bool endOfGroupReached = false;

            if(curGroupId >= it->second.size())
            {
                endOfGroupReached = true;

                ++groupBordCount;

                curGroupId = 0;

                ++it;
            }

//            qDebug() << "   endOfGroupReached: " << endOfGroupReached;

            if(orientation() == ORIENTATION::VERTICAL){
                if(endOfGroupReached)
                {
                    ++curRow;
                    curCol = 0;
                }else
                {
                    curCol = (curCol+1) % m_anzColumns;
                    if(curCol == 0)
                        ++curRow;
                }
            }else{
                if(endOfGroupReached)
                {
                    ++curCol;
                    curRow = 0;
                }else
                {
                    curRow = (curRow+1) % m_anzRows;
                    if(curRow == 0)
                        ++curCol;
                }
            }

            if(endOfGroupReached && i < m_buttons.size()-1)
            {
                qreal maxWidth = m_anzColumns * (m_btnEdge + m_offsets);
                if(orientation() == ORIENTATION::VERTICAL){
                    curY += (m_btnEdge + m_offsets) + groupBorderPadding;
                    std::pair<QPointF,QPointF> borderPoints(QPointF(startX + m_padding, curY), QPointF(startX + maxWidth - m_padding, curY));
                    m_groupSeparators.push_back( borderPoints );
                }else
                {
                    curX += (m_btnEdge + m_offsets) + groupBorderPadding;
                    std::pair<QPointF,QPointF> borderPoints(QPointF(curX, startY + m_padding), QPointF(curX, startY + maxWidth - m_padding));
                    m_groupSeparators.push_back( borderPoints );
                }
            }
        }
    }
    for(int i=0; i < m_buttons.size(); i++){
        QColor nextColor1, nextColor2;
        if(m_mouInBtns[i]){
            nextColor1 = QColor(255, 255, 255, 255);
            nextColor2 = QColor(100, 255, 100, 255);
        }else{
            nextColor1 = QColor(255, 255, 255, 255);
            nextColor2 = QColor(255, 255, 255, 255);
        }
        StaticFunctions::paintRect(painter, m_buttons[i], nextColor1, nextColor2);
        m_paintingFuncs->getFunction(QString("paintingFunction%1").arg(i))(painter, m_buttons[i]);
    }

    QPen linePen(QColor(0,255,0));
    linePen.setWidth( groupBordStrokeWidth );
    painter->setPen(linePen);

    for(int i=0; i < m_groupSeparators.size(); ++i)
    {
        painter->drawLine(m_groupSeparators[i].first, m_groupSeparators[i].second);
    }

    revalidate = false;
}

ORIENTATION MenuBar::orientation(){return m_orientation;}

void MenuBar::mousePressEvent(QGraphicsSceneMouseEvent *event){
    QPointF mouP = event->pos();
    for(int i=0; i < m_buttons.size(); i++){
        if(m_buttons[i].contains(mouP) && m_btnFuncs){
            m_btnFuncs->getFunction(QString("buttonFunction%1").arg(i))();
        }
    }
    qint64 curTime = QDateTime::currentMSecsSinceEpoch();
    if(curTime - lastTmePrsd < 300){

    }
    lastTmePrsd = curTime;
    update();
}
void MenuBar::hoverMoveEvent(QGraphicsSceneHoverEvent * event){
    QPointF mouP = event->pos();
    bool anyBtnTrue = false;
    bool updt = false;
    for(int i=0; i < m_buttons.size(); i++){
        if(m_buttons.at(i).contains(mouP)){
            if(!m_mouInBtns.at(i)){
                m_mouInBtns[i] = true;
                updt = true;
            }
        }else{
            if(m_mouInBtns.at(i))
                anyBtnTrue = true;
        }
    }
    if(anyBtnTrue){
        for(int i=0; i < m_buttons.size(); i++){
            m_mouInBtns[i] = false;
        }
        updt = true;
    }
    if(updt)
        update();
    //        return QGraphicsItem::hoverEnterEvent(event);
}

int MenuBar::groupCount()
{
    return m_groupingMap.size();
}

void MenuBar::revalGroupMap()
{
    m_groupingMap.clear();

    if(!m_groupingFunc)
    {
        std::vector<int> vec;
        for(int i=0; i < m_btnsCount; ++i)
        {
            vec.push_back(i);
        }
        m_groupingMap[0] = vec;
    }else
    {
        for(int i=0; i < m_btnsCount; ++i)
        {
            int groupId = m_groupingFunc(i);

            if(m_groupingMap[groupId].empty())
            {
                std::vector<int> vec;
                vec.push_back(i);
                m_groupingMap[groupId] = vec;
            }else
            {
                m_groupingMap[groupId].push_back(i);
            }
        }
    }

//    int i=0;
//    qDebug() << "revalGroupMap: m_groupingFunc is valid: " << (m_groupingFunc ? "true" : "false");
//    for(auto it = m_groupingMap.begin(); it != m_groupingMap.end(); ++it, ++i)
//    {
//        qDebug() << "   group[" << i << "]: key: " << int(it->first) << "    val: " << this->vec_to_str(it->second);
//    }
}

// return value std::pair<int , int    > zu interpretieren als:
//              std::pair<rows, columns>
std::pair<int, int> MenuBar::evalColumnAndRowDimensions()
{
    if (orientation() == ORIENTATION::VERTICAL ){

        if( paneHeight() > getButtonSpace( int(buttonCount()) ) )
        {
//            qDebug() << "in I";
            return std::pair<int,int>( int(buttonCount()), 1);
        }else
        {
//            qDebug() << "in II";
            qreal maxElmntsPerColumn = getMaxElementsPerRowOrColumn();

            if( groupCount() <= 1 )
            {
//                qDebug() << "in II-I";
                qreal anzColumns = ceil( buttonCount() / maxElmntsPerColumn );
                qreal anzRows = ceil( buttonCount() / anzColumns );

                return std::pair<int,int>(anzRows, anzColumns);
            }else if (groupCount() >= maxElmntsPerColumn)
            {
//                qDebug() << "in II-II";

                return std::pair<int,int>(groupCount(), getBiggestGroupSize());
            }else
            {
                // trial and error:
//                qDebug() << "in II-III";


                bool ok = false;
                int anzRows = 0;
                int anzColumns = 0;

                while( !ok && anzRows < m_btnsCount )
                {
                    ++anzColumns;
                    int curId = 0;
                    int curRow = 0;

                    for(auto it = m_groupingMap.begin(); it != m_groupingMap.end(); ++it)
                    {
                        for(int j=0; j < it->second.size(); ++j)
                        {
                            ++curId;
                            if( curId % anzColumns == 0 )
                                ++curRow;
                        }
                        if( !(curId % anzColumns == 0) )
                            ++curRow;

                        if(curRow >= maxElmntsPerColumn)
                            break;
                    }

                    anzRows = curRow;

                    if(curRow < maxElmntsPerColumn)
                        ok = true;
                }
                return std::pair<int,int>(anzRows, anzColumns);
            }
        }
    }else
    {
        if( paneWidth()  > getButtonSpace( int(buttonCount()) ) )
        {
            return std::pair<int,int>(1, int(buttonCount()));
        }else
        {
            qreal maxElmntsPerRow = getMaxElementsPerRowOrColumn();

            if( groupCount() <= 1 )
            {
                qreal anzRows = ceil( buttonCount() / maxElmntsPerRow );
                qreal anzColumns = ceil( buttonCount() / anzRows );

                return std::pair<int,int>(anzRows, anzColumns);

            }else if (groupCount() >= maxElmntsPerRow)
            {
                return std::pair<int,int>(getBiggestGroupSize(), groupCount());
            }else
            {
                // trial and error:

                bool ok = false;
                int anzRows = 0;
                int anzColumns = 0;

                while( !ok && anzColumns < m_btnsCount )
                {
                    ++anzRows;
                    int curId = 0;
                    int curColumn= 0;

                    for(auto it = m_groupingMap.begin(); it != m_groupingMap.end(); ++it)
                    {
                        for(int j=0; j < it->second.size(); ++j)
                        {
                            ++curId;
                            if( curId % anzRows == 0 )
                                ++curColumn;
                        }
                        if( !(curId % anzRows == 0) )
                            ++curColumn;

                        if(curColumn >= maxElmntsPerRow)
                            break;
                    }

                    anzColumns = curColumn;

                    if(curColumn < maxElmntsPerRow)
                        ok = true;
                }
                return std::pair<int,int>(anzRows, anzColumns);
            }
        }
    }
}

QString MenuBar::vec_to_str(const std::vector<int> vec)
{
    if(vec.empty())
        return QString("[empty]");

    std::stringstream ss;
    ss << "[";
    for(int i=0; i < vec.size(); ++i)
    {
        ss << vec[i];
        if(i < (vec.size()-1))
        {
            ss << ", ";
        }
    }
    ss << "]";
    return QString(ss.str().c_str());
}

int MenuBar::getMaxElementsPerRowOrColumn()
{
    if (orientation() == ORIENTATION::VERTICAL )
    {
        return int(floor( paintHeight() / (m_btnEdge+m_offsets) ));
    }else
    {
        return int(floor( paintWidth()  / (m_btnEdge+m_offsets) ));
    }
}

qreal MenuBar::paintHeight()
{
    qreal grpBord = (qreal)(m_groupingBorder + m_groupingPadding);

    return paneHeight() - 2.0*m_padding - bordersToPaint() * grpBord;
}
qreal MenuBar::paintWidth()
{    qreal grpBord = (qreal)m_groupingBorder;

    return paneWidth() - 2.0*m_padding - bordersToPaint() * grpBord;
}

qreal MenuBar::paneWidth()
{
    return (qreal)m_size.width();;
}
qreal MenuBar::paneHeight()
{
    return (qreal)m_size.height();;
}

qreal MenuBar::bordersToPaint()
{
    qreal grpCnt = (qreal)groupCount();
    return qreal(std::max(qreal(0), grpCnt-1.0));
}

qreal MenuBar::buttonCount()
{
    return (qreal)m_btnsCount;
}

int MenuBar::getBiggestGroupSize()
{
    int biggestGroup = 0;
    for(auto it = m_groupingMap.begin(); it != m_groupingMap.end(); ++it)
    {
        int curGroupSize = it->second.size();
        if(curGroupSize > biggestGroup)
            biggestGroup = curGroupSize;
    }
    return biggestGroup;
}

qreal MenuBar::getButtonSpace(int btnCount)
{
    return    btnCount*(m_btnEdge+m_offsets)
            + 2.0*m_padding
            + bordersToPaint()*(m_groupingBorder+m_groupingPadding);
}

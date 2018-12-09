#include "directoryselectordialog.h"

DirectorySelectorDialog::DirectorySelectorDialog(
        QString curDir,
        std::function<void(QString)> caller,
        QSizeF size,
        QPointF pos,
        QWidget *parent)
    : m_size(size),
      m_pos(pos),
      m_caller(caller),
      m_curDir(curDir),
      m_dirSelDlgVwr(new DirectorySelectorDialogViewer(curDir, [=](QString filePath){caller(filePath);close();}, size)),
      QDialog(parent)
{
    setMouseTracking(true);

    StaticFunctions::setIconToWidget(this);

    this->setModal(true);

//    this->setMinimumSize((int)m_size.width(), (int)m_size.height());
//    this->setMaximumSize((int)m_size.width(), (int)m_size.height());
    this->setMaximumHeight((int)m_size.height());
    this->setMinimumWidth((int)m_size.width());
    this->setMaximumWidth((int)m_size.width());
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);

    QPushButton* closeBtn = new QPushButton("close window");
    QObject::connect(closeBtn, &QPushButton::clicked, [=](){
        this->close();
    });

    QVBoxLayout* vBox = new QVBoxLayout();
    vBox->setAlignment(Qt::AlignBottom);
    vBox->addWidget(m_dirSelDlgVwr);
    vBox->addWidget(closeBtn);
    vBox->setSpacing(0);
    vBox->setContentsMargins(0, 0, 0, 0);
    this->setLayout(vBox);
}

QRectF DirectorySelectorDialog::boundingRect(){return QRectF(m_pos, m_size);}

void DirectorySelectorDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter* painter = new QPainter(this);

    // gaaanz wichtig: erstmal painter an boundingRect clippen!!!:
    QRectF rct = boundingRect();

    painter->setRenderHint(QPainter::Antialiasing, true);

    painter->setClipRect(rct);

    QLinearGradient gradient(rct.topLeft(), rct.bottomRight());
    gradient.setColorAt(0, QColor(255,255,255, 150));
    gradient.setColorAt(1, QColor(220,220,255, 150));
    painter->fillRect(rct, gradient);

//    painter->end();
}






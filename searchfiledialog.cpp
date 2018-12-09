#include "searchfiledialog.h"

SearchFileDialog::SearchFileDialog(QWidget *parent) : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);

    QVBoxLayout* layt = new QVBoxLayout();

    m_searchKeyword = new QLineEdit();
    m_searchKeyword->setPlaceholderText(QObject::tr("enter search keyword"));

    m_deepSearch = new QCheckBox();
    m_deepSearch->setCheckable(true);
    m_deepSearch->setChecked(false);
    QHBoxLayout* deppSearchLyt = new QHBoxLayout();
    deppSearchLyt->addWidget(new QLabel("deep search:"));
    deppSearchLyt->addWidget(m_deepSearch);

    QPushButton* okBtn = new QPushButton(QObject::tr("ok"));
    QPushButton* cnclBtn = new QPushButton(QObject::tr("cancel"));
    QHBoxLayout* confirmBtnLyt = new QHBoxLayout();
    confirmBtnLyt->addWidget(okBtn);
    confirmBtnLyt->addWidget(cnclBtn);

//    connect( this, SIGNAL(somethingChanged()), SLOT(handleChange()), Qt::DirectConnection );
    connect(okBtn, &QPushButton::clicked, [=](){
        onOkClicked();
    });
    connect(cnclBtn,&QPushButton::clicked, [=](){
        close();
    });

    this->setLayout(layt);
    layt->addWidget(new QLabel("enter search keyword:"));
    layt->addWidget(m_searchKeyword);
    layt->addLayout(deppSearchLyt);
    layt->addLayout(confirmBtnLyt);
}

void SearchFileDialog::keyPressEvent(QKeyEvent *event)
{
    switch( event->key() )
    {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        onOkClicked();
        break;
    case Qt::Key_Escape:
        close();
        break;
    case Qt::Key_Tab:
        if(m_deepSearch->hasFocus())
            m_searchKeyword->setFocus();
        else
            m_deepSearch->setFocus();
        break;
    }
}

void SearchFileDialog::onOkClicked()
{
    QTimer::singleShot(0, this, [=](){
        emit okClicked(m_searchKeyword->text(), m_deepSearch->isChecked());
    });
    close();
}

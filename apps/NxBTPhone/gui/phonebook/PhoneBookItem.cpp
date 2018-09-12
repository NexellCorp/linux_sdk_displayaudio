#include "PhoneBookItem.h"
#include <QVBoxLayout>

PhoneBookItem::PhoneBookItem(int width, int height, QWidget *parent) :
    QWidget(parent)
{
    QFont font;

    m_pTitleLabel = new QLabel(this);
    m_pTitleLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_pTitleLabel->setFixedSize(width, height/2);
    font = m_pTitleLabel->font();
    font.setPointSize(25);
    m_pTitleLabel->setFont(font);

    m_pDataLabel = new QLabel(this);
    m_pDataLabel->setStyleSheet("color: blue;");
    m_pDataLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_pDataLabel->setFixedSize(width, height/2);
    font.setPointSize(30);
    m_pDataLabel->setFont(font);

    QVBoxLayout* l = new QVBoxLayout(this);
    l->addWidget(m_pTitleLabel);
    l->addWidget(m_pDataLabel, 1);

    setLayout(l);
}

PhoneBookItem::~PhoneBookItem()
{

}

void PhoneBookItem::setTitle(QString title)
{
    m_pTitleLabel->setText(title);
}

QString PhoneBookItem::title()
{
    return m_pTitleLabel->text();
}

void PhoneBookItem::setData(QString data)
{
    QFont f = m_pDataLabel->font();

    while (1) {
        QFontMetrics fm(f);
        if (m_pDataLabel->width() <= fm.width(data)) {
            f.setPointSize(f.pointSize() - 1);
        } else {
            break;
        }
    }

    m_pDataLabel->setFont(f);
    m_pDataLabel->setText(data);
}

QString PhoneBookItem::data()
{
    return m_pDataLabel->text();
}

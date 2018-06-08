#include "CallLogItem.h"
#ifndef __set_layout_by_source_code__
#include "ui_CallLogItem.h"
#else
#include <QHBoxLayout>
#include <QVBoxLayout>
#endif

CallLogItem::CallLogItem(QWidget *parent) :
    QWidget(parent)
#ifndef __set_layout_by_source_code__
  , ui(new Ui::CallLogItem)
#endif
{
#ifndef __set_layout_by_source_code__
    ui->setupUi(this);
#else
    /*               Layout
     * ------------------------------------
     * | Call diretion  | number owner    |
     * | (S or R or M)  |-----------------|
     * |                | number type     |
     * ------------------------------------
     */

    m_pCallDirectionLabel = new QLabel(this);
    m_pCallDirectionLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    m_pCallNumberOwnerLabel = new QLabel(this);
    m_pCallNumberOwnerLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    m_pCallNumberTypeLabel = new QLabel(this);
    m_pCallNumberTypeLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QVBoxLayout* vl = new QVBoxLayout(this);
    vl->addWidget(m_pCallNumberOwnerLabel);
    vl->addWidget(m_pCallNumberTypeLabel);

    QHBoxLayout* hl = new QHBoxLayout(this);
    hl->addWidget(m_pCallDirectionLabel);
    hl->addLayout(vl);

    setLayout(hl);
#endif
}

CallLogItem::~CallLogItem()
{
#ifndef __set_layout_by_source_code__
    delete ui;
#endif
}

void CallLogItem::setCallDirection(QString text)
{
#ifndef __set_layout_by_source_code__
    ui->LABEL_CALL_DIRECTION->setText(text);
#else
    m_pCallDirectionLabel->setText(text);
#endif
}

QString CallLogItem::callDirection()
{
#ifndef __set_layout_by_source_code__
    return ui->LABEL_CALL_DIRECTION->text();
#else
    return m_pCallDirectionLabel->text();
#endif
}

void CallLogItem::setCallNumberOwner(QString text)
{
#ifndef __set_layout_by_source_code__
    ui->LABEL_CALL_NUMBER_OWNER->setText(text);
#else
    m_pCallNumberOwnerLabel->setText(text);
#endif
}

QString CallLogItem::callNumberOwner()
{
#ifndef __set_layout_by_source_code__
    return ui->LABEL_CALL_NUMBER_OWNER->text();
#else
    return m_pCallNumberOwnerLabel->text();
#endif
}

void CallLogItem::setCallNumberType(QString text)
{
#ifndef __set_layout_by_source_code__
    ui->LABEL_CALL_NUMBER_TYPE->setText(text);
#else
    m_pCallNumberTypeLabel->setText(text);
#endif
}

QString CallLogItem::callNumberType()
{
#ifndef __set_layout_by_source_code__
    return ui->LABEL_CALL_NUMBER_TYPE->text();
#else
    return m_pCallNumberTypeLabel->text();
#endif
}

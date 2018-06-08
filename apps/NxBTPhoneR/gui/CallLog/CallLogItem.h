#ifndef CALLLOGITEM_H
#define CALLLOGITEM_H

#include <QWidget>
#include <QLabel>

//#define __set_layout_by_source_code__

#ifndef __set_layout_by_source_code__
namespace Ui {
class CallLogItem;
}
#endif

class CallLogItem : public QWidget
{
    Q_OBJECT

public:
    explicit CallLogItem(QWidget *parent = 0);
    ~CallLogItem();

    void setCallDirection(QString text);

    QString callDirection();

    void setCallNumberOwner(QString text);

    QString callNumberOwner();

    void setCallNumberType(QString text);

    QString callNumberType();

#ifndef __set_layout_by_source_code__
private:
    Ui::CallLogItem *ui;
#else
    QLabel* m_pCallDirectionLabel;
    QLabel* m_pCallNumberOwnerLabel;
    QLabel* m_pCallNumberTypeLabel;
#endif
};

#endif // CALLLOGITEM_H

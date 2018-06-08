#ifndef PHONEBOOKITEM_H
#define PHONEBOOKITEM_H

#include <QWidget>
#include <QLabel>

class PhoneBookItem : public QWidget
{
    Q_OBJECT

public:
    explicit PhoneBookItem(int width, int height, QWidget *parent = 0);
    ~PhoneBookItem();

    void setTitle(QString title);

    QString title();

    void setData(QString data);

    QString data();

private:
    QLabel* m_pTitleLabel;

    QLabel* m_pDataLabel;

};

#endif // PHONEBOOKITEM_H

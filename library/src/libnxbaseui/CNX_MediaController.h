#ifndef CNX_MEDIACONTROLLER_H
#define CNX_MEDIACONTROLLER_H

#include <QFrame>

class CNX_MediaController : public QFrame
{
    Q_OBJECT

public:
    CNX_MediaController(QWidget *parent /*= 0*/);
    ~CNX_MediaController();

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    QWidget* m_pParent;
    QWidget* m_pChild;
    QObject* m_pObject;
};

#endif // CNX_MEDIACONTROLLER_H

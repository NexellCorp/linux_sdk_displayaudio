#ifndef CNX_MESSAGEBOX_H
#define CNX_MESSAGEBOX_H

#include <QDialog>
#include <QMessageBox>

namespace Ui {
class CNX_MessageBox;
}

class CNX_MessageBox : public QDialog
{
    Q_OBJECT

public:
	explicit CNX_MessageBox(QWidget *parent = 0);
	~CNX_MessageBox();

public:
    void show();
    void hide();
    int exec();

    void setDefaultButton(QPushButton * button);
    void setDetailedText(const QString & text);
    void setEscapeButton(QAbstractButton * button);
    void setInformativeText(const QString & text);
    void setText(const QString & text);
    void setTextFormat(Qt::TextFormat format);
    void setWindowModality(Qt::WindowModality windowModality);
    void setWindowTitle(const QString & title);

private:
    QMessageBox m_MsgBox;
	Ui::CNX_MessageBox *ui;
};

#endif // CNX_MESSAGEBOX_H

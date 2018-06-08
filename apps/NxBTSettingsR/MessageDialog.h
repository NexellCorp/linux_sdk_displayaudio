#ifndef MESSAGEDIALOG_H
#define MESSAGEDIALOG_H

#include <QDialog>

namespace Ui {
class MessageDialog;
}

class MessageDialog : public QDialog
{
    Q_OBJECT

public:
    enum ButtonStatePolicy {
        ButtonStatePolicy_Ok_And_Cancel,
        ButtonStatePolicy_Only_Ok,
        ButtonStatePolicy_Only_Cancel
    };

    enum ButtonType {
        ButtonType_Ok,
        ButtonType_Cancel
    };

public:
    explicit MessageDialog(QWidget *parent = 0);
    ~MessageDialog();

    void setTitle(QString text);

    void setMessage(QString text);

    void setButtonStatePolicy(ButtonStatePolicy);

    void setButtonText(ButtonType type, QString text);

private slots:
    void on_BUTTON_CANCEL_clicked();

    void on_BUTTON_OK_clicked();

private:
    ButtonStatePolicy m_ButtonStatePolicy;

private:
    Ui::MessageDialog *ui;
};

#endif // MESSAGEDIALOG_H

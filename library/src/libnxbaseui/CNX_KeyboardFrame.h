#ifndef CNX_KEYBOARDFRAME_H
#define CNX_KEYBOARDFRAME_H

#include <QFrame>
#include <QKeyEvent>

namespace Ui {
class CNX_KeyboardFrame;
}

class CNX_KeyboardFrame : public QFrame
{
    Q_OBJECT

signals:
    void signalKeyEvent(QString block);

public:
	explicit CNX_KeyboardFrame(QWidget *parent = 0);
	~CNX_KeyboardFrame();

	void SetObject(QObject* object);

	void SetEnterKeyText(QString);

	void SetSpaceKeyPosition(int x, int y);

	void SetSpacekeySize(int width, int height);

	void SetEnabled(int key, bool enabled);

private:
	void Initialize();

private slots:
    void on_BUTTON_KEY_1_clicked();

    void on_BUTTON_KEY_2_clicked();

    void on_BUTTON_KEY_3_clicked();

    void on_BUTTON_KEY_4_clicked();

    void on_BUTTON_KEY_5_clicked();

    void on_BUTTON_KEY_6_clicked();

    void on_BUTTON_KEY_7_clicked();

    void on_BUTTON_KEY_8_clicked();

    void on_BUTTON_KEY_9_clicked();

    void on_BUTTON_KEY_0_clicked();

    void on_BUTTON_KEY_Q_clicked();

    void on_BUTTON_KEY_W_clicked();

    void on_BUTTON_KEY_E_clicked();

    void on_BUTTON_KEY_R_clicked();

    void on_BUTTON_KEY_T_clicked();

    void on_BUTTON_KEY_Y_clicked();

    void on_BUTTON_KEY_U_clicked();

    void on_BUTTON_KEY_I_clicked();

    void on_BUTTON_KEY_O_clicked();

    void on_BUTTON_KEY_P_clicked();

    void on_BUTTON_KEY_A_clicked();

    void on_BUTTON_KEY_S_clicked();

    void on_BUTTON_KEY_D_clicked();

    void on_BUTTON_KEY_F_clicked();

    void on_BUTTON_KEY_G_clicked();

    void on_BUTTON_KEY_H_clicked();

    void on_BUTTON_KEY_J_clicked();

    void on_BUTTON_KEY_K_clicked();

    void on_BUTTON_KEY_L_clicked();

    void on_BUTTON_KEY_Z_clicked();

    void on_BUTTON_KEY_X_clicked();

    void on_BUTTON_KEY_C_clicked();

    void on_BUTTON_KEY_V_clicked();

    void on_BUTTON_KEY_B_clicked();

    void on_BUTTON_KEY_N_clicked();

    void on_BUTTON_KEY_M_clicked();

    void on_BUTTON_KEY_COMMA_clicked();

    void on_BUTTON_KEY_DOT_clicked();

    void on_BUTTON_KEY_BAR_clicked();

    void on_BUTTON_KEY_UNDER_BAR_clicked();

    void on_BUTTON_KEY_SPACE_clicked();

    void on_BUTTON_KEY_BACK_SPACE_clicked();

    void on_BUTTON_ENTER_clicked();

    void on_BUTTON_KEY_SHIFT_toggled(bool checked);

private:
	void InputKey(int key);

private:
	QObject* m_pDestinationPtr;

private:
	Ui::CNX_KeyboardFrame *ui;
};

#endif // CNX_KEYBOARDFRAME_H

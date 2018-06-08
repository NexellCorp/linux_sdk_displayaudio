#ifndef PLAYLISTWINDOW_H
#define PLAYLISTWINDOW_H

#include <QDialog>
#include <QListWidgetItem>

//	Base UI
#include <CNX_StatusBar.h>

#include "CNX_FileList.h"

namespace Ui {
class PlayListWindow;
}

class PlayListWindow : public QDialog
{
	Q_OBJECT

public:
	explicit PlayListWindow(QWidget *parent = 0);
	~PlayListWindow();

public:
	void setList(CNX_FileList *pFileList);
	void setCurrentIndex(int idx);
	int getCurrentIndex();

public:
	QModelIndex	m_selectIdx;
	int			m_pretIdx;
	int			m_selectCount;

private slots:
	void on_btnCancel_released();
	void on_btnOk_released();
	void on_listWidget_itemClicked(QListWidgetItem *item);

private:
	Ui::PlayListWindow *ui;
	//	UI Status Bar
	CNX_StatusBar* m_pStatusBar;
};

#endif // PLAYLISTWINDOW_H

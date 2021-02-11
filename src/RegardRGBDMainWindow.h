/**********************************************************************************************//**
 * \file RegardRGBDMainWindow.h
 * \author R. Hiestand
 * \date Feb 10, 2021
 *
 * \brief Main window of Scanner App.
 **************************************************************************************************/

#ifndef REGARD_RGBD_MAIN_WINDOW_H
#define REGARD_RGBD_MAIN_WINDOW_H


 // Qt
#include <QMainWindow>

#include "ui_regardrgbd.h"

class RegardRGBDMainWindow : public QMainWindow, public Ui::RegardRGBDMainWindow
{
	Q_OBJECT
public:

	RegardRGBDMainWindow();
	virtual ~RegardRGBDMainWindow();

public slots:
	virtual void slotAbout();
	virtual void slotOneShotTimer();

protected:
	void closeEvent(QCloseEvent* event) Q_DECL_OVERRIDE;

private:
};

#endif

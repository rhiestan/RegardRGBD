/**********************************************************************************************//**
 * \file RegardRGBDMainWindow.h
 * \author R. Hiestand
 * \date Feb 10, 2021
 *
 * \brief Main window of Scanner App.
 **************************************************************************************************/

#ifndef REGARD_RGBD_MAIN_WINDOW_H
#define REGARD_RGBD_MAIN_WINDOW_H

class ONIDevice;
class ONIToQtConverter;
class ScanImageTo3D;
class ONI3DConverter;

#include <memory>
#include <atomic>

// Qt
#include <QMainWindow>

#include "ui_regardrgbd.h"

#include "RegardRGBDModelViewHelper.h"
#include "ConverterInterface.h"

class RegardRGBDMainWindow : public QMainWindow, public Ui::RegardRGBDMainWindow
{
	Q_OBJECT
public:

	RegardRGBDMainWindow();
	virtual ~RegardRGBDMainWindow();

	void update3DScanMesh();

public slots:
	virtual void slotAbout();
	virtual void slotOneShotTimer();
	virtual void slotConnectOpenNI();
	virtual void slotDisconnectOpenNI();

	virtual void slotScan3DMeshChanged();

signals:
	void scan3DMeshChanged();

protected:
	void closeEvent(QCloseEvent* event) Q_DECL_OVERRIDE;

private:
	std::unique_ptr<RegardRGBDModelViewHelper> pRegardRGBDModelViewHelper_;

	std::unique_ptr<ONIDevice> pONIDevice_;
	std::unique_ptr<ONIToQtConverter> pONIToQtConverter_;
	std::unique_ptr<ScanImageTo3D> pScanImageTo3D_;
	std::unique_ptr<ONI3DConverter> pONI3DConverter_;

	std::atomic<bool> isDrawingScan3DMesh_{ false };
};

#endif

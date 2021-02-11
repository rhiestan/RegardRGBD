/**********************************************************************************************//**
 * \file RegardRGBDMainWindow.cpp
 * \author R. Hiestand
 * \date Feb 10, 2021
 *
 * \brief Main window of Scanner App.
 **************************************************************************************************/

// Open3D
#include "open3d/Open3D.h"

#include "RegardRGBDMainWindow.h"

// Qt
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QScreen>
#include <QIcon>
#include <QTimer>
#include <QEvent>
#include <QCloseEvent>

// boost
#include <boost/version.hpp>
#include <boost/scope_exit.hpp>
#include <boost/filesystem.hpp>

// Eigen
#include <Eigen/Core>

// OpenNI
#include <OpenNI.h>

// OpenCV
#include <opencv2/opencv.hpp>

// OpenSceneGraph
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>
#include <osg/Version>

// Constructor
RegardRGBDMainWindow::RegardRGBDMainWindow()
{
	this->setupUi(this);

	setWindowIcon(QIcon(":/icon/openni_icon.png"));

	// Set up action signals and slots
	connect(actionExit, &QAction::triggered, this, &QWidget::close);
	connect(actionAbout, &QAction::triggered, this, &RegardRGBDMainWindow::slotAbout);

	QTimer::singleShot(1, this, SLOT(slotOneShotTimer()));

}

RegardRGBDMainWindow::~RegardRGBDMainWindow()
{
}

void RegardRGBDMainWindow::closeEvent(QCloseEvent* event)
{
	// Ask user whether he really wants to quit
	bool userAnswer = true;
	if (userAnswer)
	{
		// TODO: Move to OpenNIDevice
		openni::OpenNI::shutdown();

		event->accept();
	}
	else
	{
		event->ignore();
	}
}

void RegardRGBDMainWindow::slotAbout()
{
	QString aboutString;
	aboutString = QStringLiteral("RegardRGBD 1.0")
		+ QStringLiteral("\n\n")
		+ QString::fromUtf8(u8"Copyright \u00A9 2021 Roman Hiestand\n");

	QString boostVersionString = QString("%1.%2.%3").arg(
		QString::number(BOOST_VERSION / 100000),
		QString::number(BOOST_VERSION / 100 % 1000),
		QString::number(BOOST_VERSION % 100));
	QString eigenVersionString = QString("%1.%2.%3").arg(
		QString::number(EIGEN_WORLD_VERSION),
		QString::number(EIGEN_MAJOR_VERSION),
		QString::number(EIGEN_MINOR_VERSION));

	openni::Version version = openni::OpenNI::getVersion();
	QString openNIVersionString = QString("%1.%2.%3").arg(
		QString::number(version.major),
		QString::number(version.minor),
		QString::number(version.build));

	aboutString.append(
		QString::fromUtf8("\n\nLinked to:\n")
		+ QString::fromUtf8(" - Open3D " OPEN3D_VERSION "\n")
		+ QString::fromUtf8(" - Qt " QT_VERSION_STR "\n")
		+ QString::fromUtf8(" - OpenCV " CV_VERSION "\n")
		+ QString::fromUtf8(" - OpenNI ") + openNIVersionString + QStringLiteral("\n")
		+ QStringLiteral(" - ") + QString::fromUtf8(osgGetLibraryName()) + QStringLiteral(" ") + QString::fromUtf8(osgGetVersion()) + QStringLiteral("\n")
		+ QString::fromUtf8(" - boost ") + boostVersionString + QStringLiteral("\n")
		+ QString::fromUtf8(" - Eigen ") + eigenVersionString + QStringLiteral("\n")
	);

	QMessageBox::about(this, QStringLiteral("RegardRGBD 1.0"),
		aboutString);
}

void RegardRGBDMainWindow::slotOneShotTimer()
{
	// Move to OpenNIDevice
	openni::Status rc = openni::OpenNI::initialize();
}

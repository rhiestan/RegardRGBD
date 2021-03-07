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
#include "utilities/R3DFontHandler.h"
#include "ONIDevice.h"
#include "ONIToQtConverter.h"
#include "ONI3DConverter.h"
#include "ScanImageTo3D.h"
#include "utilities/Conversions.h"

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

	pRegardRGBDModelViewHelper_ = std::make_unique<RegardRGBDModelViewHelper>();
	//pRegardRGBDModelViewHelper_->setViewer(openGLWidget->getViewer().get());

	setWindowIcon(QIcon(":/icon/openni_icon.png"));

	// Set up action signals and slots
	connect(actionExit, &QAction::triggered, this, &QWidget::close);
	connect(actionAbout, &QAction::triggered, this, &RegardRGBDMainWindow::slotAbout);
	connect(actionConnect_with_OpenNI, &QAction::triggered, this, &RegardRGBDMainWindow::slotConnectOpenNI);
	connect(actionDisconnect, &QAction::triggered, this, &RegardRGBDMainWindow::slotDisconnectOpenNI);

	QObject::connect(this, &RegardRGBDMainWindow::scan3DMeshChanged,
		this, &RegardRGBDMainWindow::slotScan3DMeshChanged, Qt::ConnectionType::QueuedConnection);

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
		slotDisconnectOpenNI();

		ONIDevice::shutdownOpenNI();

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

	R3DFontHandler::getInstance().initialize();

	/*osg::ref_ptr<osg::Node> model;
	model = pRegardRGBDModelViewHelper_->createRegard3DTextModel();

	osg::ref_ptr<osgViewer::View> view4 = openGLWidget->getView4();
	view4->setSceneData(model.get());
	openGLWidget->update();*/

	if (!ONIDevice::initializeOpenNI())
	{
		QMessageBox msgBox(this);
		msgBox.setIcon(QMessageBox::Icon::Critical);
		msgBox.setText(tr("Could not initialize OpenNI!"));
		msgBox.setInformativeText(tr("Please make sure OpenNI drivers are correctly installed"));
		msgBox.exec();

		this->close();
	}
}

void RegardRGBDMainWindow::slotConnectOpenNI()
{
	pONIDevice_.reset(new ONIDevice());
	bool isOK = pONIDevice_->connectDevice();
	if (isOK)
	{
		pONIToQtConverter_ = std::unique_ptr<ONIToQtConverter>(new ONIToQtConverter);
		pONIToQtConverter_->setup(nullptr);
		pONIToQtConverter_->setLabels(topLeftLabel, topRightLabel);

		pONIDevice_->setConverter(pONIToQtConverter_.get());

		pONI3DConverter_ = std::unique_ptr<ONI3DConverter>(new ONI3DConverter);
		pScanImageTo3D_ = std::unique_ptr<ScanImageTo3D>(new ScanImageTo3D);
		pScanImageTo3D_->setMainFrame(this);
		pONI3DConverter_->setup(pScanImageTo3D_.get());
		pONIDevice_->setConverter(pONI3DConverter_.get());

	}
	else
	{
		QMessageBox msgBox(this);
		msgBox.setIcon(QMessageBox::Icon::Critical);
		msgBox.setText(tr("Could not connect to ONI device"));
		msgBox.setInformativeText(QString::fromLatin1(pONIDevice_->getLastErrorString()));
		msgBox.exec();

		pONIDevice_.release();
	}
}

void RegardRGBDMainWindow::slotDisconnectOpenNI()
{
	if (pONIDevice_)
	{
		pONIDevice_->pause();

		pONIDevice_->disconnectDevice();

		pONIToQtConverter_->cleanup();
		pONI3DConverter_->cleanup();
	}
}

/**
 * This method emits the scan3DMeshChanged signal.
 *
 * This method can be called from any thread, since it is connected
 * via a queued connection with the slot slotScan3DMeshChanged.
 */
void RegardRGBDMainWindow::update3DScanMesh()
{
	if(!isDrawingScan3DMesh_)
		emit scan3DMeshChanged();
}

/**
 * Will be called by the signal scan3DMeshChanged.
 *
 * Since it is connected via a queued connection, this slot
 * is always called from the main message loop in the main thread.
 */
void RegardRGBDMainWindow::slotScan3DMeshChanged()
{
	if (pScanImageTo3D_)
	{
		isDrawingScan3DMesh_ = true;

		BOOST_SCOPE_EXIT(&isDrawingScan3DMesh_) {
			isDrawingScan3DMesh_ = false;
		} BOOST_SCOPE_EXIT_END

		//const auto mesh = pScanImageTo3D_->getTriangleMesh();
		//auto aa = Conversions::convertOpen3DToOSG(mesh);

		const auto pc = pScanImageTo3D_->getPointCloud();
		auto aa = Conversions::convertOpen3DToOSG(pc);

		bottomLeftOpenGLWidget->setGeometry(aa);
	}
}

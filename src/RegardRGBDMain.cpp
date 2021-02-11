#include <QApplication>
#include <QMessageBox>
#include <QCommandLineParser>

#include "RegardRGBDMainWindow.h"

#include <Eigen/Core>

#include <osgDB/Registry>

#if defined(OSG_LIBRARY_STATIC)
// Add references for static linking some plugins
USE_OSGPLUGIN(ive)
USE_OSGPLUGIN(osg)
USE_OSGPLUGIN(osg2)
USE_OSGPLUGIN(rgb)
//USE_OSGPLUGIN(freetype)
USE_OSGPLUGIN(p3d)
USE_OSGPLUGIN(obj)
USE_OSGPLUGIN(osgViewer)
USE_OSGPLUGIN(ply)
//USE_OSGPLUGIN(png)
//USE_OSGPLUGIN(jpeg)

USE_DOTOSGWRAPPER_LIBRARY(osg)
USE_DOTOSGWRAPPER_LIBRARY(osgText)
USE_DOTOSGWRAPPER_LIBRARY(osgViewer)

USE_SERIALIZER_WRAPPER_LIBRARY(osg)
USE_SERIALIZER_WRAPPER_LIBRARY(osgManipulator)
USE_SERIALIZER_WRAPPER_LIBRARY(osgText)
#endif

int main(int argc, char** argv)
{
	Eigen::initParallel();

	// Qt Stuff
	QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

	QApplication app(argc, argv);

	QCoreApplication::setApplicationName(QStringLiteral("RegardRGBD"));
	QCoreApplication::setApplicationVersion(QStringLiteral("1.0"));

	// Create main window
	RegardRGBDMainWindow regardRGBDMainWindow;
	regardRGBDMainWindow.show();

	// Run application
	int returnValue = 0;
	try
	{
		returnValue = app.exec();
	}
	catch (std::exception & e)
	{
		QMessageBox msgBox;
		msgBox.setText(QCoreApplication::translate("main", "Exception occurred"));
		msgBox.setInformativeText(QString::fromLatin1(e.what()));
		msgBox.setIcon(QMessageBox::Critical);
		msgBox.exec();
		returnValue = 2;
	}
	catch (...)
	{
		QMessageBox msgBox;
		msgBox.setText(QCoreApplication::translate("main", "Unknown exception occurred"));
		msgBox.setIcon(QMessageBox::Critical);
		msgBox.exec();
		returnValue = 2;
	}

	return returnValue;
}

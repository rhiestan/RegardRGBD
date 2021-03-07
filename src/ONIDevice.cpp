#include "ONIDevice.h"
#include "ONIListener.h"

#include <string>


// Workaround for MinGW
#if defined(_WIN32) && !defined(_MSC_VER)
#	define _MSC_VER 1300
#endif
#include <OpenNI.h>

#include <iostream>


bool ONIDevice::staticInitialized_ = false;
int ONIDevice::versionMajor_ = 0;
int ONIDevice::versionMinor_ = 0;
int ONIDevice::versionBuild_ = 0;
int ONIDevice::versionMaintenance_ = 0;

ONIDevice::ONIDevice()
	: pDevice_(NULL), pDepth_(NULL), pColor_(NULL),
	depthHFOV_(0), depthVHFOV_(0), minDepthValue_(0), maxDepthValue_(0),
	pDepthListener_(NULL), pColorListener_(NULL)
{
}

ONIDevice::~ONIDevice()
{
	if(pDepth_ != NULL
		&& pDepthListener_ != NULL)
	{
		pDepth_->removeNewFrameListener(pDepthListener_);
	}
	if(pColor_ != NULL
		&& pColorListener_ != NULL)
	{
		pColor_->removeNewFrameListener(pColorListener_);
	}

	delete pDepthListener_;
	delete pColorListener_;

	delete pDepth_;
	delete pColor_;
	delete pDevice_;
}

bool ONIDevice::connectDevice()
{
	openni::Status rc = openni::STATUS_OK;

	const char* deviceURI = openni::ANY_DEVICE;

	if(pDevice_ == NULL)
		pDevice_ = new openni::Device();
	if(pDepth_ == NULL)
		pDepth_ = new openni::VideoStream();
	if(pColor_ == NULL)
		pColor_ = new openni::VideoStream();

	rc = pDevice_->open(deviceURI);
	if (rc != openni::STATUS_OK)
	{
		return false;
	}

	// If supported, perform image registration (depth and rgb image superimposed)
	if(pDevice_->isImageRegistrationModeSupported(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR))
	{
		rc = pDevice_->setImageRegistrationMode(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR);
		//rc = pDevice_->setImageRegistrationMode(openni::IMAGE_REGISTRATION_OFF);
	}
	// If supported, sync frames between depth and rgb cameras
	rc = pDevice_->setDepthColorSyncEnabled(true);

	// Get some information about the device
	const openni::DeviceInfo &deviceInfo = pDevice_->getDeviceInfo();
	std::string uri = deviceInfo.getUri();
	std::string name = deviceInfo.getName();
	std::string vendor = deviceInfo.getVendor();

	// Find best video modes: Find highest resolution mode for depth with matching color mode
	int bestDepthIndex = 0, bestColorIndex = 0;
	const openni::SensorInfo *pSensorInfoColor = pDevice_->getSensorInfo(openni::SENSOR_COLOR);
	const openni::Array<openni::VideoMode> &videoModesColor = pSensorInfoColor->getSupportedVideoModes();
	const openni::SensorInfo *pSensorInfoDepth = pDevice_->getSensorInfo(openni::SENSOR_DEPTH);
	const openni::Array<openni::VideoMode> &videoModesDepth = pSensorInfoDepth->getSupportedVideoModes();
	for(int i = 0; i < videoModesDepth.getSize(); i++)
	{
		const openni::VideoMode &videoModeDepth = videoModesDepth[i];

		openni::PixelFormat pixelFormat = videoModeDepth.getPixelFormat();

		//std::cout << "Depth mode " << i << " pixel format: " << pixelFormat << std::endl;

		int resX = videoModeDepth.getResolutionX();
		int resY = videoModeDepth.getResolutionY();
		int fps = videoModeDepth.getFps();

		int matchingColorModeIndex = -1;
		for(int j = 0; j < videoModesColor.getSize(); j++)
		{
			const openni::VideoMode &videoModeColor = videoModesColor[j];
			if(videoModeColor.getResolutionX() == videoModeDepth.getResolutionX()
				&& videoModeColor.getResolutionY() == videoModeDepth.getResolutionY()
				&& videoModeColor.getFps() == videoModeDepth.getFps())
			{
				// Match!

//PIXEL_FORMAT_RGB888 = 200,
//PIXEL_FORMAT_YUV422 = 201,
				if(matchingColorModeIndex < 0
					|| videoModeColor.getPixelFormat() == openni::PIXEL_FORMAT_RGB888)
				{
					matchingColorModeIndex = j;
				}
			}
		}

		if((resX > videoModesDepth[bestDepthIndex].getResolutionX()
			|| resY > videoModesDepth[bestDepthIndex].getResolutionY())
			&& matchingColorModeIndex >= 0
			&& pixelFormat == openni::PIXEL_FORMAT_DEPTH_1_MM)
		{
			bestDepthIndex = i;
			bestColorIndex = matchingColorModeIndex;
		}
	}

	rc = pDepth_->create(*pDevice_, openni::SENSOR_DEPTH);
	if (rc == openni::STATUS_OK)
	{
		rc = pDepth_->setVideoMode(videoModesDepth[bestDepthIndex]);
		if (rc == openni::STATUS_OK)
		{
			rc = pDepth_->start();
			if (rc != openni::STATUS_OK)
			{
				pDepth_->destroy();
			}
		}
		else
		{
			pDepth_->destroy();
		}
	}
	else
	{
	}

	rc = pColor_->create(*pDevice_, openni::SENSOR_COLOR);
	if (rc == openni::STATUS_OK)
	{
		rc = pColor_->setVideoMode(videoModesColor[bestColorIndex]);
		if(rc == openni::STATUS_OK)
		{
			rc = pColor_->start();
			if (rc != openni::STATUS_OK)
			{
				pColor_->destroy();
			}
		}
		else
		{
			pColor_->destroy();
		}
	}
	else
	{
	}

	if (!pDepth_->isValid() || !pColor_->isValid())
	{
		return false;
	}

	// If supported, perform image registration (depth and rgb image superimposed)
	if(pDevice_->isImageRegistrationModeSupported(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR))
	{
		rc = pDevice_->setImageRegistrationMode(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR);
		//rc = pDevice_->setImageRegistrationMode(openni::IMAGE_REGISTRATION_OFF);
	}
	// If supported, sync frames between depth and rgb cameras
	rc = pDevice_->setDepthColorSyncEnabled(true);

	int mirrorFlag{ -1 };
	rc = pColor_->getProperty(openni::STREAM_PROPERTY_MIRRORING, &mirrorFlag);
	pDepth_->setProperty(openni::STREAM_PROPERTY_MIRRORING, (int)0);
	pColor_->setProperty(openni::STREAM_PROPERTY_MIRRORING, (int)0);
	rc = pColor_->getProperty(openni::STREAM_PROPERTY_MIRRORING, &mirrorFlag);

	// Add new frame listeners
	pDepthListener_ = new ONIListener();
	pDepth_->addNewFrameListener(pDepthListener_);
	pColorListener_ = new ONIListener();
	pColor_->addNewFrameListener(pColorListener_);

	// Get field of view
	depthHFOV_ = pDepth_->getHorizontalFieldOfView();
	depthVHFOV_ = pDepth_->getVerticalFieldOfView();
	minDepthValue_ = pDepth_->getMinPixelValue();
	maxDepthValue_ = pDepth_->getMaxPixelValue();

	deviceRunning_ = true;
	return true;
}

void ONIDevice::disconnectDevice()
{
	deviceRunning_ = false;

	if(pDepth_ != nullptr)
	{
		if(pDepthListener_ != nullptr)
		{
			pDepthListener_->clearConverters();
			pDepth_->removeNewFrameListener(pDepthListener_);
		}
		pDepth_->stop();
		pDepth_->destroy();
	}
	if(pColor_ != nullptr)
	{
		if(pColorListener_ != nullptr)
		{
			pColorListener_->clearConverters();
			pDepth_->removeNewFrameListener(pColorListener_);
		}
		pColor_->stop();
		pColor_->destroy();
	}
	if(pDevice_ != nullptr)
		pDevice_->close();

	delete pDepthListener_;
	pDepthListener_ = nullptr;
	delete pColorListener_;
	pColorListener_ = nullptr;
	delete pDepth_;
	pDepth_ = nullptr;
	delete pColor_;
	pColor_ = nullptr;
	delete pDevice_;
	pDevice_ = nullptr;
}

void ONIDevice::setConverter(ConverterInterface*pConverter)
{
	if(pDepthListener_ != nullptr)
		pDepthListener_->addConverter(pConverter);
	if(pColorListener_ != nullptr)
		pColorListener_->addConverter(pConverter);
}

void ONIDevice::pause()
{
	if(pColor_ != nullptr)
		pColor_->stop();
	if(pDepth_ != nullptr)
		pDepth_->stop();
}

bool ONIDevice::initializeOpenNI()
{
	openni::Status rc = openni::OpenNI::initialize();
	if(rc == openni::STATUS_OK)
	{
		staticInitialized_ = true;
		openni::Version version = openni::OpenNI::getVersion();
		versionMajor_ = version.major;
		versionMinor_ = version.minor;
		versionBuild_ = version.build;
		versionMaintenance_ = version.maintenance;
	}
	return staticInitialized_;
}

const char *ONIDevice::getLastErrorString()
{
	return openni::OpenNI::getExtendedError();
}

void ONIDevice::shutdownOpenNI()
{
	openni::OpenNI::shutdown();
}

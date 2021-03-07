#include "ONIListener.h"

#undef min
#undef max

#include "ONI3DConverter.h"


ONIListener::ONIListener()
{
}

ONIListener::~ONIListener()
{
}

void ONIListener::onNewFrame( openni::VideoStream &vs )
{
	openni::VideoFrameRef frame;
	openni::Status rs = vs.readFrame(&frame);
	int sz = frame.getDataSize();
	int frameIndex = frame.getFrameIndex();
	int height = frame.getHeight();
	int width = frame.getWidth();
	openni::SensorType sensorType = frame.getSensorType();


	int stride = frame.getStrideInBytes();
	uint64_t timeStamp = frame.getTimestamp();
	const openni::VideoMode &videoMode = frame.getVideoMode();
	openni::PixelFormat pixelFormat = videoMode.getPixelFormat();

	int fps = videoMode.getFps();
	int resX = videoMode.getResolutionX();
	int resY = videoMode.getResolutionY();
	
	for(auto pConverter : converters_)
	{
		if(sensorType == openni::SENSOR_COLOR)
			pConverter->newColorFrame(frameIndex, width, height, stride, sz, frame.getData(), &vs);
		else if(sensorType == openni::SENSOR_DEPTH)
			pConverter->newDepthFrame(frameIndex, width, height, stride, sz, frame.getData(), &vs);
	}
}

void ONIListener::addConverter(ConverterInterface* pConverter)
{
	converters_.push_back(pConverter);
}

void ONIListener::clearConverters()
{
	converters_.clear();
}

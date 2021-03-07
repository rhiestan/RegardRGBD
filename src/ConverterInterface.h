#ifndef CONVERTERINTERFACE_H
#define CONVERTERINTERFACE_H

namespace openni
{
	class VideoStream;
};

class StitchingTest;
class StitcherI;

class ConverterInterface
{
public:
	ConverterInterface() { }
	virtual ~ConverterInterface() { }

	virtual void setup(StitcherI* pStitcher) = 0;
	virtual void cleanup() = 0;

	virtual void newColorFrame(int frameIndex, int width, int height, int stride, int size, const void* data,
		const openni::VideoStream* pVS) = 0;
	virtual void newDepthFrame(int frameIndex, int width, int height, int stride, int size, const void* data,
		const openni::VideoStream* pVS) = 0;
};

#endif

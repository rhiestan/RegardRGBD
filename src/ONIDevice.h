#ifndef ONIDEVICE_H
#define ONIDEVICE_H

class ONIListener;
class ConverterInterface;
namespace openni
{
	class Device;
	class VideoStream;
};

/**
 * This is the class handling all calls to OpenNI.
 */
class ONIDevice
{
public:
	ONIDevice();
	virtual ~ONIDevice();

	bool connectDevice();
	void disconnectDevice();
	void setConverter(ConverterInterface *pConverter);
	void pause();

	float getDepthHFOV() const { return depthHFOV_; }
	float getDepthVFOV() const { return depthVHFOV_; }
	int getMinDepthValue() const { return minDepthValue_; }
	int getMaxDepthValue() const { return maxDepthValue_; }

	static bool initializeOpenNI();
	static void shutdownOpenNI();
	static const char *getLastErrorString();

private:
	bool deviceRunning_;
	openni::Device *pDevice_;
	openni::VideoStream *pDepth_, *pColor_;
	ONIListener *pDepthListener_, *pColorListener_;

	float depthHFOV_, depthVHFOV_;
	int minDepthValue_, maxDepthValue_;

	static bool staticInitialized_;
	static int versionMajor_, versionMinor_, versionBuild_, versionMaintenance_;
};

#endif

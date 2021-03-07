#ifndef ONI3DCONVERTER_H
#define ONI3DCONVERTER_H

#include "ConverterInterface.h"

#include "Vector.h"

#include <vector>
#include <mutex>
#include <thread>
#include <memory>
#include <chrono>

#include "open3d/Open3D.h"

namespace openni
{
	class VideoStream;
};

class StitchingTest;
class StitcherI;

class ONI3DConverter: public ConverterInterface
{
public:
	ONI3DConverter();
	virtual ~ONI3DConverter();

	virtual void setup(StitcherI* pStitcher);
	virtual void cleanup();

	virtual void newColorFrame(int frameIndex, int width, int height, int stride, int size, const void *data,
		const openni::VideoStream *pVS);
	virtual void newDepthFrame(int frameIndex, int width, int height, int stride, int size, const void *data,
		const openni::VideoStream *pVS);

	void get3DPoints(std::vector< SVector3f > &points,
		std::vector< SVector3b > &colors);

protected:
	void Entry();

private:

	/**
	 * DataStruct holds raw sensor data.
	 */
	class DataStruct
	{
	public:
		DataStruct() : data_(NULL), dataSize_(0), frameIndex_(0), width_(0), height_(0), stride_(0) { }
		~DataStruct() { free(data_); }
		void ensureSize(int size)
		{
			if(dataSize_ < size || data_ == NULL)
			{
				free(data_);
				data_ = malloc(size);
				dataSize_ = size;
			}
		}
		void copy(const DataStruct &other)
		{
			ensureSize(other.dataSize_);
			memcpy(data_, other.data_, other.dataSize_);
			frameIndex_ = other.frameIndex_;
			width_ = other.width_;
			height_ = other.height_;
			stride_ = other.stride_;
		}

		void *data_;
		int dataSize_, frameIndex_;
		int width_, height_, stride_;
	};

	// Mutex between ViewerListener and 3DConverter threads
	std::mutex mutex_;
	std::condition_variable mutexCond_;
	DataStruct depthData_, colorData_;
	float resX_, resY_, factorXZ_, factorYZ_;
	float focalX_, focalY_, pX_, pY_;
	bool convTermsSet_, terminate_;
	open3d::geometry::Image colorImg_, depthImg_;

	int numberOfFrames_{ 0 };
	std::chrono::time_point<std::chrono::steady_clock> startTime_;

	// Mutex between 3DConverter and 3D Data consumer threads
	std::mutex mutex3D_;
	std::vector< SVector3f > points_;
	std::vector< SVector3b > colors_;

	// Only accessed by Converter thread
	DataStruct depthDataCopy_, colorDataCopy_;

	std::thread *thread_{nullptr};

	StitcherI* pStitcher_{ nullptr };
};

#endif

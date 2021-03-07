#include "ONI3DConverter.h"

#include "Stitcher.h"

#if defined(_WIN32) && !defined(_MSC_VER)
#	define _MSC_VER 1300
#endif
#include <OpenNI.h>

#include <iostream>

#include "open3d/Open3D.h"

ONI3DConverter::ONI3DConverter()
	: ConverterInterface(), resX_(0), resY_(0), factorXZ_(0), factorYZ_(0),
	convTermsSet_(false), terminate_(false)
{
}

ONI3DConverter::~ONI3DConverter()
{
	delete thread_;
}

void ONI3DConverter::setup(StitcherI* pStitcher)
{
	pStitcher_ = pStitcher;

	std::function<void(void)> threadFunc(std::bind(&ONI3DConverter::Entry, this));
	thread_ = new std::thread(threadFunc);
}

/**
 * Clean up the thread.
 *
 * This method is called by another thread to delete this thread.
 */
void ONI3DConverter::cleanup()
{
	if(!thread_->joinable())
		return;

	{
		std::unique_lock<std::mutex> lock(mutex_);
		terminate_ = true;
		mutexCond_.notify_all();
	}
	thread_->join();

	auto totalMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime_).count();
	double totalSeconds = static_cast<double>(totalMilliseconds) / 1000.0;

	std::cout << "Number of frames: " << numberOfFrames_
		<< " ("
		<< static_cast<double>(numberOfFrames_) / totalSeconds
		<< " fps)" << std::endl;
}

void ONI3DConverter::newColorFrame(int frameIndex, int width, int height, int stride, int size, const void *data,
	const openni::VideoStream *pvS)
{
	std::unique_lock<std::mutex> lock(mutex_);

	colorData_.ensureSize(size);
	memcpy(colorData_.data_, data, size);
	colorData_.frameIndex_ = frameIndex;
	colorData_.width_ = width;
	colorData_.height_ = height;
	colorData_.stride_ = stride;

	colorImg_.Prepare(width, height, 3, 1);
	unsigned char *dataPtr = colorImg_.PointerAt<unsigned char>(0, 0, 0);
	memcpy(dataPtr, data, size);

	// Alert Converter thread of new data
	mutexCond_.notify_one();
}

void ONI3DConverter::newDepthFrame(int frameIndex, int width, int height, int stride, int size, const void *data,
	const openni::VideoStream *pVS)
{
	std::unique_lock<std::mutex> lock(mutex_);

	numberOfFrames_++;

	depthData_.ensureSize(size);
	memcpy(depthData_.data_, data, size);
	depthData_.frameIndex_ = frameIndex;
	depthData_.width_ = width;
	depthData_.height_ = height;
	depthData_.stride_ = stride;

	depthImg_.Prepare(width, height, 1, 2);
	unsigned char* dataPtr = depthImg_.PointerAt<unsigned char>(0, 0, 0);
	memcpy(dataPtr, data, size);

	if(!convTermsSet_)
	{
		startTime_ = std::chrono::steady_clock::now();

		float worldX = 0, worldY = 0, worldZ = 0;
		openni::Status rc = openni::CoordinateConverter::convertDepthToWorld(
			*pVS, 0, 0, 1,
			&worldX, &worldY, &worldZ);
		factorXZ_ = worldX * (-2.0f);
		factorYZ_ = worldY * 2.0f;

		rc = openni::CoordinateConverter::convertDepthToWorld(
			*pVS, 1, 1, 1,
			&worldX, &worldY, &worldZ);
		resX_ = 1.0f/(0.5f + worldX/factorXZ_);
		resY_ = 1.0f/(0.5f - worldY/factorYZ_);

		/*std::cout << "factorXZ: " << factorXZ_ << ", factorYZ: " << factorYZ_ << std::endl;
		std::cout << "resX: " << resX_ << ", resY: " << resY_ << std::endl;
		*/
		rc = openni::CoordinateConverter::convertDepthToWorld(
			*pVS, 0, 0, 0,
			&worldX, &worldY, &worldZ);
		rc = openni::CoordinateConverter::convertDepthToWorld(
			*pVS, 1, 0, 0,
			&worldX, &worldY, &worldZ);

		focalX_ = worldX;

		rc = openni::CoordinateConverter::convertDepthToWorld(
			*pVS, 0, 1, 0,
			&worldX, &worldY, &worldZ);

		focalY_ = worldY;

		rc = openni::CoordinateConverter::convertDepthToWorld(
			*pVS, 0, 0, 1,
			&worldX, &worldY, &worldZ);

		pX_ = worldX;
		pY_ = worldY;

		/*std::cout << "focalX: " << focalX_
			<< ", focalY:" << focalY_
			<< ", pX:" << pX_
			<< ", pY:" << pY_
			<< std::endl;*/
		rc = openni::CoordinateConverter::convertDepthToWorld(
			*pVS, 1, 0, 1,
			&worldX, &worldY, &worldZ);
		rc = openni::CoordinateConverter::convertDepthToWorld(
			*pVS, 0, 1, 1,
			&worldX, &worldY, &worldZ);
		rc = openni::CoordinateConverter::convertDepthToWorld(
			*pVS, 1, 1, 0,
			&worldX, &worldY, &worldZ);
		rc = openni::CoordinateConverter::convertDepthToWorld(
			*pVS, 1, 1, 1,
			&worldX, &worldY, &worldZ);
		rc = openni::CoordinateConverter::convertDepthToWorld(
			*pVS, 0, 0, 2,
			&worldX, &worldY, &worldZ);
		rc = openni::CoordinateConverter::convertDepthToWorld(
			*pVS, 1, 0, 2,
			&worldX, &worldY, &worldZ);
		rc = openni::CoordinateConverter::convertDepthToWorld(
			*pVS, 0, 1, 2,
			&worldX, &worldY, &worldZ);
		rc = openni::CoordinateConverter::convertDepthToWorld(
			*pVS, 1, 1, 2,
			&worldX, &worldY, &worldZ);
		rc = openni::CoordinateConverter::convertDepthToWorld(
			*pVS, 2, 1, 2,
			&worldX, &worldY, &worldZ);

		rc = openni::CoordinateConverter::convertDepthToWorld(
			*pVS, width/2, height/2, 0,
			&worldX, &worldY, &worldZ);

		float checkX, checkY;
		checkX = (2.0f / resX_ - 0.5f)*2.0f*factorXZ_;
		checkY = (0.5f - 1.0f/resY_)*2.0f*factorYZ_;

		rc = openni::CoordinateConverter::convertDepthToWorld(
			*pVS, 2, 2, 2,
			&worldX, &worldY, &worldZ);

		if (openni::Status::STATUS_OK == openni::CoordinateConverter::convertDepthToWorld(
			*pVS, 0, 0, 100,
			&worldX, &worldY, &worldZ))
			std::cout << "left top: " << worldX << "/" << worldY << "/" << worldZ << std::endl;
		if (openni::Status::STATUS_OK == openni::CoordinateConverter::convertDepthToWorld(
			*pVS, 0, 0, 1000,
			&worldX, &worldY, &worldZ))
			std::cout << "left top: " << worldX << "/" << worldY << "/" << worldZ << std::endl;
		//if (openni::Status::STATUS_OK == openni::CoordinateConverter::convertDepthToWorld(
		//	*pVS, width - 1, 0, 100,
		//	&worldX, &worldY, &worldZ))
		//	std::cout << "right top: " << worldX << "/" << worldY << "/" << worldZ << std::endl;
		if (openni::Status::STATUS_OK == openni::CoordinateConverter::convertDepthToWorld(
			*pVS, width - 1, 0, 1000,
			&worldX, &worldY, &worldZ))
			std::cout << "right top: " << worldX << "/" << worldY << "/" << worldZ << std::endl;
		rc = openni::CoordinateConverter::convertDepthToWorld(
			*pVS, 0, height-1, 1000,
			&worldX, &worldY, &worldZ);
		//std::cout << "left bottom: " << worldX << "/" << worldY << "/" << worldZ << std::endl;
		rc = openni::CoordinateConverter::convertDepthToWorld(
			*pVS, width-1, height-1, 1000,
			&worldX, &worldY, &worldZ);
		//std::cout << "right bottom: " << worldX << "/" << worldY << "/" << worldZ << std::endl;

		convTermsSet_ = true;
	}

	// Alert Converter thread of new data
	mutexCond_.notify_one();
}

void ONI3DConverter::get3DPoints(std::vector< SVector3f > &points,
	std::vector< SVector3b > &colors)
{
	std::unique_lock<std::mutex> lock(mutex3D_);
	points = points_;
	colors = colors_;
}

void ONI3DConverter::Entry()
{
	bool terminate = false;
	int processedFrameIndex = 0;
	std::vector< SVector3f > points;
	std::vector< SVector3b > colors;

	while(!terminate)
	{
		// Wait for new data
		{
			std::unique_lock<std::mutex> lock(mutex_);
			while((depthData_.data_ == NULL
					|| colorData_.data_ == NULL
					|| depthData_.frameIndex_ != colorData_.frameIndex_
					|| processedFrameIndex >= depthData_.frameIndex_)
				&& !terminate_)
			{
				mutexCond_.wait(lock);
			}
			if(!terminate_)
			{
				// Copy data locally
				depthDataCopy_.copy( depthData_ );
				colorDataCopy_.copy( colorData_ );
			}
			else
			{
				terminate = true;
			}
		}

		if(!terminate)
		{
			// Process data
#if 0
			openni::DepthPixel *pDepthData = reinterpret_cast<openni::DepthPixel *>(depthDataCopy_.data_);
			uint8_t *pColorData = reinterpret_cast<uint8_t *>(colorDataCopy_.data_);
			int depthStride = depthDataCopy_.stride_ / sizeof(openni::DepthPixel);
			int colorStride = colorDataCopy_.stride_;
			points.clear();
			colors.clear();


			for(int y = 0; y < depthDataCopy_.height_; y++)
			{
				openni::DepthPixel *pCurDepthLine = pDepthData + y * depthStride;
				uint8_t *pCurColorLine = pColorData + y * colorStride;
				for(int x = 0; x < depthDataCopy_.width_; x++)
				{
					openni::DepthPixel curDepthValue = pCurDepthLine[x];
					if(curDepthValue > 0)
					{
						float worldX, worldY, worldZ;
/*						openni::Status rc = openni::CoordinateConverter::convertDepthToWorld(
							*depthVideoStream_, x, y, curDepthValue,
							&worldX, &worldY, &worldZ);
*/
						worldX = (static_cast<float>(x) / resX_ - 0.5f)*static_cast<float>(curDepthValue)*factorXZ_;
						worldY = (0.5f - static_cast<float>(y)/resY_)*static_cast<float>(curDepthValue)*factorYZ_;
						worldZ = static_cast<float>(curDepthValue);
/*
						char buf[1024];
						sprintf_s(buf, 1024, "%g\t%g\t%g\n", worldX, worldY, worldZ);
						OutputDebugStringA(buf);*/

						uint8_t *pCurPixelColor = pCurColorLine + 3*x;
						points.push_back( SVector3f(worldX, worldY, worldZ) );
						colors.push_back( SVector3b(pCurPixelColor[0], pCurPixelColor[1], pCurPixelColor[2]) );
					}
				}
			}
#endif
			// Run stitchingtest
//			stitchingTest_.runTest(points_, colors_, points, colors);
			if (pStitcher_ != nullptr)
				pStitcher_->addNewImage(colorImg_, depthImg_);

			// Copy points over
			{
				std::unique_lock<std::mutex> lock(mutex3D_);
				points_ = points;
				colors_ = colors;
			}

			processedFrameIndex = depthDataCopy_.frameIndex_;
		}
	}
}

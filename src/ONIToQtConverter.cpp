
#include "ONIToQtConverter.h"

#include "PixmapLabel.h"

#include <QImage>
#include <QPixmap>
#include <QLabel>
#include <QDebug>

#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>


ONIToQtConverter::ONIToQtConverter()
	: ConverterInterface()
{
	QObject::connect(this, &ONIToQtConverter::colorFrameChanged,
		this, &ONIToQtConverter::slotColorFrameChanged, Qt::ConnectionType::QueuedConnection);
	QObject::connect(this, &ONIToQtConverter::depthFrameChanged,
		this, &ONIToQtConverter::slotDepthFrameChanged, Qt::ConnectionType::QueuedConnection);
}

ONIToQtConverter::~ONIToQtConverter()
{
}

void ONIToQtConverter::setup(StitcherI* pStitcher)
{
}

void ONIToQtConverter::cleanup()
{
}

void ONIToQtConverter::newColorFrame(int frameIndex, int width, int height, int stride, int size, const void* data,
	const openni::VideoStream* pVS)
{
	std::unique_lock<std::mutex> lock(mutex_);

	if (colorData_.size() < size)
		colorData_.resize(size);

	memcpy(&(colorData_[0]), data, size);

	colorImg_ = QImage(&(colorData_[0]), width, height, stride, QImage::Format::Format_RGB888);

	emit colorFrameChanged();
}

void ONIToQtConverter::newDepthFrame(int frameIndex, int width, int height, int stride, int size, const void* data,
	const openni::VideoStream* pVS)
{
	std::unique_lock<std::mutex> lock(mutex_);

	// Convert 16 bits -> 8 bits
	cv::Mat depth, depthO, depth8;
	depth.create(height, width, CV_16UC1);
	unsigned char* cvDPtr = depth.ptr<unsigned char>(0);
	memcpy(cvDPtr, data, size);

	//double minVal = 0, maxVal = 0;
	//cv::minMaxLoc(depth, &minVal, &maxVal);

	cv::normalize(depth, depthO, 0, 255.0, cv::NORM_MINMAX);
	//depth.convertTo(depth8, CV_8UC1, 1.0 / 256.0, 0);
	depthO.convertTo(depth8, CV_8UC1, 1.0, 0);

	//cv::imwrite("depth16.png", depth);
	//cv::imwrite("depth8.png", depth8);

	//qDebug() << depth.type() << ", " << depth.rows << ", " << depth.cols;
	//qDebug() << depth8.type() << ", " << depth8.rows << ", " << depth8.cols;

	if (depthData_.size() < width * height)
		depthData_.resize(width * height);

	memcpy(&(depthData_[0]), depth8.ptr<const unsigned char>(0), width * height);

	depthImg_ = QImage(&(depthData_[0]), width, height, width, QImage::Format::Format_Grayscale8);
	//depthImg_.save("qdepth8.png");

	emit depthFrameChanged();
}

void ONIToQtConverter::setLabels(PixmapLabel* pRGBLabel, PixmapLabel* pDepthLabel)
{
	pRGBLabel_ = pRGBLabel;
	pDepthLabel_ = pDepthLabel;
}

void ONIToQtConverter::slotColorFrameChanged()
{
	if(pRGBLabel_ != nullptr)
	{
		std::unique_lock<std::mutex> lock(mutex_);

		pRGBLabel_->setImage(colorImg_);

		/*QRect rect = pRGBLabel_->frameRect();
		QPixmap pixmap;
		pixmap.convertFromImage(colorImg_.scaled(rect.width(), rect.height(), Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::FastTransformation));
		pRGBLabel_->setPixmap(pixmap);*/
	}
}

void ONIToQtConverter::slotDepthFrameChanged()
{
	if(pDepthLabel_ != nullptr)
	{
		std::unique_lock<std::mutex> lock(mutex_);

		pDepthLabel_->setImage(depthImg_);

		/*QRect rect = pDepthLabel_->frameRect();
		QPixmap pixmap;
		pixmap.convertFromImage(depthImg_.scaled(rect.width(), rect.height(), Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::FastTransformation));
		pDepthLabel_->setPixmap(pixmap);*/
	}
}

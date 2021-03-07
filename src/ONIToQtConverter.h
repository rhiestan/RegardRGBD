#ifndef ONITOQTCONVERTER_H
#define ONITOQTCONVERTER_H

#include "ConverterInterface.h"

#include <QObject>
#include <QImage>

#include <mutex>

class PixmapLabel;

class ONIToQtConverter: public QObject, public ConverterInterface
{
	Q_OBJECT
public:
	ONIToQtConverter();
	virtual ~ONIToQtConverter();

	virtual void setup(StitcherI* pStitcher);
	virtual void cleanup();

	virtual void newColorFrame(int frameIndex, int width, int height, int stride, int size, const void* data,
		const openni::VideoStream* pVS);
	virtual void newDepthFrame(int frameIndex, int width, int height, int stride, int size, const void* data,
		const openni::VideoStream* pVS);


	void setLabels(PixmapLabel*pRGBLabel, PixmapLabel*pDepthLabel);

public slots:
	void slotColorFrameChanged();
	void slotDepthFrameChanged();

signals:
	void colorFrameChanged();
	void depthFrameChanged();

private:
	PixmapLabel*pRGBLabel_{nullptr}, *pDepthLabel_{nullptr};

	std::mutex mutex_;
	QImage colorImg_, depthImg_;
	std::vector<unsigned char> colorData_, depthData_;
};

#endif

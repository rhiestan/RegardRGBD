
#include "PixmapLabel.h"

#include <QDebug>

PixmapLabel::PixmapLabel(QWidget* parent, Qt::WindowFlags f)
	: QLabel(parent, f)
{
}

PixmapLabel::PixmapLabel(const QString& text, QWidget* parent, Qt::WindowFlags f)
	: QLabel(text, parent, f)
{
}

PixmapLabel::~PixmapLabel()
{
}

void PixmapLabel::setImage(QImage image)
{
	image_ = image;
	QPixmap pixmap;

	//qDebug() << image_.format() << ", " << image_.width() << ", " << image_.height();
	//qDebug() << depth8.type() << ", " << depth8.rows << ", " << depth8.cols;

	pixmap.convertFromImage(image_.scaled(width(), height(), Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::FastTransformation));
	setPixmap(pixmap);
}

void PixmapLabel::resizeEvent(QResizeEvent* event)
{
	QPixmap pixmap;
	pixmap.convertFromImage(image_.scaled(width(), height(), Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::FastTransformation));
	setPixmap(pixmap);
}

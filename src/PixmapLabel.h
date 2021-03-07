
#ifndef PIXMAPLABEL_H
#define PIXMAPLABEL_H

#include <QLabel>
#include <QImage>

/**
 * Subclass of QLabel showing a QImage, allowing free resize/shrink respecting the aspect ratio.
 */
class PixmapLabel : public QLabel
{
	Q_OBJECT
public:
	PixmapLabel(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
	PixmapLabel(const QString& text, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
	virtual ~PixmapLabel();

	void setImage(QImage image);

protected:
	virtual void resizeEvent(QResizeEvent* event);

	QImage image_;
};

#endif


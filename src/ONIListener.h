#ifndef ONILISTENER_H
#define ONILISTENER_H

class ConverterInterface;

// Workaround for MinGW
#if defined(_WIN32) && !defined(_MSC_VER)
#	define _MSC_VER 1300
#endif
#include <OpenNI.h>
#include <vector>

class ONIListener: public openni::VideoStream::NewFrameListener
{
public:
	ONIListener();
	virtual ~ONIListener();

	virtual void onNewFrame( openni::VideoStream &vs );

	void addConverter(ConverterInterface*pConverter);
	void clearConverters();

private:
	std::vector<ConverterInterface*> converters_;
};

#endif


#ifndef STITCHERI_H
#define STITCHERI_H

#include "open3d/Open3D.h"

/*
 *
 */
class StitcherI
{
public:
	StitcherI() { }
	virtual ~StitcherI() { }

	virtual void setup() = 0;

	virtual void addNewImage(const open3d::geometry::Image& colorImg, const open3d::geometry::Image& depthImg) = 0;

	virtual void saveVolume() = 0;

	virtual void reset() = 0;

	virtual void setDepthScale(double depthScale) = 0;

};

#endif

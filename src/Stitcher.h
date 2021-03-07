#ifndef STITCHER_H
#define STITCHER_H

#include "StitcherI.h"

#include "open3d/Open3D.h"

#include <vector>
#include <mutex>

#include <Eigen/StdVector>

/*
 *
 */
class Stitcher : public StitcherI
{
public:
	Stitcher();
	virtual ~Stitcher();

	virtual void setup();

	virtual void addNewImage(const open3d::geometry::Image& colorImg, const open3d::geometry::Image& depthImg);

	virtual void saveVolume();

	virtual void reset();

	virtual void setDepthScale(double depthScale) { depthScale_ = depthScale; }

private:
	std::mutex mutex_;

	double depthScale_{ 1000.0 };

	open3d::geometry::RGBDImage oldRGBDImage_;

	Eigen::Matrix4d pos_;

	std::unique_ptr<open3d::pipelines::integration::ScalableTSDFVolume> volume_;

	std::vector<open3d::geometry::RGBDImage> images_;
	std::vector<Eigen::Matrix4d> posvec_, transvec_;
	std::vector<Eigen::Matrix6d> infovec_;
};

#endif

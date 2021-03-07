#ifndef SCANIMAGETO3D_H
#define SCANIMAGETO3D_H

class RegardRGBDMainWindow;

#include "StitcherI.h"

#include "open3d/Open3D.h"

#include <vector>
#include <mutex>
#include <memory>

#include <Eigen/StdVector>

/*
 *
 */
class ScanImageTo3D : public StitcherI
{
public:
	ScanImageTo3D();
	virtual ~ScanImageTo3D();

	virtual void setup();

	virtual void addNewImage(const open3d::geometry::Image& colorImg, const open3d::geometry::Image& depthImg);

	virtual void saveVolume();

	virtual void reset();

	virtual void setDepthScale(double depthScale) { depthScale_ = depthScale; }

	const std::shared_ptr<open3d::geometry::TriangleMesh> getTriangleMesh();
	const std::shared_ptr<open3d::geometry::PointCloud> getPointCloud();

	void setMainFrame(RegardRGBDMainWindow* pRegardRGBDMainWindow) { pRegardRGBDMainWindow_ = pRegardRGBDMainWindow; }

private:
	std::mutex mutex_;

	double depthScale_{ 1000.0 };

	std::shared_ptr<open3d::geometry::TriangleMesh> triangleMesh_;
	std::shared_ptr<open3d::geometry::PointCloud> pointCloud_;

	RegardRGBDMainWindow* pRegardRGBDMainWindow_;
};

#endif

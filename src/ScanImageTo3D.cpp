
#include "ScanImageTo3D.h"
#include "RegardRGBDMainWindow.h"

#include <iostream>
#include <sstream>
#include <limits>

#include <Eigen/LU>
#include <Eigen/Geometry>

#include <open3d/pipelines/registration/GlobalOptimization.h>
#include <open3d/pipelines/color_map/ColorMapOptimization.h>

ScanImageTo3D::ScanImageTo3D()
{
}

ScanImageTo3D::~ScanImageTo3D()
{

}

void ScanImageTo3D::setup()
{
}


void ScanImageTo3D::addNewImage(const open3d::geometry::Image& colorImg, const open3d::geometry::Image& depthImg)
{
	//open3d::pipelines::integration::ScalableTSDFVolume volume(4.0 / 512, 0.04, open3d::pipelines::integration::TSDFVolumeColorType::RGB8);

	open3d::camera::PinholeCameraIntrinsic intrinsic = open3d::camera::PinholeCameraIntrinsic(
		open3d::camera::PinholeCameraIntrinsicParameters::PrimeSenseDefault);
	//intrinsic.SetIntrinsics(640, 480, 524.0, 524.0, 316.7, 238.5);	// from https://www.researchgate.net/figure/ntrinsic-parameters-of-Kinect-RGB-camera_tbl2_305108995
	//intrinsic.SetIntrinsics(640, 480, 517.3, 516.5, 318.6, 255.3);		// from Freiburg test data set
	//intrinsic.SetIntrinsics(640, 480, 537.408, 537.40877, 321.897, 236.29);		// from Calibration of my own camera
	//intrinsic.SetIntrinsics(640, 480, 533.82, 533.82, 320.55, 232.35);		// from Calibration of my own camera
	intrinsic.SetIntrinsics(640, 480, 542.7693, 544.396, 318.79, 239.99);		// from Calibration of my own camera

	//open3d::io::WriteImageToPNG("color.png", colorImg_);
	//open3d::io::WriteImageToPNG("depth.png", depthImg_);

	Eigen::Matrix4d identity = Eigen::Matrix4d::Identity();
	auto depthFlt = depthImg.ConvertDepthToFloatImage(depthScale_, std::numeric_limits<float>::max());

	open3d::geometry::RGBDImage source(colorImg, *depthFlt);

	//volume.Integrate(source, intrinsic, identity);
	{
		std::unique_lock<std::mutex> lock(mutex_);
		//triangleMesh_ = volume.ExtractTriangleMesh();
		pointCloud_ = open3d::geometry::PointCloud::CreateFromRGBDImage(source, intrinsic);
	}

	if(pRegardRGBDMainWindow_ != nullptr)
		pRegardRGBDMainWindow_->update3DScanMesh();
}

void ScanImageTo3D::saveVolume()
{
}

void ScanImageTo3D::reset()
{
	std::unique_lock<std::mutex> lock(mutex_);
	setup();
}

/**
 * Makes a copy of the triangle mesh.
 */
const std::shared_ptr<open3d::geometry::TriangleMesh> ScanImageTo3D::getTriangleMesh()
{
	std::unique_lock<std::mutex> lock(mutex_);
	return std::make_shared<open3d::geometry::TriangleMesh>(*triangleMesh_);
}

/**
 * Makes a copy of the point cloud.
 */
const std::shared_ptr<open3d::geometry::PointCloud> ScanImageTo3D::getPointCloud()
{
	std::unique_lock<std::mutex> lock(mutex_);
	return std::make_shared<open3d::geometry::PointCloud>(*pointCloud_);
}

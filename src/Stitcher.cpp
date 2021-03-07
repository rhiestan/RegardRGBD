
#include "Stitcher.h"

#include <iostream>
#include <sstream>
#include <limits>

#include <Eigen/LU>
#include <Eigen/Geometry>

#include <open3d/pipelines/registration/GlobalOptimization.h>
#include <open3d/pipelines/color_map/ColorMapOptimization.h>

Stitcher::Stitcher()
{
}

Stitcher::~Stitcher()
{

}

void Stitcher::setup()
{
	volume_ = std::make_unique<open3d::pipelines::integration::ScalableTSDFVolume>(4.0 / 512, 0.04, open3d::pipelines::integration::TSDFVolumeColorType::RGB8);
}

bool printRotMatrix(const Eigen::Matrix4d &mat)
{
	double theta = -std::asin(mat(2, 0));
	double psi = std::atan2(mat(2, 1)/std::cos(theta), mat(2, 2)/std::cos(theta));
	double phi = std::atan2(mat(1, 0)/std::cos(theta), mat(0, 0)/std::cos(theta));

	std::cout << theta << ", " << psi << ", " << phi << std::endl;

	return (std::abs(theta) > 0.1 || std::abs(psi) > 0.1 || std::abs(phi) > 0.1);
}

void printRotMatrixQ(const Eigen::Matrix4d& mat)
{
	Eigen::Matrix3d rot = mat.block<3,3>(0, 0);
	Eigen::Quaterniond q(rot);
	std::cout << mat(0, 3) << ", " << mat(1, 3) << ", " << mat(2, 3) << ", " << q.x() << ", " << q.y() << ", " << q.z() << ", " << std::endl;
}

void Stitcher::addNewImage(const open3d::geometry::Image& colorImg, const open3d::geometry::Image& depthImg)
{
	std::unique_lock<std::mutex> lock(mutex_);

	open3d::camera::PinholeCameraIntrinsic intrinsic = open3d::camera::PinholeCameraIntrinsic(
		open3d::camera::PinholeCameraIntrinsicParameters::PrimeSenseDefault);
	//intrinsic.SetIntrinsics(640, 480, 524.0, 524.0, 316.7, 238.5);	// from https://www.researchgate.net/figure/ntrinsic-parameters-of-Kinect-RGB-camera_tbl2_305108995
	//intrinsic.SetIntrinsics(640, 480, 517.3, 516.5, 318.6, 255.3);		// from Freiburg test data set
	//intrinsic.SetIntrinsics(640, 480, 537.408, 537.40877, 321.897, 236.29);		// from Calibration of my own camera
	//intrinsic.SetIntrinsics(640, 480, 533.82, 533.82, 320.55, 232.35);		// from Calibration of my own camera
	intrinsic.SetIntrinsics(640, 480, 542.7693, 544.396, 318.79, 239.99);		// from Calibration of my own camera

	//open3d::io::WriteImageToPNG("color.png", colorImg_);
	//open3d::io::WriteImageToPNG("depth.png", depthImg_);

	Eigen::Matrix4d odo_init = Eigen::Matrix4d::Identity();
	auto depthFlt = depthImg.ConvertDepthToFloatImage(depthScale_, std::numeric_limits<float>::max());
	
	/*{
		float *ptr = depthFlt->PointerAt<float>(0, 0);
		int size = depthImg.width_ * depthImg.height_;
		float maxVal{ std::numeric_limits<float>::lowest() }, minVal{ std::numeric_limits<float>::max() };
		for (int i = 0; i < size; i++)
		{
			const float& val = *ptr;
			maxVal = std::max(maxVal, val);
			minVal = std::min(minVal, val);
			ptr++;
		}

		std::cout << "Min: " << minVal << ", max: " << maxVal << std::endl;
	}*/

	open3d::geometry::RGBDImage source(colorImg, *depthFlt);

	images_.push_back(source);

	if (!oldRGBDImage_.IsEmpty())
	{
		std::tuple<bool, Eigen::Matrix4d, Eigen::Matrix6d> rgbd_odo =
			open3d::pipelines::odometry::ComputeRGBDOdometry(
				oldRGBDImage_, source, intrinsic, odo_init,
				open3d::pipelines::odometry::RGBDOdometryJacobianFromHybridTerm(),
				open3d::pipelines::odometry::OdometryOption({ 20,10,5 }, 0.2));

		std::cout << "Matching ";
		if (std::get<0>(rgbd_odo))
			std::cout << "successful";
		else
			std::cout << "unsuccessful";
		std::cout << std::endl;
		//std::cout << " " << std::get<1>(rgbd_odo) << std::endl;

		//bool doIntegrate = printRotMatrix(std::get<1>(rgbd_odo)) && (transvec_.size() < 2);
		printRotMatrixQ(std::get<1>(rgbd_odo));

		if (std::get<0>(rgbd_odo))
		{
			Eigen::Matrix4d extrinsic = std::get<1>(rgbd_odo);

			//Eigen::Matrix4d extrinsicInv = extrinsic.inverse();
			transvec_.push_back(extrinsic);
			infovec_.push_back(std::get<2>(rgbd_odo));

			pos_ = extrinsic * pos_;
			Eigen::Matrix4d posInv = pos_.inverse();

			volume_->Integrate(source, intrinsic, pos_);
		}

	}
	else
	{
		Eigen::Matrix4d identity = Eigen::Matrix4d::Identity();
		pos_ = identity;
		volume_->Integrate(source, intrinsic, identity);
		infovec_.push_back(Eigen::Matrix6d::Identity());
		transvec_.push_back(identity);
	}

	posvec_.push_back(pos_);

	oldRGBDImage_ = source;

	/*{
		std::ostringstream ostr;
		ostr << "mesh_online_" << transvec_.size() << ".ply";
		auto mesh = volume_->ExtractTriangleMesh();
		open3d::io::WriteTriangleMesh(ostr.str(),
			*mesh);
	}*/
}

void Stitcher::saveVolume()
{
	auto mesh = volume_->ExtractTriangleMesh();
	open3d::io::WriteTriangleMesh("mesh_online.ply",
		*mesh);
	/*{
		std::ostringstream ostr;
		ostr << "mesh_online_" << variant_ << ".ply";
		auto mesh = volume_->ExtractTriangleMesh();
		open3d::io::WriteTriangleMesh(ostr.str(),
			*mesh);
	}*/

	std::cout << "Online mesh saved" << std::endl;

	const size_t keyFrameInterval = 3;
	const size_t maxKeyFrameDistance = 9;
	open3d::camera::PinholeCameraIntrinsic intrinsic = open3d::camera::PinholeCameraIntrinsic(
		open3d::camera::PinholeCameraIntrinsicParameters::PrimeSenseDefault);

	open3d::pipelines::registration::PoseGraph poseGraph;
	Eigen::Matrix4d transOdometry = Eigen::Matrix4d::Identity(), transOdometryInv;
	poseGraph.nodes_.push_back(open3d::pipelines::registration::PoseGraphNode(transOdometry));

	for (size_t i = 0; i < images_.size() - 1; i++)
	{
		for (size_t j = i + 1; j < images_.size(); j++)
		{
			bool isNeighbour = (j == i + 1);
			bool doLoopClosure = (i % keyFrameInterval == 0 && j % keyFrameInterval == 0 && (j-i) <= maxKeyFrameDistance);

			const open3d::geometry::RGBDImage& source = images_[i];
			const open3d::geometry::RGBDImage& target = images_[j];

			if (isNeighbour)
			{
				//Eigen::Matrix4d odo_init = Eigen::Matrix4d::Identity();
				//std::tuple<bool, Eigen::Matrix4d, Eigen::Matrix6d> rgbd_odo =
				//	open3d::pipelines::odometry::ComputeRGBDOdometry(
				//		source, target, intrinsic, odo_init,
				//		open3d::pipelines::odometry::RGBDOdometryJacobianFromHybridTerm(),
				//		open3d::pipelines::odometry::OdometryOption());
				//
				//transOdometry = std::get<1>(rgbd_odo) * transOdometry;
				transOdometry = posvec_[j];
				transOdometryInv = transOdometry.inverse();
				poseGraph.nodes_.push_back(open3d::pipelines::registration::PoseGraphNode(transOdometryInv));
				poseGraph.edges_.push_back(open3d::pipelines::registration::PoseGraphEdge(i, j,
//					std::get<1>(rgbd_odo), std::get<2>(rgbd_odo), false));
					transvec_[j], infovec_[j], false));
			}
			else if (doLoopClosure)
			{
				Eigen::Matrix4d odo_init = Eigen::Matrix4d::Identity();
				std::tuple<bool, Eigen::Matrix4d, Eigen::Matrix6d> rgbd_odo =
					open3d::pipelines::odometry::ComputeRGBDOdometry(
						source, target, intrinsic, odo_init,
						open3d::pipelines::odometry::RGBDOdometryJacobianFromHybridTerm(),
						open3d::pipelines::odometry::OdometryOption({ 20,10,5 }, 0.1));
				if (std::get<0>(rgbd_odo))	// if success==true
				{
					poseGraph.edges_.push_back(open3d::pipelines::registration::PoseGraphEdge(i, j,
						std::get<1>(rgbd_odo), std::get<2>(rgbd_odo), true));
				}
			}
		}
	}

	open3d::utility::SetVerbosityLevel(open3d::utility::VerbosityLevel::Debug);

	// Global optimization
	open3d::pipelines::registration::GlobalOptimization(poseGraph);

	open3d::utility::SetVerbosityLevel(open3d::utility::VerbosityLevel::Error);

	// Integrate
	open3d::pipelines::integration::ScalableTSDFVolume optVolume(2.0 / 512, 0.04, open3d::pipelines::integration::TSDFVolumeColorType::RGB8);

	for (size_t i = 0; i < poseGraph.nodes_.size(); i++)
	{
		auto pose = poseGraph.nodes_[i].pose_;
		Eigen::Matrix4d poseInv = pose.inverse();
		optVolume.Integrate(images_[i], intrinsic, poseInv);
	}

	// Simplify
	auto optMesh = optVolume.ExtractTriangleMesh();
	auto simplMesh = optMesh->SimplifyQuadricDecimation(static_cast<int>(optMesh->triangles_.size() / 2), std::numeric_limits<double>::infinity(), 1.0);
	open3d::io::WriteTriangleMesh("mesh_opt.ply",
		*simplMesh);

	std::cout << "Optimized mesh saved" << std::endl;

	// Subdivide the mesh to allow for finer color resolution
	auto subdivMesh = simplMesh->SubdivideLoop(1);

	// Optimize color map
	open3d::pipelines::color_map::ColorMapOptimizationOption option(true);
	open3d::camera::PinholeCameraTrajectory camera;
	for (size_t i = 0; i < poseGraph.nodes_.size(); i++)
	{
		auto pose = poseGraph.nodes_[i].pose_;
		Eigen::Matrix4d poseInv = pose.inverse();
		open3d::camera::PinholeCameraParameters cameraParams;
		cameraParams.intrinsic_ = intrinsic;
		cameraParams.extrinsic_ = poseInv;
		camera.parameters_.push_back(cameraParams);
	}
	open3d::geometry::RGBDImagePyramid rgbdImages;
	for (const auto& img : images_)
	{
		rgbdImages.push_back(std::make_shared<open3d::geometry::RGBDImage>(img));
	}
	open3d::pipelines::color_map::ColorMapOptimization(*subdivMesh, rgbdImages, camera, option);

	open3d::io::WriteTriangleMesh("mesh_color_opt.ply",
		*subdivMesh);

}

void Stitcher::reset()
{
	std::unique_lock<std::mutex> lock(mutex_);

	/*{
		std::ostringstream ostr;
		ostr << "mesh_online_" << variant_ << ".ply";
		auto mesh = volume_->ExtractTriangleMesh();
		open3d::io::WriteTriangleMesh(ostr.str(),
			*mesh);
	}*/

	std::cout << "Reset" << std::endl;


	oldRGBDImage_.Clear();
	pos_ = Eigen::Matrix4d::Identity();

	setup();
	//volume_->Reset();

	images_.clear();
	posvec_.clear();
	transvec_.clear();
	infovec_.clear();
}
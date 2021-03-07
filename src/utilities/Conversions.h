#ifndef CONVERSIONS_H
#define CONVERSIONS_H

#include "open3d/Open3D.h"

#include <memory>

#include <osg/Geode>

/**
 *
 */
class Conversions
{
public:
	static osg::ref_ptr<osg::Group> convertOpen3DToOSG(const std::shared_ptr<open3d::geometry::TriangleMesh> triangleMesh);
	static osg::ref_ptr<osg::Group> convertOpen3DToOSG(const std::shared_ptr<open3d::geometry::PointCloud> triangleMesh);

};
#endif // !CONVERSIONS_H

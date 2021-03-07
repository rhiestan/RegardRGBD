
#include "Conversions.h"

#include <osg/Geometry>
#include <osg/ShadeModel>
#include <osg/Material>


osg::ref_ptr<osg::Group> Conversions::convertOpen3DToOSG(const std::shared_ptr<open3d::geometry::TriangleMesh> triangleMesh1)
{
	auto &triangleMesh = triangleMesh1->ComputeVertexNormals(true);

	osg::ref_ptr<osg::Group> root(new osg::Group);
	osg::ref_ptr<osg::Geode> geode(new osg::Geode());
	osg::ref_ptr<osg::Geometry> geometry(new osg::Geometry());

	osg::ref_ptr<osg::Vec3Array> vertices(new osg::Vec3Array()), normals(new osg::Vec3Array());
	osg::ref_ptr<osg::Vec4Array> colors(new osg::Vec4Array());
	osg::ref_ptr<osg::Vec2Array> texcoords(new osg::Vec2Array());
	osg::ref_ptr<osg::DrawElementsUInt> drawElements(new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES));
	drawElements->setDataVariance(osg::Object::STATIC);

	int numVertices = static_cast<int>(triangleMesh.vertices_.size());
	vertices->reserve(numVertices);
	normals->reserve(numVertices);
	colors->reserve(numVertices);

	for (int j = 0; j < numVertices; j++)
	{
		const auto &vec = triangleMesh.vertices_[j];
		const auto &normal = triangleMesh.vertex_normals_[j];
		vertices->push_back(osg::Vec3(vec.x(), vec.y(), vec.z()));
		normals->push_back(osg::Vec3(normal.x(), normal.y(), normal.z()));

		const auto &color = triangleMesh.vertex_colors_[j];
		colors->push_back(osg::Vec4f(color.x(), color.y(), color.z(), 1.0));
	}

	int numFaces = static_cast<int>(triangleMesh.triangles_.size());
	drawElements->reserveElements(static_cast<unsigned int>(3 * numFaces));
	for (int j = 0; j < numFaces; j++)
	{
		const auto &triangle = triangleMesh.triangles_[j];
		for (unsigned int k = 0; k < 3; k++)
		{
			unsigned int index = static_cast<unsigned int>(triangle[k]);
			drawElements->addElement(index);
		}
	}

	vertices->setDataVariance(osg::Object::STATIC);
	normals->setDataVariance(osg::Object::STATIC);
	normals->setBinding(osg::Array::BIND_PER_VERTEX);
	geometry->setVertexArray(vertices.get());
	geometry->setNormalArray(normals.get());
	geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
	geometry->setDataVariance(osg::Object::STATIC);
	geometry->setUseVertexBufferObjects(true);

	colors->setDataVariance(osg::Object::STATIC);
	colors->setBinding(osg::Array::BIND_PER_VERTEX);
	geometry->setColorArray(colors.get());
	geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	geometry->addPrimitiveSet(drawElements.get());

	geode->addDrawable(geometry.get());
	//geode->setUpdateCallback(new StateSetUpdater(this));

	root->addChild(geode.get());

	return root;
}

osg::ref_ptr<osg::Group> Conversions::convertOpen3DToOSG(const std::shared_ptr<open3d::geometry::PointCloud> pointCloud)
{
	osg::ref_ptr<osg::Group> root(new osg::Group);
	osg::ref_ptr<osg::Geode> geode(new osg::Geode());
	osg::ref_ptr<osg::Geometry> geometry(new osg::Geometry());

	osg::ref_ptr<osg::Vec3Array> vertices(new osg::Vec3Array());
	osg::ref_ptr<osg::Vec4Array> colors(new osg::Vec4Array());
	osg::ref_ptr<osg::DrawElementsUInt> drawElements(new osg::DrawElementsUInt(osg::PrimitiveSet::POINTS));
	drawElements->setDataVariance(osg::Object::STATIC);

	int numVertices = static_cast<int>(pointCloud->points_.size());
	vertices->reserve(numVertices);
	colors->reserve(numVertices);

	for (int j = 0; j < numVertices; j++)
	{
		const auto& vec = pointCloud->points_[j];
		vertices->push_back(osg::Vec3(vec.x(), vec.y(), vec.z()));

		const auto& color = pointCloud->colors_[j];
		colors->push_back(osg::Vec4f(color.x(), color.y(), color.z(), 1.0));
	}

	drawElements->reserveElements(numVertices);
	for (int j = 0; j < numVertices; j++)
	{
		drawElements->addElement(j);
	}

	vertices->setDataVariance(osg::Object::STATIC);
	geometry->setVertexArray(vertices.get());
	geometry->setDataVariance(osg::Object::STATIC);
	geometry->setUseVertexBufferObjects(true);

	colors->setDataVariance(osg::Object::STATIC);
	colors->setBinding(osg::Array::BIND_PER_VERTEX);
	geometry->setColorArray(colors.get());
	geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	geometry->addPrimitiveSet(drawElements.get());

	geode->addDrawable(geometry.get());
	//geode->setUpdateCallback(new StateSetUpdater(this));

	osg::StateSet* stateSet = geode->getOrCreateStateSet();
	//osg::Material* material = new osg::Material;
	//material->setColorMode(osg::Material::AMBIENT_AND_DIFFUSE);
	//stateSet->setAttributeAndModes(material, osg::StateAttribute::ON);
	stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	
	root->addChild(geode.get());

	return root;
}

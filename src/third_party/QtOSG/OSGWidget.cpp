#include "OSGWidget.h"
#include "PickHandler.h"

#include <osg/Camera>

#include <osg/DisplaySettings>
#include <osg/Geode>
#include <osg/Material>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/StateSet>
#include <osg/DrawPixels>

#include <osgDB/WriteFile>

#include <osgGA/EventQueue>
#include <osgGA/TrackballManipulator>

#include <osgUtil/IntersectionVisitor>
#include <osgUtil/PolytopeIntersector>

#include <osgViewer/View>
#include <osgViewer/ViewerEventHandlers>

#include <cassert>

#include <stdexcept>
#include <vector>

#include <QDebug>
#include <QKeyEvent>
#include <QPainter>
#include <QWheelEvent>

namespace
{

#ifdef WITH_SELECTION_PROCESSING
QRect makeRectangle( const QPoint& first, const QPoint& second )
{
  // Relative to the first point, the second point may be in either one of the
  // four quadrants of an Euclidean coordinate system.
  //
  // We enumerate them in counter-clockwise order, starting from the lower-right
  // quadrant that corresponds to the default case:
  //
  //            |
  //       (3)  |  (4)
  //            |
  //     -------|-------
  //            |
  //       (2)  |  (1)
  //            |

  if( second.x() >= first.x() && second.y() >= first.y() )
    return QRect( first, second );
  else if( second.x() < first.x() && second.y() >= first.y() )
    return QRect( QPoint( second.x(), first.y() ), QPoint( first.x(), second.y() ) );
  else if( second.x() < first.x() && second.y() < first.y() )
    return QRect( second, first );
  else if( second.x() >= first.x() && second.y() < first.y() )
    return QRect( QPoint( first.x(), second.y() ), QPoint( second.x(), first.y() ) );

  // Should never reach that point...
  return QRect();
}
#endif

}

namespace osgWidget
{
  void Viewer::setupThreading()
  {
    if( _threadingModel == SingleThreaded )
    {
      if(_threadsRunning)
        stopThreading();
    }
    else
    {
      if(!_threadsRunning)
        startThreading();
    }
  }
}

OSGWidget::OSGWidget( QWidget* parent,
                      Qt::WindowFlags f )
  : QOpenGLWidget( parent,
                   f )
  , graphicsWindow_( new osgViewer::GraphicsWindowEmbedded( this->x(),
                                                            this->y(),
                                                            this->width(),
                                                            this->height() ) )
  , viewer_( new osgWidget::Viewer )
  , view1_( new osgViewer::View )
  , view2_( new osgViewer::View )
  , view3_( new osgViewer::View )
  , view4_( new osgViewer::View )
  , selectionActive_( false )
  , selectionFinished_( true )
{

  osg::Sphere* sphere    = new osg::Sphere( osg::Vec3( 0.f, 0.f, 0.f ), 0.25f );
  osg::ShapeDrawable* sd = new osg::ShapeDrawable( sphere );
  sd->setColor( osg::Vec4( 1.f, 0.f, 0.f, 1.f ) );
  sd->setName( "A nice sphere" );

  osg::Geode* geode = new osg::Geode;
  geode->addDrawable( sd );

  // Set material for basic lighting and enable depth tests. Else, the sphere
  // will suffer from rendering errors.
  {
    osg::StateSet* stateSet = geode->getOrCreateStateSet();
    osg::Material* material = new osg::Material;

    material->setColorMode( osg::Material::AMBIENT_AND_DIFFUSE );

    stateSet->setAttributeAndModes( material, osg::StateAttribute::ON );
    stateSet->setMode( GL_DEPTH_TEST, osg::StateAttribute::ON );
  }

  float aspectRatio = static_cast<float>( this->width() / 2 ) / static_cast<float>( this->height() );
  auto pixelRatio   = this->devicePixelRatio();

  osg::Camera* camera = new osg::Camera;
  camera->setViewport( 0, 0, this->width() * pixelRatio, this->height() * pixelRatio );
  camera->setClearColor( osg::Vec4( 0.f, 0.f, 1.f, 1.f ) );
  camera->setProjectionMatrixAsPerspective( 30.f, aspectRatio, 1.f, 1000.f );
  camera->setGraphicsContext( graphicsWindow_ );

  osgViewer::View* view = new osgViewer::View;
  view->setCamera( camera );
  view->setSceneData( geode );
  view->addEventHandler( new osgViewer::StatsHandler );
#ifdef WITH_PICK_HANDLER
  view->addEventHandler( new PickHandler( this->devicePixelRatio() ) );
#endif

  osgGA::TrackballManipulator* manipulator = new osgGA::TrackballManipulator;
  manipulator->setAllowThrow( false );

  view->setCameraManipulator( manipulator );

  viewer_->addView( view );

/*	osg::ref_ptr<osg::DrawPixels> image = new osg::DrawPixels;
	viewer_->getCamera()->setGraphicsContext(graphicsWindow_);
	pOSGGLCanvas_->setViewer(viewer);

	int width, height;
	pOSGGLCanvas_->GetSize(&width, &height);
	viewer->getCamera()->setViewport(0, 0, width, height);
	viewer->addEventHandler(new osgViewer::StatsHandler);

	// On Linux (and possibly on other platforms as well), setting SingleThreaded results in
	// getting the CPU affinity mask set to CPU 0 for the whole process, i.e. only one CPU is used by Regard3D.
	viewer->setThreadingModel(osgViewer::Viewer::ThreadPerContext);
	//	viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);

	osgGA::TrackballManipulator* cameraManip = new osgGA::TrackballManipulator;

	// Disable "throwing": If the mouse is still moving while the button is released, the object keeps on moving
	cameraManip->setAllowThrow(false);

	// Make trackball smaller, i.e. object turns more quickly on mouse movements
	double trackballSize = cameraManip->getTrackballSize();
	cameraManip->setTrackballSize(trackballSize / 2.0);

	viewer->setCameraManipulator(cameraManip);

	// Adds some shorcuts to enable/disable texturing, lighting, back-face culling etc.
//	viewer->addEventHandler( new osgGA::StateSetManipulator(viewer->getCamera()->getOrCreateStateSet()) );

	pRegard3DModelViewHelper_ = new Regard3DModelViewHelper();
	pRegard3DModelViewHelper_->setViewer(viewer);

	SetViewer(viewer);
	pOSGGLCanvas_->setMainFrame(this);
	pOSGGLCanvas_->setModelViewHelper(pRegard3DModelViewHelper_);
	*/

	/*float aspectRatio = static_cast<float>(this->width() / 2) / static_cast<float>(this->height());
	auto pixelRatio = this->devicePixelRatio();

	for (int i = 0; i < 4; i++)
	{
		osgViewer::View* pView = view1_.get();
		if (i == 1)
			pView = view2_.get();
		else if (i == 2)
			pView = view3_.get();
		else if (i == 3)
			pView = view4_.get();

		osg::Camera* camera = new osg::Camera;

		if(i == 0)
			camera->setViewport(0, 0,
				this->width() / 2 * pixelRatio, this->height() / 2 * pixelRatio);
		else if (i == 1)
			camera->setViewport(this->width() / 2 * pixelRatio, 0,
				this->width() / 2 * pixelRatio, this->height() / 2 * pixelRatio);
		else if (i == 2)
			camera->setViewport(0, this->height() / 2 * pixelRatio,
				this->width() / 2 * pixelRatio, this->height() / 2 * pixelRatio);
		else if (i == 3)
			camera->setViewport(this->width() / 2 * pixelRatio, this->height() / 2 * pixelRatio,
				this->width() / 2 * pixelRatio, this->height() / 2 * pixelRatio);

		camera->setClearColor(osg::Vec4(0.f, 0.f, 1.f, 1.f));
		camera->setProjectionMatrixAsPerspective(30.f, aspectRatio, 1.f, 1000.f);
		camera->setGraphicsContext(graphicsWindow_);

		osg::ref_ptr<osg::Geode> geode = new osg::Geode;
		if(i == 0 || i == 1)
		{
			osg::ref_ptr<osg::DrawPixels> image = new osg::DrawPixels;
			image->setPosition(osg::Vec3(0.0, 0.0, 0.0));
			// image->setImage(osgDB::readImageFile("2.jpg"));
			osg::ref_ptr<osg::Image> img = new osg::Image;
			img->allocateImage(100, 100, 1, GL_RGB, GL_UNSIGNED_BYTE);
			for (int y = 0; y < 100; y++)
			{
				unsigned char* data = img->data(0, y, 0);
				for (int j = 0; j < 50; j++)
				{
					data[j * 3] = 128;
					data[j * 3+1] = j * 2;
					data[j * 3+2] = 255-j*2;
				}
			}
			img->dirty();
			image->setImage(img);
			image->setSubImageDimensions(0, 0, 100, 100);
			geode->addChild(image);
		}
		else
		{

		}
		osgViewer::View* sideView = new osgViewer::View;
		sideView->setCamera(camera);
		sideView->setSceneData(geode);
		sideView->addEventHandler(new osgViewer::StatsHandler);
		sideView->setCameraManipulator(new osgGA::TrackballManipulator);

		viewer_->addView(pView);
	}*/
  viewer_->setThreadingModel( osgViewer::CompositeViewer::SingleThreaded );
  viewer_->realize();

  // This ensures that the widget will receive keyboard events. This focus
  // policy is not set by default. The default, Qt::NoFocus, will result in
  // keyboard events that are ignored.
  this->setFocusPolicy( Qt::StrongFocus );
  this->setMinimumSize( 100, 100 );

  // Ensures that the widget receives mouse move events even though no
  // mouse button has been pressed. We require this in order to let the
  // graphics window switch viewports properly.
  this->setMouseTracking( true );
}

OSGWidget::~OSGWidget()
{
}

void OSGWidget::paintEvent( QPaintEvent* /* paintEvent */ )
{
  this->makeCurrent();

  QPainter painter( this );
  painter.setRenderHint( QPainter::Antialiasing );

  this->paintGL();

#ifdef WITH_SELECTION_PROCESSING
  if( selectionActive_ && !selectionFinished_ )
  {
    painter.setPen( Qt::black );
    painter.setBrush( Qt::transparent );
    painter.drawRect( makeRectangle( selectionStart_, selectionEnd_ ) );
  }
#endif

  painter.end();

  this->doneCurrent();
}

void OSGWidget::paintGL()
{
  viewer_->frame();
}

void OSGWidget::resizeGL( int width, int height )
{
  this->getEventQueue()->windowResize( this->x(), this->y(), width, height );
  graphicsWindow_->resized( this->x(), this->y(), width, height );

  this->onResize( width, height );
}

void OSGWidget::keyPressEvent( QKeyEvent* event )
{
  QString keyString   = event->text();
  const char* keyData = keyString.toLocal8Bit().data();

  if( event->key() == Qt::Key_S )
  {
#ifdef WITH_SELECTION_PROCESSING
    selectionActive_ = !selectionActive_;
#endif

    // Further processing is required for the statistics handler here, so we do
    // not return right away.
  }
  else if( event->key() == Qt::Key_D )
  {
    return;
  }
  else if( event->key() == Qt::Key_H )
  {
    this->onHome();
    return;
  }

  this->getEventQueue()->keyPress( osgGA::GUIEventAdapter::KeySymbol( *keyData ) );
}

void OSGWidget::keyReleaseEvent( QKeyEvent* event )
{
  QString keyString   = event->text();
  const char* keyData = keyString.toLocal8Bit().data();

  this->getEventQueue()->keyRelease( osgGA::GUIEventAdapter::KeySymbol( *keyData ) );
}

void OSGWidget::mouseMoveEvent( QMouseEvent* event )
{
  // Note that we have to check the buttons mask in order to see whether the
  // left button has been pressed. A call to `button()` will only result in
  // `Qt::NoButton` for mouse move events.
  if( selectionActive_ && event->buttons() & Qt::LeftButton )
  {
    selectionEnd_ = event->pos();

    // Ensures that new paint events are created while the user moves the
    // mouse.
    this->update();
  }
  else
  {
    auto pixelRatio = this->devicePixelRatio();

    this->getEventQueue()->mouseMotion( static_cast<float>( event->x() * pixelRatio ),
                                        static_cast<float>( event->y() * pixelRatio ) );
  }
}

void OSGWidget::mousePressEvent( QMouseEvent* event )
{
  // Selection processing
  if( selectionActive_ && event->button() == Qt::LeftButton )
  {
    selectionStart_    = event->pos();
    selectionEnd_      = selectionStart_; // Deletes the old selection
    selectionFinished_ = false;           // As long as this is set, the rectangle will be drawn
  }

  // Normal processing
  else
  {
    // 1 = left mouse button
    // 2 = middle mouse button
    // 3 = right mouse button

    unsigned int button = 0;

    switch( event->button() )
    {
    case Qt::LeftButton:
      button = 1;
      break;

    case Qt::MiddleButton:
      button = 2;
      break;

    case Qt::RightButton:
      button = 3;
      break;

    default:
      break;
    }

    auto pixelRatio = this->devicePixelRatio();

    this->getEventQueue()->mouseButtonPress( static_cast<float>( event->x() * pixelRatio ),
                                             static_cast<float>( event->y() * pixelRatio ),
                                             button );
    }
}

void OSGWidget::mouseReleaseEvent(QMouseEvent* event)
{
  // Selection processing: Store end position and obtain selected objects
  // through polytope intersection.
  if( selectionActive_ && event->button() == Qt::LeftButton )
  {
    selectionEnd_      = event->pos();
    selectionFinished_ = true; // Will force the painter to stop drawing the
                               // selection rectangle

    this->processSelection();
  }

  // Normal processing
  else
  {
    // 1 = left mouse button
    // 2 = middle mouse button
    // 3 = right mouse button

    unsigned int button = 0;

    switch( event->button() )
    {
    case Qt::LeftButton:
      button = 1;
      break;

    case Qt::MiddleButton:
      button = 2;
      break;

    case Qt::RightButton:
      button = 3;
      break;

    default:
      break;
    }

    auto pixelRatio = this->devicePixelRatio();

    this->getEventQueue()->mouseButtonRelease( static_cast<float>( pixelRatio * event->x() ),
                                               static_cast<float>( pixelRatio * event->y() ),
                                               button );
  }
}

void OSGWidget::wheelEvent( QWheelEvent* event )
{
  // Ignore wheel events as long as the selection is active.
  if( selectionActive_ )
    return;

  event->accept();
  int delta = event->delta();

  osgGA::GUIEventAdapter::ScrollingMotion motion = delta > 0 ?   osgGA::GUIEventAdapter::SCROLL_UP
                                                               : osgGA::GUIEventAdapter::SCROLL_DOWN;

  this->getEventQueue()->mouseScroll( motion );
}

bool OSGWidget::event( QEvent* event )
{
  bool handled = QOpenGLWidget::event( event );

  // This ensures that the OSG widget is always going to be repainted after the
  // user performed some interaction. Doing this in the event handler ensures
  // that we don't forget about some event and prevents duplicate code.
  switch( event->type() )
  {
  case QEvent::KeyPress:
  case QEvent::KeyRelease:
  case QEvent::MouseButtonDblClick:
  case QEvent::MouseButtonPress:
  case QEvent::MouseButtonRelease:
  case QEvent::MouseMove:
  case QEvent::Wheel:
    this->update();
    break;

  default:
    break;
  }

  return handled;
}

void OSGWidget::onHome()
{
  osgViewer::ViewerBase::Views views;
  viewer_->getViews( views );

  for( std::size_t i = 0; i < views.size(); i++ )
  {
    osgViewer::View* view = views.at(i);
    view->home();
  }
}

void OSGWidget::onResize( int width, int height )
{
  std::vector<osg::Camera*> cameras;
  viewer_->getCameras( cameras );

  auto pixelRatio = this->devicePixelRatio();

  if(cameras.size() == 0)
	  cameras[0]->setViewport(0, 0, width * pixelRatio, height * pixelRatio);

#if 0
  if (cameras.size() > 1)
  {
	  cameras[0]->setViewport(0, 0, width / 2 * pixelRatio, height * pixelRatio);
	  cameras[1]->setViewport(width / 2 * pixelRatio, 0, width / 2 * pixelRatio, height * pixelRatio);
  }
  else
  {
	  for (int i = 0; i < 4; i++)
	  {
		  osgViewer::View* pView = view1_.get();
		  if (i == 1)
			  pView = view2_.get();
		  else if (i == 2)
			  pView = view3_.get();
		  else if (i == 3)
			  pView = view4_.get();

		  osg::Camera* camera = new osg::Camera;

		  if (i == 0)
			  camera->setViewport(0, 0,
				  width / 2 * pixelRatio, height / 2 * pixelRatio);
		  else if (i == 1)
			  camera->setViewport(width / 2 * pixelRatio, 0,
				  width / 2 * pixelRatio, height / 2 * pixelRatio);
		  else if (i == 2)
			  camera->setViewport(0, height / 2 * pixelRatio,
				  width / 2 * pixelRatio, height / 2 * pixelRatio);
		  else if (i == 3)
			  camera->setViewport(width / 2 * pixelRatio, height / 2 * pixelRatio,
				  width / 2 * pixelRatio, height / 2 * pixelRatio);
	  }
  }
#endif
}

osgGA::EventQueue* OSGWidget::getEventQueue() const
{
  osgGA::EventQueue* eventQueue = graphicsWindow_->getEventQueue();

  if( eventQueue )
    return eventQueue;
  else
    throw std::runtime_error( "Unable to obtain valid event queue");
}

void OSGWidget::processSelection()
{
#ifdef WITH_SELECTION_PROCESSING
  QRect selectionRectangle = makeRectangle( selectionStart_, selectionEnd_ );
  auto widgetHeight        = this->height();
  auto pixelRatio          = this->devicePixelRatio();

  double xMin = selectionRectangle.left();
  double xMax = selectionRectangle.right();
  double yMin = widgetHeight - selectionRectangle.bottom();
  double yMax = widgetHeight - selectionRectangle.top();

  xMin *= pixelRatio;
  yMin *= pixelRatio;
  xMax *= pixelRatio;
  yMax *= pixelRatio;

  osgUtil::PolytopeIntersector* polytopeIntersector
      = new osgUtil::PolytopeIntersector( osgUtil::PolytopeIntersector::WINDOW,
                                          xMin, yMin,
                                          xMax, yMax );

  // This limits the amount of intersections that are reported by the
  // polytope intersector. Using this setting, a single drawable will
  // appear at most once while calculating intersections. This is the
  // preferred and expected behaviour.
  polytopeIntersector->setIntersectionLimit( osgUtil::Intersector::LIMIT_ONE_PER_DRAWABLE );

  osgUtil::IntersectionVisitor iv( polytopeIntersector );

  for( unsigned int viewIndex = 0; viewIndex < viewer_->getNumViews(); viewIndex++ )
  {
    qDebug() << "View index:" << viewIndex;

    osgViewer::View* view = viewer_->getView( viewIndex );

    if( !view )
      throw std::runtime_error( "Unable to obtain valid view for selection processing" );

    osg::Camera* camera = view->getCamera();

    if( !camera )
      throw std::runtime_error( "Unable to obtain valid camera for selection processing" );

    camera->accept( iv );

    if( !polytopeIntersector->containsIntersections() )
      continue;

    auto intersections = polytopeIntersector->getIntersections();

    for( auto&& intersection : intersections )
      qDebug() << "Selected a drawable:" << QString::fromStdString( intersection.drawable->getName() );
  }
#endif
}


void OSGWidget::setGeometry(osg::ref_ptr<osg::Group> geom)
{
	osgViewer::View *pView = viewer_->getView(0);
	if (pView != nullptr)
	{
		pView->setSceneData(geom);
		//pView->updateSlaves();
	}

	update();
}

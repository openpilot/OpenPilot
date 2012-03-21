TEMPLATE = lib
TARGET = GLC_lib
DEFINES += GLC_LIB_LIBRARY
include(../../openpilotgcslibrary.pri)

# GLC_lib qmake configuration
#TEMPLATE = lib
QT += opengl \
    core


CONFIG += exceptions \
    warn_on
#TARGET = GLC_lib
#VERSION = 2.2.0

DEFINES += CREATE_GLC_LIB_DLL
DEFINES += LIB3DS_EXPORTS

DEFINES += _CRT_SECURE_NO_WARNINGS

unix:OBJECTS_DIR = ./Build
unix:MOC_DIR = ./Build
unix:UI_DIR = ./Build

DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += ./3rdparty/zlib

RESOURCES += glc_lib.qrc

# Input					
HEADERS_QUAZIP +=	3rdparty/quazip/crypt.h \
					3rdparty/quazip/ioapi.h \
					3rdparty/quazip/quazip.h \
					3rdparty/quazip/quazipfile.h \
					3rdparty/quazip/quazipfileinfo.h \
					3rdparty/quazip/quazipnewinfo.h \
					3rdparty/quazip/unzip.h \
					3rdparty/quazip/zip.h

HEADERS_LIB3DS += 3rdparty/lib3ds/atmosphere.h \
           3rdparty/lib3ds/background.h \
           3rdparty/lib3ds/camera.h \
           3rdparty/lib3ds/chunk.h \
           3rdparty/lib3ds/chunktable.h \
           3rdparty/lib3ds/ease.h \
           3rdparty/lib3ds/file.h \
           3rdparty/lib3ds/io.h \
           3rdparty/lib3ds/light.h \
           3rdparty/lib3ds/material.h \
           3rdparty/lib3ds/matrix.h \
           3rdparty/lib3ds/mesh.h \
           3rdparty/lib3ds/node.h \
           3rdparty/lib3ds/quat.h \
           3rdparty/lib3ds/shadow.h \
           3rdparty/lib3ds/tcb.h \
           3rdparty/lib3ds/tracks.h \
           3rdparty/lib3ds/types.h \
           3rdparty/lib3ds/vector.h \
           3rdparty/lib3ds/viewport.h
           
HEADERS_GLEXT += 3rdparty/glext/glext.h

HEADERS_GLC_MATHS += 	maths/glc_utils_maths.h \
						maths/glc_vector2d.h \
						maths/glc_vector2df.h \
						maths/glc_vector3d.h \
						maths/glc_vector4d.h \
						maths/glc_vector3df.h \
						maths/glc_matrix4x4.h \
						maths/glc_interpolator.h \
						maths/glc_plane.h \
						maths/glc_geomtools.h \
						maths/glc_line3d.h
						
HEADERS_GLC_IO +=		io/glc_objmtlloader.h \
						io/glc_objtoworld.h \
						io/glc_stltoworld.h \
						io/glc_offtoworld.h \
						io/glc_3dstoworld.h \
						io/glc_3dxmltoworld.h \
						io/glc_colladatoworld.h \
						io/glc_worldto3dxml.h \
						io/glc_bsreptoworld.h \
						io/glc_xmlutil.h \
						io/glc_fileloader.h \
						io/glc_worldreaderplugin.h \
						io/glc_worldreaderhandler.h

HEADERS_GLC_SCENEGRAPH +=	sceneGraph/glc_3dviewcollection.h \
							sceneGraph/glc_3dviewinstance.h \
							sceneGraph/glc_structreference.h \
							sceneGraph/glc_structinstance.h \
							sceneGraph/glc_structoccurence.h \
							sceneGraph/glc_world.h \
							sceneGraph/glc_attributes.h \
							sceneGraph/glc_worldhandle.h \
							sceneGraph/glc_spacepartitioning.h \
							sceneGraph/glc_octree.h \
							sceneGraph/glc_octreenode.h \
							sceneGraph/glc_selectionset.h
							
HEADERS_GLC_GEOMETRY +=		geometry/glc_geometry.h \
							geometry/glc_circle.h \
							geometry/glc_cylinder.h \
							geometry/glc_point.h \
							geometry/glc_box.h \
							geometry/glc_meshdata.h \
							geometry/glc_primitivegroup.h \
							geometry/glc_mesh.h \
							geometry/glc_lod.h \
							geometry/glc_rectangle.h \
							geometry/glc_line.h \
							geometry/glc_rep.h \
							geometry/glc_3drep.h \
							geometry/glc_pointsprite.h \
							geometry/glc_bsrep.h \
							geometry/glc_wiredata.h \
							geometry/glc_arrow.h \
							geometry/glc_polylines.h \
							geometry/glc_disc.h \
							geometry/glc_cone.h \
							geometry/glc_sphere.h \
							geometry/glc_pointcloud.h

HEADERS_GLC_SHADING +=	shading/glc_material.h \						
						shading/glc_texture.h \
						shading/glc_shader.h \
						shading/glc_selectionmaterial.h \
						shading/glc_light.h \
						shading/glc_renderproperties.h
						
HEADERS_GLC_VIEWPORT +=	viewport/glc_camera.h \
						viewport/glc_imageplane.h \
						viewport/glc_viewport.h \
						viewport/glc_movercontroller.h\
						viewport/glc_mover.h \
						viewport/glc_panmover.h \
						viewport/glc_repmover.h \
						viewport/glc_repcrossmover.h \
						viewport/glc_zoommover.h \
						viewport/glc_trackballmover.h \
						viewport/glc_reptrackballmover.h \
						viewport/glc_settargetmover.h \
						viewport/glc_turntablemover.h \
						viewport/glc_frustum.h \
						viewport/glc_flymover.h \
						viewport/glc_repflymover.h \
						viewport/glc_userinput.h \
						viewport/glc_tsrmover.h

HEADERS_GLC += glc_global.h \
           glc_object.h \
           glc_factory.h \
           glc_boundingbox.h \
           glc_exception.h \
           glc_openglexception.h \
           glc_fileformatexception.h \
           glc_ext.h \
           glc_state.h \
           glc_config.h \
           glc_cachemanager.h \
           glc_renderstatistics.h \
           glc_log.h \
           glc_errorlog.h \
           glc_tracelog.h \
           glc_context.h \
           glc_contextmanager.h \
           glc_contextshareddata.h \
           glc_uniformshaderdata.h
           
HEADERS_GLC_3DWIDGET += 3DWidget/glc_3dwidget.h \
						3DWidget/glc_cuttingplane.h \
						3DWidget/glc_3dwidgetmanager.h \
						3DWidget/glc_3dwidgetmanagerhandle.h \
						3DWidget/glc_abstractmanipulator.h \
						3DWidget/glc_pullmanipulator.h \
						3DWidget/glc_rotationmanipulator.h \
						3DWidget/glc_axis.h

HEADERS_GLC_GLU +=	glu/glc_glu.h

HEADERS += $${HEADERS_QUAZIP} $${HEADERS_LIB3DS} $${HEADERS_GLC_MATHS} $${HEADERS_GLC_IO}
HEADERS += $${HEADERS_GLC} $${HEADERS_GLEXT} $${HEADERS_GLC_SCENEGRAPH} $${HEADERS_GLC_GEOMETRY}
HEADERS += $${HEADERS_GLC_SHADING} $${HEADERS_GLC_VIEWPORT} $${HEADERS_GLC_3DWIDGET} $${HEADERS_GLC_GLU}
		   
SOURCES += 3rdparty/zlib/adler32.c \
           3rdparty/zlib/compress.c \
           3rdparty/zlib/crc32.c \
           3rdparty/zlib/deflate.c \
           3rdparty/zlib/gzio.c \
           3rdparty/zlib/inffast.c \
           3rdparty/zlib/inflate.c \
           3rdparty/zlib/inftrees.c \
           3rdparty/zlib/trees.c \
           3rdparty/zlib/uncompr.c \
           3rdparty/zlib/zutil.c

SOURCES += 3rdparty/quazip/ioapi.c \
           3rdparty/quazip/quazip.cpp \
           3rdparty/quazip/quazipfile.cpp \
           3rdparty/quazip/quazipnewinfo.cpp \
           3rdparty/quazip/unzip.c \
           3rdparty/quazip/zip.c

SOURCES += 3rdparty/lib3ds/atmosphere.c \
           3rdparty/lib3ds/background.c \
           3rdparty/lib3ds/camera.c \
           3rdparty/lib3ds/chunk.c \
           3rdparty/lib3ds/ease.c \
           3rdparty/lib3ds/file.c \
           3rdparty/lib3ds/io.c \
           3rdparty/lib3ds/light.c \
           3rdparty/lib3ds/material.c \
           3rdparty/lib3ds/matrix.c \
           3rdparty/lib3ds/mesh.c \
           3rdparty/lib3ds/node.c \
           3rdparty/lib3ds/quat.c \
           3rdparty/lib3ds/shadow.c \
           3rdparty/lib3ds/tcb.c \
           3rdparty/lib3ds/tracks.c \
           3rdparty/lib3ds/vector.c \
           3rdparty/lib3ds/viewport.c
   
SOURCES +=	maths/glc_matrix4x4.cpp \
			maths/glc_vector4d.cpp \
			maths/glc_interpolator.cpp \
			maths/glc_plane.cpp \
			maths/glc_geomtools.cpp \
			maths/glc_line3d.cpp

SOURCES +=	io/glc_objmtlloader.cpp \
			io/glc_objtoworld.cpp \
			io/glc_stltoworld.cpp \
			io/glc_offtoworld.cpp \
			io/glc_3dstoworld.cpp \
			io/glc_3dxmltoworld.cpp \
			io/glc_colladatoworld.cpp \
			io/glc_worldto3dxml.cpp \
			io/glc_bsreptoworld.cpp \
			io/glc_fileloader.cpp

SOURCES +=	sceneGraph/glc_3dviewcollection.cpp \
			sceneGraph/glc_3dviewinstance.cpp \
			sceneGraph/glc_structreference.cpp \
			sceneGraph/glc_structinstance.cpp \
			sceneGraph/glc_structoccurence.cpp \
			sceneGraph/glc_world.cpp \
			sceneGraph/glc_attributes.cpp \
			sceneGraph/glc_worldhandle.cpp \
			sceneGraph/glc_spacepartitioning.cpp \
			sceneGraph/glc_octree.cpp \
			sceneGraph/glc_octreenode.cpp \
			sceneGraph/glc_selectionset.cpp

SOURCES +=	geometry/glc_geometry.cpp \
			geometry/glc_circle.cpp \
			geometry/glc_cylinder.cpp \
			geometry/glc_point.cpp \
			geometry/glc_box.cpp \
			geometry/glc_meshdata.cpp \
			geometry/glc_primitivegroup.cpp \
			geometry/glc_mesh.cpp \
			geometry/glc_lod.cpp \
			geometry/glc_rectangle.cpp \
			geometry/glc_line.cpp \
			geometry/glc_rep.cpp \
			geometry/glc_3drep.cpp \
			geometry/glc_pointsprite.cpp \
			geometry/glc_bsrep.cpp \
			geometry/glc_wiredata.cpp \
			geometry/glc_arrow.cpp \
			geometry/glc_polylines.cpp \
			geometry/glc_disc.cpp \
			geometry/glc_cone.cpp \
			geometry/glc_sphere.cpp \
			geometry/glc_pointcloud.cpp


SOURCES +=	shading/glc_material.cpp \
			shading/glc_texture.cpp \
			shading/glc_light.cpp \
			shading/glc_selectionmaterial.cpp \
			shading/glc_shader.cpp \
			shading/glc_renderproperties.cpp

SOURCES +=	viewport/glc_camera.cpp \
			viewport/glc_imageplane.cpp \
			viewport/glc_viewport.cpp \
			viewport/glc_movercontroller.cpp\
			viewport/glc_mover.cpp \
			viewport/glc_panmover.cpp \
			viewport/glc_repmover.cpp \
			viewport/glc_repcrossmover.cpp \
			viewport/glc_zoommover.cpp \
			viewport/glc_trackballmover.cpp \
			viewport/glc_reptrackballmover.cpp \
			viewport/glc_settargetmover.cpp \
			viewport/glc_turntablemover.cpp \
			viewport/glc_frustum.cpp \
			viewport/glc_flymover.cpp \
			viewport/glc_repflymover.cpp \
			viewport/glc_userinput.cpp \
			viewport/glc_tsrmover.cpp
		
SOURCES +=	glc_global.cpp \
			glc_object.cpp \			
			glc_factory.cpp \
			glc_boundingbox.cpp \
			glc_exception.cpp \
			glc_openglexception.cpp \
			glc_fileformatexception.cpp \
			glc_ext.cpp \
			glc_state.cpp \
			glc_cachemanager.cpp \
			glc_renderstatistics.cpp \
			glc_log.cpp \
			glc_errorlog.cpp \
			glc_tracelog.cpp \
			glc_context.cpp \
			glc_contextmanager.cpp \
			glc_contextshareddata.cpp \
			glc_uniformshaderdata.cpp

SOURCES +=	3DWidget/glc_3dwidget.cpp \
			3DWidget/glc_cuttingplane.cpp \
 			3DWidget/glc_3dwidgetmanager.cpp \
			3DWidget/glc_3dwidgetmanagerhandle.cpp \
			3DWidget/glc_abstractmanipulator.cpp \
			3DWidget/glc_pullmanipulator.cpp \
			3DWidget/glc_rotationmanipulator.cpp \
			3DWidget/glc_axis.cpp
			
SOURCES +=	glu/glc_project.cpp

# Windows compilation configuration
win32:CONFIG *= dll

# install header
HEADERS_INST = include/GLC_BoundingBox \
    		   include/GLC_Box \
    		   include/GLC_Camera \
    		   include/GLC_Circle \
    		   include/GLC_3DViewCollection \
    		   include/GLC_Cylinder \
    		   include/GLC_Exception \
    		   include/GLC_Factory \
    		   include/GLC_FileFormatException \
    		   include/GLC_Geometry \
    		   include/GLC_ImagePlane \
    		   include/GLC_3DViewInstance \
    		   include/GLC_Interpolator \
    		   include/GLC_Light \
    		   include/GLC_Material \
    		   include/GLC_Matrix4x4 \
    		   include/GLC_Node \
    		   include/GLC_Object \
    		   include/GLC_OpenGlException \
    		   include/GLC_Point \
    		   include/GLC_Point2d \
    		   include/GLC_Point2df \
    		   include/GLC_Point3d \
    		   include/GLC_Point3df \
    		   include/GLC_Texture \
    		   include/GLC_Vector2d \
    		   include/GLC_Vector2df \
    		   include/GLC_Vector3d \
    		   include/GLC_Vector3df \
    		   include/GLC_Vector4d \
    		   include/GLC_Viewport \
    		   include/GLC_World \
    		   include/GLC_Shader \
    		   include/GLC_SelectionMaterial \
    		   include/GLC_State \
    		   include/GLC_Mover \
    		   include/GLC_MoverController \
    		   include/GLC_PanMover \
    		   include/GLC_ZoomMover \
    		   include/GLC_TrackBallMover \
    		   include/GLC_RepMover \
    		   include/GLC_RepCrossMover \
    		   include/GLC_RepTrackBallMover \
    		   include/GLC_TurnTableMover \
    		   include/GLC_Attributes \
    		   include/GLC_Rectangle \
    		   include/GLC_Mesh \
    		   include/GLC_StructOccurence \
    		   include/GLC_StructInstance \
    		   include/GLC_StructReference \
    		   include/GLC_Line \
    		   include/GLC_Rep \
    		   include/GLC_3DRep \
    		   include/GLC_PointSprite \
    		   include/GLC_CacheManager \
    		   include/GLC_BSRep \
    		   include/GLC_RenderProperties \
    		   include/GLC_Global \
    		   include/GLC_SpacePartitioning \
    		   include/GLC_Octree \
    		   include/GLC_OctreeNode \
    		   include/GLC_Plane \
    		   include/GLC_Frustum \
    		   include/GLC_GeomTools \
    		   include/GLC_Line3d \
    		   include/GLC_3DWidget \
    		   include/GLC_CuttingPlane \
    		   include/GLC_3DWidgetManager \
    		   include/GLC_3DWidgetManagerHandle \
    		   include/GLC_Arrow \
    		   include/GLC_Polylines \
    		   include/GLC_Disc \
    		   include/GLC_AbstractManipulator \
    		   include/GLC_PullManipulator \
    		   include/GLC_RotationManipulator \
    		   include/GLC_FlyMover \
    		   include/GLC_RepFlyMover \
    		   include/GLC_WorldTo3dxml \
    		   include/GLC_RenderStatistics \
    		   include/GLC_Ext \
    		   include/GLC_Cone \
    		   include/GLC_Sphere \
    		   include/GLC_Axis \
    		   include/GLC_Log \
    		   include/GLC_ErrorLog \
    		   include/GLC_TraceLog \
    		   include/glcXmlUtil \
    		   include/GLC_RenderState \
    		   include/GLC_FileLoader \
    		   include/GLC_WorldReaderPlugin \
    		   include/GLC_WorldReaderHandler \
    		   include/GLC_PointCloud \
    		   include/GLC_SelectionSet \
    		   include/GLC_UserInput \
    		   include/GLC_TsrMover \
    		   include/GLC_Glu \
    		   include/GLC_Context \
    		   include/GLC_ContextManager

    			   
# Linux and macx install configuration
unix {
    # Location of HEADERS and library
    LIB_DIR = /usr/local/lib
    INCLUDE_DIR = /usr/local/include
    # Adds a -P to preserve link
	QMAKE_COPY_FILE = $${QMAKE_COPY_FILE} -P
	include.path = $${INCLUDE_DIR}/GLC_lib
	include_lib3ds.path = $${INCLUDE_DIR}/GLC_lib/3rdparty/lib3ds
	include_glext.path = $${INCLUDE_DIR}/GLC_lib/3rdparty/glext
	include_quazip.path = $${INCLUDE_DIR}/GLC_lib/3rdparty/quazip
	include_glc_maths.path = $${INCLUDE_DIR}/GLC_lib/maths
	include_glc_io.path = $${INCLUDE_DIR}/GLC_lib/io
	include_glc_scengraph.path = $${INCLUDE_DIR}/GLC_lib/sceneGraph
	include_glc_geometry.path = $${INCLUDE_DIR}/GLC_lib/geometry
	include_glc_shading.path = $${INCLUDE_DIR}/GLC_lib/shading
	include_glc_viewport.path = $${INCLUDE_DIR}/GLC_lib/viewport
	include_glc_3dwidget.path = $${INCLUDE_DIR}/GLC_lib/3DWidget
	include_glc_glu.path = $${INCLUDE_DIR}/GLC_lib/glu
}

# Windows Install configuration
win32 { 
    # Location of HEADERS and library
    LIB_DIR = C:/GLC_lib/lib
    INCLUDE_DIR = C:/GLC_lib/include
    include.path = $${INCLUDE_DIR}
    include_lib3ds.path = $${INCLUDE_DIR}/3rdparty/lib3ds
    include_glext.path = $${INCLUDE_DIR}/3rdparty/glext
    include_quazip.path = $${INCLUDE_DIR}/3rdparty/quazip
    include_glc_maths.path = $${INCLUDE_DIR}/maths
    include_glc_io.path = $${INCLUDE_DIR}/io
    include_glc_scengraph.path = $${INCLUDE_DIR}/sceneGraph
    include_glc_geometry.path = $${INCLUDE_DIR}/geometry
    include_glc_shading.path = $${INCLUDE_DIR}/shading
    include_glc_viewport.path = $${INCLUDE_DIR}/viewport
    include_glc_3dwidget.path = $${INCLUDE_DIR}/3DWidget
    include_glc_glu.path = $${INCLUDE_DIR}/glu
}    

include.files = $${HEADERS_GLC} $${HEADERS_INST}
include_lib3ds.files = $${HEADERS_LIB3DS}
include_glext.files =$${HEADERS_GLEXT}
include_quazip.files = $${HEADERS_QUAZIP}
include_glc_maths.files= $${HEADERS_GLC_MATHS}
include_glc_io.files= $${HEADERS_GLC_IO}
include_glc_scengraph.files= $${HEADERS_GLC_SCENEGRAPH}
include_glc_geometry.files= $${HEADERS_GLC_GEOMETRY}
include_glc_shading.files = $${HEADERS_GLC_SHADING}
include_glc_viewport.files = $${HEADERS_GLC_VIEWPORT}
include_glc_3dwidget.files = $${HEADERS_GLC_3DWIDGET}
include_glc_glu.files = $${HEADERS_GLC_GLU}

# install library
target.path = $${LIB_DIR}
   
# "make install" configuration options
INSTALLS += include_lib3ds include_glext include_quazip include_glc_maths include_glc_io
INSTALLS += include_glc_scengraph include_glc_geometry include_glc_shading include_glc_viewport
INSTALLS += include_glc_3dwidget include_glc_glu

INSTALLS += target
INSTALLS +=include

OTHER_FILES += \
    qtc_packaging/debian_harmattan/rules \
    qtc_packaging/debian_harmattan/README \
    qtc_packaging/debian_harmattan/copyright \
    qtc_packaging/debian_harmattan/control \
    qtc_packaging/debian_harmattan/compat \
    qtc_packaging/debian_harmattan/changelog

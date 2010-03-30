TEMPLATE = lib
TARGET = GLC_lib
DEFINES += GLC_LIB_LIBRARY
include(../../openpilotgcslibrary.pri)

# GLC_lib qmake configuration
TEMPLATE = lib
QT += opengl \
    core

CONFIG+= x86
CONFIG += exceptions \
    release \
    warn_on
VERSION = 1.2.0

unix:OBJECTS_DIR = .
unix:MOC_DIR = .
unix:UI_DIR = .

DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += ./zlib

# Input					
HEADERS_QUAZIP +=	quazip/crypt.h \
					quazip/ioapi.h \
					quazip/quazip.h \
					quazip/quazipfile.h \
					quazip/quazipfileinfo.h \
					quazip/quazipnewinfo.h \
					quazip/unzip.h \
					quazip/zip.h

HEADERS_LIB3DS += lib3ds/atmosphere.h \
           lib3ds/background.h \
           lib3ds/camera.h \
           lib3ds/chunk.h \
           lib3ds/chunktable.h \
           lib3ds/ease.h \
           lib3ds/file.h \
           lib3ds/io.h \
           lib3ds/light.h \
           lib3ds/material.h \
           lib3ds/matrix.h \
           lib3ds/mesh.h \
           lib3ds/node.h \
           lib3ds/quat.h \
           lib3ds/shadow.h \
           lib3ds/tcb.h \
           lib3ds/tracks.h \
           lib3ds/types.h \
           lib3ds/vector.h \
           lib3ds/viewport.h
           
HEADERS_GLEXT += glext/glext.h

HEADERS_GLC_MATHS += 	maths/glc_utils_maths.h \
						maths/glc_vector2d.h \
						maths/glc_vector2df.h \
						maths/glc_vector3d.h \
						maths/glc_vector3df.h \
						maths/glc_vector4d.h \
						maths/glc_matrix4x4.h \
						maths/glc_interpolator.h \
						maths/glc_distance.h
						
HEADERS_GLC_IO +=		io/glc_objmtlloader.h \
						io/glc_objtoworld.h \
						io/glc_stltoworld.h \
						io/glc_offtoworld.h \
						io/glc_3dstoworld.h \
						io/glc_3dxmltoworld.h \
						io/glc_colladatoworld.h

HEADERS_GLC_SCENEGRAPH +=	sceneGraph/glc_3dviewcollection.h \
							sceneGraph/glc_3dviewinstance.h \
							sceneGraph/glc_structreference.h \
							sceneGraph/glc_structinstance.h \
							sceneGraph/glc_structoccurence.h \
							sceneGraph/glc_world.h \
							sceneGraph/glc_attributes.h \
							sceneGraph/glc_worldhandle.h
							
HEADERS_GLC_GEOMETRY +=		geometry/glc_vbogeom.h \
							geometry/glc_circle.h \
							geometry/glc_cylinder.h \
							geometry/glc_point.h \
							geometry/glc_box.h \
           					geometry/glc_geomtools.h \
							geometry/glc_geomengine.h \
							geometry/glc_simplegeomengine.h \
							geometry/glc_extendedgeomengine.h \
							geometry/glc_primitivegroup.h \
							geometry/glc_extendedmesh.h \
							geometry/glc_enginelod.h \
							geometry/glc_rectangle.h \
							geometry/glc_line.h \
							geometry/glc_rep.h \
							geometry/glc_3drep.h \
							geometry/glc_pointsprite.h

HEADERS_GLC_SHADING +=	shading/glc_material.h \						
						shading/glc_texture.h \
						shading/glc_shader.h \
						shading/glc_selectionmaterial.h \
						shading/glc_light.h
						
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
						viewport/glc_turntablemover.h


HEADERS_GLC += glc_enum.h \
           glc_object.h \
           glc_factory.h \
           glc_boundingbox.h \
           glc_exception.h \
           glc_openglexception.h \
           glc_fileformatexception.h \
           glc_ext.h \
           glc_state.h
           
HEADERS_PQP +=		PQP/PQP_Compile.h \
					PQP/TriDist.h \
					PQP/Tri.h \
					PQP/RectDist.h \
					PQP/PQP.h \
					PQP/PQP_Internal.h \
					PQP/OBB_Disjoint.h \
					PQP/MatVec.h \
					PQP/GetTime.h \
					PQP/BVTQ.h \
					PQP/BV.h \
					PQP/build.h

HEADERS += $${HEADERS_QUAZIP} $${HEADERS_LIB3DS} $${HEADERS_GLC_MATHS} $${HEADERS_GLC_IO}
HEADERS += $${HEADERS_GLC} $${HEADERS_GLEXT} $${HEADERS_GLC_SCENEGRAPH} $${HEADERS_GLC_GEOMETRY}
HEADERS += $${HEADERS_GLC_SHADING} $${HEADERS_GLC_VIEWPORT} $${HEADERS_PQP}
		   
SOURCES += quazip/ioapi.c \
           quazip/quazip.cpp \
           quazip/quazipfile.cpp \
           quazip/quazipnewinfo.cpp \
           quazip/unzip.c \
           quazip/zip.c

SOURCES += lib3ds/atmosphere.c \
           lib3ds/background.c \
           lib3ds/camera.c \
           lib3ds/chunk.c \
           lib3ds/ease.c \
           lib3ds/file.c \
           lib3ds/io.c \
           lib3ds/light.c \
           lib3ds/material.c \
           lib3ds/matrix.c \
           lib3ds/mesh.c \
           lib3ds/node.c \
           lib3ds/quat.c \
           lib3ds/shadow.c \
           lib3ds/tcb.c \
           lib3ds/tracks.c \
           lib3ds/vector.c \
           lib3ds/viewport.c
   
SOURCES +=	maths/glc_vector4d.cpp \
			maths/glc_matrix4x4.cpp \
			maths/glc_interpolator.cpp \
			maths/glc_distance.cpp

SOURCES +=	io/glc_objmtlloader.cpp \
			io/glc_objtoworld.cpp \
			io/glc_stltoworld.cpp \
			io/glc_offtoworld.cpp \
			io/glc_3dstoworld.cpp \
			io/glc_3dxmltoworld.cpp \
			io/glc_colladatoworld.cpp

SOURCES +=	sceneGraph/glc_3dviewcollection.cpp \
			sceneGraph/glc_3dviewinstance.cpp \
			sceneGraph/glc_structreference.cpp \
			sceneGraph/glc_structinstance.cpp \
			sceneGraph/glc_structoccurence.cpp \
			sceneGraph/glc_world.cpp \
			sceneGraph/glc_attributes.cpp \
			sceneGraph/glc_worldhandle.cpp

SOURCES +=	geometry/glc_vbogeom.cpp \
			geometry/glc_circle.cpp \
			geometry/glc_cylinder.cpp \
			geometry/glc_point.cpp \
			geometry/glc_box.cpp \
			geometry/glc_geomtools.cpp \
			geometry/glc_geomengine.cpp \
			geometry/glc_simplegeomengine.cpp \
			geometry/glc_extendedgeomengine.cpp \
			geometry/glc_primitivegroup.cpp \
			geometry/glc_extendedmesh.cpp \
			geometry/glc_enginelod.cpp \
			geometry/glc_rectangle.cpp \
			geometry/glc_line.cpp \
			geometry/glc_rep.cpp \
			geometry/glc_3drep.cpp \
			geometry/glc_pointsprite.cpp


SOURCES +=	shading/glc_material.cpp \
			shading/glc_texture.cpp \
			shading/glc_light.cpp \
			shading/glc_selectionmaterial.cpp \
			shading/glc_shader.cpp

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
			viewport/glc_turntablemover.cpp
		
SOURCES +=	glc_enum.cpp \
			glc_object.cpp \			
			glc_factory.cpp \
			glc_boundingbox.cpp \
			glc_exception.cpp \
			glc_openglexception.cpp \
			glc_fileformatexception.cpp \
			glc_ext.cpp \
			glc_state.cpp
           
SOURCES += PQP/TriDist.cpp \
		   PQP/PQP.cpp \
		   PQP/BV.cpp \
		   PQP/Build.cpp

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
    		   include/GLC_VboGeom \
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
    		   include/GLC_Point4d \
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
    		   include/GLC_Attribute \
    		   include/GLC_Rectangle \
    		   include/GLC_ExtendedMesh \
    		   include/GLC_StructOccurence \
    		   include/GLC_StructInstance \
    		   include/GLC_StructReference \
    		   include/GLC_Distance \
    		   include/GLC_Line \
    		   include/GLC_Rep \
    		   include/GLC_3DRep \
    		   include/GLC_PointSprite
    		   
    			   
# Linux install configuration
unix { 
    # Location of HEADERS and library
    LIB_DIR = ./install/lib
    INCLUDE_DIR = ./install/include
    # Adds a -P to preserve link
	QMAKE_COPY_FILE = $${QMAKE_COPY_FILE} -P
	include.path = $${INCLUDE_DIR}/GLC_lib
	include_lib3ds.path = $${INCLUDE_DIR}/GLC_lib/lib3ds
	include_glext.path = $${INCLUDE_DIR}/GLC_lib/glext
	include_quazip.path = $${INCLUDE_DIR}/GLC_lib/quazip
	include_pqp.path = $${INCLUDE_DIR}/GLC_lib/PQP
	include_glc_maths.path = $${INCLUDE_DIR}/GLC_lib/maths
	include_glc_io.path = $${INCLUDE_DIR}/GLC_lib/io
	include_glc_scengraph.path = $${INCLUDE_DIR}/GLC_lib/sceneGraph
	include_glc_geometry.path = $${INCLUDE_DIR}/GLC_lib/geometry
	include_glc_shading.path = $${INCLUDE_DIR}/GLC_lib/shading
	include_glc_viewport.path = $${INCLUDE_DIR}/GLC_lib/viewport
}

# Windows Install configuration
win32 { 
    # Location of HEADERS and library
    LIB_DIR = C:\GLC_lib\lib
    INCLUDE_DIR = C:\GLC_lib\include
    include.path = $${INCLUDE_DIR}
    include_lib3ds.path = $${INCLUDE_DIR}/lib3ds
    include_glext.path = $${INCLUDE_DIR}/glext
    include_quazip.path = $${INCLUDE_DIR}/quazip
    include_pqp.path = $${INCLUDE_DIR}/PQP
    include_glc_maths.path = $${INCLUDE_DIR}/maths
    include_glc_io.path = $${INCLUDE_DIR}/io
    include_glc_scengraph.path = $${INCLUDE_DIR}/sceneGraph
    include_glc_geometry.path = $${INCLUDE_DIR}/geometry
    include_glc_shading.path = $${INCLUDE_DIR}/shading
    include_glc_viewport.path = $${INCLUDE_DIR}/viewport
}    

include.files = $${HEADERS_GLC} $${HEADERS_INST}
include_lib3ds.files = $${HEADERS_LIB3DS}
include_glext.files =$${HEADERS_GLEXT}
include_quazip.files = $${HEADERS_QUAZIP}
include_pqp.files = $${HEADERS_PQP}
include_glc_maths.files= $${HEADERS_GLC_MATHS}
include_glc_io.files= $${HEADERS_GLC_IO}
include_glc_scengraph.files= $${HEADERS_GLC_SCENEGRAPH}
include_glc_geometry.files= $${HEADERS_GLC_GEOMETRY}
include_glc_shading.files = $${HEADERS_GLC_SHADING}
include_glc_viewport.files = $${HEADERS_GLC_VIEWPORT}

# install library
target.path = $${LIB_DIR}
   
# "make install" configuration options
INSTALLS += include_lib3ds include_glext include_quazip include_glc_maths include_glc_io
INSTALLS += include_glc_scengraph include_glc_geometry include_glc_shading include_glc_viewport
INSTALLS += include_pqp

INSTALLS += target
INSTALLS +=include


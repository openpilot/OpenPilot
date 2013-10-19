/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2013 Laurent Ribon (laumaya@users.sourceforge.net)
 http://glc-lib.sourceforge.net

 GLC-lib is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 GLC-lib is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with GLC-lib; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*****************************************************************************/

//! \file glc_extrudedmesh.cpp Implementation of the GLC_ExtrudedMesh class.

#include <QtDebug>

#include "glc_extrudedmesh.h"
#include "../maths/glc_plane.h"
#include "../maths/glc_geomtools.h"

// Class chunk id
quint32 GLC_ExtrudedMesh::m_ChunkId= 0xA712;

GLC_ExtrudedMesh::GLC_ExtrudedMesh(const QList<GLC_Point3d> &points, const GLC_Vector3d &dir, double lenght)
    :GLC_Mesh()
    , m_Points(points)
    , m_ExtrusionVector(GLC_Vector3d(dir).normalize())
    , m_ExtrusionLenght(lenght)
    , m_GivenFaceNormal()
{
    Q_ASSERT(m_Points.size() > 2);
    Q_ASSERT(pointsLieOnAPlane());
    Q_ASSERT(!glc::compare(m_Points.first(), m_Points.last(), glc::EPSILON));

    computeGivenFaceNormal();
    createMeshAndWire();
}

GLC_ExtrudedMesh::GLC_ExtrudedMesh(const GLC_ExtrudedMesh &other)
    :GLC_Mesh(other)
    , m_Points(other.m_Points)
    , m_ExtrusionVector(other.m_ExtrusionVector)
    , m_ExtrusionLenght(other.m_ExtrusionLenght)
    , m_GivenFaceNormal(other.m_GivenFaceNormal)
{
    if (GLC_Mesh::isEmpty())
    {
        createMeshAndWire();
    }
}

GLC_ExtrudedMesh::~GLC_ExtrudedMesh()
{

}

quint32 GLC_ExtrudedMesh::chunckID()
{
    return m_ChunkId;
}

GLC_Geometry *GLC_ExtrudedMesh::clone() const
{
    return new GLC_ExtrudedMesh(*this);
}

const GLC_BoundingBox &GLC_ExtrudedMesh::boundingBox()
{
    if (GLC_Mesh::isEmpty())
    {
        createMeshAndWire();
    }
    return GLC_Mesh::boundingBox();
}

GLC_ExtrudedMesh &GLC_ExtrudedMesh::operator =(const GLC_ExtrudedMesh &other)
{
    if (this != &other)
    {
        GLC_Mesh::operator =(other);
        m_Points= other.m_Points;
        m_ExtrusionVector= other.m_ExtrusionVector;
        m_ExtrusionLenght= other.m_ExtrusionLenght;
        m_GivenFaceNormal= other.m_GivenFaceNormal;
    }

    return *this;
}

void GLC_ExtrudedMesh::setGeometry(const QList<GLC_Point3d> &points, const GLC_Vector3d &extrudedVector, double lenght)
{
    m_Points= points;
    m_ExtrusionVector= extrudedVector;
    m_ExtrusionLenght= lenght;

    GLC_Mesh::clearMeshWireAndBoundingBox();
    createMeshAndWire();
}

void GLC_ExtrudedMesh::setPoints(const QList<GLC_Point3d> &points)
{
    m_Points= points;

    GLC_Mesh::clearMeshWireAndBoundingBox();
    createMeshAndWire();
}

void GLC_ExtrudedMesh::setExtrudedVector(const GLC_Vector3d &extrudedVector)
{
    m_ExtrusionVector= extrudedVector;

    GLC_Mesh::clearMeshWireAndBoundingBox();
    createMeshAndWire();
}

void GLC_ExtrudedMesh::setExtrudedLenght(double lenght)
{
    m_ExtrusionLenght= lenght;

    GLC_Mesh::clearMeshWireAndBoundingBox();
    createMeshAndWire();
}

void GLC_ExtrudedMesh::createMeshAndWire()
{
    Q_ASSERT(GLC_Mesh::isEmpty());
    Q_ASSERT(m_WireData.isEmpty());

    createMesh();
    createWire();

    finish();
}

void GLC_ExtrudedMesh::createMesh()
{
    // Bulk data
    GLfloatVector vertices;
    GLfloatVector normals;
    GLfloatVector texels= baseFaceTexels() + createdFaceTexels() + basedOutlineFacesTexels() + createdOutlineFacesTexels();
    GLC_Mesh::addTexels(texels);

    // Set the material to use
    GLC_Material* pMaterial;
    if (hasMaterial()) pMaterial= this->firstMaterial();
    else pMaterial= new GLC_Material();

    {
        // Given face (Face 1)
        GLfloatVector face1Vertices= baseFaceVertices();
        GLfloatVector face1Normals= baseFaceNormals();
        IndexList face1Index;
        const GLuint count= m_Points.count();
        for (GLuint i= 0; i < (count + 1); ++i)
        {
            face1Index.append(i % count);
        }
        vertices+= face1Vertices;
        normals+= face1Normals;
        Q_ASSERT(vertices.size() == normals.size());
        glc::triangulatePolygon(&face1Index, vertices.toList());
        addTriangles(pMaterial, face1Index);
    }

    {
        // Created face (Face2)
        GLfloatVector face2Vertices= createdFaceVertices();
        GLfloatVector face2Normals= createdFaceNormals();
        IndexList face2Index;
        const int offset= vertices.size() / 3;
        const GLuint count= m_Points.count();
        for (GLuint i= 0; i < (count + 1); ++i)
        {
            face2Index.append(offset + (i % count));
        }
        vertices+= face2Vertices;
        normals+= face2Normals;
        Q_ASSERT(vertices.size() == normals.size());
        glc::triangulatePolygon(&face2Index, vertices.toList());
        addTriangles(pMaterial, face2Index);
    }


    {
        // Add outline faces
        const GLuint count= m_Points.count();
        GLuint offset1= vertices.size() / 3;
        GLfloatVector facesNormals= baseOutlineNormals();
        GLuint indexLenght= (facesNormals.size() / 3);
        GLuint offset2= offset1 + indexLenght;
        facesNormals= facesNormals + createdOutlineNormals();
        GLfloatVector facesVertices= baseOutlineFacesVertices() + createdOutlineFacesVertices();
        vertices+= facesVertices;
        normals+= facesNormals;

        for (GLuint face= 0; face < count; ++face)
        {
            IndexList faceIndex;
            GLuint startIndex1= offset1 + (face * 2);
            GLuint startIndex2= offset2 + (indexLenght -1) - (face * 2);
            faceIndex << startIndex1 << startIndex2 << (startIndex1 + 1) << (startIndex2 - 1);
            addTrianglesStrip(pMaterial, faceIndex);
        }
    }

    // Add bulk data in to the mesh
    GLC_Mesh::addVertice(vertices);
    GLC_Mesh::addNormals(normals);

}

void GLC_ExtrudedMesh::createWire()
{
    GLfloatVector baseFaceWire= baseFaceVertices();
    baseFaceWire << baseFaceWire.at(0) << baseFaceWire.at(1) << baseFaceWire.at(2);
    GLC_Geometry::addVerticeGroup(baseFaceWire);

    GLfloatVector createdFaceWire= createdFaceVertices();
    createdFaceWire << createdFaceWire.at(0) << createdFaceWire.at(1) << createdFaceWire.at(2);
    GLC_Geometry::addVerticeGroup(createdFaceWire);

    // Outline edges
    const int count= m_Points.count();
    for (int i= 0; i < count; ++i)
    {
        GLfloatVector edge(6); // the edge is a line
        edge[0]= baseFaceWire.at(i * 3);
        edge[1]= baseFaceWire.at(i * 3 + 1);
        edge[2]= baseFaceWire.at(i * 3 + 2);

        edge[3]= createdFaceWire.at(((count - i) % count) * 3);
        edge[4]= createdFaceWire.at(((count - i) % count) * 3 + 1);
        edge[5]= createdFaceWire.at(((count - i) % count) * 3 + 2);

        GLC_Geometry::addVerticeGroup(edge);
    }
}

void GLC_ExtrudedMesh::computeGivenFaceNormal()
{
    Q_ASSERT(m_Points.count() > 2);

    const GLC_Vector3d edge1(GLC_Vector3d(m_Points.at(1) - m_Points.at(0)).normalize());
    const GLC_Vector3d edge2(GLC_Vector3d(m_Points.at(2) - m_Points.at(1)).normalize());

    m_GivenFaceNormal= edge1 ^ edge2;
}

bool GLC_ExtrudedMesh::pointsLieOnAPlane() const
{
    bool subject= true;
    const int count= m_Points.count();
    Q_ASSERT(count > 2);
    if (count > 3)
    {
        // Create a plane on the first 3 points
        GLC_Plane plane(m_Points.at(0), m_Points.at(1), m_Points.at(2));

        // Check if others points lie on the plane
        for (int i= 3; i < count; ++i)
        {
            subject= plane.lieOnThisPlane(m_Points.at(i));
            if (!subject) break;
        }
    }

    return subject;
}

GLfloatVector GLC_ExtrudedMesh::baseOutlineNormals() const
{
    const int count= m_Points.count();
    GLfloatVector subject(count * 6); // 2 normals by vertex and 3 components pear normal => 6
    for (int i= 0; i < count; ++i)
    {
        GLC_Vector3d vect(m_Points.at((i + 1) % count) - m_Points.at(i));
        GLC_Vector3d normal= GLC_Vector3d(m_ExtrusionVector ^ vect).normalize();

        // First segment point
        subject[(6 * i)]= static_cast<GLfloat>(normal.x());
        subject[(6 * i) + 1]= static_cast<GLfloat>(normal.y());
        subject[(6 * i) + 2]= static_cast<GLfloat>(normal.z());

        // Second segment point
        subject[(6 * i) + 3]= static_cast<GLfloat>(normal.x());
        subject[(6 * i) + 4]= static_cast<GLfloat>(normal.y());
        subject[(6 * i) + 5]= static_cast<GLfloat>(normal.z());
    }

    return subject;
}

GLfloatVector GLC_ExtrudedMesh::createdOutlineNormals() const
{
    const int count= m_Points.count();
    GLfloatVector subject(count * 6); // 2 normals by vertex and 3 components pear normal => 6
    int index= 0.0;
    for (int i= count; i > 0; --i)
    {
        GLC_Vector3d vect(m_Points.at(i % count) - m_Points.at(i - 1));
        GLC_Vector3d normal= GLC_Vector3d(m_ExtrusionVector ^ vect).normalize();

        // First segment point
        subject[(6 * index)]= static_cast<GLfloat>(normal.x());
        subject[(6 * index) + 1]= static_cast<GLfloat>(normal.y());
        subject[(6 * index) + 2]= static_cast<GLfloat>(normal.z());

        // Second segment point
        subject[(6 * index) + 3]= static_cast<GLfloat>(normal.x());
        subject[(6 * index) + 4]= static_cast<GLfloat>(normal.y());
        subject[(6 * index) + 5]= static_cast<GLfloat>(normal.z());
        ++index;
    }

    return subject;
}

GLfloatVector GLC_ExtrudedMesh::baseFaceVertices() const
{
    const int count= m_Points.count();
    GLfloatVector subject(count * 3); // 3 components pear vertice
    for (int i= 0; i < count; ++i)
    {
        GLC_Point3d point= m_Points.at(i);
        subject[(3 * i)]= static_cast<GLfloat>(point.x());
        subject[(3 * i) + 1]= static_cast<GLfloat>(point.y());
        subject[(3 * i) + 2]= static_cast<GLfloat>(point.z());
    }

    return subject;
}

GLfloatVector GLC_ExtrudedMesh::baseFaceTexels() const
{
    const int count= m_Points.count();
    GLfloatVector subject(count * 2);

    QList<GLC_Point2d> baseFace2DPolygon= glc::polygonIn2d(m_Points);
    QList<GLC_Point2d> normalizePolygon= glc::normalyzePolygon(baseFace2DPolygon);

    for (int i= 0; i < count; ++i)
    {
        GLC_Point2d texel= normalizePolygon.at(i);
        subject[i * 2]= static_cast<GLfloat>(texel.x());
        subject[i * 2 + 1]= static_cast<GLfloat>(texel.y());
    }

    return subject;
}

GLfloatVector GLC_ExtrudedMesh::baseOutlineFacesVertices() const
{
    const int count= m_Points.count();
    GLfloatVector subject(count * 6); // 3 components pear vertice * 2
    for (int i= 0; i < count; ++i)
    {
        GLC_Point3d point1= m_Points.at(i);
        subject[(6 * i)]= static_cast<GLfloat>(point1.x());
        subject[(6 * i) + 1]= static_cast<GLfloat>(point1.y());
        subject[(6 * i) + 2]= static_cast<GLfloat>(point1.z());

        GLC_Point3d point2= m_Points.at((i + 1) % count);
        subject[(6 * i) + 3]= static_cast<GLfloat>(point2.x());
        subject[(6 * i) + 4]= static_cast<GLfloat>(point2.y());
        subject[(6 * i) + 5]= static_cast<GLfloat>(point2.z());
    }

    return subject;
}

GLfloatVector GLC_ExtrudedMesh::basedOutlineFacesTexels() const
{
    const int count= m_Points.count();
    GLfloatVector subject(count * 4);

    QList<GLC_Point2d> baseFace2DPolygon= glc::polygonIn2d(m_Points);
    QList<GLC_Point2d> normalizePolygon= glc::normalyzePolygon(baseFace2DPolygon);

    for (int i= 0; i < count; ++i)
    {
        GLC_Point2d texel1= normalizePolygon.at(i);
        subject[i * 4]= static_cast<GLfloat>(texel1.x());
        subject[i * 4 + 1]= static_cast<GLfloat>(texel1.y());

        GLC_Point2d texel2= normalizePolygon.at((i + 1) % count);
        subject[i * 4 + 2]= static_cast<GLfloat>(texel2.x());
        subject[i * 4 + 3]= static_cast<GLfloat>(texel2.y());
    }

    return subject;
}

GLfloatVector GLC_ExtrudedMesh::baseFaceNormals() const
{
    const int count= m_Points.count();
    GLfloatVector subject(count * 3); // 3 components pear normal
    for (int i= 0; i < count; ++i)
    {
        subject[(3 * i)]= static_cast<GLfloat>(m_GivenFaceNormal.x());
        subject[(3 * i) + 1]= static_cast<GLfloat>(m_GivenFaceNormal.y());
        subject[(3 * i) + 2]= static_cast<GLfloat>(m_GivenFaceNormal.z());
    }

    return subject;
}

QList<GLC_Point3d> GLC_ExtrudedMesh::createdFacePoints() const
{
    // The order is inverted to preserve face counterclockwise order
    const GLC_Vector3d offset(m_ExtrusionVector * m_ExtrusionLenght);
    const int count= m_Points.count();
    QList<GLC_Point3d> subject;
    for (int i= 0; i < count; ++i)
    {
        subject.append(m_Points.at((count -i) % count) + offset);
    }

    return subject;
}

GLfloatVector GLC_ExtrudedMesh::createdFaceNormals() const
{
    const GLC_Vector3d normal= m_GivenFaceNormal.inverted();

    const int count= m_Points.count();
    GLfloatVector subject(count * 3); // 3 components pear normal
    for (int i= 0; i < count; ++i)
    {
        subject[(3 * i)]= static_cast<GLfloat>(normal.x());
        subject[(3 * i) + 1]= static_cast<GLfloat>(normal.y());
        subject[(3 * i) + 2]= static_cast<GLfloat>(normal.z());
    }

    return subject;
}

GLfloatVector GLC_ExtrudedMesh::createdFaceVertices() const
{
    QList<GLC_Point3d> points= createdFacePoints();

    const int count= points.count();
    GLfloatVector subject(count * 3); // 3 components pear normal
    for (int i= 0; i < count; ++i)
    {
        GLC_Point3d point= points.at(i);
        subject[(3 * i)]= static_cast<GLfloat>(point.x());
        subject[(3 * i) + 1]= static_cast<GLfloat>(point.y());
        subject[(3 * i) + 2]= static_cast<GLfloat>(point.z());
    }

    return subject;
}

GLfloatVector GLC_ExtrudedMesh::createdFaceTexels() const
{
    const int count= m_Points.count();
    GLfloatVector subject(count * 2);

    QList<GLC_Point2d> baseFace2DPolygon= glc::polygonIn2d(createdFacePoints());
    QList<GLC_Point2d> normalizePolygon= glc::normalyzePolygon(baseFace2DPolygon);

    for (int i= 0; i < count; ++i)
    {
        GLC_Point2d texel= normalizePolygon.at(i);
        subject[i * 2]= static_cast<GLfloat>(texel.x());
        subject[i * 2 + 1]= static_cast<GLfloat>(texel.y());
    }

    return subject;

}

GLfloatVector GLC_ExtrudedMesh::createdOutlineFacesVertices() const
{
    QList<GLC_Point3d> points= createdFacePoints();
    const int count= points.count();
    GLfloatVector subject(count * 6); // 3 components pear vertice * 2
    for (int i= 0; i < count; ++i)
    {
        GLC_Point3d point1= points.at(i);
        subject[(6 * i)]= static_cast<GLfloat>(point1.x());
        subject[(6 * i) + 1]= static_cast<GLfloat>(point1.y());
        subject[(6 * i) + 2]= static_cast<GLfloat>(point1.z());

        GLC_Point3d point2= points.at((i + 1) % count);
        subject[(6 * i) + 3]= static_cast<GLfloat>(point2.x());
        subject[(6 * i) + 4]= static_cast<GLfloat>(point2.y());
        subject[(6 * i) + 5]= static_cast<GLfloat>(point2.z());
    }

    return subject;

}

GLfloatVector GLC_ExtrudedMesh::createdOutlineFacesTexels() const
{
    const int count= m_Points.count();
    GLfloatVector subject(count * 4);

    QList<GLC_Point2d> baseFace2DPolygon= glc::polygonIn2d(createdFacePoints());
    QList<GLC_Point2d> normalizePolygon= glc::normalyzePolygon(baseFace2DPolygon);

    for (int i= 0; i < count; ++i)
    {
        GLC_Point2d texel1= normalizePolygon.at(i);
        subject[i * 4]= static_cast<GLfloat>(texel1.x());
        subject[i * 4 + 1]= static_cast<GLfloat>(texel1.y());

        GLC_Point2d texel2= normalizePolygon.at((i + 1) % count);
        subject[i * 4 + 2]= static_cast<GLfloat>(texel2.x());
        subject[i * 4 + 3]= static_cast<GLfloat>(texel2.y());
    }

    return subject;
}

void GLC_ExtrudedMesh::glDraw(const GLC_RenderProperties& renderProperties)
{
    if (GLC_Mesh::isEmpty())
    {
        createMeshAndWire();
    }

    GLC_Mesh::glDraw(renderProperties);
}

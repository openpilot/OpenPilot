/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2008 Laurent Ribon (laumaya@users.sourceforge.net)
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

//! \file glc_renderer.h interface for the GLC_Renderer class.

#ifndef GLC_RENDERER_H_
#define GLC_RENDERER_H_

#include <QHash>

#include "glc_renderproperties.h"

#include "../glc_config.h"

class GLC_3DViewCollection;

//////////////////////////////////////////////////////////////////////
//! \class GLC_Renderer
/*! \brief GLC_Renderer : Is used to store and retrieve overload rendering properties*/

/*! An GLC_Renderer is attached to a GLC_3DViewCollection \n
 * The renderer is used to render a scene in a specific way.*/
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_Renderer
{
public:
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////

	GLC_Renderer();
	GLC_Renderer(GLC_3DViewCollection* pCollection);
	GLC_Renderer(const GLC_Renderer& other);
	virtual ~GLC_Renderer();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return true if this renderer has an attached collection
	inline bool hasCollection() const
	{return m_pCollection != NULL;}

	//! Return the 3DView collection attached to this renderer
	inline GLC_3DViewCollection* collection() const
	{return m_pCollection;}

	//! Return true if the  renderProperties of the given instance id is available
	bool instanceRenderPropertiesIsAvailable(GLC_uint id) const;

	//! Return the renderProperties of the given instance id
	const GLC_RenderProperties& renderPropertiesOfInstance(GLC_uint id) const;

    //! Return true if this renderer is current
    inline bool isCurrent() const
    {return m_IsCurrent;}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Clear the content of this render
	void clear();

	//! Assignement operator
	GLC_Renderer& operator=(const GLC_Renderer& other);

	//! Set the collection to use
	void setCollection(GLC_3DViewCollection* pCollection);

	//! Set this renderer the current renderer
	/*! Apply stored renderProperties to the attached collection*/
	void setCurrent();

	//! Unset this rendere the current rendere
	/*! Save the render properties of all instance of the scene*/
	void unSetCurrent();

	//! Add the renderProperties of the given instance id
	void addRenderPropertiesOfInstanceId(GLC_uint id);
//@}

//////////////////////////////////////////////////////////////////////
// Private services fonction
//////////////////////////////////////////////////////////////////////
private:

//////////////////////////////////////////////////////////////////////
// Private Members
//////////////////////////////////////////////////////////////////////
private:
	//! The 3DView collection attached to this renderer
	GLC_3DViewCollection* m_pCollection;

	QHash<GLC_uint, GLC_RenderProperties> m_IdToRenderProperties;

    bool m_IsCurrent;

};

#endif /* GLC_RENDERER_H_ */

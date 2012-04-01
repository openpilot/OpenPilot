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
//! \file glc_contextmanager.h interface for the GLC_ContextManager class.

#ifndef GLC_CONTEXTMANAGER_H_
#define GLC_CONTEXTMANAGER_H_

#include <QSet>

#include "glc_config.h"


class GLC_Context;

//////////////////////////////////////////////////////////////////////
//! \class GLC_ContextManager
/*! \brief GLC_ContextManager : Manager a set of GLC_Context*/
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_ContextManager
{
private:
	GLC_ContextManager();
public:
	virtual ~GLC_ContextManager();

//@}
//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the unique instance of context manager
	static GLC_ContextManager* instance();

	//! Return the current context
	GLC_Context* currentContext() const;

	//! Return true if there is a current context
	inline bool currentContextExists() const
	{return (NULL != m_pCurrentContext);}

	//! Return true if this manager has context
	inline bool hasContext() const
	{return !m_SetOfContext.isEmpty();}

//@}
//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Add the given context
	void addContext(GLC_Context* pContext);

	//! Remove the given context
	void remove(GLC_Context* pContext);

	//! Set the current the given context
	void setCurrent(GLC_Context* pContext);

//////////////////////////////////////////////////////////////////////
/*! \name Private services Functions*/
//@{
//////////////////////////////////////////////////////////////////////
private:
//@{

//@}


//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:
	//! The unique instance of the context manager
	static GLC_ContextManager* m_pContextManager;

	//! The current context
	GLC_Context* m_pCurrentContext;

	//! The Set of context to manage
	QSet<GLC_Context*> m_SetOfContext;
};

#endif /* GLC_CONTEXTMANAGER_H_ */

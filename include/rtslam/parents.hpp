/*
 * parents.hpp
 *
 *  Created on: Apr 19, 2010
 *      Author: nmansard
 */

#ifndef __rtslam_parents_H__
#define __rtslam_parents_H__

#include <list>
#include <vector>
#include <iostream>
#include <boost/smart_ptr.hpp>
#include <boost/bind.hpp>

#include "rtslam/rtSlam.hpp"


namespace jafar {
	/**
	 * Namespace rtslam for real-time slam module.
	 * \ingroup rtslam
	 */
	namespace rtslam {



/* --- PARENT --------------------------------------------------------------- */
/* --- PARENT --------------------------------------------------------------- */
/* --- PARENT --------------------------------------------------------------- */

/* Use this generic class by inhereting from it in the parent class.
 * The child class can be known only partially:
 * (ie class Child; class Parent : public ParentOf<Child> {...}; class Child { ... };)
 */
template<class Child>
class ParentOf {
public:
	typedef boost::shared_ptr<Child> Child_ptr;
	typedef std::list<Child_ptr> ChildList;

public:
	ChildList childList;

public:
	~ParentOf(void) {
//		std::cout << "Destroy Parent. " << std::endl;
	}

	void registerChild(const Child_ptr & ptr) {
		childList.push_back(ptr);
	}
	void unregisterChild(const Child_ptr & ptr) {
//		remove(childList.begin(), childList.end(), ptr);
		childList.remove(ptr);
//		childList.pop_back();
	}

	void display(std::ostream& os) const {
		os << "PTR LIST [ ";
		for (typename ChildList::const_iterator iter = childList.begin(); iter
				!= childList.end(); ++iter) {
			os << iter->get() << " ";
		}
		os << " ]";
	}
};

/* The Macro define a type to access to ParentOf::ChildList, with name <TYPE>List,
 * plus two accessors (const and non-const) to access directly to the list
 * of children, with name <ACCESS>List().
 */
#define ENABLE_ACCESS_TO_CHILDREN(Child,typeName,accessName)    \
  public: typedef ParentOf<Child>::ChildList typeName##List; \
  typeName##List & accessName##List( void )           \
  {  return ParentOf<Child>::childList; }     \
  const typeName##List & accessName##List( void ) const           \
  {  return ParentOf<Child>::childList; }     \


/* --- CHILD ---------------------------------------------------------------- */
/* --- CHILD ---------------------------------------------------------------- */
/* --- CHILD ---------------------------------------------------------------- */

/* Use this generic class by inhereting from it in the child class.
 * The parent class can be known only partially:
 * (ie class Parent; class Child : public ChildOf<Parent> {...}; class Parent { ... };)
 */
template<class Parent>
class ChildOf {
public:
	typedef boost::weak_ptr<Parent> Parent_wptr;
	typedef boost::shared_ptr<Parent> Parent_ptr;

private:
	Parent_wptr parent_wptr;

public:

	/** Unregister at destruction. */
	~ChildOf(void) {
		unlinkFromParent();
	}

	void linkToParent(const boost::shared_ptr<Parent> & ptr) {
		parent_wptr = ptr;
	}
	/** Remove the link to the Parent, and unregister from parent list. */
	void unlinkFromParent(void) {
		parent_wptr.reset();
	}

	/** Access the shared pointer to parent. Throw if parent has
	 * been destroyed. */
	Parent_ptr parentPtr(void) {
		Parent_ptr sptr = parent_wptr.lock();
		if (!sptr) {
			std::cerr << __FILE__ << ":" << __LINE__ << " ChildOf::parentPtr threw weak" << std::endl;
			throw "WEAK";
		}
		return sptr;
	}
	Parent& parent(void) {
		Parent_ptr sptr = parent_wptr.lock();
		if (!sptr) {
			std::cerr << __FILE__ << ":" << __LINE__ << " ChildOf::parent threw weak" << std::endl;
			throw "WEAK";
		}
		return *sptr;
	}
	const Parent& parent(void) const {
		Parent_ptr sptr = parent_wptr.lock();
		if (!sptr) {
			std::cerr << __FILE__ << ":" << __LINE__ << " ChildOf::parent const threw weak" << std::endl;
			throw "WEAK";
		}
		return *sptr;
	}

	void display(std::ostream& os) const {
		os << this << "->" << parent_wptr.lock().get();
	}

};

/* Connect the link to the Parent, and register into parent.
 * To be defined as a public member of the child class. The name
 * of the Child class has to be specified explicitely.
 * PRE:
 *  - The class <Parent> should be known at the call of the macros.
 *  - The child class (where the macro is called) should have enable_shared_from_this.
 */
#define ENABLE_LINK_TO_FATHER(Parent,Child)                   \
		public: void linkToParent( boost::shared_ptr<Parent> ptr )          \
  {                                                           \
    ChildOf<Parent>::linkToParent(ptr);                       \
    ptr->ParentOf<Child>::registerChild(shared_from_this());  \
  }

/* Same as before, except that the function is defined with
 * name linkToParent<Parent> (ie, for Parent=Sen, linkToParentSen). */
#define ENABLE_LINK_TO_PARENT(Parent,name,Child)              \
		public: void linkToParent##name( boost::shared_ptr<Parent> ptr )  \
  {                                                           \
    ChildOf<Parent>::linkToParent(ptr);                       \
    ptr->ParentOf<Child>::registerChild(shared_from_this());  \
  }

/* Define three accessor function, one to the pointer, two on
 * the reference (const and non-const). The function are called
 * fatherPtr() and father() respectively.
 */
#define ENABLE_ACCESS_TO_FATHER(Parent)                       \
		public: boost::shared_ptr<Parent> father##Ptr( void )               \
  {  return ChildOf<Parent>::parentPtr(); }                   \
  Parent& father( void )                                      \
  {    return ChildOf<Parent>::parent();  }                   \
  const Parent& father( void ) const                          \
  {    return ChildOf<Parent>::parent();  }

/* Same as before, except that the function is given an
 * explicit name, to handle the case of multiple parent.
 */
#define ENABLE_ACCESS_TO_PARENT(Parent,accessName)            \
		public: boost::shared_ptr<Parent> accessName##Ptr( void )           \
  {  return ChildOf<Parent>::parentPtr(); }                   \
  Parent& accessName( void )                                  \
  {    return ChildOf<Parent>::parent();  }                   \
  const Parent& accessName( void ) const                      \
  {    return ChildOf<Parent>::parent();  }

/* The ENABLE_LINK_TO_PARENT could be modified to link in the other way.
 * However, it is not possible a priori to have both initialization enabled
 * at the same time. Work to be done if need.
 */

/* --- SPECIFIC CHILD ------------------------------------------------------- */
/* --- SPECIFIC CHILD ------------------------------------------------------- */
/* --- SPECIFIC CHILD ------------------------------------------------------- */

/* Use this generic class by inhereting from it in the child class.
 * The parent class can be known only partially:
 * (ie class Parent; class Child : public ChildOf<Parent> {...}; class Parent { ... };)
 */
template<class Parent>
class SpecificChildOf {
public:
	typedef boost::weak_ptr<Parent> Parent_wptr;
	typedef boost::shared_ptr<Parent> Parent_ptr;

public:
	Parent_wptr parent_wptr;

public:

	/** Unregister at destruction. */
	~SpecificChildOf(void) {
		unlinkFromParent();
	}

	template<class ParentAbstract>
	void linkToParentSpecific(const boost::shared_ptr<ParentAbstract> & ptr) {
		boost::shared_ptr<Parent> spec_ptr =
				SPTR_CAST<Parent>(ptr);
		parent_wptr = spec_ptr;
	}

	void linkToParentSpecific(const boost::shared_ptr<Parent> & ptr) {
		parent_wptr = ptr;
	}

	/** Remove the link to the Parent, and unregister from parent list. */
	void unlinkFromParent(void) {
		parent_wptr.reset();
	}

	/** Access the shared pointer to parent. Throw if parent has
	 * been destroyed. */
	Parent_ptr parentPtr(void) {
		Parent_ptr sptr = parent_wptr.lock();
		if (!sptr) {
			std::cerr << __FILE__ << ":" << __LINE__ << " SpecificChildOf::parentPtr threw weak" << std::endl;
			throw "WEAK";
		}
		return sptr;
	}
	Parent& parent(void) {
		Parent_ptr sptr = parent_wptr.lock();
		if (!sptr) {
			std::cerr << __FILE__ << ":" << __LINE__ << " SpecificChildOf::parent threw weak" << std::endl;
			throw "WEAK";
		}
		return *sptr;
	}
	const Parent& parent(void) const {
		Parent_ptr sptr = parent_wptr.lock();
		if (!sptr) {
			std::cerr << __FILE__ << ":" << __LINE__ << " SpecificChildOf::parent const threw weak" << std::endl;
			throw "WEAK";
		}
		return *sptr;
	}

	void display(std::ostream& os) const {
		os << this << "->" << parent_wptr.lock().get();
	}

};

/** Connect the link to the Parent, and register into parent.
 * To be defined as a public member of the child class.
 * PRE:
 *  - The class <Parent> should be known at the call of the macros.
 *  - The child class (where the macro is called) should have enable_shared_from_this.
 * The <Child> class is the real child, not the specific class (that is a
 * SpecificChild). For example, the ObservationAHP is not the Child,
 * but ObservationAbstract is.
 */
#define ENABLE_LINK_TO_SPECIFIC_FATHER(ParentGen,ParentSpec,Child)   \
		public: void linkToParent( const boost::shared_ptr<ParentGen>& ptr )       \
  {                                                                  \
    ChildOf<ParentGen>::linkToParent( ptr );                         \
    SpecificChildOf<ParentSpec>::linkToParentSpecific( ptr );        \
    ptr->ParentOf<Child>::registerChild(shared_from_this());         \
  } \
  void linkToParent( const boost::shared_ptr<ParentSpec>& ptr )      \
  {                                                                  \
    ChildOf<ParentGen>::linkToParent( ptr );                         \
    SpecificChildOf<ParentSpec>::linkToParentSpecific( ptr );        \
    ptr->ParentOf<Child>::registerChild(shared_from_this());         \
  }

#define ENABLE_LINK_TO_SPECIFIC_PARENT(ParentGen,ParentSpec,name,Child)         \
		public: void linkToParent##name( const boost::shared_ptr<ParentGen>& ptr )  \
  {                                                                             \
    ChildOf<ParentGen>::linkToParent( ptr );                                    \
    SpecificChildOf<ParentSpec>::linkToParentSpecific( ptr );                   \
    ptr->ParentOf<Child>::registerChild(shared_from_this());                    \
  }                                                                             \
  void linkToParent##name( const boost::shared_ptr<ParentSpec>& ptr )           \
  {                                                                             \
    ChildOf<ParentGen>::linkToParent( ptr );                                    \
    SpecificChildOf<ParentSpec>::linkToParentSpecific( ptr );                   \
    ptr->ParentOf<Child>::registerChild(shared_from_this());                    \
  }

/* Define three accessors (to the pointer and to the const/non-const reference)
 * with names <ACCESS>Ptr() and <ACCESS>().
 */
#define ENABLE_ACCESS_TO_SPECIFIC_PARENT(Parent,accessName)         \
		public: boost::shared_ptr<Parent> accessName##Ptr( void )       \
  {  return SpecificChildOf<Parent>::parentPtr(); }                 \
  Parent& accessName( void )                                        \
  {    return SpecificChildOf<Parent>::parent();  }                 \
  const Parent& accessName( void ) const                            \
  {    return SpecificChildOf<Parent>::parent();  }






/* --- WEAK PARENT --------------------------------------------------------- */
/* --- WEAK PARENT --------------------------------------------------------- */
/* --- WEAK PARENT --------------------------------------------------------- */

/* The weak-parent paradigm is similar to the parent one, except that
 * only weak-ptr are stored. This means that there is no "owner" relation
 * from the parent on the child (ie, the child is not destroyed when father dies
 * (good news for the child ;) ).
 *
 * Use this generic class by inhereting from it in the parent class.
 * The child class can be known only partially:
 * (ie class Child; class WeakParent : public WeakParentOf<Child> {...}; class Child { ... };)
 *
 * Use: the class just have to inherit (public) from WeakParentOf<Child>, with Child
 * a partially known class. For convenience, the macro ENABLE_ACCESS_TO_CHILDREN()
 * macro can be called inside the class for some extra accessor-aliases definition.
 * The child class has to inherit from the standard ChildOf<Parent> class, with
 * parent full defined. The weak-specific macros has to be used (see below).
 * As a new requierement, the UNREGISTER_FROM_WEAK_PARENT has to be called in the
 * destructor of the (weak)-child.
 */
template<class Child>
class WeakParentOf {
public:
  typedef boost::shared_ptr<Child> Child_ptr;
  typedef boost::weak_ptr<Child> Child_wptr;
  typedef std::vector<Child_wptr> ChildList;

protected: // Iterator

  /* iterator and const_iterator enables a direct use of  the list of child,
   * by defining a *iter that lock() the weak to a shared.
   * use for( iterator iter = childList().begin(); iter != childList().end(); ++iter ) ...
   */

  template< class basic_iterator>
  class generic_iterator
  {
  public:
    typedef Child_ptr Insider;

  protected:
    basic_iterator iter;
  private:
    generic_iterator( void ) {}
  public:
    generic_iterator( const  basic_iterator&  i ) : iter(i) {}
    generic_iterator& operator++ ( void ) { iter++; return *this; }
    generic_iterator& operator++ ( int ) { iter++; return *this; }
    friend bool operator== ( const generic_iterator & i1, const generic_iterator & i2 )
    { return i1.iter == i2.iter; }
    friend bool operator!= ( const generic_iterator & i1, const generic_iterator & i2 )
    { return i1.iter != i2.iter; }
    friend bool operator== ( const generic_iterator & i1, const  basic_iterator & i2 )
    { return i1.iter == i2; }
    friend bool operator!= ( const generic_iterator & i1, const  basic_iterator & i2 )
    { return i1.iter != i2; }
    friend bool operator== ( const  basic_iterator & i1, const generic_iterator & i2 )
    { return i1 == i2.iter; }
    friend bool operator!= ( const  basic_iterator & i1, const generic_iterator & i2 )
    { return i1 != i2.iter; }

    operator bool ( void )
    { return (! iter->expired()); }

    Insider operator* (void)
    {
      Insider ptr = iter->lock();
      if(! ptr )
	{
	  std::cerr << __FILE__ << ":" << __LINE__ << " WeakParentOf::operator* threw weak" << std::endl;
	  throw "WEAK";
	}
      return ptr;
    }
  };

  typedef typename WeakParentOf<Child>::ChildList ExChildList;
  typedef typename WeakParentOf<Child>::ChildList::iterator basic_iterator;
  typedef typename WeakParentOf<Child>::ChildList::const_iterator basic_const_iterator;

public:
  typedef generic_iterator<basic_iterator> iterator;
  typedef generic_iterator<basic_const_iterator> const_iterator;

public:
  ChildList childList;

public:
  ~WeakParentOf(void) {
//    std::cout << "Destroy Parent. " << std::endl;
  }

  void registerChild(const Child_ptr & ptr) {
    Child_wptr wptr = ptr;
    childList.push_back(wptr);
  }

  /* Predicate fonctor to compare a weak_ptr to a reference
   * shared_ptr. */
  struct WeakPtrComparison
  {
    Child_ptr refptr;
    WeakPtrComparison( const Child_ptr & ptr ) : refptr(ptr) {}
    bool operator() (const Child_wptr & wptr)
    {
      Child_ptr ptr = wptr.lock();
      return refptr == ptr;
    }
  };


  void unregisterChild(const Child_ptr & ptr)
  {
    childList.erase(remove_if(childList.begin(), childList.end(),
			      WeakPtrComparison(ptr)  ),
		    childList.end());
  }

  /* Remove all expired weak pointers from the list.
   * Use that in the destructor of the weak child (when the weak pointed object
   * is destroyed, the corresponding weak pointer is set to expired
   * before entering the weak-pointed destructor).
   */
  void cleanExpired( void )
  {
    childList.erase(remove_if(childList.begin(), childList.end(),
			      boost::bind(&Child_wptr::expired,_1)  ),
		    childList.end());
  }

  void display(std::ostream& os) const { // TODO: should be const
    os << "PTR LIST [ ";
    for ( const_iterator iter = childList.begin(); iter!= childList.end(); ++iter)
      {
 	if( iter )
	  {Child_ptr toto = *iter; 	os << toto.get() << " ";}
	else { os << "--expired-- ";}
      }
    os << " ]";
  }
};

/* The Macro define a type to access to ParentOf::ChildList, with name <TYPE>List,
 * plus two accessors (const and non-const) to access directly to the list
 * of children, with name <ACCESS>List().
 */
#define ENABLE_ACCESS_TO_WEAK_CHILDREN(Child,typeName,accessName)    	\
  public: typedef WeakParentOf<Child>::ChildList typeName##List;	\
  typeName##List & accessName##List( void )				\
  {  return WeakParentOf<Child>::childList; }				\
  const typeName##List & accessName##List( void ) const			\
  {  return WeakParentOf<Child>::childList; }



/* --- WEAK CHILD ----------------------------------------------------------- */
/* --- WEAK CHILD ----------------------------------------------------------- */
/* --- WEAK CHILD ----------------------------------------------------------- */

/* Use this macro to define the linkToWeakParent## function. This function
 * **HAS TO** be called after the construction for enabling the link (cannot
 * be called from inside the constructor, due to shared_from_this limitation).
 * Similtaneously, the classical ENABLE_ACCESS_TO_PARENT can be used as upper.
 */
#define ENABLE_LINK_TO_WEAK_PARENT(Parent,Child) \
  void linkToWeakParent##Parent( boost::shared_ptr<Parent> ptr )	\
  {									\
    ChildOf<Parent>::linkToParent(ptr);					\
    ptr->WeakParentOf<Child>::registerChild                             \
         (boost::enable_shared_from_this<Child>::shared_from_this());	\
  }


/* This macro **HAS TO** be called from inside the destructor, for removing
 * the corresponding (expired) smart pointer from the child list of the father.
 */
#define UNREGISTER_FROM_WEAK_PARENT(Parent)	         \
	try { ChildOf<Parent>::parentPtr()->cleanExpired(); } \
	catch(const char *e) { if (strcmp(e,"WEAK")) throw e; }


/* --- SPEC WEAK CHILD ------------------------------------------------------ */
/* --- SPEC WEAK CHILD ------------------------------------------------------ */
/* --- SPEC WEAK CHILD ------------------------------------------------------ */

/* The weak pointer can be used for specific link (ie link from abstract-parent
 * to the abstract-child, while simultaneously the same link from the specific-
 * parent to the specific-child. The abstract child inherit from ChildOf<ParentA>
 * and from enable_shared_from_this. The specific child inherit from
 * ChildOf<ParentS> but not from enable_shared_from_this. The macro
 * ENABLE_ACCESS can be used in both child, but the macros ENABLE_LINK
 * and UNREGISTER have to be used **ONLY** in the specific child class.
 * Example below.
 *
 * struct ChildSpec; class ChildAbs;
 * struct ParentAbs : public WeakParentOf<ChildAbs>
 * {
 *   ENABLE_ACCESS_TO_WEAK_CHILDREN(ChildAbs,ChildAbs,childAbs);
 * };
 * struct ParentSpec : public ParentAbs, public WeakParentOf<ChildSpec>
 * {
 *   ENABLE_ACCESS_TO_WEAK_CHILDREN(ChildSpec,ChildSpec,childSpec);
 * };
 * struct ChildAbs : public ChildOf<ParentAbs>,
 * 		     public boost::enable_shared_from_this<ParentAbs>
 * {
 *   ENABLE_ACCESS_TO_PARENT(ParentAbs,parentAbs);
 * };
 * struct ChildSpec: public ChildAbs, public ChildOf<ParentSpec>
 * {
 *   ENABLE_ACCESS_TO_PARENT(ParentSpec,parentSpec);
 *   ENABLE_LINK_TO_WEAK_SPECIFIC_PARENT(ParentAbs,ParentSpec,
 * 				      ChildAbs,ChildSpec,NAME);
 *   virtual ~ChildSpec( void )
 *   {
 *     UNREGISTER_FROM_WEAK_SPECIFIC_PARENT(ParentSpec,ChildSpec);
 *     UNREGISTER_FROM_WEAK_SPECIFIC_PARENT(ParentAbs,ChildAbs);
 *   }
 * };
 *
 */

/* Call this macro inside the (weak-specific) child class (inheriting from
 * ChildOf<>), to define the function linkToWeakParent##name. The 4 types
 * parent/child have to be specified. */
#define ENABLE_LINK_TO_WEAK_SPECIFIC_PARENT(ParentAbs,ParentSpec,ChildAbs,ChildSpec,name) \
  void linkToWeakParent##name( boost::shared_ptr<ParentSpec> ptr )	    \
  {									    \
    ChildOf<ParentSpec>::linkToParent(ptr);                                 \
    ptr->WeakParentOf<ChildSpec>::registerChild                             \
         (SPTR_CAST<ChildSpec>(shared_from_this()));	    \
    ChildOf<ParentAbs>::linkToParent(ptr);                                  \
    ptr->WeakParentOf<ChildAbs>::registerChild(shared_from_this());         \
  }

/* Call this macro from inside the destructor of the specific child.
 * Remove the _this_ reference from the parents (spec and abs) childlist.
 */
#define UNREGISTER_FROM_WEAK_SPECIFIC_PARENT(Parent,Child)  \
	try { ChildOf<Parent>::parentPtr()->WeakParentOf<Child>::cleanExpired(); } \
	catch(const char *e) { if (strcmp(e,"WEAK")) throw e; }

	}} // namespace jafar/rtslam

#endif // #ifndef __rtslam_parents_H__

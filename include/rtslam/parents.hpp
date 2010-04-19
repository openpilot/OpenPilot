/*
 * parents.hpp
 *
 *  Created on: Apr 19, 2010
 *      Author: nmansard
 */

#ifndef __rtslam_parents_H__
#define __rtslam_parents_H__

#include <list>
#include <iostream>
#include <boost/smart_ptr.hpp>


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
		std::cout << "Destroy Parent. " << std::endl;
	}

	void registerChild(const Child_ptr & ptr) {
		childList.push_back(ptr);
	}
	void unregisterChild(const Child_ptr & ptr) {
		remove(childList.begin(), childList.end(), ptr);
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
  typedef ParentOf<Child>::ChildList typeName##List; \
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
			std::cerr << "Throw weak" << std::endl;
			throw "WEAK";
		}
		return sptr;
	}
	Parent& parent(void) {
		Parent_ptr sptr = parent_wptr.lock();
		if (!sptr) {
			std::cerr << "Throw weak" << std::endl;
			throw "WEAK";
		}
		return *sptr;
	}
	const Parent& parent(void) const {
		Parent_ptr sptr = parent_wptr.lock();
		if (!sptr) {
			std::cerr << "Throw weak" << std::endl;
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
  void linkToParent( boost::shared_ptr<Parent> ptr )          \
  {                                                           \
    ChildOf<Parent>::linkToParent(ptr);                       \
    ptr->ParentOf<Child>::registerChild(shared_from_this());  \
  }

/* Same as before, except that the function is defined with
 * name linkToParent<Parent> (ie, for Parent=Sen, linkToParentSen). */
#define ENABLE_LINK_TO_PARENT(Parent,name,Child)              \
  void linkToParent##name( boost::shared_ptr<Parent> ptr )  \
  {                                                           \
    ChildOf<Parent>::linkToParent(ptr);                       \
    ptr->ParentOf<Child>::registerChild(shared_from_this());  \
  }

/* Define three accessor function, one to the pointer, two on
 * the reference (const and non-const). The function are called
 * fatherPtr() and father() respectively.
 */
#define ENABLE_ACCESS_TO_FATHER(Parent)                       \
  boost::shared_ptr<Parent> father##Ptr( void )               \
  {  return ChildOf<Parent>::parentPtr(); }                   \
  Parent& father( void )                                      \
  {    return ChildOf<Parent>::parent();  }                   \
  const Parent& father( void ) const                          \
  {    return ChildOf<Parent>::parent();  }

/* Same as before, except that the function is given an
 * explicit name, to handle the case of multiple parent.
 */
#define ENABLE_ACCESS_TO_PARENT(Parent,accessName)            \
  boost::shared_ptr<Parent> accessName##Ptr( void )           \
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
				boost::dynamic_pointer_cast<Parent>(ptr);
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
			std::cerr << "Throw weak" << std::endl;
			throw "WEAK";
		}
		return sptr;
	}
	Parent& parent(void) {
		Parent_ptr sptr = parent_wptr.lock();
		if (!sptr) {
			std::cerr << "Throw weak" << std::endl;
			throw "WEAK";
		}
		return *sptr;
	}
	const Parent& parent(void) const {
		Parent_ptr sptr = parent_wptr.lock();
		if (!sptr) {
			std::cerr << "Throw weak" << std::endl;
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
  void linkToParent( const boost::shared_ptr<ParentGen>& ptr )       \
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

/* Define three accessors (to the pointer and to the const/non-const reference)
 * with names <ACCESS>Ptr() and <ACCESS>().
 */
#define ENABLE_ACCESS_TO_SPECIFIC_PARENT(Parent,accessName)         \
  boost::shared_ptr<Parent> accessName##Ptr( void )                 \
  {  return SpecificChildOf<Parent>::parentPtr(); }                 \
  Parent& accessName( void )                                        \
  {    return SpecificChildOf<Parent>::parent();  }                 \
  const Parent& accessName( void ) const                            \
  {    return SpecificChildOf<Parent>::parent();  }


	};}; // namespace jafar/rtslam

#endif // #ifndef __rtslam_parents_H__

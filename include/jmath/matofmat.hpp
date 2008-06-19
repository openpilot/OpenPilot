#ifndef JMATH_MATOFMAT_HPP
#define JMATH_MATOFMAT_HPP

#include <cstdlib>
#include <fstream>
#include <vector>

#include "kernel/jafarMacro.hpp"

#include "jmath/jblas.hpp"


namespace jafar {

  namespace jmath {
    
    template <typename T, std::size_t M, std::size_t N>
    class matofmat {
      
      /** The index table is used to retrieve in the data vector
	  the matrix corresponding to (i,j)th element. Very useful
	  for sparse matrix of matrix 
       */
      unsigned int *indextab;

      /** Number of non nzero elements (an element = one matrix)	 
       */
      unsigned int nbnze;
      
      /** Indicator for sparse matrix
       */
      bool isSparse_;
      
      /** number of columns 
       */      
      unsigned int nbc;
      
      /* numfer of rows 
       */
      unsigned int nbr;

      /** The container used to store the data. 
       */
      std::vector < typename ublas::bounded_matrix<T,M,N> > data;
             
    public :
      
      typedef boost::numeric::ublas::bounded_matrix<T,M,N>& reference;
      typedef const boost::numeric::ublas::bounded_matrix<T,M,N>& const_reference;           

      //
      // Constructors and Destructors
      //
      //==============================


      /** Constructor 1 : allocate memory for dense matrix of matrix.
	  User gives size of the container
       */
      matofmat (int nbr_, int nbc_) : data (nbr_*nbc_){
	
	unsigned int cpt;
	
	// 
	isSparse_ = false;
	nbr = nbr_;
	nbc = nbc_;
	nbnze = nbr_*nbc_;
	
	// Memory alloc for tabindex
	indextab = new unsigned int[nbnze];

	// Initialize tabindex
	for (cpt=0;cpt<nbnze;cpt++)
	  indextab[cpt] = cpt;
		
      }

      /** 
	  Constructor 2 : allocate memory for sparse matrix of matrix.
 	  User gives an matrix of visibility (matrix of boolean).
      */
      matofmat (jblas::bool_mat const& matvis_) : data() {
	
	unsigned int cpt1, cpt2, pos, indTrue=0;	
	ublas::bounded_matrix<T,M,N> nullmatrix;

	// 
	isSparse_ = false;
	nbr = matvis_.size1();
	nbc = matvis_.size2(); 	


	// Memory alloc for tabindex 	
	indextab = new unsigned int[nbr*nbc];

	// How many non zero elements ?
	nbnze = 0;
	for(cpt1=0;cpt1<nbr;cpt1++)
	  for (cpt2=0;cpt2<nbc;cpt2++)
	    if (matvis_(cpt1,cpt2))
	      {
		++nbnze;
	      }

	
	if (nbnze==nbr*nbc)
	  {
	    isSparse_ = false;	    
	    data.resize(nbnze);

	    for(pos=0;pos<nbnze;pos++)
	      indextab[pos] = pos;
	  }
	else 
	  {
	    
	    data.resize(nbnze+1);       
	    
	    // Fill tabindex
	    for (cpt1 = 0, indTrue=0, pos = 0; cpt1<nbr; cpt1++) 
	      for (cpt2 = 0; cpt2<nbc; cpt2++, pos++)	    
		if (matvis_(cpt1,cpt2))
		  indextab[pos] = indTrue++;
		else
		  indextab[pos] = nbnze;		
	    
	    // Last element of data is null matrix 
	    for (cpt1=0;cpt1<nullmatrix.size1();cpt1++)
	      for (cpt2=0;cpt2<nullmatrix.size2();cpt2++)
		nullmatrix(cpt1,cpt2) = 0.0;	  
	    
	    data[nbnze].assign(nullmatrix);	 	
	  }
      }


      /** Constructeur by recopy
       */
      matofmat (const matofmat& mom) : data(mom.capacity()) {
	
	unsigned int cpt1, cpt2, pos;
	ublas::bounded_matrix<T,M,N> nullmatrix;

	// 
	isSparse_ = mom.isSparse();
	nbr = mom.size1();
	nbc = mom.size2();
	nbnze = mom.capacity();
	
	// Get tabindex
	indextab = new unsigned int[nbr*nbc];
	mom.getIndexTab(indextab);

	if (isSparse_)
	  {
	    // Resize vector if is sparse
	    data.resize(nbnze+1);


	    // First element of data is null matrix 
	    for (cpt1=0;cpt1<nullmatrix.size1();cpt1++)
	      for (cpt2=0;cpt2<nullmatrix.size2();cpt2++)
		nullmatrix(cpt1,cpt2) = 0.0;	  
	    data[0] = nullmatrix;
	   
	    // copy of vector 
	    for (cpt1=0,pos=0; cpt1<nbr; cpt1++)
	      for (cpt2=0;cpt2<nbc;pos++,cpt2++)
		{
		  if (indextab[pos] != 0)
		    data[indextab[pos]] = mom(cpt1,cpt2);
		}
	  }
	else
	  {
	    // copy of vector
	    for (cpt1=0,pos=0;cpt1<nbr;cpt1++)
	      for (cpt2=0;cpt2<nbc;pos++,cpt2++)
		data[indextab[pos]] = mom(cpt1,cpt2);	
	  }
      }
      

      /** Destructor 
       */
      ~matofmat() {	
	// Free memory used by vector of matrix !
	data.clear();
	// Free memory used to store indextab
	delete[] indextab;
	
      }
      
      //
      // Export en matrice pleine
      //
      //=============================

      void exportFullMatrix (jblas::mat &MatM)
      {
	JFR_PRECOND(MatM.size1()==(M*nbr)," M has an invalid size to store the exported matrix");
	JFR_PRECOND(MatM.size2()==(N*nbc)," M has an invalid size to store the exported matrix");

	unsigned int cptc, cptr, pos;
	unsigned int ii,jj,decalR,decalC;	
	ublas::bounded_matrix<T,M,N> toto;

	for (cptr=0,pos=0; cptr<nbr ; cptr++)
	  {
	    for (cptc=0; cptc<nbc; cptc++,pos++)
	      {
		toto =  data[indextab[pos]];

		decalR = cptr*M;
		decalC = cptc*N;
				
		for ( ii=0 ; ii<M ; ii++ )
		  for ( jj=0 ; jj< N ; jj++ )
		    MatM(decalR+ii,decalC+jj) = toto(ii,jj);
	      }
    
	  }
	
      }

      //
      // Accessors
      //
      //===========

      /** Return true if MoM is sparse
       */
      bool isSparse () const {
	return (isSparse_);
      }

      /** Return number of rows 
       */ 
      unsigned int size1 () const {
	return (nbr);
      }
      
      /** Return number of columns
       */
      unsigned int size2 () const {
	return (nbc);
      }

      /** Return capacity = number of non zero elements
       */
      unsigned int capacity () const {
	return (nbnze);
      }

      void getIndexTab (unsigned int *tab) const {
	unsigned int cpt;

	for (cpt=0;cpt<nbr*nbc;cpt++)
	  tab[cpt] = (this->indextab)[cpt];
      }

      /** Return the index tab. 
	  Used by the copy constructor
       */
      void getIndexTab2 (unsigned int **tab) const {
	
	unsigned int cpt;
	
	*tab = new unsigned int(nbr*nbc);
	for (cpt=0;cpt<nbr*nbc;cpt++)
	  (*tab)[cpt] = indextab[cpt];
      }
      
      void assign (jblas::mat const Mat, unsigned int i, unsigned int j)
      {
// 	JFR_PRECOND(Mat.size1()==M,"jmath::matofmat : you try to assign a matrix with a wrong size");
// 	JFR_PRECOND(Mat.size2()==N,"jmath::matofmat : you try to assign a matrix with a wrong size");
	JFR_PRECOND((i>=0)&&(i<nbr),"jmath::matofmat : you try to put an element outside of the matrix");
	JFR_PRECOND((j>=0)&&(j<nbc),"jmath::matofmat : you try to put an element outside of the matrix");
	JFR_PRECOND(indextab[i*nbc+j]>0,"jmath::matofmat : you try to put an element on an zero element");
	
	data[indextab[i*nbc+j]].assign(Mat);
		
      };

      
      void add (jblas::mat const Mat, unsigned int i, unsigned int j)
      {
	JFR_PRECOND((i>=0)&&(i<nbr),"jmath::matofmat : you try to put an element outside of the matrix");
	JFR_PRECOND((j>=0)&&(j<nbc),"jmath::matofmat : you try to put an element outside of the matrix");
	JFR_PRECOND(indextab[i*nbc+j]>0,"jmath::matofmat : you try to put an element on an zero element");	
	
	data[indextab[i*nbc*j]].assign(data[indextab[i*nbc*j]]+Mat);
      }


      //
      // Operators
      //
      //===========

      /** Accessor for an element of MoM
       */
      reference operator () (unsigned int i, unsigned int j) {
	JFR_PRECOND((i>=0)&&(i<nbr),"jmath::matofmat : you try to put an element outside of the matrix");
	JFR_PRECOND((j>=0)&&(j<nbc),"jmath::matofmat : you try to put an element outside of the matrix");
	JFR_PRECOND(indextab[i*nbc+j]>=0,"jmath::matofmat : you try to put an element on an zero element");

	return data [indextab[i*nbc+j]] ;
      }
      
      /** Accessor for a constant element of MoM
       */
      const_reference operator () (unsigned int i, unsigned int j) const{
	return data [indextab[i*nbc+j]];
      }     

      //
      // Other function
      //
      //================
      void clear () {
	data.clear();
	delete[] indextab;
	nbr = 0;
	nbc = 0;
	nbnze = 0;
	isSparse_ = false;
      }
      
    }; // end class matofmat
    
  } // namespace jmath
  
} // namespace jafar





#endif

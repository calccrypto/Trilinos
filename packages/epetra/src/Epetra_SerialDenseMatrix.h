
/* Copyright (2001) Sandia Corportation. Under the terms of Contract 
 * DE-AC04-94AL85000, there is a non-exclusive license for use of this 
 * work by or on behalf of the U.S. Government.  Export of this program
 * may require a license from the United States Government. */


/* NOTICE:  The United States Government is granted for itself and others
 * acting on its behalf a paid-up, nonexclusive, irrevocable worldwide
 * license in ths data to reproduce, prepare derivative works, and
 * perform publicly and display publicly.  Beginning five (5) years from
 * July 25, 2001, the United States Government is granted for itself and
 * others acting on its behalf a paid-up, nonexclusive, irrevocable
 * worldwide license in this data to reproduce, prepare derivative works,
 * distribute copies to the public, perform publicly and display
 * publicly, and to permit others to do so.
 * 
 * NEITHER THE UNITED STATES GOVERNMENT, NOR THE UNITED STATES DEPARTMENT
 * OF ENERGY, NOR SANDIA CORPORATION, NOR ANY OF THEIR EMPLOYEES, MAKES
 * ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LEGAL LIABILITY OR
 * RESPONSIBILITY FOR THE ACCURACY, COMPLETENESS, OR USEFULNESS OF ANY
 * INFORMATION, APPARATUS, PRODUCT, OR PROCESS DISCLOSED, OR REPRESENTS
 * THAT ITS USE WOULD NOT INFRINGE PRIVATELY OWNED RIGHTS. */

#ifndef EPETRA_SERIALDENSEMATRIX_H
#define EPETRA_SERIALDENSEMATRIX_H

#include "Epetra_Object.h" 
#include "Epetra_CompObject.h"
#include "Epetra_BLAS.h"
class Epetra_SerialSymDenseMatrix;
class Epetra_VbrMatrix;

//! Epetra_SerialDenseMatrix: A class for constructing and using real double precision general dense matrices.

/*! The Epetra_SerialDenseMatrix class enables the construction and use of real-valued, general, 
    double-precision dense matrices.  It is built on the BLAS, and derives from the Epetra_BLAS. 

The Epetra_SerialDenseMatrix class is intended to provide very basic support for dense rectangular matrices.


<b>Constructing Epetra_SerialDenseMatrix Objects</b>

There are four Epetra_SerialDenseMatrix constructors.  The first constructs a zero-sized object which should be made
to appropriate length using the Shape() or Reshape() functions and then filled with the [] or () operators. 
The second constructs an object sized to the dimensions specified, which should be filled with the [] or () operators.
The third is a constructor that accepts user
data as a 2D array, and the fourth is a copy constructor. The third constructor has
two data access modes (specified by the Epetra_DataAccess argument):
<ol>
  <li> Copy mode - Allocates memory and makes a copy of the user-provided data. In this case, the
       user data is not needed after construction.
  <li> View mode - Creates a "view" of the user data. In this case, the
       user data is required to remain intact for the life of the object.
</ol>

\warning View mode is \e extremely dangerous from a data hiding perspective.
Therefore, we strongly encourage users to develop code using Copy mode first and 
only use the View mode in a secondary optimization phase.

<b>Extracting Data from Epetra_SerialDenseMatrix Objects</b>

Once a Epetra_SerialDenseMatrix is constructed, it is possible to view the data via access functions.

\warning Use of these access functions cam be \e extremely dangerous from a data hiding perspective.


<b>Vector and Utility Functions</b>

Once a Epetra_SerialDenseMatrix is constructed, several mathematical functions can be applied to
the object.  Specifically:
<ul>
  <li> Multiplication.
  <li> Norms.
</ul>

<b>Counting floating point operations </b>
The Epetra_SerialDenseMatrix class has Epetra_CompObject as a base class.  Thus, floating point operations 
are counted and accumulated in the Epetra_Flop object (if any) that was set using the SetFlopCounter()
method in the Epetra_CompObject base class.

*/


//=========================================================================
class Epetra_SerialDenseMatrix : public Epetra_CompObject, public Epetra_Object, public Epetra_BLAS {

  public:
  
  //@{ \name Constructor/Destructor Methods
  //! Default constructor; defines a zero size object.
  /*!
    Epetra_SerialDenseMatrix objects defined by the default constructor should be sized with the 
    Shape() or Reshape functions.  
    Values should be defined by using the [] or () operators.
   */
  Epetra_SerialDenseMatrix();

  //! Shaped constructor; defines a variable-sized object
  /*!
    \param In 
           NumRows - Number of rows in object.
    \param In 
           NumCols - Number of columns in object.

    Epetra_SerialDenseMatrix objects defined by the shaped constructor are already shaped to the
		dimensions given as a parameters. All values are initialized to 0. Calling this constructor 
		is equivalent to using the default constructor, and then calling the Shape function on it.
    Values should be defined by using the [] or () operators.
   */
  Epetra_SerialDenseMatrix(int NumRows, int NumCols);
  
  //! Set object values from two-dimensional array.
  /*!
    \param In 
           Epetra_DataAccess - Enumerated type set to Copy or View.
    \param In
           A - Pointer to an array of double precision numbers.  The first vector starts at A.
	   The second vector starts at A+LDA, the third at A+2*LDA, and so on.
    \param In
           LDA - The "Leading Dimension", or stride between vectors in memory.
    \param In 
           NumRows - Number of rows in object.
    \param In 
           NumCols - Number of columns in object.

	   See Detailed Description section for further discussion.
  */
  Epetra_SerialDenseMatrix(Epetra_DataAccess CV, double* A, int LDA, int NumRows, int NumCols);
  
  //! Epetra_SerialDenseMatrix copy constructor.
  
  Epetra_SerialDenseMatrix(const Epetra_SerialDenseMatrix& Source);

  //! Epetra_SerialDenseMatrix destructor.  
  virtual ~Epetra_SerialDenseMatrix ();
  //@}

  //@{ \name Shaping/sizing Methods
  //! Set dimensions of a Epetra_SerialDenseMatrix object; init values to zero.
  /*!
    \param In 
           NumRows - Number of rows in object.
    \param In 
           NumCols - Number of columns in object.

	   Allows user to define the dimensions of a Epetra_SerialDenseMatrix at any point. This function can
	   be called at any point after construction.  Any values that were previously in this object are
	   destroyed and the resized matrix starts off with all zero values.

    \return Integer error code, set to 0 if successful.
  */
  int Shape(int NumRows, int NumCols);
  
  //! Reshape a Epetra_SerialDenseMatrix object.
  /*!
    \param In 
           NumRows - Number of rows in object.
    \param In 
           NumCols - Number of columns in object.

	   Allows user to define the dimensions of a Epetra_SerialDenseMatrix at any point. This function can
	   be called at any point after construction.  Any values that were previously in this object are
	   copied into the new shape.  If the new shape is smaller than the original, the upper left portion
	   of the original matrix (the principal submatrix) is copied to the new matrix.

    \return Integer error code, set to 0 if successful.
  */
  int Reshape(int NumRows, int NumCols);
  //@}

  //@{ \name Mathematical methods

  //! Matrix-Matrix multiplication, \e this = ScalarThis*\e this + ScalarAB*A*B.
  /*! This function performs a variety of matrix-matrix multiply operations.

  \param In
         TransA - Operate with the transpose of A if = 'T', else no transpose if = 'N'.
  \param In
         TransB - Operate with the transpose of B if = 'T', else no transpose if = 'N'.

  \param In
         ScalarAB - Scalar to multiply with A*B.
  \param In
         A - Dense Matrix.
  \param In
         B - Dense Matrix.
  \param In
         ScalarThis - Scalar to multiply with \e this.

    \return Integer error code, set to 0 if successful.
	 
  */
  int Multiply(char TransA, char TransB, double ScalarAB, 
							 const Epetra_SerialDenseMatrix& A, 
							 const Epetra_SerialDenseMatrix& B,
							 double ScalarThis);

  //! Matrix-Matrix multiplication with a symmetric matrix A.
  /*! If SideA = 'L', compute \e this = ScalarThis*\e this + ScalarAB*A*B.
      If SideA = 'R', compute \e this = ScalarThis*\e this + ScalarAB*B*A.

This function performs a variety of matrix-matrix multiply operations.

  \param In
         SideA - Specifies order of A relative to B.

  \param In
         ScalarAB - Scalar to multiply with A*B.
  \param In
         A - Symmetric Dense Matrix, either upper or lower triangle will be used depending on
	 value of A.Upper().
  \param In
         B - Dense Matrix.
  \param In
         ScalarThis - Scalar to multiply with \e this.

    \return Integer error code, set to 0 if successful.
	 
  */
  int Multiply(char SideA, double ScalarAB, 
							 const Epetra_SerialSymDenseMatrix& A, 
							 const Epetra_SerialDenseMatrix& B,
							 double ScalarThis);

  //! Inplace scalar-matrix product A = \e a A.
  /*! Scale a matrix, entry-by-entry using the value ScalarA.


  \param ScalarA (In) Scalar to multiply with A.

   \return Integer error code, set to 0 if successful.
	 
  */
  int Scale(double ScalarA);

  //! Computes the 1-Norm of the \e this matrix.
  /*!
    \return Integer error code, set to 0 if successful.
  */
  virtual double NormOne() const;

  //! Computes the Infinity-Norm of the \e this matrix.
  virtual double NormInf() const;

  //@}

  //@{ \name Data Accessor methods

  //! Value copy from one matrix to another.
  /*!
    The operator= allows one to copy the values from one existing SerialDenseMatrix to another, as
    long as there is enough room in the target to hold the source.

    \return Values of the left hand side matrix are modified by the values of the right hand side matrix.
  */
    Epetra_SerialDenseMatrix & operator = (const Epetra_SerialDenseMatrix& Source);

  //! Add one matrix to another.
  /*!
    The operator+= allows one to add the values from one existin SerialDenseMatrix to another, as
    long as there is enough room in the target to hold the source.

    \return Values of the left hand side matrix are modified by the addition
    of the values of the right hand side matrix.
  */
    Epetra_SerialDenseMatrix & operator += (const Epetra_SerialDenseMatrix& Source);

  //! Element access function.
  /*!
    The parentheses operator returns the element in the ith row and jth column if A(i,j) is
    specified, the expression A[j][i] (note that i and j are reversed) will return the same element.
    Thus, A(i,j) = A[j][i] for all valid i and j.

    \return Element from the specified row and column.

		\warning No bounds checking is done unless Epetra is compiled with EPETRA_ARRAY_BOUNDS_CHECK.
  */
	    double& operator () (int RowIndex, int ColIndex);

  //! Element access function.
  /*!
    The parentheses operator returns the element in the ith row and jth column if A(i,j) is
    specified, the expression A[j][i] (note that i and j are reversed) will return the same element.
    Thus, A(i,j) = A[j][i] for all valid i and j.

    \return Element from the specified row and column.

		\warning No bounds checking is done unless Epetra is compiled with EPETRA_ARRAY_BOUNDS_CHECK.
  */
    const double& operator () (int RowIndex, int ColIndex) const;

  //! Column access function.
  /*!
    The parentheses operator returns the element in the ith row and jth column if A(i,j) is
    specified, the expression A[j][i] (note that i and j are reversed) will return the same element.
    Thus, A(i,j) = A[j][i] for all valid i and j.

    \return Pointer to address of specified column.

    \warning No bounds checking can be done for the index i in the expression A[j][i].
		\warning No bounds checking is done unless Epetra is compiled with EPETRA_ARRAY_BOUNDS_CHECK.
  */
    double* operator [] (int ColIndex);

  //! Column access function.
  /*!
    The parentheses operator returns the element in the ith row and jth column if A(i,j) is
    specified, the expression A[j][i] (note that i and j are reversed) will return the same element.
    Thus, A(i,j) = A[j][i] for all valid i and j.

    \return Pointer to address of specified column.

    \warning No bounds checking can be done for the index i in the expression A[j][i].
		\warning No bounds checking is done unless Epetra is compiled with EPETRA_ARRAY_BOUNDS_CHECK.
  */
    const double* operator [] (int ColIndex) const;
    
  //! Set matrix values to random numbers.
  /*! 
		SerialDenseMatrix uses the random number generator provided by Epetra_Util.
		The matrix values will be set to random values on the interval (-1.0, 1.0).
    
		\return Integer error code, set to 0 if successful.
  */
  int Random();

  //! Returns row dimension of system.
  int M() const {return(M_);};

  //! Returns column dimension of system.
  int N() const {return(N_);};

  //! Returns pointer to the \e this matrix.
  double* A() const {return(A_);};

  //! Returns pointer to the \e this matrix.
  double* A() {return(A_);};

  //! Returns the leading dimension of the \e this matrix.
  int LDA() const {return(LDA_);};

	//! Returns the data access mode of the \e this matrix.
	Epetra_DataAccess CV() const {return(CV_);};
  //@}
  
  //@{ \name I/O methods
  //! Print service methods; defines behavior of ostream << operator.
  virtual void Print(ostream& os) const;
  //@}

  //@{ \name Deprecated methods (will be removed in later versions of this class).

  //! Computes the 1-Norm of the \e this matrix (identical to NormOne() method).
  /*!
    \return Integer error code, set to 0 if successful.
  */
  virtual double OneNorm() const {return(NormOne());};

  //! Computes the Infinity-Norm of the \e this matrix (identical to NormInf() method).
  virtual double InfNorm() const {return(NormInf());};
  //@}

 protected:

  void CopyMat(double* Source, int Source_LDA, int NumRows, int NumCols,
							 double* Target, int Target_LDA, bool add=false);
  void CleanupData();

  int M_;
  int N_;
  bool A_Copied_;
	Epetra_DataAccess CV_;

  //For performance reasons, it's better if Epetra_VbrMatrix can access the
  //LDA_ and A_ members of this class directly without going through an
  //accessor method. Rather than making them public members, we'll make
  //Epetra_VbrMatrix a friend class.

  friend class Epetra_VbrMatrix;

  int LDA_;
  double* A_;
};

// inlined definitions of op() and op[]
//=========================================================================
inline double& Epetra_SerialDenseMatrix::operator () (int RowIndex, int ColIndex) {
#ifdef EPETRA_ARRAY_BOUNDS_CHECK
  if (RowIndex >= M_ || RowIndex < 0)
		throw ReportError("Row index = " +toString(RowIndex) + 
											" Out of Range 0 - " + toString(M_-1),-1);
  if (ColIndex >= N_ || ColIndex < 0) 
		throw ReportError("Column index = " +toString(ColIndex) + 
											" Out of Range 0 - " + toString(N_-1),-2);
#endif
   return(A_[ColIndex*LDA_ + RowIndex]);
}
//=========================================================================
inline const double& Epetra_SerialDenseMatrix::operator () (int RowIndex, int ColIndex) const {
#ifdef EPETRA_ARRAY_BOUNDS_CHECK
  if (RowIndex >= M_ || RowIndex < 0) 
		throw ReportError("Row index = " +toString(RowIndex) + 
											" Out of Range 0 - " + toString(M_-1),-1);
  if (ColIndex >= N_ || ColIndex < 0) 
		throw ReportError("Column index = " +toString(ColIndex) + 
											" Out of Range 0 - " + toString(N_-1),-2);
#endif
  return(A_[ColIndex*LDA_ + RowIndex]);
}
//=========================================================================
inline double* Epetra_SerialDenseMatrix::operator [] (int ColIndex) {
#ifdef EPETRA_ARRAY_BOUNDS_CHECK
  if (ColIndex >= N_ || ColIndex < 0) 
		throw ReportError("Column index = " +toString(ColIndex) + 
											" Out of Range 0 - " + toString(N_-1),-2);
#endif
  return(A_ + ColIndex*LDA_);
}
//=========================================================================
inline const double* Epetra_SerialDenseMatrix::operator [] (int ColIndex) const {
#ifdef EPETRA_ARRAY_BOUNDS_CHECK
  if (ColIndex >= N_ || ColIndex < 0) 
		throw ReportError("Column index = " +toString(ColIndex) + 
											" Out of Range 0 - " + toString(N_-1),-2);
#endif
  return(A_+ ColIndex*LDA_);
}
//=========================================================================

#endif /* EPETRA_SERIALDENSEMATRIX_H */

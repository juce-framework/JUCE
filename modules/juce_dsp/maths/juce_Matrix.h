/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce::dsp
{

/**
    General matrix and vectors class, meant for classic math manipulation such as
    additions, multiplications, and linear systems of equations solving.

    @see LinearAlgebra

    @tags{DSP}
*/
template <typename ElementType>
class Matrix
{
public:
    //==============================================================================
    /** Creates a new matrix with a given number of rows and columns. */
    Matrix (size_t numRows, size_t numColumns)
        : rows (numRows), columns (numColumns)
    {
        resize();
        clear();
    }

    /** Creates a new matrix with a given number of rows and columns, with initial
        data coming from an array, stored in row-major order.
    */
    Matrix (size_t numRows, size_t numColumns, const ElementType* dataPointer)
        : rows (numRows), columns (numColumns)
    {
        resize();
        memcpy (data.getRawDataPointer(), dataPointer, rows * columns * sizeof (ElementType));
    }

    /** Creates a copy of another matrix. */
    Matrix (const Matrix&) = default;

    /** Moves a copy of another matrix. */
    Matrix (Matrix&&) noexcept = default;

    /** Creates a copy of another matrix. */
    Matrix& operator= (const Matrix&) = default;

    /** Moves another matrix into this one */
    Matrix& operator= (Matrix&&) noexcept = default;

    //==============================================================================
    /** Creates the identity matrix */
    static Matrix identity (size_t size);

    /** Creates a Toeplitz Matrix from a vector with a given squared size */
    static Matrix toeplitz (const Matrix& vector, size_t size);

    /** Creates a squared size x size Hankel Matrix from a vector with an optional offset.

        @param vector    The vector from which the Hankel matrix should be generated.
                         Its number of rows should be at least 2 * (size - 1) + 1
        @param size      The size of resulting square matrix.
        @param offset    An optional offset into the given vector.
    */
    static Matrix hankel (const Matrix& vector, size_t size, size_t offset = 0);

    //==============================================================================
    /** Returns the number of rows in the matrix. */
    size_t getNumRows() const noexcept                 { return rows; }

    /** Returns the number of columns in the matrix. */
    size_t getNumColumns() const noexcept              { return columns; }

    /** Returns an Array of 2 integers with the number of rows and columns in the
        matrix.
    */
    Array<size_t> getSize() const noexcept             { return { rows, columns }; }

    /** Fills the contents of the matrix with zeroes. */
    void clear() noexcept                              { zeromem (data.begin(), (size_t) data.size() * sizeof (ElementType)); }

    //==============================================================================
    /** Swaps the contents of two rows in the matrix and returns a reference to itself. */
    Matrix& swapRows (size_t rowOne, size_t rowTwo) noexcept;

    /** Swaps the contents of two columns in the matrix and returns a reference to itself. */
    Matrix& swapColumns (size_t columnOne, size_t columnTwo) noexcept;

    //==============================================================================
    /** Returns the value of the matrix at a given row and column (for reading). */
    inline ElementType operator() (size_t row, size_t column) const noexcept
    {
        jassert (row < rows && column < columns);
        return data.getReference (static_cast<int> (dataAcceleration.getReference (static_cast<int> (row))) + static_cast<int> (column));
    }

    /** Returns the value of the matrix at a given row and column (for modifying). */
    inline ElementType& operator() (size_t row, size_t column) noexcept
    {
        jassert (row < rows && column < columns);
        return data.getReference (static_cast<int> (dataAcceleration.getReference (static_cast<int> (row))) + static_cast<int> (column));
    }

    /** Returns a pointer to the raw data of the matrix object, ordered in row-major
        order (for modifying).
    */
    inline ElementType* getRawDataPointer() noexcept                    { return data.getRawDataPointer(); }

    /** Returns a pointer to the raw data of the matrix object, ordered in row-major
        order (for reading).
     */
    inline const ElementType* getRawDataPointer() const noexcept        { return data.begin(); }

    //==============================================================================
    /** Addition of two matrices */
    inline Matrix& operator+= (const Matrix& other) noexcept            { return apply (other, [] (ElementType a, ElementType b) { return a + b; } ); }

    /** Subtraction of two matrices */
    inline Matrix& operator-= (const Matrix& other) noexcept            { return apply (other, [] (ElementType a, ElementType b) { return a - b; } ); }

    /** Scalar multiplication */
    inline Matrix& operator*= (ElementType scalar) noexcept
    {
        std::for_each (begin(), end(), [scalar] (ElementType& x) { x *= scalar; });
        return *this;
    }

    /** Addition of two matrices */
    inline Matrix operator+ (const Matrix& other) const                 { Matrix result (*this); result += other;  return result; }

    /** Addition of two matrices */
    inline Matrix operator- (const Matrix& other) const                 { Matrix result (*this); result -= other;  return result; }

    /** Scalar multiplication */
    inline Matrix operator* (ElementType scalar) const                  { Matrix result (*this); result *= scalar; return result; }

    /** Matrix multiplication */
    Matrix operator* (const Matrix& other) const;

    /** Does a hadarmard product with the receiver and other and stores the result in the receiver */
    inline Matrix& hadarmard (const Matrix& other) noexcept             { return apply (other, [] (ElementType a, ElementType b) { return a * b; } ); }

    /** Does a hadarmard product with a and b returns the result. */
    static Matrix hadarmard (const Matrix& a, const Matrix& b)          { Matrix result (a); result.hadarmard (b); return result; }

    //==============================================================================
    /** Compare to matrices with a given tolerance */
    static bool compare (const Matrix& a, const Matrix& b, ElementType tolerance = 0) noexcept;

    /* Comparison operator */
    inline bool operator== (const Matrix& other) const noexcept      { return compare (*this, other); }

    //==============================================================================
    /** Tells if the matrix is a square matrix */
    bool isSquare() const noexcept                                   { return rows == columns; }

    /** Tells if the matrix is a vector */
    bool isVector() const noexcept                                   { return isOneColumnVector() || isOneRowVector(); }

    /** Tells if the matrix is a one column vector */
    bool isOneColumnVector() const noexcept                          { return columns == 1; }

    /** Tells if the matrix is a one row vector */
    bool isOneRowVector() const noexcept                             { return rows == 1; }

    /** Tells if the matrix is a null matrix */
    bool isNullMatrix() const noexcept                               { return rows == 0 || columns == 0; }

    //==============================================================================
    /** Solves a linear system of equations represented by this object and the argument b,
        using various algorithms depending on the size of the arguments.

        The matrix must be a square matrix N times N, and b must be a vector N times 1,
        with the coefficients of b. After the execution of the algorithm,
        the vector b will contain the solution.

        Returns true if the linear system of equations was successfully solved.
     */
    bool solve (Matrix& b) const noexcept;

    //==============================================================================
    /** Returns a String displaying in a convenient way the matrix contents. */
    String toString() const;

    //==============================================================================
    ElementType* begin() noexcept                   { return data.begin(); }
    ElementType* end() noexcept                     { return data.end(); }

    const ElementType* begin() const noexcept       { return &data.getReference (0); }
    const ElementType* end()   const noexcept       { return begin() + data.size(); }

private:
    //==============================================================================
    /** Resizes the matrix. */
    void resize()
    {
        data.resize (static_cast<int> (columns * rows));
        dataAcceleration.resize (static_cast<int> (rows));

        for (size_t i = 0; i < rows; ++i)
            dataAcceleration.setUnchecked (static_cast<int> (i), i * columns);
    }

    template <typename BinaryOperation>
    Matrix& apply (const Matrix& other, BinaryOperation binaryOp)
    {
        jassert (rows == other.rows && columns == other.columns);

        auto* dst = getRawDataPointer();

        for (auto src : other)
        {
            *dst = binaryOp (*dst, src);
            ++dst;
        }

        return *this;
    }

    //==============================================================================
    Array<ElementType> data;
    Array<size_t> dataAcceleration;

    size_t rows, columns;

    //==============================================================================
    JUCE_LEAK_DETECTOR (Matrix)
};

} // namespace juce::dsp

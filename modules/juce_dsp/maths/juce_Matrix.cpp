/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce::dsp
{

template <typename ElementType>
Matrix<ElementType> Matrix<ElementType>::identity (size_t size)
{
    Matrix result (size, size);

    for (size_t i = 0; i < size; ++i)
        result (i, i) = 1;

    return result;
}

template <typename ElementType>
Matrix<ElementType> Matrix<ElementType>::toeplitz (const Matrix& vector, size_t size)
{
    jassert (vector.isOneColumnVector());
    jassert (size <= vector.rows);

    Matrix result (size, size);

    for (size_t i = 0; i < size; ++i)
        result (i, i) = vector (0, 0);

    for (size_t i = 1; i < size; ++i)
        for (size_t j = i; j < size; ++j)
            result (j, j - i) = result (j - i, j) = vector (i, 0);

    return result;
}

template <typename ElementType>
Matrix<ElementType> Matrix<ElementType>::hankel (const Matrix& vector, size_t size, size_t offset)
{
    jassert (vector.isOneColumnVector());
    jassert (vector.rows >= (2 * (size - 1) + 1));

    Matrix result (size, size);

    for (size_t i = 0; i < size; ++i)
        result (i, i) = vector ((2 * i) + offset, 0);

    for (size_t i = 1; i < size; ++i)
        for (size_t j = i; j < size; ++j)
            result (j, j - i) = result (j - i, j) = vector (i + 2 * (j - i) + offset, 0);

    return result;
}

//==============================================================================
template <typename ElementType>
Matrix<ElementType>& Matrix<ElementType>::swapColumns (size_t columnOne, size_t columnTwo) noexcept
{
    jassert (columnOne < columns && columnTwo < columns);

    auto* p = data.getRawDataPointer();

    for (size_t i = 0; i < rows; ++i)
    {
        auto offset = dataAcceleration.getUnchecked (static_cast<int> (i));
        std::swap (p[offset + columnOne], p[offset + columnTwo]);
    }

    return *this;
}

template <typename ElementType>
Matrix<ElementType>& Matrix<ElementType>::swapRows (size_t rowOne, size_t rowTwo) noexcept
{
    jassert (rowOne < rows && rowTwo < rows);

    auto offset1 = rowOne * columns;
    auto offset2 = rowTwo * columns;

    auto* p = data.getRawDataPointer();

    for (size_t i = 0; i < columns; ++i)
        std::swap (p[offset1 + i], p[offset2 + i]);

    return *this;
}

//==============================================================================
template <typename ElementType>
Matrix<ElementType> Matrix<ElementType>::operator* (const Matrix<ElementType>& other) const
{
    auto n = getNumRows(), m = other.getNumColumns(), p = getNumColumns();
    Matrix result (n, m);

    jassert (p == other.getNumRows());

    size_t offsetMat = 0, offsetlhs = 0;

    auto* dst = result.getRawDataPointer();
    auto* a = getRawDataPointer();
    auto* b = other.getRawDataPointer();

    for (size_t i = 0; i < n; ++i)
    {
        size_t offsetrhs = 0;

        for (size_t k = 0; k < p; ++k)
        {
            auto ak = a[offsetlhs++];

            for (size_t j = 0; j < m; ++j)
                dst[offsetMat + j] += ak * b[offsetrhs + j];

            offsetrhs += m;
        }

        offsetMat += m;
    }

    return result;
}

//==============================================================================
template <typename ElementType>
bool Matrix<ElementType>::compare (const Matrix& a, const Matrix& b, ElementType tolerance) noexcept
{
    if (a.rows != b.rows || a.columns != b.columns)
        return false;

    tolerance = std::abs (tolerance);

    auto* bPtr = b.begin();
    for (auto aValue : a)
        if (std::abs (aValue - *bPtr++) > tolerance)
            return false;

    return true;
}

//==============================================================================
template <typename ElementType>
bool Matrix<ElementType>::solve (Matrix& b) const noexcept
{
    auto n = columns;
    jassert (n == n && n == b.rows && b.isOneColumnVector());

    auto* x = b.getRawDataPointer();
    const auto& A = *this;

    switch (n)
    {
        case 1:
        {
            auto denominator = A (0,0);

            if (approximatelyEqual (denominator, (ElementType) 0))
                return false;

            b (0, 0) /= denominator;
        }
        break;

        case 2:
        {
            auto denominator = A (0, 0) * A (1, 1) - A (0, 1) * A (1, 0);

            if (approximatelyEqual (denominator, (ElementType) 0))
                return false;

            auto factor = (1 / denominator);
            auto b0 = x[0], b1 = x[1];

            x[0] = factor * (A (1, 1) * b0 - A (0, 1) * b1);
            x[1] = factor * (A (0, 0) * b1 - A (1, 0) * b0);
        }
        break;

        case 3:
        {
            auto denominator = A (0, 0) * (A (1, 1) * A (2, 2) - A (1, 2) * A (2, 1))
                             + A (0, 1) * (A (1, 2) * A (2, 0) - A (1, 0) * A (2, 2))
                             + A (0, 2) * (A (1, 0) * A (2, 1) - A (1, 1) * A (2, 0));

            if (approximatelyEqual (denominator, (ElementType) 0))
                return false;

            auto factor = 1 / denominator;
            auto b0 = x[0], b1 = x[1], b2 = x[2];

            x[0] =  ( ( A (0, 1) * A (1, 2) - A (0, 2) * A (1, 1)) * b2
                    + (-A (0, 1) * A (2, 2) + A (0, 2) * A (2, 1)) * b1
                    + ( A (1, 1) * A (2, 2) - A (1, 2) * A (2, 1)) * b0) * factor;

            x[1] = -( ( A (0, 0) * A (1, 2) - A (0, 2) * A (1, 0)) * b2
                    + (-A (0, 0) * A (2, 2) + A (0, 2) * A (2, 0)) * b1
                    + ( A (1, 0) * A (2, 2) - A (1, 2) * A (2, 0)) * b0) * factor;

            x[2] =  ( ( A (0, 0) * A (1, 1) - A (0, 1) * A (1, 0)) * b2
                    + (-A (0, 0) * A (2, 1) + A (0, 1) * A (2, 0)) * b1
                    + ( A (1, 0) * A (2, 1) - A (1, 1) * A (2, 0)) * b0) * factor;
        }
        break;


        default:
        {
            Matrix<ElementType> M (A);

            for (size_t j = 0; j < n; ++j)
            {
                if (approximatelyEqual (M (j, j), (ElementType) 0))
                {
                    auto i = j;
                    while (i < n && approximatelyEqual (M (i, j), (ElementType) 0))
                        ++i;

                    if (i == n)
                        return false;

                    for (size_t k = 0; k < n; ++k)
                        M (j, k) += M (i, k);

                    x[j] += x[i];
                }

                auto t = 1 / M (j, j);

                for (size_t k = 0; k < n; ++k)
                    M (j, k) *= t;

                x[j] *= t;

                for (size_t k = j + 1; k < n; ++k)
                {
                    auto u = -M (k, j);

                    for (size_t l = 0; l < n; ++l)
                        M (k, l) += u * M (j, l);

                    x[k] += u * x[j];
                }
            }

            for (int k = static_cast<int> (n) - 2; k >= 0; --k)
                for (size_t i = static_cast<size_t> (k) + 1; i < n; ++i)
                    x[k] -= M (static_cast<size_t> (k), i) * x[i];
        }
    }

    return true;
}

//==============================================================================
template <typename ElementType>
String Matrix<ElementType>::toString() const
{
    StringArray entries;
    int sizeMax = 0;

    auto* p = data.begin();

    for (size_t i = 0; i < rows; ++i)
    {
        for (size_t j = 0; j < columns; ++j)
        {
            String entry (*p++, 4);
            sizeMax = jmax (sizeMax, entry.length());

            entries.add (entry);
        }
    }

    sizeMax = ((sizeMax + 1) / 4 + 1) * 4;

    MemoryOutputStream result;

    auto n = static_cast<size_t> (entries.size());

    for (size_t i = 0; i < n; ++i)
    {
        result << entries[(int) i].paddedRight (' ', sizeMax);

        if (i % columns == (columns - 1))
            result << newLine;
    }

    return result.toString();
}

template class Matrix<float>;
template class Matrix<double>;

} // namespace juce::dsp

// ********************************************************
//   File: Matrix.h
//   Description:  Definition of class Matrix and function 
//                 Gauss  (not a complete or convenient 
//                 class definition;  it only provides the 
//                 operations required by class basic_FIR)
//
//   Author: Carlos Moreno
// ********************************************************

#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <valarray>
#include <algorithm>
#include <iostream>
#include <iomanip>

using namespace std;

template <class T>
inline T Abs (const T & x)
{
    return x > 0 ? x : -x;
}


class Matrix 
{
    friend int Gauss (const Matrix &, valarray<long double> &, const valarray<long double> &);
public:
	Matrix(size_t r_size, size_t c_size)
        : rows(r_size), columns(c_size) 
    {
        data.resize(r_size);
        for (int i = 0; i < r_size; i++)
        {
            data[i].resize(c_size);
        }
    }

    size_t dim_rows() const
    {
        return rows;
    }

    size_t dim_columns() const
    {
        return columns;
    }

    valarray<long double> & operator[] (int r)
    {
        return data[r];
    }

    const valarray<long double> & operator[] (int r) const
    {
        return (const_cast<Matrix *> (this))->data[r];
    }

    Matrix operator* (const Matrix &) const;

    valarray<long double> operator* (const valarray<long double> &) const;

    void print(ostream & output) const      // For debugging purposes
    {
        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < columns; j++)
            {
                output << setprecision(2) << setw(6) << data[i][j];
            }
            output << endl;
        }
    }

private:
    typedef valarray<long double> row_type;
    valarray<row_type> data;
    size_t rows, columns;

    int MaxPosition (int col) const;
    void swap_rows (int row1, int row2);
};

int Gauss (const Matrix & _A, valarray<long double> & x, const valarray<long double> & b);


#endif

// ********************************************************
//   File: Matrix.h
//   Description:  Implementation of class Matrix and 
//                 function Gauss  (not a complete, 
//                 convenient or efficient class definition; 
//                 it only provides the operations required 
//                 by class basic_FIR)
//
//   Author: Carlos Moreno
// ********************************************************

#include "Matrix.h"

#include <stdexcept>
using namespace std;

Matrix Matrix::operator* (const Matrix & other) const
{
    if (columns != other.rows)
    {
        throw runtime_error ("Incompatible matrix sizes");
    }

    Matrix result (rows, other.columns);

    for (int n = 0; n < rows; n++)
    {
        for (int m = 0; m < other.columns; m++)
        {
            result[n][m] = 0;
            for (int i = 0; i < columns; i++)
            {
                result[n][m] += data[n][i] * other[i][m];
            }
        }
    }

    return result;
}


valarray<long double> Matrix::operator* (const valarray<long double> & v) const
{
    int N = v.size();
    valarray<long double> result (N);

    for (int i = 0; i < N; i++)
    {
        result[i] = 0;
        for (int j = 0; j < N; j++)
        {
            result[i] += data[i][j] * v[j];
        }
    }

    return result;
}


int Matrix::MaxPosition (int col) const
{
    long double valmax;
    int maxvalpos = col;

    valmax = Abs(data[col][col]);
    for (int i = col+1; i < rows; i++)
    {
        if (Abs(data[i][col]) > valmax)
        {
            valmax = Abs(data[i][col]);
            maxvalpos = i;
		}
    }

    return maxvalpos;
}


void Matrix::swap_rows (int row1, int row2)
{
    for (int col = 0; col < columns; col++)
    {
        swap (data[row1][col], data[row2][col]);
    }
}

int Gauss (const Matrix & _A, valarray<long double> & x, const valarray<long double> & _b)
{
    Matrix A = _A;
    valarray<long double> b = _b;

    for (int row = 0; row < A.rows-1; row++)
    {
        int maxval = A.MaxPosition(row);
        if (Abs(A[maxval][row]) < 1e-50)
		{
            throw runtime_error ("Singular matrix solving set of linear equations");
        }

        if (maxval != row)
        {
            A.swap_rows (row, maxval);
            swap (b[row], b[maxval]);
        }

        for (int i = row+1; i < A.rows; i++)
        {
            if (A[i][row] != 0)
            {
				double coeff = A[i][row]/A[row][row];
                A[i][row] = 0;
                for (int j = row+1; j < A.columns; j++)
                {
                    A[i][j] -= coeff * A[row][j];
                }
                b[i] -= coeff * b[row];
            }
        }
    }

    int N = A.columns - 1;

    x[N] = b[N]/A[N][N];
    for (int i = N-1; i >= 0; i--)
    {
        long double sum = 0;
        for (int j = i+1; j <= N; j++)
		{
            sum += A[i][j]*x[j];
        }
        x[i] = (b[i] - sum) / A[i][i];
	}

    return -1;      // Successful operation
}

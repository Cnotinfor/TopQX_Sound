// ********************************************************
//   File: FIR.h
//   Description:  Definition and implementation of 
//                 template class basic_FIR and type FIR
//
//   Author: Carlos Moreno
// ********************************************************

#ifndef __FIR_H__
#define __FIR_H__

#include <valarray>
#include <vector>
#include <cmath>
#include <numeric>
#include <complex>
#include <algorithm>
using namespace std;

const double pi = 3.14159265359;
const complex<double> j(0,1);            // Imaginary unit


#include "filter_types.h"
#include "conversions.h"
#include "Matrix.h"


template <class T>
class basic_FIR
{
public:
    struct Constraint_point { double w, gain; };    // For the frequency-sampling 
                                                    // filter design method

        // ****************  Constructors  *********************

    basic_FIR (valarray<T> coefficients)
        : h (coefficients)
    {
        reverse (&h[0], &h[0] + h.size());
    }

    basic_FIR (const T * coefficients, size_t N)
        : h (coefficients, N)
    {
        reverse (&h[0], &h[0] + N);
    }

    basic_FIR (struct Low_pass filter_descriptor, double scale_factor = 1);
    basic_FIR (struct High_pass filter_descriptor, double scale_factor = 1);
    basic_FIR (struct Band_pass filter_descriptor, double scale_factor = 1);
    
    basic_FIR (double (*frequency_response) (double), int N, double scale_factor = 1);
    basic_FIR (const vector <struct Constraint_point> & H, double scale_factor = 1);


        // *****************  Filtering operations  *******************

    template <class Iterator, class Type>
    void output (Iterator sample, Type & result) const
    {
            // In case rounding is required, assign using convert function

        convert (inner_product(sample-h.size()+1, sample+1, 
                               &(const_cast<basic_FIR *>(this)->h[0]), 
                               Type()), 
                 result);
    }

    template <class Iterator, class IteratorOut>
    void operator() (Iterator first, Iterator last, IteratorOut OutputSignal) const
    {
        for (Iterator current = first; current != last; ++current)
        {
            output (current, *OutputSignal++);
        }
    }


        // **************  Misc. member functions  *******************

    complex<double> H (double w) const;     // Frequency response

    int length () const 
    {
        return h.size();
    }


private:
    valarray<T> h;

    void compute_coefficients (const vector<struct Constraint_point> & H);
};

typedef basic_FIR<double> FIR;



// *************************************
//
//      Member functions definitions
//
// *************************************

template <class T>
basic_FIR<T>::basic_FIR (struct Low_pass filter_descriptor, double scale_factor)
{
    if (filter_descriptor.N < 2)
    {
        filter_descriptor.N = 2;    // N can not be < 2
    }

    vector<Constraint_point> H (filter_descriptor.N);

    const double Wc = filter_descriptor.Fc * 2 * pi / filter_descriptor.Fs;

    for (int i = 0; i < H.size(); i++)
    {
        const double delta_w = pi / (H.size() - 1);     // delta_w is the difference
                                                        // in w between two consecutive 
                                                        // freq. samples
        H[i].w = i * delta_w;

        if (H[i].w <= Wc - 2 * delta_w)         // In the pass-band
        {
            H[i].gain = 1 * scale_factor;
        }
        else if (H[i].w >= Wc + 2 * delta_w)    // In the stop-band
        {
            H[i].gain = 0;
        }
        else                                    // In the transition band (near Wc)
        {                                       // Use a raised cosine to calculate values 
            H[i].gain = (1 + cos(pi * (H[i].w - (Wc - 2 * delta_w)) / 
                        (4 * delta_w))) * scale_factor / 2;
        }
    }

    compute_coefficients (H);
}


template <class T>
basic_FIR<T>::basic_FIR (struct High_pass filter_descriptor, double scale_factor)
{
    if (filter_descriptor.N < 2)
    {
        filter_descriptor.N = 2;    // N can not be < 2
    }

    vector<Constraint_point> H (filter_descriptor.N);

    const double Wc = filter_descriptor.Fc * 2 * pi / filter_descriptor.Fs;

    for (int i = 0; i < H.size(); i++)
    {
        const double delta_w = pi / (H.size() - 1);     // delta_w is the difference
                                                        // in w between two consecutive 
                                                        // freq. samples
        H[i].w = i * delta_w;

        if (H[i].w <= Wc - 2 * delta_w)         // In the stop-band
        {
            H[i].gain = 0;
        }
        else if (H[i].w >= Wc + 2 * delta_w)    // In the pass-band
        {
            H[i].gain = 1 * scale_factor;
        }
        else                                    // In the transition band (near Wc)
        {                                       // Use a raised inverted cosine to 
                                                // calculate values 
            H[i].gain = (1 - cos(pi * (H[i].w - (Wc - 2 * delta_w)) / 
                        (4 * delta_w))) * scale_factor / 2;
        }
    }

    compute_coefficients (H);
}


template <class T>
basic_FIR<T>::basic_FIR (struct Band_pass filter_descriptor, double scale_factor)
{
    if (filter_descriptor.N < 2)
    {
        filter_descriptor.N = 2;    // N can not be < 2
    }

    vector<Constraint_point> H (filter_descriptor.N);

    const double Wl = filter_descriptor.Fl * 2 * pi / filter_descriptor.Fs;
    const double Wh = filter_descriptor.Fh * 2 * pi / filter_descriptor.Fs;

    for (int i = 0; i < H.size(); i++)
    {
        const double delta_w = pi / (H.size() - 1);     // delta_w is the difference
                                                        // in w between two consecutive 
                                                        // freq. samples
        H[i].w = i * delta_w;

        if (H[i].w <= Wl - 2 * delta_w || H[i].w >= Wh + 2 * delta_w)
                                                // In one of the stop-bands
        {
            H[i].gain = 0;
        }
        else if (H[i].w >= Wl + 2 * delta_w && H[i].w <= Wh - 2 * delta_w)
                                                // In the pass-band
        {
            H[i].gain = 1 * scale_factor;
        }
        else if (H[i].w < Wl + 2 * delta_w)     // In the transition near Wl
        {
            H[i].gain = (1 - cos(pi * (H[i].w - (Wl - 2 * delta_w)) / 
                        (4 * delta_w))) * scale_factor / 2;
        }
        else                                    // In the transition near Wh
        {
            H[i].gain = (1 + cos(pi * (H[i].w - (Wh - 2 * delta_w)) / 
                        (4 * delta_w))) * scale_factor / 2;
        }
    }

    compute_coefficients (H);
}


template <class T>
basic_FIR<T>::basic_FIR (double (*frequency_response) (double), int N, double scale_factor)
{
    if (N < 2)
    {
        N = 2;      // N can't be < 2
    }

    vector <struct Constraint_point> H(N);

    for (int i = 0; i < N; i++)
    {
        H[i].w = pi * i / (N - 1);
        H[i].gain = scale_factor * frequency_response (H[i].w);
    }

    compute_coefficients (H);    // Function compute_coefficients resizes h
}


template <class T>
basic_FIR<T>::basic_FIR (const vector <struct Constraint_point> & _H, double scale_factor)
{
    vector <struct Constraint_point> H(_H);     // create working copy
    for (int i = 0; i < H.size(); i++)
    {
        H[i].gain *= scale_factor;      // multiply first, to get the most
    }                                   // from the available resolution

    compute_coefficients (H);
}


template <class T>
complex<double> basic_FIR<T>::H (double w) const
{
    complex<double> result = complex<double> (0,0);

    for (int n = 0; n < h.size(); n++)
    {
        result +=  static_cast<double>(h[n]) * exp (-j * (w * n));
    }

    return result;
}


template <class T>
void basic_FIR<T>::compute_coefficients (const vector<struct Constraint_point> & H)
{
    const int N = H.size();

    Matrix A (N, N);
    valarray<long double> x(N), b(N);

    for (int n = 0; n < N; n++)
    {
        A[n][0] = 1;
        for (int k = 1; k < N; k++)
        {
            A[n][k] = 2 * cos(k * H[n].w);
        }

        b[n] = H[n].gain;
    }

    Gauss (A,x,b);

    h.resize (2*N - 1);

    convert (x[0], h[N-1]);
    for (int i = 1; i < N; i++)
    {
        convert (x[i], h[N-1 + i]);
        h[N-1 - i] = h[N-1 + i];    // Linear-phase symmety condition
    }
}

#endif

// ********************************************************
//   File: filter_types.h
//   Description:  Definition of structures to represent 
//                 basic filter types used as initializers 
//                 for filter objects instantiations
//
//   Author: Carlos Moreno
// ********************************************************

#ifndef __FILTER_TYPES_H__
#define __FILTER_TYPES_H__


struct Low_pass
{
    double Fc;      // Cutoff frequency
    double Fs;      // Sampling frequency
    int N;          // number of taps (number of non-repeated coefficients)

                // Constructor
    Low_pass (double fc, double fs, int n)
        : Fc(fc), Fs(fs), N(n) {}
};


struct High_pass 
{
    double Fc;      // Cutoff frequency
    double Fs;      // Sampling frequency
    int N;          // number of taps (number of non-repeated coefficients)

                // Constructor
    High_pass (double fc, double fs, int n)
        : Fc(fc), Fs(fs), N(n) {}
};


struct Band_pass 
{
    double Fl;      // Low-frequency cutoff
    double Fh;      // High-frequency cutoff
    double Fs;      // Sampling frequency
    int N;          // number of taps (number of non-repeated coefficients)

                // Constructor
    Band_pass (double fl, double fh, double fs, int n)
        : Fl(fl), Fh(fh), Fs(fs), N(n) {}
};

#endif

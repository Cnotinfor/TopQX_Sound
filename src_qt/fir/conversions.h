// ********************************************************
//   File: conversions.h
//   Description:  Definition of customized conversions
//                 to provide rounding and other specific 
//                 features.
//
//   Author: Carlos Moreno
// ********************************************************

#ifndef __CONVERSIONS_H__
#define __CONVERSIONS_H__

    // Default conversion is the standard static cast
template <class SrcType, class DstType>
inline DstType & convert (const SrcType & src, DstType & dst)
{
    return dst = static_cast<DstType>(src);
}


    // Now define specific conversions:

inline int convert (double src, int & dst)
{
    return dst = (src > 0) ? static_cast<int>(src + 0.5) : 
                             static_cast<int>(src - 0.5);
}

inline int convert (float src, int & dst)
{
    return convert (static_cast<double>(src), dst);
}


inline unsigned int convert (double src, unsigned int & dst)
{
    return dst = static_cast<unsigned int>(src + 0.5);
}

inline unsigned int convert (float src, unsigned int & dst)
{
    return convert (static_cast<double>(src), dst);
}


inline short int convert (double src, short int & dst)
{
    return dst = (src > 0) ? static_cast<short int>(src + 0.5) : 
                             static_cast<short int>(src - 0.5);
}

inline short int convert (float src, short int & dst)
{
    return convert (static_cast<double>(src), dst);
}


inline unsigned short int convert (double src, unsigned short int & dst)
{
    return dst = static_cast<unsigned short int>(src + 0.5);
}

inline unsigned short int convert (float src, unsigned short int & dst)
{
    return convert (static_cast<double>(src), dst);
}


inline long int convert (double src, long int & dst)
{
    return dst = (src > 0) ? static_cast<long int>(src + 0.5) : 
                             static_cast<long int>(src - 0.5);
}

inline long int convert (float src, long int & dst)
{
    return convert (static_cast<double>(src), dst);
}


inline unsigned long int convert (double src, unsigned long int & dst)
{
    return dst = static_cast<unsigned long int>(src + 0.5);
}

inline unsigned long int convert (float src, unsigned long int & dst)
{
    return convert (static_cast<double>(src), dst);
}


inline char convert (double src, char & dst)
{
    return dst = (src > 0) ? static_cast<char>(src + 0.5) : 
                             static_cast<char>(src - 0.5);
}

inline signed char convert (float src, signed char & dst)
{
    return convert (static_cast<double>(src), dst);
}


inline unsigned char convert (double src, unsigned char & dst)
{
    return dst = static_cast<unsigned char>(src + 0.5);
}

inline unsigned char convert (float src, unsigned char & dst)
{
    return convert (static_cast<double>(src), dst);
}

#endif

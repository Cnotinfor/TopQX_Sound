// ********************************************************
//   File: Signals.h
//   Description:  Definition and implementation of 
//                 template class Signal and its iterators 
//                 (Signal<T>::const_iterator and 
//                  Signal<T>::iterator)
//
//   Author: Carlos Moreno
// ********************************************************

#ifndef __SIGNALS_H__
#define __SIGNALS_H__

#include <valarray>
#include <stdexcept>
using namespace std;


#include "FIR.h"

enum Fade_in_modes
{
    linear, 
    cosine_shaped
};


template <class T>
class Signal
{
public:
            // **************  Constructor  ******************

    Signal (int requested_size = 256, int fade_in_len = 10, Fade_in_modes mode = linear);


            // ***************  Samples insertion methods  ***************

    Signal<T> & operator<< (const T & value)
	{
		recent_sample_pos = (recent_sample_pos + 1) & mask;
		x[recent_sample_pos] = value;

		return *this;
	}

    void fade_in (const T & value);

    template <class Iterator>
    void insert_block (Iterator block_start, size_t block_size)
    {
        if (block_size > capacity() || block_size < 1)
        {
            throw invalid_argument ("Buffer overflow or incorrect block_size in Signal::insert_block()");
        }

        if (recent_sample_pos + block_size < x.size())
        {
            copy (block_start, block_start + block_size, &x[recent_sample_pos + 1]);
            recent_sample_pos += block_size;
        }
        else
        {
            copy (block_start, 
                  block_start + x.size() - recent_sample_pos - 1, 
                  &x[recent_sample_pos + 1]);

            copy (block_start + x.size() - recent_sample_pos - 1, 
                  block_start + block_size, 
                  &x[0]);

            recent_sample_pos += block_size - x.size();
        }
    }


            // ***************  Samples read methods  ***************

    const T & operator[] (int subscript) const
	{
		if (subscript <= 0)
		{
			return (const_cast<Signal<T> *>(this)->x[(recent_sample_pos + subscript) & mask]);
		}
        else
        {
            throw out_of_range("Subscript must be negative");
        }
	}

    const T & most_recent_sample () const
    {
        return const_cast <Signal<T> *>(this)->x[recent_sample_pos];
    }

    const T & front () const
    {
        return most_recent_sample();
    }


        // ************  Iterators  ***************

    class const_iterator;

    class iterator
    {
        friend class const_iterator;
    public:
        iterator (T * sig = NULL, int it = 0, int msk = 0)
            : signal(sig), iter(it), mask(msk) {}

        T & operator* () const
        {
            return signal[iter];    // iter was already "ANDed" with mask 
        }                           // in iterator arithmetic operators

        const iterator & operator++ ()
        {
            iter = (iter + 1) & mask;
            return *this;
        }

        const iterator operator++ (int)
        {
            iterator orig = *this;
            iter = (iter + 1) & mask;
            return orig;
        }

        const iterator & operator-- ()
        {
            iter = (iter - 1) & mask;
            return *this;
        }

        const iterator operator-- (int)
        {
            iterator orig = *this;
            iter = (iter - 1) & mask;
            return orig;
        }

        const iterator operator+ (int offset) const
        {
            return iterator (signal, (iter + offset) & mask, mask);
        }

        const iterator operator- (int offset) const
        {
            return *this + (-offset);
        }

        bool operator== (const iterator & other) const
        {
            return signal == other.signal && 
                   iter == other.iter && 
                   mask == other.mask;
        }

        bool operator!= (const iterator & other) const
        {
            return !(*this == other);
        }

    private:
        T * signal;
        int iter;
        int mask;
    };


    class const_iterator
    {
    public:
        const_iterator (const T * sig = NULL, int it = 0, int msk = 0)
            : signal(sig), iter(it), mask(msk) {}

        const_iterator (const iterator & it)
            : signal (it.signal), iter (it.iter), mask (it.mask) {}

        const T & operator* () const 
        {
            return signal[iter];    // iter was already "ANDed" with mask 
        }                           // in iterator arithmetic operators


        const const_iterator & operator++ ()
        {
            iter = (iter + 1) & mask;
            return *this;
        }

        const const_iterator operator++ (int)
        {
            const_iterator orig = *this;
            iter = (iter + 1) & mask;
            return orig;
        }

        const const_iterator & operator-- ()
        {
            iter = (iter - 1) & mask;
            return *this;
        }

        const const_iterator operator-- (int)
        {
            const_iterator orig = *this;
            iter = (iter - 1) & mask;
            return orig;
        }

        const const_iterator operator+ (int offset) const
        {
            return const_iterator (signal, (iter + offset) & mask, mask);
        }

        const const_iterator operator- (int offset) const
        {
            return *this + (-offset);
        }

        bool operator== (const const_iterator & other) const
        {
            return signal == other.signal && 
                   iter == other.iter && 
                   mask == other.mask;
        }

        bool operator!= (const const_iterator & other) const
        {
            return !(*this == other);
        }

    private:
        const T * signal;
        int iter;
        int mask;
    };


    iterator begin ()
    {
        return iterator(&x[0], recent_sample_pos, mask);
    }

    const_iterator begin () const
    {
        return const_iterator (const_cast<Signal<T> *>(this)->begin());
    }


        // ************ Filtering Functions ******************

    template <class FilterCoeff>
    void filtered_output (const basic_FIR<FilterCoeff> & filter, T & result) const
    {
        if (recent_sample_pos >= filter.length() - 1)
        {
            filter.output (&const_cast<Signal<T> *>(this)->x[recent_sample_pos], 
                           result);
        }
        else
        {
            filter.output (begin(), result);
        }
    }

    template <class FilterCoeff>
    T filtered_output (const basic_FIR<FilterCoeff> & filter) const
    {
        T result;
        filtered_output (filter, result);
        return result;
    }

    template <class FilterCoeff, class OutputIterator>
    void filtered_block (const basic_FIR<FilterCoeff> & filter, 
                         int num_samples, OutputIterator y) const
    {
        for (int i = -num_samples + 1; i <= 0; i++)
        {
            if (((recent_sample_pos + i) & mask) >= filter.length() - 1)
            {
                filter.output (&const_cast<Signal<T> *>(this)->x [(recent_sample_pos + i) & mask], 
                               *y++);
            }
            else
            {
                filter.output (begin() + i, *y++);
            }
        }
    }


            // **************  Misc. / Utility  ***************

    int capacity () const
    {
        return size;
    }

private:
    valarray<T> x;
    int mask, size;
    int recent_sample_pos;

    Fade_in_modes fade_in_mode;
    int fade_in_counter, fade_in_length;
};



// *************************************
//
//      Member functions definitions
//
// *************************************

template <class T>
Signal<T>::Signal (int requested_size, int fade_len, Fade_in_modes mode)
    : size(requested_size), fade_in_length(fade_len), fade_in_mode(mode), 
      recent_sample_pos(0), fade_in_counter(0)
{
    mask = -1;
    while (mask & (size-1))
    {
        mask <<= 1;
    }
    mask = ~mask;
    x.resize(mask + 1);
    x = T();          // Fill the buffer (a valarray) with zeroes
}


template <class T>
void Signal<T>::fade_in (const T & value)
{
    if (fade_in_counter < fade_in_length)
    {
        double x = static_cast<double>(fade_in_counter++) / fade_in_length;
            // x between 0 and 1 for the interpolation formula

        T converted_value;       // To use convert in case rounding is required
        switch (fade_in_mode)
        {
            case linear:
                *this << convert (value * x, converted_value);
                break;

            case cosine_shaped:
                    // raised cosine is approximated by the 
                    // polynomial P(x) = 3x^2 - 2x^3
                *this << convert (value * x*x * (3 - 2*x), converted_value);
                break;
        }
    }
    else	// counter > len means that fade-in was already completed
    {
        *this << value;
    }
}

#endif

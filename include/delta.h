#ifndef DELTA_H
#define DELTA_H

#include <random>

// To keep track of the best solution found so far
// Returns true if value was updated, false otherwise
template <class T> struct Delta
{
    T value;
    int counter;

    Delta() : value(T()), counter(-1) {}

    Delta(T value, int counter) : value(value), counter(counter) {}

    bool Update(T new_value) 
    {
        // New value better than previous value
        if (new_value < value) 
        {
            value = new_value;
            counter = 1;
            
            return true;
        } 
        
        // Return true/false randomly
        else if (new_value == value && counter != -1)
        {
            ++counter;
            return ((rand() % counter + 1) == 1);
        }

        return false;
    }

    bool Update(const Delta<T> &delta) 
    {
        // Return true
        if (delta.value < value) 
        {
            value = delta.value;
            counter = delta.counter;
        
            return true;
        }

        // Return true/false randomly
        else if (delta.value == value && counter != -1) 
        {
            counter += delta.counter;
            return (rand() % counter + 1) <= delta.counter;
        }
        
        return false;
    }
};

#endif
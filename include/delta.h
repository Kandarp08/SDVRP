#include <random>

template <class T> struct Delta
{
    T value;
    int counter;

    Delta() : value(T()), counter(-1) {}

    Delta(T value, int counter) : value(value), counter(counter) {}

    bool Update(T new_value) 
    {
        if (new_value < value) 
        {
            value = new_value;
            counter = 1;
            
            return true;
        } 
        
        else if (new_value == value && counter != -1)
        {
            ++counter;
            return ((rand() % counter + 1) == 1);
        }

        return false;
    }

    bool Update(const Delta<T> &delta) 
    {
        if (delta.value < value) 
        {
            value = delta.value;
            counter = delta.counter;
        
            return true;
        }

        else if (delta.value == value && counter != -1) 
        {
            counter += delta.counter;
            return (rand() % counter + 1) <= delta.counter;
        }
        
        return false;
    }
};
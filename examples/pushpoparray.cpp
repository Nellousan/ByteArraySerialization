#include "bas.hpp"

void example(void)
{
    bas::SerializedObject obj;

    int array[] = {2, 5, 3, 6, 1};

    obj.pushData(array, 5);           // 5 is the size of the array.

    bas::PoppedArray<int> popped;     // We use the popped array to retreive c-style arrays

    popped = obj.popDataArray<int>();

    int *array = popped.get();        // We use get() to retreive the pointer to the popped data

    // Now let's say we want to assign this array to a std::vector<int>,
    // to do that, we need the size of the popped array.

    std::vector<int> vector;

    size_t size = popped.size();       // We retreive the size of the popped array with size()

    vector.assign(array, array + size);// We assign the data to the vector this way.

    return;
}
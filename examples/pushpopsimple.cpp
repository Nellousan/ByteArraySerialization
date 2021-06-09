/*
** ByteArraySerialisation
** File description:
** Short example on how to use pushData and popData
*/

#include "bas.hpp"

void example(void)
{
    bas::SerializedObject obj;
    int i = 5;
    int j = 0;

    obj.pushData(i);        // No typename is needed.

    j = obj.popData<int>(); // Typename required to pop the data. j is now equal to 5.

    return;
}
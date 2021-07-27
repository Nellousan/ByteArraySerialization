# Byte Array Serialization for C++

## Description

Multi-purpose byte array serialization lib for C++ classes or any type of data.\
This lib provide a way to easily serialize classes for networking purpose or to save them into binary file (even though you're probably better using json or xml in this case)

--------
## Usage Example

Here is a quick example of how the serialization process works

```c++
#include "bas.hpp"
#include <string>

class SerializablePerson : public bas::Serializable {        // inherit from the Serializable class
public:

    SerializablePerson(const std::string& name, int age)
        : name(name)
        , age(age)
    {
    }

    ~SerializablePerson() = default;

    // this function contains the serialization process of your class
    void makeSerialization(bas::SerializedObject& obj) override
    {
        obj.pushData(name);                     
        obj.pushData(age);
    }

    // this function contains the unserialization process of your class
    void makeUnserialization(bas::SerializedObject& obj) override
    {
        name = obj.popData<std::string>();
        age = obj.popData<int>();
    }
    
    std::string name;
    int age;
};

int main(void)
{
    SerializablePerson person1("David", 32);
    SerializablePerson person2("Robert", 45);

    bas::SerializedObject obj;

    obj = person1.serialize();                              // Use serialize() and not makeSerialization()

    person2.unserialize(obj);                               // Use unserialize() and not makeUnserialization()
    // person2 is now a copy of person1
    return 0;
}
```
--------
## Installation

```
git clone https://github.com/Nellousan/ByteArraySerialization.git
cd ByteArraySerialisation
cp include/bas.hpp the/path/to/your/poject's/include/folder
```

Since this lib is header only and fits into one file, using it is a simple as grabbing the bas.hpp file and add it into your include path of your project and it's ready to go.

--------
## Documentation
\
Here is a [link](https://nellousan.github.io/ByteArraySerialization/) to the documentation of the project.

--------
## TO-DO

- Unit-Tests
- CI
- Make a release and versionning stuff

/*
** ByteArraySerialisation
** File description:
** Serialization example
*/

#include "bas.hpp"

#include <string>

class SerializablePerson : public bas::Serializable {                      // inherit from the Serializable class
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
        obj.pushData(name);                                                // Notice that the library contains a custom variant of pushData and popData for std::string
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

    obj = person1.serialize();                                             // Use serialize() and not makeSerialization()

    person2.unserialize(obj);                                              // Use unserialize() and not makeUnserialization()

    // person2 is now a copy of person1

    return 0;
}

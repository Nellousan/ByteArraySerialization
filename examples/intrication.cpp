#include "bas.hpp"

#include <string>

// here is our wallet class the SerializablePerson will be holding, the trick here is to make the wallet serializable as well
class SerializableWallet : public bas::Serializable {
public: 
    SerializableWallet(const std::vector<int>& money, const std::string id_card) 
        : money(money)
        , id_card(id_card)
    {
    }

    ~SerializableWallet() = default;

    void makeSerialization(bas::SerializedObject& obj) override
    {
        obj.pushData(money); // Just like with std::string, pushData handles the std::vector<T> class as long as T is a primary type.
        obj.pushData(id_card);
    }

    void makeUnserialization(bas::SerializedObject& obj) override
    {
        money = obj.popData<std::vector<int>>();
        id_card = obj.popData<std::string>();
    }

    std::vector<int> money;
    std::string id_card;
};

class SerializablePerson : public bas::Serializable {
public:
    SerializablePerson(const std::string& name, int age, const SerializableWallet& wallet)
        : name(name)
        , age(age)
        , wallet(wallet)
    {
    }

    ~SerializablePerson() = default;

    void makeSerialization(bas::SerializedObject& obj) override
    {
        obj.pushData(name);
        obj.pushData(age);
        obj.pushData(wallet.serialize());                           // Here is the workaround, pushData handles SerializedObject automatically
    }

    void makeUnserialization(bas::SerializedObject& obj) override
    {
        name = obj.popData<std::string>();
        age = obj.popData<int>();
        wallet.unserialize(obj.popData<bas::SerializedObject>());   // popping SerializedObject is as easy a pushing it
    }

    std::string name;
    int age;
    SerializableWallet wallet;
};

int main(void)
{
    SerializablePerson person1("David", 32, {{5, 10, 5}, "David"});
    SerializablePerson person2("Robert", 45, {{20, 5, 1}, "Robert"});

    bas::SerializedObject obj;

    obj = person1.serialize();

    person2.unserialize(obj);

    // person2 is now a copy of person1

    return 0;
}
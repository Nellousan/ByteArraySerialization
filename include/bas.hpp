/*
** Byte Array Serialization
** File description:
** Header only lib which purpose is to be able to easily serialize data/classes
** to send them over networking protocol and unserialize them easily afterward.
** Author:
** Nell Fauveau
** https://github.com/Nellousan/ByteArraySerialisation
*/

/* Current Changelogs:
    - BugFix: pushData specialization on the SerializedObject
    - Changed the way serialization works, now the makeSerialization function
    needs override, serialization() is still the way of retrieving the SerializedObject,
    same goes for makeUnserialization and unserialization()
    - SerializedObject doesn't need the size to construct from char * anymore. A Checksum has been added
    to make this possible, and to add possibilities in networking (knowing if multiple objects have been received and such).
    - Moved bodies of pushData(), popData() and popDataArray() to separate function to
    avoid code duplication and make the addition of specialised function template easier.
    - Added documentation of the library, generated by doxygen.
*/

#ifndef BAS_HPP_
#define BAS_HPP_

#include <cstring>
#include <memory>
#include <tuple>
#include <vector>

#include <iostream>

////////////////////////////////////////////

#define _BAS_SIZE_BYTES_ 2 // Number of bytes used to store var size and array size in the payload,
#define _BAS_ARRAY_SIZE_ 2 // MAX is your OS size_t byte size (4 is max recommended as it will be compatible in most cases)
#define _BAS_CHECKSUM_SIZE_ 4 // If this lib is used for networking, make sure that these 3 numbers are the same on both ends

////////////////////////////////////////////

namespace bas {

template <typename T>
class Helper;

template <typename T>
class PoppedArray;

/**
 * @brief The SerializedObject contains and manages the payload of your serializations.
 * 
 * This class is the result of a Serializable::serialization. 
 */
class SerializedObject {
public:
    SerializedObject()
    {
        prepareChecksum();
    }

    /**
     * @brief Construct object from payload.
     * 
     * This operation makes a copy of data.
     * @param data payload the SerializedObject will be assigned.
     */
    inline SerializedObject(const char* data)
    {
        constructFromPayload(data);
    }


    /**
     * @brief Copy-construct object from another SerializedObject.
     * @param other The SerializedObject to be copied from.
     */
    inline SerializedObject(const SerializedObject& other)
        : _data(other._data)
        , _isChecksumRemoved(other._isChecksumRemoved)
    {
    }

    /**
     * @brief Default destructor.
     */
    ~SerializedObject() = default;

    /**
     * @brief Pushes data into the payload.
     * 
     * To retreive data pushed by this variant of pushData(), 
     * use popData<T>() where T is the type of the data pushed.
     * @param data Data to be pushed.
     * @see popData<>()
     */
    template <typename T>
    inline void pushData(const T& data)
    {
        Helper<T> helper;
        helper.pushData(*this, data);
    }

    /**
     * @brief Pushes an array of data into the object payload.
     * 
     * To retreive data pushed by this variant of pushData(),
     * use popDataArray<T>() where T is the type of the data pushed.
     * @param data A pointer to the data to be pushed.
     * @param array_size The size of the array to be pushed.
     * @see popDataArray<>()
     */
    template <typename T>
    inline void pushData(const T* data, size_t array_size)
    {
        const char* data_ = (const char*)data;
        size_t size = sizeof(T);

        pushSizes(size, array_size);

        pushRawData(size, array_size, data_);

        checksumUpdate();
    }

    /**
     * @brief Pop the next data in the payload.
     *
     * @return The popped value from the payload.
     */
    template <typename T>
    inline T popData(void)
    {
        Helper<T> helper;
        return helper.popData(*this);
    }

    /**
     * @brief Pop the next array in the payload and returns a PoppedArray.
     * 
     * The PoppedArray contains the popped data along with the size of the popped data.
     * @see PoppedArray
     */
    template <typename T>
    inline PoppedArray<T> popDataArray(void)
    {
        size_t size = 0;
        size_t array_size = 0;
        T* var;

        std::tie(size, array_size) = getSizes();

        var = new T[array_size];

        copyMem(size, array_size, var);

        return PoppedArray<T>(array_size, var);
    }

    /**
     * @brief Returns a pointer to the payload of the object.
     * 
     * This function is the go-to to get the 'finished product' of your serialization.
     * @return Pointer to the payload of the SerializedObject 
     */
    inline const char* payload(void) const
    {
        return _data.data();
    }

    /**
     * @brief Returns a reference to the std::vector containing the payload.
     * 
     * User-end should use this function only in rare case, manually modifying 
     * the payload may end up in undefined behaviours.
     */
    inline std::vector<char>& vector(void)
    {
        return _data;
    }

    /**
     * @brief Returns the size of the payload in Bytes.
     * 
     * This size include all the metadata that is added by the SerializedObject to the payload.
     * @return The size of the payload.
     */
    inline size_t size(void) const
    {
        return _data.size();
    }

    /**
     * @brief Clear the SerializedObject and resets it to its default values.
     */
    inline void clear()
    {
        _data.clear();
        _isChecksumRemoved = false;
    }

    /**
     * @brief Assign values to the SerializedObject.
     * @param other SerializedObject to be copied from
     */
    inline SerializedObject& operator=(const SerializedObject& other)
    {
        _data = other._data;
        _isChecksumRemoved = other._isChecksumRemoved;
        return *this;
    }

    /**
     * @brief Assign a copy of data to the payload.
     * 
     * This function erase the current payload.\n
     * If data is not a payload extracted with SerializedObject::payload(),
     * behaviour might be undefined.
     * @param data payload the SerializedObject will be assigned.
     */
    inline SerializedObject& operator=(const char* data)
    {
        constructFromPayload(data);
        return *this;
    }

    /**
     * @brief Remove Checksum at the beggining of payload.
     * 
     * If checksum has already been removed, nothing is done.\n
     * This function should not be used by user end, except for extremely rare case.
     */
    inline void removeChecksum(void)
    {
        if (_isChecksumRemoved)
            return;
        _isChecksumRemoved = true;
        for (size_t i = 0; i < _BAS_CHECKSUM_SIZE_; i++)
            _data.erase(_data.begin());
    }

    /**
     * @brief Add Checksum at the beggining of payload.
     * 
     * If checksum has already been added, nothing is done.\n
     * This function should not be used by user end, except for extremely rare case.
     */
    inline void addChecksum(void)
    {
        if (!_isChecksumRemoved)
            return;
        for (size_t i = 0; i < _BAS_CHECKSUM_SIZE_; i++)
            _data.insert(_data.begin(), 0);
        checksumUpdate();
    }

private:
    inline void prepareChecksum(void)
    {
        for (size_t i = 0; i < _BAS_CHECKSUM_SIZE_; i++)
            _data.push_back(0);
    }

    inline void constructFromPayload(const char* data)
    {
        size_t size = 0;
        for (size_t i = 0; i < _BAS_CHECKSUM_SIZE_; i++) {
            size |= (data[i] << (i * 8));
        }
        _data.assign(data, data + size + _BAS_CHECKSUM_SIZE_);
    }

    inline void checksumUpdate(void)
    {
        size_t size = _data.size();

        if (_isChecksumRemoved)
            addChecksum();

        for (size_t i = 0; i < _BAS_CHECKSUM_SIZE_; i++) {
            _data[i] = (size >> (i * 8));
        }
    }

    inline void pushRawData(size_t size, size_t array_size, const char* data)
    {
        for (size_t j = 0; j < array_size; j++) {
            for (size_t i = 0; i < size; i++) {
                _data.push_back(data[j * size + i]);
            }
        }
    }

    inline void pushSizes(size_t size, size_t array_size)
    {
        for (size_t i = 0; i < _BAS_SIZE_BYTES_; i++)
            _data.push_back((size >> (i * 8)) & 0xFF);

        for (size_t i = 0; i < _BAS_ARRAY_SIZE_; i++)
            _data.push_back((array_size >> (i * 8)) & 0xFF);
    }

    inline std::tuple<int, int> getSizes(void)
    {
        size_t size = 0, array_size = 0;

        if (!_isChecksumRemoved) {
            removeChecksum();
        }

        for (size_t i = 0; i < _BAS_SIZE_BYTES_; i++) {
            size |= (_data.front() << (i * 8));
            _data.erase(_data.begin());
        }

        for (size_t i = 0; i < _BAS_ARRAY_SIZE_; i++) {
            array_size |= (_data.front() << (i * 8));
            _data.erase(_data.begin());
        }
        return std::make_tuple(size, array_size);
    }

    template <typename T>
    inline void copyMem(size_t size, size_t array_size, T* var)
    {
        for (size_t j = 0; j < array_size; j++) {
            std::memcpy(var + j, _data.data(), size);
            for (size_t i = 0; i < size; i++) {
                _data.erase(_data.begin());
            }
        }
    }

    std::vector<char> _data;
    bool _isChecksumRemoved = false;

    template <typename T>
    friend class Helper;
    friend class Helper<std::string>;
};

/**
 * @brief The Serializable class, when herited from, allows you to
 * serialize your own classes.
 * @see makeSerialization()
 * @see makeUnserialization()
 */
class Serializable {
public:

    /**
     * @brief Default destructor.
     */
    virtual ~Serializable() = default;

    /**
     * @brief Allow serialization of your class.
     * 
     * This function should be overriden, and contain the serialization
     * process of your class.\n Note that this function is called by serialize() 
     * should not be called directly bu user end. (see Examples)
     * @param obj The object to push the data into.
     */
    virtual void makeSerialization(SerializedObject& obj) = 0;

    /**
     * @brief Allow unserialization of your class.
     * 
     * This function should be overriden, and contain the unserialization
     * process of your class.\n Note that this function is called by unserialize() 
     * should not be called directly bu user end. (see Examples)
     * @param obj The object to pop the data from.
     */
    virtual void makeUnserialization(SerializedObject& obj) = 0;

    /**
     * @brief Reconstruct object from a SerializedObject
     * 
     * This function copies the SerializedObject before unserializing,
     * meaning that the same object can be used to unserialize multiple times.\n
     * The unserializing process of this function is defined by overriding makeUnserialization.
     * @param obj SerializedObject to unserialize from.
     */
    inline void unserialize(SerializedObject obj)
    {
        makeUnserialization(obj);
    }

    /**
     * @brief Serialize the class.
     * 
     * This function create a SerializedObject of your class.\n
     * The serializing process of this function is defined by overriding makeSerialization.
     * @return The SerializedObject containing the data of the class.
     */
    inline SerializedObject serialize(void)
    {
        SerializedObject obj;
        makeSerialization(obj);
        return obj;
    }

private:
};

/**
 * @brief The PoppedArray is the return value of SerializedObject::popDataArray<>().
 * 
 * The PoppedArray contains the data popped by popDataArray<>() along with
 * the size of the popped data.
 */
template <typename T>
class PoppedArray {

public:
/// \cond
    PoppedArray() = default;

    inline PoppedArray(size_t size, T* ptr)
        : _size(size)
        , _ptr(ptr)
    {
    }
/// \endcond

    /**
     * @brief Returns the size of the array popped by popDataArray<>().
     * 
     * @return size of the popped array.
     */

    inline size_t size(void)
    {
        return _size;
    }

    /**
     * @brief Returns the pointer to the array popped by popDataArray<>().
     * 
     * Be mindful that this memory space is destroyed with the PoppedArray.
     * 
     * @return Pointer to the memory space containing the popped array.
     */
    inline T* get(void)
    {
        return _ptr.get();
    }

private:
    size_t _size;
    std::shared_ptr<T> _ptr;
};


/**
 * @brief Helper class are used by the SerializedObject to allow partial
 * specialization of SerializedObject::pushData<T>() and SerializedObject::popData<T>(), and should not be used by
 * user end.
 */
template <typename T>
class Helper {
public:
    inline void pushData(SerializedObject& obj, const T& data_)
    {
        const char* data = (const char*)&data_;
        size_t size = sizeof(T);
        size_t array_size = 1;

        obj.pushSizes(size, array_size);

        obj.pushRawData(size, array_size, data);

        obj.checksumUpdate();
    }

    inline T popData(SerializedObject& obj)
    {
        size_t size = 0;
        size_t array_size = 0;
        T var;

        std::tie(size, array_size) = obj.getSizes();

        obj.copyMem(size, array_size, &var);

        return var;
    }

};
/// \cond
template <>
class Helper<std::string> {
public:
    inline void pushData(SerializedObject& obj, const std::string& data_)
    {
        const char* data = data_.c_str();
        size_t size = sizeof(char);
        size_t array_size = data_.size() + 1;
        
        obj.pushSizes(size, array_size);

        obj.pushRawData(size, array_size, data);

        obj.checksumUpdate();
    }

    inline std::string popData(SerializedObject &obj)
    {
        size_t size = 0;
        size_t array_size = 0;
        std::string str;
        char* var;

        std::tie(size, array_size) = obj.getSizes();
    
        var = new char[array_size];
    
        obj.copyMem(size, array_size, var);
    
        str = var;
    
        delete var;
    
        return str;
    }

};

template <typename T>
class Helper<std::vector<T>> {
public:
    inline void pushData(SerializedObject& obj, const std::vector<T>& data_)
    {
        const char* data = (char*)data_.data();
        size_t size = sizeof(T);
        size_t array_size = data_.size();
        
        obj.pushSizes(size, array_size);

        obj.pushRawData(size, array_size, data);

        obj.checksumUpdate();
    }

    inline std::vector<T> popData(SerializedObject &obj)
    {
        size_t size = 0;
        size_t array_size = 0;
        std::vector<T> vector;
        T* var;

        std::tie(size, array_size) = obj.getSizes();
    
        var = new T[array_size];
    
        obj.copyMem(size, array_size, var);
    
        vector.assign(var, var + array_size);
    
        delete var;
    
        return vector;
    }

};

template <>
class Helper<SerializedObject> {
public:
    inline void pushData(SerializedObject& obj, const SerializedObject& data_)
    {
        const char* data = (char*)data_.payload();
        size_t size = sizeof(char);
        size_t array_size = data_.size();
        
        obj.pushSizes(size, array_size);

        obj.pushRawData(size, array_size, data);

        obj.checksumUpdate();
    }

    inline SerializedObject popData(SerializedObject &obj)
    {
        size_t size = 0;
        size_t array_size = 0;
        SerializedObject _obj;
        char* var;

        std::tie(size, array_size) = obj.getSizes();
    
        var = new char[array_size];
    
        obj.copyMem(size, array_size, var);
    
        _obj = var;
    
        delete var;
    
        return _obj;
    }

};
/// \endcond
}

/**
 * @example serialization.cpp
 * A simple example on how to serialize a class.
 */

/**
 * @example pushpopsimple.cpp
 * A simple example on how to use pushData and popData.
 */

/**
 * @brief This example shows how to push and pop c-style arrays.\n 
 * 
 * This method is suited for every type of arrays, and is the workaround
 * for every type that are not explicitly handled by the library.
 * @example pushpoparray.cpp
 */

/**
 * @brief Let's take our first example and push it a little further.\n
 * Let's say we want our SerializablePerson to hold a wallet class, since our wallet class won't
 * be a primary type nor a c-style array, we won't be able to push the wallet into the SerializedObject
 * by directly calling pushData.\n
 * Fortunately, the library provide an handy solution:
 * @example intrication.cpp
 */

/**
 * @mainpage
 * Simple example to start with:
 * @code
#include "bas.hpp"

#include <string>

class SerializablePerson : public bas::Serializable {                        // inherit from the Serializable class
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
 *@endcode
 */
#endif /* !BAS_HPP_ */


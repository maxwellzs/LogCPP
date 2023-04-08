//
// Created by yangj on 2023/3/17.
//

#include <string>
#include <cstring>
#include <vector>
#include <exception>
#include <map>
#include <iostream>
#include <fstream>

#ifndef JSONCPP_JSON_H
#define JSONCPP_JSON_H

#define SAFE_DELETE(x) if(!x){delete x;}
#define IS_DIGITAL(x) ((x=='-')||(x=='0')||(x=='1')||(x=='2')|| \
                (x=='3')||(x=='4')||(x=='5')||(x=='6')||(x=='7')||(x=='8')||(x=='9'))
#define IS_FLOAT(x) (IS_DIGITAL(x)||(x=='.'))

namespace JsonCPP {

    // can be adjusted later
    static int RECURSIVE_MAX_DEEP = 10000;

    /*!
     * class contains util functions
     */
    class JsonUtils {
    private:
        JsonUtils();
    public:
        /*!
         * indicate whether the following data is an integer
         * @param ptr pointing to the start of the data
         * @return the end of the data
         */
        static char * NextInteger(char * ptr);
        /*!
         * indicates the next float data
         * @param ptr the start of the parsing
         * @return the end of the float
         */
        static char * NextFloat(char * ptr);
        /*!
         * indicates where the next string ends
         * @param ptr the pointer pointing to the first byte of the string charactoer
         * @return a pointer pointing to " or '
         */
        static char * NextString(char * ptr);
        static char * NextBoolean(char * ptr);
        static char * NextNull(char * ptr);
        static char * NextArray(char * ptr);
        static char * NextObject(char * ptr);
        /*!
         * copy the data between the two pointers, returning a new buffer with the data in between
         * the data in the end pointer will not be copied
         * this method will set the buffer to all 0x0
         * @param start start of the data
         * @param end end of the data
         * @return the new buffer, must be deleted afterwards
         */
        static char * CopyToBuffer(char * start,char * end);
    };

    class JsonParsingException : public std::exception {
    private:
        std::string msg;
    public:
        JsonParsingException(const std::string& msg);
        virtual const char* what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT;
    };

    const static char ARRAY_START       = '[';
    const static char ARRAY_END         = ']';
    const static char OBJECT_START      = '{';
    const static char OBJECT_END        = '}';
    const static char DIVIDE            = ',';
    const static char FIELD             = ':';
    const static char SPACE             = ' ';
    const static char SINGLE_QUOTE      = '\'';
    const static char DOUBLE_QUOTE      = '"';
    const static char RETURN            = '\r';
    const static char NEW_LINE          = '\n';
    const static char ESCAPE            = '\\';
    const static char B_ZERO            = 0x0;

    /*!
     * the types of the json elements
     */
    typedef enum {
        Integer,
        Float,
        Boolean,
        Object,
        Array,
        Null,
        String,
        Invalid
    } TYPES;

    /*!
     * the base class of all json objects
     * contains a TYPE value that indicates the type of json element
     * contains a GetValue member function returning the value of the instance
     */
    class JsonInstance {
    protected:
        TYPES type;
    public:
        TYPES GetType();
        /*!
         * turn the instance in to the corresponding string
         * @return the string version of this instance
         */
        virtual std::string ToString() const;
    };

    /*!
     * when invoking the method in the invalid object, throw this exception
     */
    class InvalidJsonObjectException : public std::exception {
        virtual const char*
        what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT override;
    };

    /*!
     * call when reached recursive limit
     * when there are too much inherit array or object
     */
    class RecursiveException : public std::exception {
        virtual const char*
        what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT override;
    };

    /*!
     * when query has no result, like out of bound, return this object
     */
    class JsonInvalid : public JsonInstance {
    public:
        JsonInvalid();
        virtual std::string ToString() const override {
            // invalid calling !
            throw InvalidJsonObjectException();
        }
    };

    // when meet invalid query return this object
    const static JsonInstance* INVALID_INSTANCE = new JsonInvalid();

    /*!
     * The integer value in the json object
     */
    class JsonInteger : public JsonInstance {
    private:
        int value;
    public:
        JsonInteger(const int& value);
        int GetValue() const;
        virtual std::string ToString() const override;
    };

    /*!
     * The float value in json object
     */
    class JsonFloat : public JsonInstance {
    private:
        float value;
    public:
        JsonFloat(const float& value);
        float GetValue() const;
        virtual std::string ToString() const override;
    };

    /*!
     * true or false in json object
     */
    class JsonBoolean : public JsonInstance {
    private:
        bool value;
    public:
        JsonBoolean(const bool& value);
        bool GetValue() const;
        virtual std::string ToString() const override;
    };

    /*!
     * null value in json object
     */
    class JsonNull : public JsonInstance {
    public:
        JsonNull();
        virtual std::string ToString() const override;
    };

    class JsonString : public JsonInstance {
    private:
        std::string value;
    public:
        /*!
         * create the string object with c++ string
         * @param value the content
         */
        JsonString(std::string& value);
        /*!
         * the c type constructor
         * @param value c type content
         */
        JsonString(const char * value);
        virtual std::string ToString() const override;
    };

    typedef std::vector<JsonInstance *>::const_iterator JsonArrayIterator;

    class JsonArray : public JsonInstance {
    private:
        // inner array stored the data
        // must be cleaned when deconstruct
        std::vector<JsonInstance *> values;
    public:
        /*!
         * create empty array
         */
        JsonArray();
        ~JsonArray();
        virtual std::string ToString() const override;
        void AppendElement(JsonInstance& element);
        JsonInstance& operator [](int index);
        JsonArrayIterator Begin();
        JsonArrayIterator End();
        size_t Size();
    };

    class JsonObject : public JsonInstance {
    private:
        // store every value and key in a map
        std::map<std::string,JsonInstance *> fields;
    public:
        /*!
         * create an empty object
         */
        JsonObject();
        ~JsonObject();
        /*!
         * query the value by string key
         * @param key the key of the field
         * @return the object, not found then return INVALID
         */
        JsonInstance& operator [](std::string& key);
        JsonInstance& operator [](const std::string& key);
        /*!
         * set the field by key
         * @param key the key of the field
         * @param value the value of the field
         * @return the previous value of the key, if the key don't exist, then return nullptr
         */
        JsonInstance * SetField(std::string& key,JsonInstance* value);
        size_t Size();
        virtual std::string ToString() const override;
    };

    /*!
     * this factory class is used to create the complex objects
     * such as array or objects
     */
    class JsonFactory {
    private:
        /*!
         * don't allow creation of the factory instance
         */
        JsonFactory();
    public:
        /*!
         * creat json array from data in buffer
         * if any exception occurred , use the macro SAFE_DELETE out of the try-catch block
         * @param buf the buffer
         * @param bufSize the size of the buffer
         * @return a JsonArray object pointer, should be released afterward
         */
        static JsonArray * CreateJsonArray(char *buf, int bufSize);
        /*!
         * create an object from the data in a buffer
         * if any exception occurred , use the macro SAFE_DELETE out of the try-catch block to prevent leak
         * @param buf start of the buffer
         * @param bufSize the size of the buffer
         * @return the object created, should be deleted
         */
        static JsonObject * CreateJsonObject(char *buf, int bufSize);
        /*!
         * build a json object from a .json file
         * if failed to open file, then return nullptr
         * @param fPath the path use to create the json object
         * @return the json object created, should be deleted;
         */
        static JsonObject * CreateJsonObject(std::string& fPath);
    };

};

#endif //JSONCPP_JSON_H

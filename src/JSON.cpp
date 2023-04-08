//
// Created by yangj on 2023/3/17.
//

#include "JSON.h"

JsonCPP::JsonFactory::JsonFactory() = default;

JsonCPP::JsonArray * JsonCPP::JsonFactory::CreateJsonArray(char *buf, int bufSize) {
    if(*buf != ARRAY_START) throw JsonParsingException("expecting array start");
    //create new empty array
    JsonArray &final = *(new JsonArray());
    bool closed = false;
    for(char * ptr = buf + 1; ptr != buf + bufSize; ptr ++) {

        if(*ptr == ARRAY_END) {
            closed = true;
            break;
        }

        if(*ptr == DIVIDE) {
            // no first element
            if(final.Size() == 0) {
                delete &final;
                throw JsonParsingException("expecting array element");
            }
            // else continue loop
            continue;
        }

        // no content, continue loop
        if(*ptr == SPACE || *ptr == RETURN || *ptr == NEW_LINE) continue;

        // unclosed array
        if(*ptr == B_ZERO) {
            delete &final;
            throw JsonParsingException("unclosed array");
        }

        // try types
        if(*ptr == DOUBLE_QUOTE) {
            // meet a string
            // if no error occurred, the stringEnd should point at the " character
            char * stringEnd = JsonUtils::NextString(ptr);
            if(stringEnd) {
                if(stringEnd == ptr + 2) {
                    // empty string
                    final.AppendElement(*(new JsonString("")));
                    ptr ++;
                    continue;
                }
                char * buf = JsonUtils::CopyToBuffer(ptr + 1,stringEnd - 1);
                final.AppendElement(*(new JsonString(buf)));
                SAFE_DELETE(buf);
                // continue parsing
                ptr = stringEnd - 1;
                continue;
            }
        }

        try {
            // try array
            if(*ptr == ARRAY_START) {
                char * arrayEnd = JsonUtils::NextArray(ptr);
                if(arrayEnd) {
                    char * temp = JsonUtils::CopyToBuffer(ptr,arrayEnd);
                    JsonArray * inner = JsonFactory::CreateJsonArray(temp,strlen(temp));
                    final.AppendElement(*inner);
                    SAFE_DELETE(temp);
                    // continue parsing
                    ptr = arrayEnd - 1;
                    continue;
                }
            }

            // try object
            if(*ptr == OBJECT_START) {
                char * objectEnd = JsonUtils::NextObject(ptr);
                if(objectEnd) {
                    char * temp = JsonUtils::CopyToBuffer(ptr,objectEnd);
                    JsonObject * inner = JsonFactory::CreateJsonObject(temp,strlen(temp));
                    final.AppendElement(*inner);
                    SAFE_DELETE(temp);
                    ptr = objectEnd - 1;
                    continue;
                }
            }
        } catch (RecursiveException& r) {
            delete &final;
            throw r;
        }


        // trying other types
        char * parseResult = nullptr;
        parseResult = JsonUtils::NextInteger(ptr);
        // integer type
        if(parseResult) {
            char * buf = JsonUtils::CopyToBuffer(ptr,parseResult);
            final.AppendElement(*(new JsonInteger(atoi(buf))));
            SAFE_DELETE(buf);
            // continue parsing
            ptr = parseResult - 1;
            continue;
        }
        parseResult = JsonUtils::NextFloat(ptr);
        //float type
        if(parseResult) {
            char * buf = JsonUtils::CopyToBuffer(ptr,parseResult);
            final.AppendElement(*(new JsonFloat(atof(buf))));
            SAFE_DELETE(buf);
            // continue parsing
            ptr = parseResult - 1;
            continue;
        }
        parseResult = JsonUtils::NextBoolean(ptr);
        // boolean type
        if(parseResult) {
            char * buf = JsonUtils::CopyToBuffer(ptr,parseResult);
            final.AppendElement(*(new JsonBoolean((parseResult - ptr == 4))));
            SAFE_DELETE(buf);
            // continue parsing
            ptr = parseResult - 1;
            continue;
        }
        // null type
        parseResult = JsonUtils::NextNull(ptr);
        if(parseResult) {
            char * buf = JsonUtils::CopyToBuffer(ptr,parseResult);
            final.AppendElement(*(new JsonNull()));
            SAFE_DELETE(buf);
            // continue parsing
            ptr = parseResult - 1;
            continue;
        }
        // unknown type
        // invalid value
        delete &final;
        throw JsonParsingException("expecting array elements or ARRAY_CLOSE");
    }
    if(!closed) {
        delete &final;
        throw JsonParsingException("unclosed array");
    }
    return &final;
}

JsonCPP::JsonObject *JsonCPP::JsonFactory::CreateJsonObject(char *buf, int bufSize) {
    if(*buf != OBJECT_START) throw JsonParsingException("expected object start");
    // create empty object
    JsonObject& final = *(new JsonObject());
    bool closed = false;
    bool key = false;
    bool field = false;
    bool shouldMeetDevide = false;
    // create empty string
    std::string fieldKey;
    // start parsing
    for(char * ptr = buf + 1; ptr != buf + bufSize; ptr ++) {
        // finish parsing
        if(*ptr == OBJECT_END) {
            closed = true;
            break;
        }

        // no content, jump
        if(*ptr == SPACE || *ptr == RETURN || *ptr == NEW_LINE) continue;
        // jump character ',' after parsing a pair
        // only jump once
        if(*ptr == DIVIDE && shouldMeetDevide) {
            shouldMeetDevide = false;
            continue;
        }
        // proceed to find a key
        if(!key) {
            // hasn't found a key, no values should appear
            if(*ptr != DOUBLE_QUOTE) {
                delete &final;
                throw JsonParsingException("expected a key");
            } else {
                // found a key
                char * keyEnd = JsonUtils::NextString(ptr);
                char * temp = JsonUtils::CopyToBuffer(ptr + 1,keyEnd - 1);
                fieldKey.clear();
                fieldKey += temp;
                SAFE_DELETE(temp);
                key = true;
                ptr = keyEnd - 1;
                continue;
            }
        }

        if(!field) {
            // hasn't started field parsing
            if(*ptr != FIELD) {
                delete &final;
                throw JsonParsingException("expected ':' character");
            } else {
                if(field) {
                    delete &final;
                    // two ':' character
                    throw JsonParsingException("excepted value after ':' character");
                }
                field = true;
                continue;
            }
        } else {

#define CONTINUE_LOOP field = false; \
            key = false; \
            shouldMeetDevide = true; \
            continue;

            // has start to parse field
            char * parseResult = nullptr;
            if(*ptr == DOUBLE_QUOTE) {
                // try string
                if((parseResult = JsonUtils::NextString(ptr)) != nullptr) {
                    // if is empty string
                    if(parseResult == ptr + 2) {
                        ptr += 1;
                        final.SetField(fieldKey,new JsonString(""));
                        CONTINUE_LOOP;
                    }
                    // is string element
                    char * value = JsonUtils::CopyToBuffer(ptr + 1,parseResult - 1);
                    final.SetField(fieldKey,new JsonString(value));
                    SAFE_DELETE(value);
                    ptr = parseResult - 1;
                    CONTINUE_LOOP
                }
            }
            try {
                // try array
                if(*ptr == ARRAY_START) {
                    if((parseResult = JsonUtils::NextArray(ptr)) != nullptr) {
                        char * value = JsonUtils::CopyToBuffer(ptr,parseResult);
                        JsonArray * inner = JsonFactory::CreateJsonArray(value, strlen(value));
                        final.SetField(fieldKey,inner);
                        SAFE_DELETE(value);
                        ptr = parseResult - 1;
                        CONTINUE_LOOP
                    }
                }
                // try object
                if(*ptr == OBJECT_START) {
                    if((parseResult = JsonUtils::NextObject(ptr)) != nullptr) {
                        char * value = JsonUtils::CopyToBuffer(ptr,parseResult);
                        JsonObject * inherit = JsonFactory::CreateJsonObject(value,strlen(value));
                        final.SetField(fieldKey,inherit);
                        SAFE_DELETE(value);
                        ptr = parseResult - 1;
                        CONTINUE_LOOP;
                    }
                }
            } catch(RecursiveException& r) {
                // reach max deep
                delete &final;
                throw r;
            }

            // try integer
            if((parseResult = JsonUtils::NextInteger(ptr)) != nullptr) {
                char * value = JsonUtils::CopyToBuffer(ptr,parseResult);
                final.SetField(fieldKey,new JsonInteger(std::atoi(value)));
                SAFE_DELETE(value);
                ptr = parseResult - 1;
                CONTINUE_LOOP;
            }
            // try float
            if((parseResult = JsonUtils::NextFloat(ptr)) != nullptr) {
                char * value = JsonUtils::CopyToBuffer(ptr,parseResult);
                final.SetField(fieldKey,new JsonFloat(std::atof(value)));
                SAFE_DELETE(value);
                ptr = parseResult - 1;
                CONTINUE_LOOP;
            }
            // try boolean
            if((parseResult = JsonUtils::NextBoolean(ptr)) != nullptr) {
                char * value = JsonUtils::CopyToBuffer(ptr,parseResult);
                final.SetField(fieldKey,new JsonBoolean(strcmp(value,"true") == 0));
                SAFE_DELETE(value);
                ptr = parseResult - 1;
                CONTINUE_LOOP;
            }
            // try null
            if((parseResult = JsonUtils::NextNull(ptr)) != nullptr) {
                char * value = JsonUtils::CopyToBuffer(ptr,parseResult);
                final.SetField(fieldKey,new JsonNull());
                SAFE_DELETE(value);
                ptr = parseResult - 1;
                CONTINUE_LOOP;
            }
            // unknown type
            delete &final;
            throw JsonParsingException("excepted an OBJECT_END or a value");

        }

    }
    if(!closed) {
        delete &final;
        throw JsonParsingException("unclosed object");
    }
    // return the parsing result
    return &final;
}

JsonCPP::JsonObject *JsonCPP::JsonFactory::CreateJsonObject(std::string &fPath) {
    std::ifstream inputFile;
    inputFile.open(fPath,std::ios::in);
    if(!inputFile.is_open()) return nullptr;
    
    // inputFile.unsetf(std::ios::skipws);

    std::string builder,buffer;
    while(std::getline(inputFile,buffer)) {
        builder += buffer;
        buffer.clear();
    }

    // call parsing method
    return JsonFactory::CreateJsonObject(const_cast<char *>(builder.c_str()),builder.length());
}

JsonCPP::TYPES JsonCPP::JsonInstance::GetType() {
    return this->type;
}

std::string JsonCPP::JsonInstance::ToString() const {
    // nothing was provided here
    std::string temp = "null";
    return temp;
}

JsonCPP::JsonInteger::JsonInteger(const int& value) {
    this->value = value;
    this->type = TYPES::Integer;
}

int JsonCPP::JsonInteger::GetValue() const {
    return this->value;
}

std::string JsonCPP::JsonInteger::ToString() const {
    std::string temp = std::to_string(value);
    return temp;
}

JsonCPP::JsonFloat::JsonFloat(const float& value) {
    this->value = value;
    this->type = TYPES::Float;
}

float JsonCPP::JsonFloat::GetValue() const {
    return this->value;
}

std::string JsonCPP::JsonFloat::ToString() const {
    std::string temp = std::to_string(value);
    return temp;
}

JsonCPP::JsonBoolean::JsonBoolean(const bool& value) {
    this->value = value;
    this->type = TYPES::Boolean;
}

bool JsonCPP::JsonBoolean::GetValue() const {
    return this->value;
}

std::string JsonCPP::JsonBoolean::ToString() const {
    std::string raw(value?"true":"false");
    return raw;
}

JsonCPP::JsonString::JsonString(std::string &value) {
    this->value = std::string(value);
    this->type = TYPES::String;
}

JsonCPP::JsonString::JsonString(const char *value) {
    std::string temp(value);
    new (this) JsonString(temp);
}

std::string JsonCPP::JsonString::ToString() const {
    std::string temp = value;
    return temp;
}

std::string JsonCPP::JsonNull::ToString() const {
    std::string temp("null");
    return temp;
}

JsonCPP::JsonNull::JsonNull() {
    this->type = TYPES::Null;
}

JsonCPP::JsonArray::JsonArray() {
    // create empty array
    values = std::vector<JsonInstance *>();
    this->type = TYPES::Array;
}

JsonCPP::JsonArray::~JsonArray() {
    // clean every element in the array
    for(JsonArrayIterator i = values.cbegin(); i != values.cend(); i++) {
        delete *i;
    }
}

std::string JsonCPP::JsonArray::ToString() const {
    std::string result;
    result += ARRAY_START;
    for(JsonArrayIterator i = values.cbegin(); i != values.cend(); i++) {
        // append every element
        result += (*i)->ToString();
        // after every element but the last one append ','
        if(i != --values.cend()) result += DIVIDE;
    }
    result += ARRAY_END;
    return result;
}

void JsonCPP::JsonArray::AppendElement(JsonCPP::JsonInstance &element) {
    this->values.push_back(&element);
}

JsonCPP::JsonInstance& JsonCPP::JsonArray::operator[](int index) {
    // check out of bound
    if(index >= values.size()) return const_cast<JsonInstance &>(*INVALID_INSTANCE);
    JsonInstance * instance = values.at(index);
    return *instance;
}

JsonCPP::JsonArrayIterator JsonCPP::JsonArray::Begin() {
    return values.cbegin();
}

JsonCPP::JsonArrayIterator JsonCPP::JsonArray::End() {
    return values.cend();
}

size_t JsonCPP::JsonArray::Size() {
    return values.size();
}

JsonCPP::JsonParsingException::JsonParsingException(const std::string &msg) {
    this->msg = std::string(msg);
}

const char *JsonCPP::JsonParsingException::what() const noexcept {
    return msg.c_str();
}

JsonCPP::JsonUtils::JsonUtils() = default;

char *JsonCPP::JsonUtils::NextInteger(char * ptr) {
   for(char * p = ptr;;p++) {
       if(*p==SPACE||*p==DIVIDE||*p==RETURN||*p==NEW_LINE||
       *p==ARRAY_END||*p==OBJECT_END) {
           // number end
           return p;
       }
       // not a ditigal
       if(p != ptr && *p == '-') break;
       if(!IS_DIGITAL(*p)) break;
   }
   return nullptr;
}

char *JsonCPP::JsonUtils::CopyToBuffer(char *start, char *end) {
    char * newBuf = new char[end - start + 1];
    memset(newBuf,0x0,end - start + 1);
    int writeIndex = 0;
    for(char * p = start;p < end;p++) {
        if(*p==ESCAPE && (p[1] == SINGLE_QUOTE||p[1] == DOUBLE_QUOTE)) {
            // jump ' and "
            writeIndex ++;
            continue;
        }
        newBuf[p - start - writeIndex] = *p;
    }
    return newBuf;
}

char *JsonCPP::JsonUtils::NextFloat(char *ptr) {
    for(char * p = ptr;;p++) {
        if(*p==SPACE||*p==DIVIDE||*p==RETURN||*p==NEW_LINE||
           *p==ARRAY_END||*p==OBJECT_END) {
            // number end
            return p;
        }
        if(p != ptr && *p == '-') break;
        if(!IS_FLOAT(*p)) break;
    }
    return nullptr;
}

char *JsonCPP::JsonUtils::NextString(char *ptr) {
    // not a valid string
    if(*ptr != DOUBLE_QUOTE) return nullptr;
    // start must be the same as the end
    for(char * p = ptr+1;;p ++) {
        // string not closed or meet invalid character
        if(*p == B_ZERO || *p == NEW_LINE) return nullptr;
        // is the string escaped ?
        if(*p==DOUBLE_QUOTE) {
            // is the previous "\" ?
            if(*(p-1)!=ESCAPE) {
                // the end of the string
                return p + 1;
            }
        }
    }
}

char *JsonCPP::JsonUtils::NextBoolean(char *ptr) {
    char temp[6];
    memset(temp,0x0,6);
    memcpy(temp,ptr,4);
    // is the value true ?
    if(strcmp(temp,"true") == 0) return ptr + 4;
    // is the value false ?
    memcpy(temp,ptr,5);
    if(strcmp(temp,"false") == 0) return ptr + 5;
    // not boolean value
    return nullptr;
}

char *JsonCPP::JsonUtils::NextNull(char *ptr) {
    char temp[5];
    memset(temp,0x0,5);
    memcpy(temp,ptr,4);
    if(strcmp(temp,"null") == 0) return ptr + 4;
    // not null
    return nullptr;
}

char *JsonCPP::JsonUtils::NextArray(char *ptr) {
    // not a array
    if(*ptr != ARRAY_START) return nullptr;
    // count the sub array in this array
    int openCount = 0;
    for(char * p = ptr + 1;;p++) {
        // meet a subarray
        if(*p == ARRAY_START) openCount ++;
        if(openCount >= RECURSIVE_MAX_DEEP) throw RecursiveException();
        if(*p == B_ZERO) return nullptr;
        // finish parsing when closing all array
        if(*p == ARRAY_END) {
            if(openCount == 0) return p + 1;
            openCount --;
        }
    }
}

char *JsonCPP::JsonUtils::NextObject(char *ptr) {
    if(*ptr != OBJECT_START) return nullptr;
    // has meet 0 inherit object
    int openCount = 0;
    for(char * p = ptr + 1;;p++) {
        if(*p == OBJECT_START) {
            if(openCount >= RECURSIVE_MAX_DEEP) throw RecursiveException();
            // meet inherit object, then increase the count
            openCount ++;
        }
        if(*p == B_ZERO) return nullptr;
        if(*p == OBJECT_END) {
            // the parent array has already been closed
            // finish parsing
            if(openCount == 0) return p + 1;
            // else decrease the count
            // closing sub object
            openCount --;
        }
    }
}

JsonCPP::JsonObject::JsonObject() {
    // create empty object
    fields = std::map<std::string,JsonInstance *>();
    this->type = TYPES::Object;
}

JsonCPP::JsonObject::~JsonObject() {
    for(auto i = fields.cbegin();i!=fields.cend();i++) {
        // delete value
        // no need to release key
        delete i->second;
    }
}

JsonCPP::JsonInstance &JsonCPP::JsonObject::operator[](std::string &key) {
    auto result = fields.find(key);
    if(result == fields.end()) return const_cast<JsonInstance&>(*INVALID_INSTANCE);
    return *(result->second);
}

JsonCPP::JsonInstance * JsonCPP::JsonObject::SetField(std::string &key, JsonCPP::JsonInstance *value) {
    auto hasExist = fields.find(key);
    JsonInstance * previous = nullptr;
    if(hasExist != fields.end()) {
        // extract the pointer
        previous = hasExist->second;
        // already exist in the object, then alter the value
        fields.erase(hasExist);
    }
    // insert the new key into the field map
    fields.insert(std::make_pair(key,value));
    // if remove an pair, then return the value of it
    return previous;
}

size_t JsonCPP::JsonObject::Size() {
    return fields.size();
}

std::string JsonCPP::JsonObject::ToString() const {
    std::string builder = "{";
    for(auto i = fields.begin();i!=fields.end();i++) {
        bool isStringElement = (i->second->GetType() == TYPES::String);
        builder += DOUBLE_QUOTE;
        builder += i->first;
        builder += DOUBLE_QUOTE;
        builder += FIELD;
        if(isStringElement) builder += DOUBLE_QUOTE;
        builder += i->second->ToString();
        if(isStringElement) builder += DOUBLE_QUOTE;
        if(i != --fields.end()){
            builder += DIVIDE;
        }
    }
    builder += OBJECT_END;
    return builder;
}

JsonCPP::JsonInstance &JsonCPP::JsonObject::operator[](const std::string &key) {
    return (*this)[const_cast<std::string&>(key)];
}

JsonCPP::JsonInvalid::JsonInvalid() {
    this->type = TYPES::Invalid;
}

const char *JsonCPP::InvalidJsonObjectException::what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT {
    return "Calling the method from an invalid object";
}

const char *JsonCPP::RecursiveException::what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT {
    return "Reached Recursive limit when parsing json data";
}

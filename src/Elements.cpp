#include "Elements.h"

using namespace LogCPP;

void BaseElement::CompileElement(std::string &builder)
{
    // pass down the builder to next element, nothing is append here
    if (next)
        next->CompileElement(builder);
}

BaseElement &LogCPP::BaseElement::operator<<(BaseElement *newElement)
{
    // TODO: 在此处插入 return 语句
    if (newElement->next != nullptr || (newElement == this))
    {
        // dupilcated!
        return *this;
    }
    if (!next)
    {
        // the last
        next = newElement;
        // return for further appending
        return *newElement;
    }
    else
    {
        // pass this elemet till final
        return (*next) << newElement;
    }
}

BaseElement::~BaseElement()
{
    if (next)
        delete next;
}

void LogCPP::BaseElement::ClearNext()
{
    this->next = nullptr;
}

const std::string LogCPP::LevelElement::GetLevelString() const
{
    return LEVEL_STRINGS.at(this->currentlevel);
}

LogCPP::LevelElement::LevelElement(LEVELS level)
{
    this->currentlevel = level;
}

void LogCPP::LevelElement::CompileElement(std::string &builder)
{
    builder += START_ELEMENT;
    builder += GetLevelString();
    builder += END_ELEMENT;
    // call super to pass down the parameter
    BaseElement::CompileElement(builder);
}

std::string LogCPP::TimeElement::GetCurrentTime() const
{
    // create empty string
    std::string result;
    // read out the time
    time_t rawTime;
    struct tm *info;
    rawTime = time(NULL);
    info = localtime(&rawTime);
    const static size_t PARSING_BUFFER_SIZE = 128;
    char buffer[PARSING_BUFFER_SIZE];
    // reset buffer
    memset(buffer, 0, PARSING_BUFFER_SIZE);

    strftime(buffer, PARSING_BUFFER_SIZE - 1, format.c_str(), info);

    // append final
    result += buffer;

    return result;
}

LogCPP::TimeElement::TimeElement(const std::string &format)
{
    // record the format
    this->format = std::string(format);
}

void LogCPP::TimeElement::CompileElement(std::string &builder)
{
    builder += START_ELEMENT;
    builder += GetCurrentTime();
    builder += END_ELEMENT;
    // call super
    BaseElement::CompileElement(builder);
}

LogCPP::ConstantStringElement::ConstantStringElement(const std::string &value)
{
    this->content = std::string(value);
}

void LogCPP::ConstantStringElement::CompileElement(std::string &builder)
{
    // nothing extra
    builder += content;
    BaseElement::CompileElement(builder);
}

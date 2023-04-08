#include "LogStream.h"

using namespace LogCPP;

BaseStream &ConsoleStream::operator<<(const std::string &content)
{
    // TODO: 在此处插入 return 语句
    // redirect the log content into the console
    std::cout << content << std::endl;
    // call super
    return BaseStream::operator<<(content);
}

ConsoleStream::ConsoleStream()
{
    // redirect the log into terminal
}

LogFileOpenException::LogFileOpenException(const std::string &prefix)
{
    // mark the file prefix failed to be opened
    msg += "Error when creating file stream! can't open file \"";
    msg += prefix;
    msg += "\"";
}

const char *LogFileOpenException::what() const noexcept
{
    // return the error messages
    return msg.c_str();
}

FileStream::FileStream(const std::string &prefix)
{
    this->prefix = std::string(prefix);
    // can't open the file
    if (!openFile())
        throw LogFileOpenException(prefix);
}

FileStream::~FileStream()
{
    // finish up
    if (output.is_open())
        output.close();
}

BaseStream &FileStream::operator<<(const std::string &content)
{
    // TODO: 在此处插入 return 语句
    output << content << std::endl;
    // call super
    return BaseStream::operator<<(content);
}

bool FileStream::openFile()
{
    output.open(prefix, std::ios::app | std::ios::out);
    // try to open the file
    return output.is_open();
}

#include <iostream>
#include <sstream>
#include "JSON.h"
#include "LogStream.h"
#include "Elements.h"
#include "Loggers.h"
#include "Global.h"

using namespace LogCPP;

void test_streams();
void test_element();
void test_logger();

int main(int args, char *argv[])
{
    test_logger();
    return 0x0;
}

void test_streams()
{
    ConsoleStream s1;
    FileStream s2("test.log");
    s1 << "sample text 1"
       << "sample text 2";
    s2 << "sample text 1"
       << "sample text 2";
}

void test_element()
{
    ConsoleStream s1;
    BaseElement *e = new LevelElement(INFO);
    (*e) << new TimeElement("%Y-%m-%d %H:%M:%S") << new TimeElement("%Y-%m-%d %H:%M:%S") << new ConstantStringElement("I am constant");
    std::string res;
    e->CompileElement(res);
    s1 << res;
    delete e;
}

void test_logger()
{
    BaseLogger::SetConfigPath("logger.json");
    for (size_t i = 0; i < 1000; i++)
    {
        std::stringstream s;
        s << i;
        std::string data = s.str();
        /* code */
        INFO_ASYN(data);
        DEBUG_SYN(data);
        WARNING_ASYN(data);
        ERROR_ASYN(data);
        FATAL_ASYN(data);
    }
    CLEAN_UP;
}

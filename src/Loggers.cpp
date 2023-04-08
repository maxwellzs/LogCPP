#include "Loggers.h"

std::string LogCPP::BaseLogger::configPath = "logger.json";
LogCPP::SynchronizedLogger *LogCPP::SynchronizedLogger::instance = nullptr;
LogCPP::AsynchronizedLogger *LogCPP::AsynchronizedLogger::instance = nullptr;
std::thread *LogCPP::AsynchronizedLogger::daemon = nullptr;

LogCPP::BaseLogger::~BaseLogger()
{
    for (auto i = streams.begin(); i != streams.end(); i++)
    {
        delete *i;
    }
    // reset the pointer in levels
    for (auto i = levelMap.begin(); i != levelMap.end(); i++)
    {
        if (i == levelMap.begin())
        {
            delete i->second;
        }
        else
        {
            // prevent re-delete
            i->second->ClearNext();
            delete i->second;
        }
    }
}

void LogCPP::BaseLogger::AddStream(BaseStream *newStream)
{
    streams.push_back(newStream);
}

void LogCPP::BaseLogger::AddElement(BaseElement *newElement)
{
    if (!hasElements)
    {
        hasElements = true;
        for (auto i = levelMap.begin(); i != levelMap.end(); i++)
        {
            // iterate each level element
            *(i->second) << newElement;
        }
        return;
    }
    // append after the first
    *(levelMap.at(INFO)) << newElement;
}

void LogCPP::BaseLogger::SetConfigPath(const std::string &path)
{
    // clean previous
    configPath.clear();
    configPath += path;
}

void LogCPP::SynchronizedLogger::Log(LEVELS level, const std::string &msg)
{

    // build final messages
    std::string f;
    levelMap.at(level)->CompileElement(f);
    f += msg;

    // flush contents
    for (auto i = streams.begin(); i != streams.end(); i++)
    {
        if (LEVEL_VALUES.at(level) < filterMap.at(*i))
            continue;
        *(*i) << f;
    }
}

LogCPP::BaseLogger &LogCPP::SynchronizedLogger::GetInstance()
{
    // TODO: 在此处插入 return 语句
    // already created
    if (instance)
        return *(dynamic_cast<BaseLogger *>(instance));
    // load config
    JsonObject *config = nullptr;
    try
    {
        // parsing config
        config = JsonFactory::CreateJsonObject(configPath);
        if (!config)
        {
            throw LoggerConfigException("failed to load logger config at " + configPath);
        }
        JsonObject &configref = *config;

        // no elements
        if (configref["streams"].GetType() != TYPES::Array ||
            configref["elements"].GetType() != TYPES::Array)
        {
            throw LoggerConfigException("missing fields \"streams\" or \"elements\"");
        }

        JsonArray &streamConfig = *(dynamic_cast<JsonArray *>(&configref["streams"]));
        JsonArray &elementConfig = *(dynamic_cast<JsonArray *>(&configref["elements"]));

        // initiate the actual object
        instance = new SynchronizedLogger();

        // iterate streamConfig
        for (auto i = elementConfig.Begin(); i != elementConfig.End(); i++)
        {
            // jump none obejct
            if ((*i)->GetType() != TYPES::Object)
                continue;
            JsonObject &thisStream = *(dynamic_cast<JsonObject *>(*i));
            // missing keys, jump
            if (thisStream["name"].GetType() == TYPES::Invalid ||
                thisStream["value"].GetType() == TYPES::Invalid)
                continue;
            std::string name = thisStream["name"].ToString();
            std::string value = thisStream["value"].ToString();

            // specific elements impl
            if (name == "constantString")
            {
                // case 1 const string
                instance->AddElement(new ConstantStringElement(value));
            }
            else if (name == "time")
            {
                // case 2 time element
                instance->AddElement(new TimeElement(value));
            }
        }

        for (auto i = streamConfig.Begin(); i != streamConfig.End(); i++)
        {
            if ((*i)->GetType() != TYPES::Object)
                continue;
            JsonObject &thisStream = *(dynamic_cast<JsonObject *>(*i));
            // no name fields !
            if (thisStream["name"].GetType() == TYPES::Invalid)
                continue;
            std::string name = thisStream["name"].ToString();
            // default
            int level = 0;
            if (thisStream["level"].GetType() == TYPES::Integer)
                level = dynamic_cast<JsonInteger &>(thisStream["level"]).GetValue();

            // specific stream impl
            if (name == "console")
            {
                BaseStream *s = new ConsoleStream();
                instance->AddStream(s);
                instance->filterMap.insert(std::make_pair(s, level));
            }
            else if (name == "file")
            {
                // there are no dir to output log
                if (thisStream["outputDir"].GetType() == TYPES::Invalid)
                    continue;
                BaseStream *s = new FileStream(thisStream["outputDir"].ToString());
                instance->AddStream(s);
                instance->filterMap.insert(std::make_pair(s, level));
            }
        }
    }
    catch (std::exception &e)
    {
        std::cout << e.what() << std::endl;
        if (config)
            delete config;

        throw LoggerConfigException("failed to load logger config");
    }
    if (config)
        delete config;
    return *(dynamic_cast<BaseLogger *>(instance));
}

LogCPP::LoggerConfigException::LoggerConfigException(const std::string &which)
{
    this->whichConfig += "error when configurating : ";
    this->whichConfig += which;
}

const char *LogCPP::LoggerConfigException::what() const noexcept
{
    return whichConfig.c_str();
}

std::vector<std::string> LogCPP::SynchronizedMessageQueue::PullMessage()
{
    std::unique_lock<std::mutex> lock(mtx);
    while (messages.empty())
    {
        cond.wait(lock);
    }
    // pull all the messages
    std::vector<std::string> rt(messages);
    messages.clear();
    return rt;
}

void LogCPP::SynchronizedMessageQueue::WaitForClean()
{
    std::unique_lock<std::mutex> lock(mtx);
    cond.wait(lock);
}

void LogCPP::SynchronizedMessageQueue::AcknowledgeClean()
{
    cond.notify_all();
}

void LogCPP::SynchronizedMessageQueue::SubmitMessage(const std::string &message)
{
    // lock
    std::unique_lock<std::mutex> lock(mtx);
    // copy the message
    messages.push_back(std::string(message));
    cond.notify_all();
}

LogCPP::AsynchronizedLogger::AsynchronizedLogger()
{
    // start daemon latter
}

void LogCPP::AsynchronizedLogger::Log(LEVELS level, const std::string &msg)
{

    std::string f;
    levelMap.at(level)->CompileElement(f);
    f += msg;
    queue.SubmitMessage(f);
}

void LogCPP::AsynchronizedLogger::SafeDelete()
{
    shouldStop = true;
    queue.WaitForClean();
    delete this;
}

void LogCPP::AsynchronizedLogger::CreateDaemon()
{
    while (true)
    {
        // flush all logs
        std::vector<std::string> pullResult = queue.PullMessage();
        if (pullResult.size() == 0)
            continue;
        for (auto i = pullResult.begin(); i != pullResult.end(); i++)
        {
            // flush all
            for (auto j = streams.begin(); j != streams.end(); j++)
            {
                *(*j) << (*i);
            }
        }
        if (queue.RemainSize() == 0)
        {
            queue.AcknowledgeClean();
            // end thread if no more tasks
            if (shouldStop)
                return;
        }
    }
}

LogCPP::BaseLogger &LogCPP::AsynchronizedLogger::GetInstance()
{
    // TODO: 在此处插入 return 语句
    // already created
    if (instance)
        return *(dynamic_cast<BaseLogger *>(instance));
    // load config
    JsonObject *config = nullptr;
    try
    {
        // parsing config
        config = JsonFactory::CreateJsonObject(configPath);
        if (!config)
        {
            // invalid config
            throw LoggerConfigException("cannot load config at path " + configPath);
        }
        JsonObject &configref = *config;

        // no elements
        if (configref["streams"].GetType() != TYPES::Array ||
            configref["elements"].GetType() != TYPES::Array)
        {
            throw LoggerConfigException("missing fields \"streams\" or \"elements\"");
        }

        JsonArray &streamConfig = *(dynamic_cast<JsonArray *>(&configref["streams"]));
        JsonArray &elementConfig = *(dynamic_cast<JsonArray *>(&configref["elements"]));

        // initiate the actual object
        instance = new AsynchronizedLogger();

        // iterate streamConfig
        for (auto i = elementConfig.Begin(); i != elementConfig.End(); i++)
        {
            // jump none obejct
            if ((*i)->GetType() != TYPES::Object)
                continue;
            JsonObject &thisStream = *(dynamic_cast<JsonObject *>(*i));
            // missing keys, jump
            if (thisStream["name"].GetType() == TYPES::Invalid ||
                thisStream["value"].GetType() == TYPES::Invalid)
                continue;
            std::string name = thisStream["name"].ToString();
            std::string value = thisStream["value"].ToString();

            // specific elements impl
            if (name == "constantString")
            {
                // case 1 const string
                instance->AddElement(new ConstantStringElement(value));
            }
            else if (name == "time")
            {
                // case 2 time element
                instance->AddElement(new TimeElement(value));
            }
        }

        for (auto i = streamConfig.Begin(); i != streamConfig.End(); i++)
        {
            if ((*i)->GetType() != TYPES::Object)
                continue;
            JsonObject &thisStream = *(dynamic_cast<JsonObject *>(*i));
            // no name fields !
            if (thisStream["name"].GetType() == TYPES::Invalid)
                continue;
            std::string name = thisStream["name"].ToString();

            // default
            int level = 0;
            if (thisStream["level"].GetType() == TYPES::Integer)
                level = dynamic_cast<JsonInteger &>(thisStream["level"]).GetValue();

            // specific stream impl
            if (name == "console")
            {
                BaseStream *s = new ConsoleStream();
                instance->AddStream(s);
                instance->filterMap.insert(std::make_pair(s, level));
            }
            else if (name == "file")
            {
                // there are no dir to output log
                if (thisStream["outputDir"].GetType() == TYPES::Invalid)
                    continue;
                BaseStream *s = new FileStream(thisStream["outputDir"].ToString());
                instance->AddStream(s);
                instance->filterMap.insert(std::make_pair(s, level));
            }
        }
    }
    catch (std::exception &e)
    {
        std::cout << e.what() << std::endl;
        if (config)
            delete config;

        throw LoggerConfigException("failed to load logger config");
    }
    if (config)
        delete config;

    // create daemon
    daemon = new std::thread(&AsynchronizedLogger::CreateDaemon, instance);
    // isolate from main thread
    daemon->detach();

    return *(dynamic_cast<BaseLogger *>(instance));
}

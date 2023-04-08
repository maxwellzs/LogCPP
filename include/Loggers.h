/**
 * @file Loggers.h
 * @author maxwellzs
 * @brief this file implements the basic loggers
 * all the loggers are single instanced
 * @version 0.1
 * @date 2023-03-25
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>
#include "LogStream.h"
#include "Elements.h"
#include "JSON.h"

#ifndef Loggers_h
#define Loggers_h

using namespace JsonCPP;

#define SAFE_DELETE_ASYN(ref) (dynamic_cast<LogCPP::AsynchronizedLogger *>(&ref))->SafeDelete();

namespace LogCPP
{

    const static std::map<LEVELS,int> LEVEL_VALUES = {
        {DEBUG,     0},
        {INFO,      1},
        {WARNING,   2},
        {ERROR,     3},
        {FATAL,     4}
    };

    /**
     * @brief throw when can't do config, missing the config file
     *
     */
    class LoggerConfigException : public std::exception
    {
    private:
        std::string whichConfig;

    public:
        LoggerConfigException(const std::string &what);
        const char *
        what() const noexcept override;
    };

    /**
     * @brief the base class implements the basic functions of a logger
     *
     */
    class BaseLogger
    {
    protected:
        /// @brief default config file path
        static std::string configPath;
        /// @brief stores all the stream to where the data will be output
        std::vector<BaseStream *> streams = {};
        /// @brief the map stores the root elements
        /// every logger start with logging the level
        std::map<LEVELS, LevelElement *> levelMap = {
            {DEBUG, new LevelElement(DEBUG)},
            {INFO, new LevelElement(INFO)},
            {WARNING, new LevelElement(WARNING)},
            {ERROR, new LevelElement(ERROR)},
            {FATAL, new LevelElement(FATAL)}};
        /// @brief when no elements, append after every elements in level map
        bool hasElements = false;
        /// @brief record the log level
        /// if the log is bellow the level, the logger will not output
        std::map<BaseStream *,int> filterMap = {};
    public:
        ~BaseLogger();
        // no logs are implemented in this class
        virtual void Log(LEVELS level, const std::string &msg) = 0;
        virtual void SafeDelete() = 0;
        void AddStream(BaseStream *newStream);
        void AddElement(BaseElement *newElement);
        /**
         * @brief Set the Config Path
         *
         * @param path new path
         */
        static void SetConfigPath(const std::string &path);
    };

    /**
     * @brief log synchroncized into the given logging stream
     *
     */
    class SynchronizedLogger : public BaseLogger
    {
    private:
        /// @brief no creating this object from outside
        SynchronizedLogger(){};
        /// @brief no default instancce
        static SynchronizedLogger *instance;

    public:
        // nothing to clear
        void SafeDelete() override{};
        void Log(LEVELS level, const std::string &msg) override;
        /**
         * @brief Get the single instance, if there are no instance, then create
         *
         * @return SynchronizedLogger&
         */
        static BaseLogger &GetInstance();
    };

    /**
     * @brief this inner class implements a synchronized queue that enables the outside thread to
     * submit messages
     *
     */
    class SynchronizedMessageQueue
    {
    private:
        std::vector<std::string> messages;
        std::mutex mtx;
        std::condition_variable cond;

    public:
        /**
         * @brief make this thread wait until there are messages in the queue
         *
         * @return std::vector<std::string> the vector containing the messages
         */
        std::vector<std::string> PullMessage();
        /**
         * @brief submit a message into the queue
         *
         * @param message
         */
        void SubmitMessage(const std::string &message);
        /**
         * @brief calling this method will wait untill all messages are cleaned
         *
         */
        void WaitForClean();
        /**
         * @brief tell the main thread all the data are flushed
         *
         */
        void AcknowledgeClean();
        size_t RemainSize() { return messages.size(); };
    };

    /**
     * @brief log asynchronized into the streams
     * this class requires the pthread libs
     * when calling GetInstance, this class will instantly create a daemon thread
     *
     */
    class AsynchronizedLogger : public BaseLogger
    {

    private:
        bool shouldStop = false;
        /// @brief the only instance
        static AsynchronizedLogger *instance;
        static std::thread *daemon;
        /// @brief no creating this object outside this class
        AsynchronizedLogger();
        /// @brief use to store all the messages
        SynchronizedMessageQueue queue;
        /**
         * @brief Create a Daemon thread after the instance is successfully constructed
         *
         */
        void CreateDaemon();

    public:
        /**
         * @brief Destroy the Asynchronized Logger object,
         * calling this constructor will cause thread block untill all the message
         * are flushed into the stream
         *
         */
        void SafeDelete() override;
        void Log(LEVELS level, const std::string &msg) override;
        static BaseLogger &GetInstance();
    };
    

};

#endif
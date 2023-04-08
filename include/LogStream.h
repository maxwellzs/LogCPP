/**
 * @file LogStream.h
 * @author maxwells Turring
 * @brief this file implements the basic ouput streams, no specific loggers provided
 * @version 0.1
 * @date 2023-03-25
 *
 * @copyright GNU
 *
 */

#include <string>
#include <iostream>
#include <fstream>
#include <exception>
#include <unistd.h>
#include <stdio.h>

#ifndef LogStream_h
#define LogStream_h

namespace LogCPP
{
    /**
     * @brief the base stream interface
     *
     */
    class BaseStream
    {
    public:
        /**
         * @brief the function that implements the io functons
         *
         * @param content the content of the raw messages, contains no return
         * @return BaseStream& this object
         */
        virtual BaseStream &operator<<(const std::string &content)
        {
            // no implementation here
            return *this;
        };
    };

    class ConsoleStream : public BaseStream
    {
    public:
        BaseStream &operator<<(const std::string &content) override;
        ConsoleStream();
    };

    /**
     * @brief thrown when can't open the log file in the specific file path
     *
     */
    class LogFileOpenException : public std::exception
    {
    private:
        std::string msg;

    public:
        const char *
        what() const noexcept override;

        /**
         * @brief mark wich file can't be opened
         *
         * @param prefix the prefix of the file failed to be opened
         */
        LogFileOpenException(const std::string &prefix);
    };

    class FileStream : public BaseStream
    {

    private:
        /// @brief the path of the log file
        std::string prefix;

        /// @brief where to store the ouput stream
        std::ofstream output;

        /**
         * @brief try to create or open the given file
         *
         * @return true successfully direct the output into the file
         * @return false else
         */
        bool openFile();

    public:
        /**
         * @brief Construct a new File Stream object using the prefix of the output logging file
         *
         * @param prefix the output file directory
         */
        FileStream(const std::string &prefix);
        ~FileStream();
        BaseStream &operator<<(const std::string &content) override;
    };
};

#endif
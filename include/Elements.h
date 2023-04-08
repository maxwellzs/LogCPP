/**
 * @file Elements.h
 * @author maxwellzs
 * @brief implements the elements that will appear in logs
 * @version 0.1
 * @date 2023-03-25
 *
 * @copyright GNU
 *
 */

#include <string>
#include <map>
#include <ctime>
#include <cstdlib>
#include <cstring>

#ifndef Elements_h
#define Elements_h

namespace LogCPP
{

    const static char START_ELEMENT = '[';
    const static char END_ELEMENT = ']';

    /**
     * @brief the abstract class defines the elements in log, like time, level, filename .etc
     *
     */
    class BaseElement
    {
    protected:
        /// @brief default no next
        BaseElement *next = nullptr;

    public:
        /**
         * @brief append the content of this element to the given builder object
         *
         * @param builder the builder object
         */
        virtual void CompileElement(std::string &builder);
        /**
         * @brief append a new element to compile chain
         * can't be overrided
         *
         * @param newElement the new element created
         * @return BaseElement* the pointer pointing to the element appended
         */
        virtual BaseElement &operator<<(BaseElement *newElement) final;
        /**
         * @brief Destroy the Base Element object, will delete all the element in the compile chain
         *
         */
        ~BaseElement();
        void ClearNext();
    };

    /**
     * @brief the level of the logs
     * constant enum
     *
     */
    typedef enum
    {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        FATAL
    } LEVELS;

    /// @brief map the level with string constants
    const static std::map<LEVELS, const std::string> LEVEL_STRINGS = {
        {DEBUG, " Debug "}, {INFO, " Info  "}, {WARNING, "Warning"}, {ERROR, " Error "}, {FATAL, " Fatal "}};

    /**
     * @brief this element append a time string indicating the level of the log
     *
     */
    class LevelElement : public BaseElement
    {
    private:
        // default is info
        LEVELS currentlevel = LEVELS::INFO;
        /**
         * @brief Get the string of the current level
         *
         * @return const std::string the string mapping the current level
         */
        const std::string GetLevelString() const;

    public:
        /**
         * @brief Construct a new Level Element object with the given level
         *
         * @param level the level of this element
         */
        LevelElement(LEVELS level);
        void CompileElement(std::string &builder) override;
    };

    /**
     * @brief this element append the time the log was created
     *
     */
    class TimeElement : public BaseElement
    {
    private:
        /**
         * @brief Get the string containing the current time
         *
         * @return const std::string the string containing the time in the given pattern
         */
        std::string GetCurrentTime() const;
        /// @brief the format of the current object
        /// this is how the time will be output
        std::string format;

    public:
        /**
         * @brief Construct a new Time Element ,this object will output the time with given pattern
         *
         * @param format
         */
        TimeElement(const std::string &format);
        void CompileElement(std::string &builder) override;
    };

    /**
     * @brief this class implements the constant string values in logs
     *
     */
    class ConstantStringElement : public BaseElement
    {
    private:
        /// @brief strores the content of the string
        std::string content;

    public:
        /**
         * @brief Construct a new Constant String Element object, holding a default empty value
         *
         * @param value the value of this string element
         */
        ConstantStringElement(const std::string &value = "");
        void CompileElement(std::string &builder) override;
    };

};

#endif
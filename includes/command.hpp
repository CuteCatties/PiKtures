_Pragma("once");
#include<utility.hpp>
#include<functional>
#include<map>
#include<memory>
#include<vector>
#include<ostream>
#include<concepts>
namespace PiKtures::Command{
    using PiKtures::Utility::ErrorCode;
    struct CommandSpecifier;
    struct ParserNode{
        ParserNode* parent = nullptr;
        std::map<char, ParserNode*> children;
        unsigned int command_count = 0;
        CommandSpecifier* command_specifier = nullptr;
    };
    struct CommandSpecifier{
        const char*const command;
        const char*const help_message;
        std::function<ErrorCode(std::ostream&, const char*, const int, const char**)> call_back;
        ParserNode* end_of_command = nullptr;
    };
    class CommandParser{
        private:
            ParserNode top_;
            bool finalized_;
            std::vector<CommandSpecifier*> specifiers_;
            const char*const parser_prefix_;

            CommandParser(const char*const);
            void insertCommand(CommandSpecifier&);
            void finalizeCommands();
            ParserNode* locateExact(const char*const);
            template <typename...Args, std::invocable<CommandSpecifier*, Args...> F>
            void foreachSubtreeCommandSpecifier(ParserNode*, F&&, Args&&...args);
            void freeLink(ParserNode*);
        public:
            ~CommandParser();
            static CommandParser& getInstance(const char*const);
            void insertCommand(std::vector<CommandSpecifier>&);
            ErrorCode parse(const char*const, std::ostream&, const int, const char**);
            void listCommands(const char*const, std::ostream&, const char*const, bool, size_t);
    };
}
_Pragma("once");
#include<utility.hpp>
#include<functional>
#include<map>
#include<memory>
#include<vector>
#include<ostream>
#include<concepts>
#include<string>
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
        std::function<unsigned int(std::ostream&, const char*, const int, const char**)> call_back;
        ParserNode* end_of_command = nullptr;
    };
    class CommandParser{
        private:
            ParserNode top_;
            bool finalized_;
            std::vector<CommandSpecifier*> specifiers_;
            const char*const parser_prefix_;

            void insertCommand(CommandSpecifier&);
            void finalizeCommands();
            ParserNode* locateExact(const char*const);
            template <typename...Args, std::invocable<CommandSpecifier*, Args...> F>
            void foreachSubtreeCommandSpecifier(ParserNode*, F&&, Args&&...args);
            void freeLink(ParserNode*);
        public:
            CommandParser(const char*const);
            ~CommandParser();
            void insertCommand(std::vector<CommandSpecifier>&);
            unsigned int parse(const char*const, std::ostream&, const int, const char**);
            void listCommands(const char*const, std::ostream&, const char*const, bool, size_t);
    };
    void splitCommandLine(const std::string&, std::vector<std::string>&);
}
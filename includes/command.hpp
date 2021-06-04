_Pragma("once");
#include<functional>
#include<map>
#include<memory>
#include<vector>
namespace PiKtures::Command{
    constexpr int NO_SUCH_COMMAND = 42;
    struct CommandSpecifier;
    struct ParserNode{
        ParserNode* parent = nullptr;
        std::map<char, ParserNode*> children;
        CommandSpecifier* command_specifier = nullptr;
    };
    struct CommandSpecifier{
        const char*const command;
        std::function<int(const int, const char**)> call_back;
        ParserNode* end_of_command = nullptr;
    };
    class CommandParser{
        private:
            ParserNode top_;
            bool finalized_;
            std::vector<CommandSpecifier*> specifiers_;

            CommandParser();
            void insertCommand(CommandSpecifier&);
            void finalizeCommands();
            CommandSpecifier* findCommandSpecifier(const char*const);
            void freeLink(ParserNode*);
        public:
            ~CommandParser();
            static CommandParser& getInstance();
            void insertCommand(std::vector<CommandSpecifier>&);
            int parse(const char*const, const int, const char**);
    };
}
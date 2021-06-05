#include<command.hpp>
#include<iostream>
#include<cstdlib>
#include<vector>
using namespace std;
using PiKtures::Command::CommandParser;
using PiKtures::Utility::ErrorCode;
int main(int argc, char** argv){
    vector<PiKtures::Command::CommandSpecifier> commands({
        {
            "hello",
            "say hello",
            [](ostream& out, const char* pfx, const int argc, const char** argv){
                out<<"hello world!\n";
                return ErrorCode::OK;
            },
            nullptr
        },
        {
            "helloneko",
            "say hello to kitties",
            [](ostream& out, const char* pfx, const int argc, const char** argv){
                out<<"hello, cute kitties!\n";
                return ErrorCode::OK;
            },
            nullptr
        },
        {
            "world",
            "stop the time",
            [](ostream& out, const char* pfx, const int argc, const char** argv){
                out<<"THE WORLD!\n"; 
                return ErrorCode::OK;
            },
            nullptr
        },
        {
            "echo",
            "repeat your number",
            [](ostream& out, const char* pfx, const int argc, const char** argv){
                if(argc == 2){
                    out<<strtol(argv[1], nullptr, 10)<<endl;
                    return ErrorCode::OK;
                }
                out<<pfx<<"echo requires exactly one parameter.\n";
                return ErrorCode::COMMAND_INVALID_PARAMETER_NUMBER;
            },
            nullptr
        }
    });
    CommandParser cp = CommandParser::getInstance("command_test: ");
    cp.insertCommand(commands);
    ErrorCode r = cp.parse(argv[1], cout, argc - 1, const_cast<const char**>(argv + 1));
    if(r == ErrorCode::COMMAND_NOT_FOUND){
        cout<<"Available commands:\n";
        cp.listCommands("", cout, "\t", true, 10);
    }else if(r == ErrorCode::COMMAND_AMBIGUOUS){
        cout<<"Did you mean:\n";
        cp.listCommands(argv[1], cout, "\t", false, 10);
    }
    return static_cast<int>(r);
}
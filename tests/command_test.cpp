#include<command.hpp>
#include<iostream>
#include<cstdlib>
#include<vector>
using namespace std;
using PiKtures::Command::CommandParser;
int main(int argc, char** argv){
    vector<PiKtures::Command::CommandSpecifier> commands({
        {"hello", [](const int argc, const char** argv)->int{cout<<"hello world!\n"; return 0;}, nullptr},
        {"world", [](const int argc, const char** argv)->int{cout<<"THE WORLD!\n"; return 0;}, nullptr},
        {"echo", [](const int argc, const char** argv)->int{
            if(argc == 2){cout<<strtol(argv[1], nullptr, 10)<<endl; return 0;}
            else return 1;
        }, nullptr}
    });
    CommandParser cp = CommandParser::getInstance();
    cp.insertCommand(commands);
    return cp.parse(argv[1], argc - 1, const_cast<const char**>(argv + 1));
}
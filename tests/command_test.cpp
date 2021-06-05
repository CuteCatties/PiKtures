#include<command.hpp>
#include<iostream>
#include<cstdlib>
#include<vector>
#include<string>
using namespace std;
using namespace PiKtures::Command;
using PiKtures::Utility::ErrorCode;
CommandParser hello_cp("hello: ");
CommandParser top_cp("");
vector<PiKtures::Command::CommandSpecifier> hello_commands({
    {
        "world",
        "say hello to this world",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            out<<"hello world!\n";
            return static_cast<unsigned int>(ErrorCode::OK);
        },
        nullptr
    },
    {
        "neko",
        "say hello to Cute Katties",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            out<<"hello cute kitties!\n";
            return static_cast<unsigned int>(ErrorCode::OK);
        },
        nullptr
    }
});
vector<PiKtures::Command::CommandSpecifier> top_commands({
    {
        "hello",
        "say hello",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            return hello_cp.parse(argv[1], out, argc - 1, argv + 1);
        },
        nullptr
    },
    {
        "world",
        "stop the time",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            out<<"THE WORLD!\n"; 
            return static_cast<unsigned int>(ErrorCode::OK);
        },
        nullptr
    },
    {
        "echo",
        "repeat your number",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            if(argc == 2){
                out<<strtol(argv[1], nullptr, 10)<<endl;
                return static_cast<unsigned int>(ErrorCode::OK);
            }
            out<<pfx<<"echo requires exactly one parameter.\n";
            return static_cast<unsigned int>(ErrorCode::PARAMETER_TOO_LESS);
        },
        nullptr
    },
    {
        "quit",
        "terminate this program",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            std::exit(0);
            return static_cast<unsigned int>(ErrorCode::OK);
        },
        nullptr
    },
    {
        "exit",
        "terminate this program",
        [](ostream& out, const char* pfx, const int argc, const char** argv){
            std::exit(0);
            return static_cast<unsigned int>(ErrorCode::OK);
        },
        nullptr
    }
});
int main(){
    top_cp.insertCommand(top_commands);
    hello_cp.insertCommand(hello_commands);
    string read_buffer;
    vector<string> split_buffer;
    unsigned int r = 0;
    bool side = 1;
    const char** args = nullptr;
    while(true){
        if(r != 0){
            cout<<r;
            side = !side;
            if(side) cout<<"m(=.ω·=)o~ ";
            else cout<<"m(=·ω.=)o~ ";
        }else{
            cout<<"m(=·ω·=)o~ ";
        }
        getline(cin, read_buffer);
        //cout<<"debug "<<read_buffer<<endl;
        splitCommandLine(read_buffer, split_buffer);
        //for(const auto& s: split_buffer) cout<<"debug "<<s<<endl;
        if(split_buffer.size() == 0){
            cout<<"say something...\n";
            r = 1024;
        }
        args = new const char*[split_buffer.size() + 1];
        for(decltype(split_buffer.size()) i = 0; i < split_buffer.size(); ++i) args[i] = split_buffer.at(i).data();
        args[split_buffer.size()] = nullptr;
        r = top_cp.parse(split_buffer.at(0).c_str(), cout, split_buffer.size(), args);
        if(args != nullptr) delete[] args;
        args = nullptr;
    }
}
#include<command.hpp>
#include<stdexcept>
PiKtures::Command::CommandParser::CommandParser():finalized_(false){}
void PiKtures::Command::CommandParser::insertCommand(CommandSpecifier& cs){
    if(finalized_) throw std::logic_error("Panic: Commands can be only set up once!");
    ParserNode* t = &top_;
    for(size_t i = 0; cs.command[i]; ++i){
        if(t->children.count(cs.command[i]) == 0){
            ParserNode* n = new ParserNode;
            n->parent = t;
            t->children[cs.command[i]] = n;
        }
        t = t->children[cs.command[i]];
    }
    if(t->command_specifier == nullptr){
        specifiers_.push_back(&cs);
        t->command_specifier = &cs;
        cs.end_of_command = t;
    }
}
void PiKtures::Command::CommandParser::finalizeCommands(){
    for(const auto& p: specifiers_){
        ParserNode* t = p->end_of_command->parent;
        while(t != nullptr && t->children.size() == 1){
            t->command_specifier = p;
            t = t->parent;
        }
    }
    specifiers_.clear();
    finalized_ = true;
}
PiKtures::Command::CommandSpecifier* PiKtures::Command::CommandParser::findCommandSpecifier(const char*const command){
    ParserNode* t = &top_;
    for(size_t i = 0; command[i]; ++i){
        if(t->command_specifier != nullptr) return t->command_specifier;
        if(t->children.count(command[i]) == 0) return nullptr;
        t = t->children[command[i]];
    }
    return t->command_specifier;
}
void PiKtures::Command::CommandParser::freeLink(ParserNode* t){
    if(t == nullptr) return;
    for(auto& nt: t->children) freeLink(nt.second);
    delete t;
}
PiKtures::Command::CommandParser::~CommandParser(){
    for(auto& t: top_.children) freeLink(t.second);
}
PiKtures::Command::CommandParser& PiKtures::Command::CommandParser::getInstance(){
    static CommandParser instance;
    return instance;
}
void PiKtures::Command::CommandParser::insertCommand(std::vector<CommandSpecifier>& css){
    if(finalized_) throw std::logic_error("Panic: Commands can be only set up once!");
    for(auto& cs: css) insertCommand(cs);
    finalizeCommands();
}
int PiKtures::Command::CommandParser::parse(const char*const command, const int argc, const char** argv){
    CommandSpecifier* cs = findCommandSpecifier(command);
    if(cs == nullptr) return NO_SUCH_COMMAND;
    return cs->call_back(argc, argv);
}
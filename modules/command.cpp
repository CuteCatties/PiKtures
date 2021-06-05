#include<command.hpp>
#include<stdexcept>
#include<ranges>
#include<algorithm>
#include<cstring>
#include<iomanip>
#include<cctype>
void PiKtures::Command::CommandParser::insertCommand(CommandSpecifier& cs){
    if(finalized_) throw std::logic_error("panic: Commands can be only set up once!");
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
        while(t != nullptr){
            ++(t->command_count);
            t = t->parent;
        }
    }
}
void PiKtures::Command::CommandParser::finalizeCommands(){
    for(const auto& p: specifiers_){
        ParserNode* t = p->end_of_command->parent;
        while(t != nullptr && t->command_count == 1){
            t->command_specifier = p;
            t = t->parent;
        }
    }
    specifiers_.clear();
    finalized_ = true;
}
PiKtures::Command::ParserNode* PiKtures::Command::CommandParser::locateExact(const char*const sequence){
    ParserNode* t = &top_;
    for(size_t i = 0; sequence[i]; ++i){
        if(t->children.count(sequence[i]) == 0) return nullptr;
        t = t->children[sequence[i]];
    }
    return t;
}
template <typename...Args, std::invocable<PiKtures::Command::CommandSpecifier*, Args...> F>
void PiKtures::Command::CommandParser::foreachSubtreeCommandSpecifier(ParserNode* root, F&& f, Args&&...args){
    if(root == nullptr) return;
    if(root->command_specifier != nullptr){
        f(root->command_specifier, std::forward<Args>(args)...);
        if(root->command_count == 1) return;
    }
    for(auto& t: root->children) foreachSubtreeCommandSpecifier(t.second, f, std::forward<Args>(args)...);
}
void PiKtures::Command::CommandParser::freeLink(ParserNode* t){
    if(t == nullptr) return;
    for(auto& nt: t->children) freeLink(nt.second);
    delete t;
}
PiKtures::Command::CommandParser::CommandParser(const char*const parser_prefix):finalized_(false), parser_prefix_(parser_prefix){}
PiKtures::Command::CommandParser::~CommandParser(){
    for(auto& t: top_.children) freeLink(t.second);
}
void PiKtures::Command::CommandParser::insertCommand(std::vector<CommandSpecifier>& css){
    if(finalized_) throw std::logic_error("panic: Commands can be only set up once!");
    for(auto& cs: css) insertCommand(cs);
    finalizeCommands();
}
unsigned int PiKtures::Command::CommandParser::parse(
    const char*const command,
    std::ostream& out,
    const int argc,
    const char** argv
){
    if(command == nullptr){
        out<<parser_prefix_<<"Please specify a command. You can use:\n";
        listCommands("", out, "\t", false, 10);
        return static_cast<unsigned int>(ErrorCode::EMPTY_COMMAND);
    }
    ParserNode* node = locateExact(command);
    if(node == nullptr){
        out<<parser_prefix_<<"No such command\n";
        return static_cast<unsigned int>(ErrorCode::COMMAND_NOT_FOUND);
    }
    if(node->command_specifier != nullptr){
        try{
            return static_cast<unsigned int>(node->command_specifier->call_back(out, parser_prefix_, argc, argv));
        }catch(ErrorCode& e){
            out<<parser_prefix_<<PiKtures::Utility::errorMessage(e)<<std::endl;
            return static_cast<unsigned int>(e);
        }
    }
    if(node->command_count < 2) throw std::logic_error("Panic: Parser in invalid and unknown status!");
    out<<parser_prefix_<<"Ambiguous command. Did you mean:\n";
    listCommands(command, out, "\t", false, 10);
    return static_cast<unsigned int>(ErrorCode::COMMAND_AMBIGUOUS);
}
void PiKtures::Command::CommandParser::listCommands(
    const char*const command_prefix,
    std::ostream& out,
    const char*const line_prefix,
    bool with_help,
    size_t command_width
){
    ParserNode* t = locateExact(command_prefix);
    if(t == nullptr){
        out<<parser_prefix_<<"No such commands.\n";
        return;
    }
    foreachSubtreeCommandSpecifier(
        t,
        [](
            CommandSpecifier* p,
            std::ostream& out,
            const char*const line_prefix,
            bool with_help,
            size_t command_width
        ){
            out<<line_prefix;
            out<<std::setiosflags(std::ios::left)<<std::setw(command_width)<<p->command<<std::resetiosflags(std::ios::left);
            if(with_help) out<<p->help_message;
            out<<std::endl;
        },
        out,
        line_prefix,
        with_help,
        command_width
    );
}
void PiKtures::Command::splitCommandLine(const std::string& line, std::vector<std::string>& split){
    decltype(split.size()) split_index = 0;
    decltype(line.size()) line_index = 0;
    // remove prefix spaces
    for(; line_index < line.size() && std::isspace(line.at(line_index)); ++line_index);
    if(line_index == line.size()){  // line is empty
        split.clear();
        return;
    }
    bool in_space = false;
    // ensure there is at least one element in split
    if(split.size() == 0) split.emplace_back(std::string());
    else split.at(0).clear();
    for(; line_index < line.size(); ++line_index){
        if(std::isspace(line.at(line_index))) in_space = true;
        else{
            if(in_space){
                in_space = false;
                if(++split_index == split.size()) split.emplace_back(std::string());
                else split.at(split_index).clear();
            }
            split.at(split_index).push_back(line.at(line_index));
        }
    }
    // free unused strings
    split.resize(split_index + 1);
}
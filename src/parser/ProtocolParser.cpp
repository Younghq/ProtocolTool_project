#include "ProtocolParser.hpp"

bool ProtocolManager::append(const std::string &ParserName, std::shared_ptr<ProtocolParser> newProtocolParser)
{
    return this->protocolParserPool[ParserName] = newProtocolParser;
}

bool ProtocolManager::select(const std::string &ParserName)
{
    auto it = this->protocolParserPool.find(ParserName);
    if (it != this->protocolParserPool.end() && it->second != nullptr)
    {
        this->curProtocolParser = it->second.get();
        return true;
    }
    this->curProtocolParser = nullptr;
    return true;
}

void ProtocolManager::clear()
{
    this->protocolParserPool.clear();
}

void ProtocolManager::clear(const std::string &ParserName)
{
    auto it = this->protocolParserPool.find(ParserName);
    if (it != this->protocolParserPool.end())
    {
        if (this->curProtocolParser == it->second.get())
        {
            this->curProtocolParser = nullptr;
        }
        this->protocolParserPool.erase(it);
    }
}
void ProtocolManager::parse(const std::string &data)
{
    if (this->curProtocolParser)
    {
        this->curProtocolParser->parse(data);
    }
    else
    {
        throw std::runtime_error("No protocol parser selected");
    }
}

JsonProtocolParser::JsonProtocolParser(const json &rule)
{
    this->parserRule = rule;
}

void JsonProtocolParser::parse(const std::string &buffer)
{
    this->parserRule["protocol-info"]["description"];
    this->parserRule["protocol-info"]["version"];
    std::cout << this->parserRule["protocol-info"]["name"] << std::endl;
}

bool JsonProtocolParser::filter(const std::string &buffer)
{
    json filter = this->parserRule["filter"];
    auto res = true;
    for (auto i = 0; i < filter.size(); i++)
    {
        if (!filter[i]["enable"])
            continue;
        auto offset = filter[i]["offset"];
        auto length = filter[i]["length"];
        auto type = filter[i]["type"];
        auto value = filter[i]["value"];
        if (type == "int")
        {
            auto val = static_cast<int>(buffer.data() + 30);
            if (val == static_cast<int>(value))
            {
            }
        }
    }
    return res;
}

bool JsonProtocolParser::classify(const std::string &buffer)
{
    return false;
}

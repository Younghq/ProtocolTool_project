#include <map>
#include <memory>
#include <string>
#include <iostream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

class ProtocolParser
{
public:
    virtual ~ProtocolParser() = default;
    virtual void parse(const std::string &buffer) = 0;
};

class JsonProtocolParser : public ProtocolParser
{
public:
    JsonProtocolParser(const json &rule);
    void parse(const std::string &buffer) override;

private:
    bool filter(const std::string &buffer);
    bool classify(const std::string &buffer);

    const json &parserRule;
    const std::string &buffer;
};

class ProtocolManager
{
public:
    bool append(const std::string &ParserName, std::shared_ptr<ProtocolParser> newProtocolParser);
    bool select(const std::string &ParserName);
    void clear();
    void clear(const std::string &ParserName);
    void parse(const std::string &data);

private:
    std::map<std::string, std::shared_ptr<ProtocolParser>> protocolParserPool;
    ProtocolParser *curProtocolParser = nullptr;
};

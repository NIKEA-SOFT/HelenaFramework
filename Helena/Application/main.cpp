#include <iostream>
#include <filesystem>

#include <Common/Xml.hpp>
#include <Common/Meta.hpp>
#include <Common/Util.hpp>
#include <Common/ModuleManager.hpp>

using namespace Helena;

int main(int argc, char** argv)
{
    if(argc != 2) {
        std::cerr << "[Error] Incorrect args, usage App.exe Service.xml" << std::endl;
        return 0;
    }

    // Parse service xml path from arg
    const std::filesystem::path service{ argv[1] };
    if(std::error_code error; !std::filesystem::is_regular_file(service, error) || !std::filesystem::exists(service, error)) {
        std::cerr << "[Error] Service file: " << service << " not found!" << std::endl;
        return 0;
    }

    // Load xml file
    pugi::xml_document xmlDoc;
    if(!xmlDoc.load_file(service.c_str(), pugi::parse_default | pugi::parse_comments)) {
        std::cerr << "[Error] Parse file: " << service << " failure!" << std::endl;
        return 0;
    }


    // Find xml node
    const auto node = xmlDoc.child(Meta::ConfigService::Service());
    if(node.empty()) {
        std::cerr << "[Error] Parse file: " << service << ", Node: " << Meta::ConfigService::Service() << " not found!" << std::endl;
        return 0;
    }

    // Read attribute from xml node
    std::filesystem::path configPath{ node.attribute(Meta::ConfigService::PathConfigs()).as_string() };
    std::filesystem::path modulePath{ node.attribute(Meta::ConfigService::PathModules()).as_string() };
    std::filesystem::path resourcePath{ node.attribute(Meta::ConfigService::PathResources()).as_string() };
    std::string_view modules = node.attribute(Meta::ConfigService::Modules()).as_string();

    // Create configs/modules/resources directories if not exists
    std::error_code error;
    if(!std::filesystem::exists(configPath, error) && !std::filesystem::create_directory(configPath, error)) {
        std::cerr << "[Error] Path: " << configPath << " create folder failure!" << std::endl;
        return 0;
    }

    if(!std::filesystem::exists(modulePath, error) && !std::filesystem::create_directory(modulePath, error)) {
        std::cerr << "[Error] Path: " << modulePath << " create folder failure!" << std::endl;
        return 0;
    }

    if(!std::filesystem::exists(resourcePath, error) && !std::filesystem::create_directory(resourcePath, error)) {
        std::cerr << "[Error] Path: " << resourcePath << " create folder failure!" << std::endl;
        return 0;
    }

    // Get absolute path if path is relative
    configPath = std::filesystem::absolute(configPath, error);
    modulePath = std::filesystem::absolute(modulePath, error);
    resourcePath = std::filesystem::absolute(resourcePath, error);
    auto moduleNames = Util::Split<std::string>(modules);
    auto appName = service.stem().string();

    std::cout
        << " ______________________________________" << std::endl
        << "| \t    Helena Framework" << std::endl
        << "|--------------------------------------" << std::endl
        << "| Application: \t\"" << appName << "\"" << std::endl
        << "| PathService: \t" << service << std::endl
        << "| PathConfig: \t" << configPath << std::endl
        << "| PathModule: \t" << modulePath << std::endl
        << "| PathResource:\t" << resourcePath << std::endl
        << "| Modules: \t\"" << modules << "\"" << std::endl
        << "|______________________________________" << std::endl
        << std::endl;

    // Check exist directories and module names not empty
    if(!std::filesystem::exists(configPath, error)) {
        std::cerr
            << "[Error] " << std::endl
            << "Parse: " << service << std::endl
            << "Node: " << Meta::ConfigService::Service() << std::endl
            << "Attribute: " << Meta::ConfigService::PathConfigs() << std::endl
            << "Path: \"" << configPath << "\" not exist!" << std::endl;
        return 0;
    }
    else if(!std::filesystem::exists(modulePath, error)) {
        std::cerr
            << "[Error] " << std::endl
            << "Parse file: " << service << std::endl
            << "Node: " << Meta::ConfigService::Service() << std::endl
            << "Attribute: " << Meta::ConfigService::PathModules() << std::endl
            << "Path: \"" << modulePath << "\" not exist!" << std::endl;
        return 0;
    }
    else if(!std::filesystem::exists(resourcePath, error)) {
        std::cerr
            << "[Error] " << std::endl
            << "Parse file: " << service << std::endl
            << "Node: " << Meta::ConfigService::Service() << std::endl
            << "Attribute: " << Meta::ConfigService::PathResources() << std::endl
            << "Path: \"" << resourcePath << "\" not exist!" << std::endl;
        return 0;
    }
    else if(moduleNames.empty()) {
        std::cerr
            << "[Error] Parse file: " << service << std::endl
            << "Node: " << Meta::ConfigService::Service() << std::endl
            << "Attribute: " << Meta::ConfigService::Modules() << " is empty!" << std::endl;
        return 0;
    }

    HelenaFramework(service.stem().string(), configPath.string(), modulePath.string(), resourcePath.string(), std::move(moduleNames));
    system("pause");
    return 0;
}
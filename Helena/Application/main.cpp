#include <iostream>
#include <filesystem>

#include <Common/Xml.hpp>
#include <Common/Meta.hpp>
#include <Common/ModuleManager.hpp>

using namespace Helena;

int main(int argc, char** argv)
{
    if(argc != 2) {
        UTIL_CONSOLE_ERROR("Incorrect args, usage Helena.exe Service.xml");
        return 0;
    }

    // Parse service xml path from arg
    const std::filesystem::path service{argv[1]};
    if(std::error_code error; !std::filesystem::is_regular_file(service, error) || !std::filesystem::exists(service, error)) {
        UTIL_CONSOLE_ERROR("Service file: {} not found!", service.string());
        return 0;
    }

    // Load xml file
    pugi::xml_document xmlDoc;
    if(!xmlDoc.load_file(service.c_str(), pugi::parse_default | pugi::parse_comments)) {
        UTIL_CONSOLE_ERROR("Parse file: {} failed!", service.string());
        return 0;
    }


    // Find xml node
    const auto node = xmlDoc.child(Meta::ConfigService::Service());
    if(node.empty()) {
        UTIL_CONSOLE_ERROR("Parse file: {}, node: {} not found!", service.string(), Meta::ConfigService::Service());
        return 0;
    }

    // Read attribute from xml node
    std::filesystem::path pathConfig {node.attribute(Meta::ConfigService::PathConfigs()).as_string()};
    std::filesystem::path pathModule {node.attribute(Meta::ConfigService::PathModules()).as_string()};
    std::filesystem::path pathResource {node.attribute(Meta::ConfigService::PathResources()).as_string()};
    std::string_view modules = node.attribute(Meta::ConfigService::Modules()).as_string();

    // Create configs/modules/resources directories if not exists
    std::error_code error;
    if(!std::filesystem::exists(pathConfig, error) && !std::filesystem::create_directory(pathConfig, error)) {
        UTIL_CONSOLE_ERROR("Path: {} create folder failed!", pathConfig.string());
        return 0;
    }

    if(!std::filesystem::exists(pathModule, error) && !std::filesystem::create_directory(pathModule, error)) {
        UTIL_CONSOLE_ERROR("Path: {} create folder failed!", pathModule.string());
        return 0;
    }

    if(!std::filesystem::exists(pathResource, error) && !std::filesystem::create_directory(pathResource, error)) {
        UTIL_CONSOLE_ERROR("Path: {} create folder failed!", pathResource.string());
        return 0;
    }

    // Get absolute path if path is relative
    pathConfig = std::filesystem::absolute(pathConfig, error);
    pathModule = std::filesystem::absolute(pathModule, error);
    pathResource = std::filesystem::absolute(pathResource, error);
    auto moduleNames = Util::Split<std::string>(modules);
    auto serviceName = service.stem().string();

    std::cout
        << " ______________________________________" << std::endl
        << "| \t    Helena Framework" << std::endl
        << "|--------------------------------------" << std::endl
        << "| Service: \t\"" << serviceName << "\"" << std::endl
        << "| PathService: \t" << service << std::endl
        << "| PathConfig: \t" << pathConfig << std::endl
        << "| PathModule: \t" << pathModule << std::endl
        << "| PathResource:\t" << pathResource << std::endl
        << "| Modules: \t\"" << modules << "\"" << std::endl
        << "|______________________________________" << std::endl
        << std::endl;

    // Check exist directories and module names not empty
    if(!std::filesystem::exists(pathConfig, error)) {
        UTIL_CONSOLE_ERROR("Parse: {}, node: {}, attribute: {} failed! Path: {} path not exist!", 
            service.string(), 
            Meta::ConfigService::Service(), 
            Meta::ConfigService::PathConfigs(), 
            pathConfig.string());
    }
    else if(!std::filesystem::exists(pathModule, error)) {
        UTIL_CONSOLE_ERROR("Parse: {}, node: {}, attribute: {} failed! Path: {} not exist!",
            service.string(), 
            Meta::ConfigService::Service(),
            Meta::ConfigService::PathModules(),
            pathModule.string());
    }
    else if(!std::filesystem::exists(pathResource, error)) {
        UTIL_CONSOLE_ERROR("Parse: {}, node: {}, attribute: {} failed! Path: {} not exist!",
            service.string(), 
            Meta::ConfigService::Service(),
            Meta::ConfigService::PathResources(),
            pathResource.string());
    }
    else if(moduleNames.empty()) {
        UTIL_CONSOLE_ERROR("Parse: {}, node: {}, attribute: {} failed! Attribute is empty!",
            service.string(),
            Meta::ConfigService::Service(),
            Meta::ConfigService::Modules());
    } else {
        HelenaFramework(serviceName, pathConfig.string(), pathModule.string(), pathResource.string(), moduleNames);
    }

#if HF_PLATFORM == HF_PLATFORM_WIN
    system("pause");
#endif
    return 0;
}
#include <iostream>
#include <filesystem>

#include <Common/Service.hpp>
#include <Common/Xml.hpp>
#include <Common/Meta.hpp>

using namespace Helena;

std::unique_ptr<Service> ServiceParse(const std::filesystem::path service, std::string& moduleNames)
{
    auto serviceName = service.stem().string();
    std::error_code error;
    if(!std::filesystem::is_regular_file(service, error) || !std::filesystem::exists(service, error)) {
        HF_CONSOLE_ERROR("Service file: {} not found!", serviceName);
        return std::unique_ptr<Service>();
    }

    pugi::xml_document xmlDoc;
    if(!xmlDoc.load_file(service.c_str(), pugi::parse_default | pugi::parse_comments)) {
        HF_CONSOLE_ERROR("Parse file: {} failed!", serviceName);
        return std::unique_ptr<Service>();
    }

    // Find xml node
    const auto node = xmlDoc.child(Meta::ConfigService::Service());
    if(node.empty()) {
        HF_CONSOLE_ERROR("Parse file: {}, node: {} not found!", serviceName, Meta::ConfigService::Service());
        return std::unique_ptr<Service>();
    }

    // Create configs/modules/resources directories if not exists
    std::string pathConfigs = std::filesystem::absolute(node.attribute(Meta::ConfigService::PathConfigs()).as_string()).string();
    if(!std::filesystem::exists(pathConfigs, error)) {
        HF_CONSOLE_ERROR("Service: {}, node: {}, path: {} folder not exist!", serviceName, Meta::ConfigService::PathConfigs(), pathConfigs);
        return std::unique_ptr<Service>();
    }

    std::string pathModules = std::filesystem::absolute(node.attribute(Meta::ConfigService::PathModules()).as_string()).string();
    if(!std::filesystem::exists(pathModules, error)) {
        HF_CONSOLE_ERROR("Service: {}, node: {}, path: {} folder not exist!", serviceName, Meta::ConfigService::PathModules(), pathModules);
        return std::unique_ptr<Service>();
    }

    std::string pathResources = std::filesystem::absolute(node.attribute(Meta::ConfigService::PathResources()).as_string()).string();
    if(!std::filesystem::exists(pathResources, error)) {
        HF_CONSOLE_ERROR("Service: {}, node: {}, path: {} folder not exist!", serviceName, Meta::ConfigService::PathResources(), pathResources);
        return std::unique_ptr<Service>();
    }
   
    if(pathConfigs.back() != HF_SEPARATOR) {
        pathConfigs += HF_SEPARATOR;
    }

    if(pathModules.back() != HF_SEPARATOR) {
        pathModules += HF_SEPARATOR;
    }

    if(pathResources.back() != HF_SEPARATOR) {
        pathResources += HF_SEPARATOR;
    }
    
    moduleNames = node.attribute(Meta::ConfigService::Modules()).as_string();
    return std::make_unique<Service>(serviceName, pathConfigs, pathModules, pathResources);
}

int main(int argc, char** argv)
{
#if HF_PLATFORM == HF_PLATFORM_WIN
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode{};

    if(!GetConsoleMode(hOutput, &dwMode)) {
        std::cout << "GetConsoleMode failed!" << std::endl;
        return 0;
    }

    dwMode |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if(!SetConsoleMode(hOutput, dwMode)) {
        std::cout << "SetConsoleMode failed!" << std::endl;
        return 0;
    }
#endif

    if(argc != 2) {
        HF_CONSOLE_ERROR("Incorrect args, usage Helena.exe Service.xml");
        return 0;
    }

    std::string moduleNames;
    if(const auto service = ServiceParse(argv[1], moduleNames); service) {
        std::cout
            << " ______________________________________" << std::endl
            << "| \t    Helena Framework" << std::endl
            << "|--------------------------------------" << std::endl
            << "| Service: \t\"" << service->GetName() << "\"" << std::endl
            << "| PathConfig: \t" << service->GetDirectories().GetPathConfigs() << std::endl
            << "| PathModule: \t" << service->GetDirectories().GetPathModules() << std::endl
            << "| PathResource:\t" << service->GetDirectories().GetPathResources() << std::endl
            << "| Modules: \t\"" << moduleNames << "\"" << std::endl
            << "|______________________________________" << std::endl
            << std::endl;

        service->Initialize(moduleNames);
        service->Finalize();
    }

#if HF_PLATFORM == HF_PLATFORM_WIN
    system("pause");
    ExitProcess(NULL);
#endif
    return 0;
}
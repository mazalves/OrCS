#include "../simulator.hpp"

configure_t::configure_t(){
    loadConfig();
}

configure_t::~configure_t()=default;

void configure_t::loadConfig(){
    try {
        cfg.readFile (orcs_engine.config_file);
    } catch (const libconfig::FileIOException &fioex){
        std::cerr << "I/O error while reading file." << std::endl;
    } catch (const libconfig::ParseException &pex){
        std::cerr << "Parse error at " << pex.getFile() << ':' << pex.getLine() << " - " << pex.getError() << std::endl;
    }
}

int configure_t::getSetting (std::string setting) {
    int result = 0;
    if (conf.find (setting) != conf.end()) result = conf[setting];
    else {
        try {
            result = cfg.lookup (setting);
            conf[setting] = result;
        } catch (const libconfig::SettingNotFoundException &pex){
            std::cerr << "Setting " << setting << " Not Found!" << std::endl;
        }
    }
    std::cout << setting << " " << result << "\n";
    return result;
}
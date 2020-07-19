#include "../simulator.hpp"

configure_t::configure_t(){
    cfg = new libconfig::Config();
    loadConfig();
}

configure_t::~configure_t() {
    delete cfg;
}

void configure_t::loadConfig(){
    try {
        cfg->readFile (orcs_engine.config_file);
    } catch (const libconfig::FileIOException &fioex){
        std::cerr << "I/O error while reading file." << std::endl;
    } catch (const libconfig::ParseException &pex){
        std::cerr << "Parse error at " << pex.getFile() << ':' << pex.getLine() << " - " << pex.getError() << std::endl;
    }
}

libconfig::Setting &configure_t::getConfig(){
    libconfig::Setting &cfg_root = cfg->getRoot();
    return cfg_root;
}
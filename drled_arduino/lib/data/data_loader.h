#ifndef DATA_LOADER_H
#define DATA_LOADER_H

#include "../log/logger.h"
#include "../json/parser.h"
#include "../json/generator.h"
#include "../file_system.h"

namespace DevRelief
{
    

class DataLoader {
    public:
        DataLoader() {
            SET_LOGGER(DataObjectLogger);

        }

        bool writeJsonFile(const char * path,IJsonElement* json) {
            m_logger->debug("write JSON file %s",path);
            DRString buffer;
            m_logger->debug("\tgen JSON");
            JsonGenerator gen(buffer);
            gen.generate(json);
            m_logger->debug("\twrite JSON: %s",buffer.text());
            return m_fileSystem.write(path,buffer.text());
        }

        bool writeFile(const char * path, const char * text) {
            return m_fileSystem.write(path,text);
        }

        bool loadJsonFile(const char * path,auto reader) {
            DRFileBuffer buffer;
            m_logger->debug("loadJsonFile %s",path);
            if (m_fileSystem.read(path,buffer)){
                m_logger->debug("file: %s",buffer.text());
                JsonParser parser;
                JsonRoot* root = parser.read(buffer.text());
                if (root) {
                    buffer.clear(); // free file read memory before parsing json
                    bool result = reader(root->asObject());
                    root->destroy();
                    return result;
                }
            }
            return false;
        }

        bool deleteFile(const char * path){
            if (m_fileSystem.exists(path)){
                m_fileSystem.deleteFile(path); 
                return true;
            }
            return false;
        }

    protected:
        DECLARE_LOGGER(); 
        static DRFileSystem m_fileSystem;
};

DRFileSystem DataLoader::m_fileSystem;

} // namespace DevRelief

#endif
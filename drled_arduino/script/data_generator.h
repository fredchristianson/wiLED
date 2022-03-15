#ifndef SCRIPT_DATA_GENERATOR_H
#define SCRIPT_DATA_GENERATOR_H

#include "../lib/log/logger.h"
#include "../lib/data/data_loader.h"
#include "../lib/json/parser.h"
#include "./script.h"
#include "./script_element.h"
#include "./script_container.h"
#include "./strip_element.h"
#include "./data_generator.h"
#include "./json_names.h"

namespace DevRelief {

    class ScriptDataGenerator {
        public:
            ScriptDataGenerator(){
                SET_LOGGER(ScriptLoaderLogger);
            }


            JsonRoot* toJson(Script* script) {
                JsonRoot* root = new JsonRoot();
                JsonObject* obj  = root->getTopObject();
                obj->setString(S_SCRIPT_NAME,script->getName());
                obj->setInt(S_DURATION,script->getDuration());
                obj->setInt(S_FREQUENCY,script->getFrequency());
                //addContainerElements(obj,script->getRootContainer());
                script->getRootContainer()->toJson(obj);
                return root;
            }

            void addContainerElements(JsonObject* parent, ScriptContainer* container) {
                JsonArray* array = parent->createArray(S_ELEMENTS);
                container->getChildren().each([&](IScriptElement* child){
                    JsonObject* childObj = array->addNewObject();
                    elementToJson(childObj,child);
                });
            }

            void elementToJson(JsonObject*json, IScriptElement* element) {
                m_logger->debug("elementToJson: %s",element->getType());
                json->setString("type",element->getType());
                element->toJson(json);
                if (element->isContainer()) {
                    addContainerElements(json,(ScriptContainer*)element);
                }
            }


        private:
            DECLARE_LOGGER();
    };

    
    ScriptElementCreator::ScriptElementCreator(IScriptContainer* container) {
        m_container = container;
        SET_LOGGER(ScriptElementLogger);
    }

    IScriptElement* ScriptElementCreator::elementFromJson(IJsonElement* json,ScriptContainer* container){
        m_logger->debug("parse Json type=%d",json->getType());
        JsonObject* obj = json ? json->asObject() : NULL;
        if (obj == NULL){
            m_logger->error("invalid IJsonElement to create script element");
            return NULL;
        }

        const char * type = obj->getString("type",NULL);
        if (type == NULL) {
            type = guessType(obj);
        }
        IScriptElement* element = NULL;
        if (type == NULL) {
            m_logger->error("json is missing a type");
            return NULL;
        } else if (Util::equal(type,S_VALUES)) {
            element = new ValuesElement();
        } else if (Util::equalAny(type,S_HSL)) {
            element = new HSLElement();
        } else if (Util::equalAny(type,S_RHSL)) {
            element = new RainbowHSLElement();
        } else if (Util::equalAny(type,S_RGB)) {
            element = new RGBElement();
        } else if (Util::equalAny(type,S_SEGMENT)) {
            element = new ScriptSegmentContainer(container);
        }  else if (Util::equalAny(type,S_MAKER)) {
            element = new MakerContainer(container);
        } else if (Util::equalAny(type,S_MIRROR)) {
            element = new MirrorElement(container);
        } else if (Util::equalAny(type,S_COPY)) {
            element = new CopyElement(container);
        } else if (Util::equalAny(type,S_REPEAT)) {
            element = new RepeatElement(container);
        }
        if (element) {
            element->fromJson(obj);
            m_logger->debug("created element type %s %x",element->getType(),element);
        }
        return element;
    }

    const char * ScriptElementCreator::guessType(JsonObject* json){
        if (json->getPropertyValue("rhue")){
            return S_RHSL;
        } else if (json->getPropertyValue("hue")||json->getPropertyValue("lightness")||json->getPropertyValue("saturation")){
            return S_HSL;
        } else if (json->getPropertyValue("red")||json->getPropertyValue("blue")||json->getPropertyValue("green")){
            return S_RGB;
        } else if (json->getPropertyValue("elements")){
            return S_SEGMENT;
        } else {
            return S_VALUES;
        }
        return NULL;
    }

};

#endif

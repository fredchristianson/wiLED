#ifndef PARSE_GEN_H
#define PARSE_GEN_H

#include "../log/logger.h"
#include "../util/buffer.h"
#include "../util/util.h"
#include "./json_interface.h"
#include "./json.h"


namespace DevRelief {


class TokenParser {
    public:
        TokenParser(const char  * data) {
            m_data = data;
            m_pos = data;
            m_errorMessage = NULL;
            m_token = TOK_START;
        }

        TokenType peek(){
            const char* pos = m_pos;            
            const char * tpos = m_tokPos;
            TokenType old = m_token;
            TokenType type = next();
            m_pos = pos;
            m_tokPos = tpos;
            m_token = old;
            return type;
        }

        TokenType next() {
            TokenType t = TOK_ERROR;
            const char * pos = m_pos;

            if (!skipWhite()) {
                t = TOK_EOD;
                return m_token;
            }
            char c = m_pos[0];
            m_tokPos = m_pos;
            m_pos+= 1;
            if (c == '['){
                m_token = TOK_ARRAY_START;
            } else if (c == ']'){
                m_token = TOK_ARRAY_END;
            } else if (c == '{'){
                m_token = TOK_OBJECT_START;
            } else if (c == '}'){
                m_token = TOK_OBJECT_END;
            }  else if (c == '"'){
                m_token = TOK_STRING;
            }  else if (isdigit(c) || (c)=='-' && isdigit(*m_pos)){
                const char * n = m_tokPos;
                if (*n == '-') { n+= 1;}
                while(*n !=0 && isdigit(*n)){
                    n++;
                }
                m_token = *n == '.' ? TOK_FLOAT : TOK_INT;
            }  else if (c == ':'){
                m_token = TOK_COLON;
            }  else if (c == ','){
                m_token = TOK_COMMA;
            }  else if (c == 'n' && strncmp(m_pos,"ull",3)==0){
                m_token = TOK_NULL;
                m_pos+=3;
            }  else if (c == 't' && strncmp(m_pos,"rue",3)==0){
                m_token = TOK_TRUE;
                m_pos+=3;
            }  else if (c == 'f' && strncmp(m_pos,"alse",4)==0){
                m_token = TOK_FALSE;
                m_pos+=4;
            }  else {

                m_token = TOK_ERROR;
            }
            return m_token;
        }

        bool skipWhite() {
            while(strchr(" \t\n\r",m_pos[0]) != NULL) {
                m_pos++;
            }
            if (m_pos[0] == 0) {
                m_token = TOK_EOD;
                return false;
            }
            return true;
        }

        bool nextInt(int& val) {
            val = 0;
            if (next() == TOK_INT) {
                val = atoi(m_tokPos);
                skipNumber();
                return true;
            }
            return false;
        }

        bool nextFloat(double& val) {
            val = 0;
            TokenType n = next();
            if (n == TOK_INT || n == TOK_FLOAT) {
                val = atof(m_tokPos);
                skipNumber();
                return true;
            }
            return false;
        }

        bool skipNumber() {

            while(*m_pos == '.' || isdigit(*m_pos)){
                m_pos++;
            }
            return true;
        }

        bool nextString(const char *& start, size_t& len) {

            skipWhite();
            start = NULL;
            len = 0;
            if (m_pos[0] != '"'){
                return false;
            }
            m_tokPos = m_pos;
            m_token = TOK_STRING;
            m_pos += 1;
            start = m_pos;
            while(m_pos[len] != '"' && m_pos[len] != 0){
                if (m_pos[len] == '\\'){
                    len += 2;
                } else {
                    len += 1;
                }
            }
            m_pos += len;
            if (m_pos[0] == '"') {
                m_pos+= 1;
                return true;
            }
            return false;
        }

        const char * getPos() { return m_pos;}
        const char * getTokPos() { return m_tokPos;}

        int getCurrentPos(){
            int p = m_pos - m_data;
            return p;
        }
        int getCharacterCount(){
            if (m_pos[0] == 0) {
                return m_pos-m_data;
            }
            int len = strlen(m_pos);
            return m_pos-m_data+len;
        }
        int getCurrentLine(){
            char * nl = strchr(m_data,'\n');
            int p = 1;
            while(nl > 0 && nl <= m_pos)  {
                nl = strchr(nl+1,'\n');
                p++;
            }
            return p;
        }

        int getLineCount(){
            const char * nl = strchr(m_data,'\n');
            int p = 1;
            while(nl > 0)  {
                nl = strchr(nl+1,'\n');
                p++;
            }
            return p;
        }

        int getLinePos(){
            const char * pos = m_pos;
            while(pos > m_data && pos[0] != '\n'){
                pos--;
            }
            return m_pos-pos;
        }
        int getLineCharacterCount(){
            const char * pos = m_pos;
            while(pos[0] != 0 && pos[0] != '\n'){
                pos++;
            }
            return getLinePos()+pos- m_pos;
        }

        DRString getCurrentLineText(){
            const char * start = m_pos;
            while(start > m_data && start[-1] != '\n'){
                start--;
            }
            const char * end = m_pos;
            while(end[0] != 0 && end[0] != '\n'){
                end++;
            }
            return DRString(start,(size_t)(end-start));
        }
    private: 
        const char * m_data;
        const char * m_pos;
        const char * m_tokPos;
        TokenType           m_token;
        const char *    m_errorMessage;
};

class JsonParser : public IParseGen {
public:
    JsonParser() {
        SET_LOGGER(JsonParserLogger);
        m_errorMessage = NULL;
        m_hasError = false;
        m_root = NULL;
    }    

    ~JsonParser() { 
    }


    JsonRoot* read(const char * data) {
        m_logger->debug("parsing %s",data);
        
        m_errorMessage = NULL;
        m_hasError = false;
        JsonRoot * root = new JsonRoot();
        m_root = root;
        m_logger->never("created root");

        if (data == NULL) {
            return root;
        }
        
        TokenParser tokParser(data);
        m_logger->never("created TokenParser");
        IJsonElement * json = parseNext(tokParser);
        if (json != NULL) {
            m_logger->never("got top");
            root->setTopElement(json);
        } else {
            m_logger->never("no top element found");

            m_logger->never("\tpos %d/%d.  line%d/%d.  char %d/%d",
                tokParser.getCurrentPos(),
                tokParser.getCharacterCount(),
                tokParser.getCurrentLine(),
                tokParser.getLineCount(),
                tokParser.getLinePos(),
                tokParser.getLineCharacterCount()
                );
            delete root;
            m_hasError = true;

            root = NULL; 
            m_errorMessage = "parse error";
            m_errorLine = tokParser.getCurrentLineText();
            m_errorLineNumber = tokParser.getCurrentLine();
            m_errorCharacter = tokParser.getLinePos();
            m_errorPosition = tokParser.getCurrentPos();
        }
        
        m_logger->never("parse complete");
        m_root = NULL;
        return root;
    }

    IJsonElement* parseNext(TokenParser&tok) {
        TokenType next = tok.peek();
        m_logger->debug("next token %d",next);

        IJsonElement* elem = NULL;
        if (next == TOK_OBJECT_START) {
            elem = parseObject(tok);
        } else if (next == TOK_ARRAY_START) {
            elem = parseArray(tok);
        } else if (next == TOK_STRING) {
            elem = parseString(tok);
        }  else if (next == TOK_INT) {
            elem = parseInt(tok);
        }  else if (next == TOK_FLOAT) {
            elem = parseFloat(tok);
        } else if (next == TOK_NULL) {
            elem = new JsonNull(m_root);
            skipToken(tok,TOK_NULL);
        } else if (next == TOK_TRUE) {
            elem = new JsonBool(m_root,true);
            skipToken(tok,TOK_TRUE);
        } else if (next == TOK_FALSE) {
            elem = new JsonBool(m_root,false);
            skipToken(tok,TOK_FALSE);
        }
        if (elem == NULL) {
            DRString errLine = tok.getCurrentLineText();
            m_logger->error("Parse error:");
            m_logger->error(errLine.get());
            DRString pos('-',tok.getLinePos());
            pos += "^";
            m_logger->error(pos.get());
        }
        return elem;
    }

    bool skipToken(TokenParser&tok, TokenType type) {
        TokenType next = tok.next();
        if (next != type) {
            m_hasError = true;
            m_logger->error("expected token type %d but got %d",(int)type,(int)next);
            m_logger->error("\ttoken pos %.20s",tok.getTokPos());
            m_logger->error("\ttokenparser pos %.20s",tok.getPos());
            return false;
        }
        return true;
    }

    
    bool skipOptional(TokenParser&tok, TokenType type) {
        TokenType next = tok.peek();
        if (next == type) {
            tok.next();
            return true;
        }
        return false;
    }

    IJsonElement* parseString(TokenParser& tok) {
        const char * nameStart;
        size_t nameLen;
        if(tok.nextString(nameStart,nameLen)){
           return new JsonString(m_root,nameStart,nameLen);
        }
        return NULL;
    }
    IJsonElement* parseInt(TokenParser& tok) {
        int val=0;
        if(tok.nextInt(val)){
            return new JsonInt(m_root,val);
        }
        return NULL;
    }

    IJsonElement* parseFloat(TokenParser& tok) {
        double val=0;
        if(tok.nextFloat(val)){
            return new JsonFloat(m_root,val);
        }
        return NULL;
    }

    JsonObject* parseObject(TokenParser& tok) {
        m_logger->debug("parseObject");
        if (!skipToken(tok,TOK_OBJECT_START)) {
            m_logger->debug("\t{ not found");
            return NULL;
        }
        JsonObject* obj = new JsonObject(m_root);
        const char * nameStart;
        size_t nameLen;
        m_logger->debug("\tread string");

        while(tok.nextString(nameStart,nameLen)){
            m_logger->debug("\tgot string");
            m_logger->debug("\t\t len=%d",nameLen);
            if (nameStart != NULL) {
                m_logger->debug("\t\tchar %c",nameStart[0]);
            } else {
                m_logger->debug("\t\t nameStart is null");
            }
            m_logger->debug("\ttokenparser pos ~%.20s",tok.getPos());

            if (!skipToken(tok,TOK_COLON)) {
                delete obj;
                return NULL;
            }
            m_logger->debug("got colon");
            IJsonElement* val = parseNext(tok);
            if (val == NULL) {
                delete obj;
                return NULL;
            }
            m_logger->debug("got val");
            DRString name(nameStart,nameLen);
            obj->set(name,val);
            skipOptional(tok,TOK_COMMA);
        }


        if (!skipToken(tok,TOK_OBJECT_END)) {
            delete obj;
            return NULL;
        }
        return obj;
    }

    JsonArray* parseArray(TokenParser& tok) {
        if (!skipToken(tok,TOK_ARRAY_START)) {
            return NULL;
        }
        JsonArray* arr = new JsonArray(m_root);
        TokenType peek = tok.peek();
        if (peek == TOK_ARRAY_END) {
            // empty array
            m_logger->info("empty array");
            tok.next();
            return arr;
        }
        IJsonElement * next = parseNext(tok);
        while(next != NULL){
            arr->addItem(next);
            skipOptional(tok,TOK_COMMA);
            peek = tok.peek();
            if (peek == TOK_ARRAY_END) {
                next = NULL;
            } else {
                next = parseNext(tok);
            }
        }
        if (!skipToken(tok,TOK_ARRAY_END)) {
            m_logger->error("Array end ] not found");
            delete arr;
            return NULL;
        }
        return arr;
    }

    bool hasError() { return m_hasError;}
    const char * errorMessage() { return m_errorMessage.text();}
    const char * errorLine() { return m_errorLine.text();}
    int errorLineNumber() { return m_errorLineNumber;}
    int errorCharacter() { return m_errorCharacter;}
    int errorPosition() { return m_errorPosition;}
    private:
        DECLARE_LOGGER();
        bool        m_hasError;
        JsonRoot* m_root;
        int m_errorLineNumber;
        int m_errorCharacter;
        int m_errorPosition;
        DRString m_errorMessage;
        DRString m_errorLine;

};

}
#endif

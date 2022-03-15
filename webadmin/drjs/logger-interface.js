"use strict";

var LoggerClassImplementation = null;


export class LogLevel {
    constructor(value, name) {
        this.value = value;
        this.name = name;
    }

    toString() {
        return this.name;
    }
}

export const LOG_LEVEL = {
    DEBUG: new LogLevel(100, "DEBUG"),
    INFO: new LogLevel(80, "INFO"),
    WARN: new LogLevel(60, "WARN"),
    ERROR: new LogLevel(40, "ERROR"),
    FATAL: new LogLevel(0, "FATAL")
};

export class LoggerInterface {
    constructor(moduleName,...options) {
        this.moduleName = moduleName;
        this.options = options;
        this.implementation = null;
    } 

    checkImplementation() {
        if (this.implementation == null) {
            if (LoggerClassImplementation != null) {
                this.implementation = new LoggerClassImplementation(this.moduleName,...this.options);
            }
        }
    }

    write(messages) {
        this.checkImplementation();
        if (this.implementation == null) {
            console.error("Logger implementation is not set");
            return;
        }
        this.implementation.write(messages);
    }

    log(level,messageParts) { 
        // messageParts is an array of all the arguments passed. for example
        //  log.debug("a",1,{foo:bar})
        // results in messageParts = ["a",1,{foo:bar}]
        this.checkImplementation();
        if (this.implementation == null) {
            console.error("Logger implementation is not set");
            return;
        }
        this.implementation.log(level,messageParts);
    }

    debug(...msg) {
        this.log(LOG_LEVEL.DEBUG,msg);
    }

    info(...msg) {
        this.log(LOG_LEVEL.INFO,msg);
    }

    warn(...msg) {
        this.log(LOG_LEVEL.WARN,msg);
    }

    error(...msg) {
        this.log(LOG_LEVEL.ERROR,msg);
    }

    fatal(...msg) {
        this.log(LOG_LEVEL.FATAL,msg);
        if (typeof process !== "undefined" && typeof process.abort === "function"){
            process.abort();
        }
        this.log(LOG_LEVEL.ERROR,"log.fatal called but environment doesn't support aborting");
    }
}

export function setLoggerImplementation(implementation) {
    LoggerClassImplementation = implementation;
}

export function createLogger(moduleName,...options) {
    return new LoggerInterface(moduleName,options);
}


export default LoggerInterface;
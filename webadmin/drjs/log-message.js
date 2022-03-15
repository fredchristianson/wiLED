import ENV from './env.js';


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

if (ENV.DEBUG) {
    LOG_LEVEL.DEFAULT= new LogLevel(100,"DEBUG");
} else {
    LOG_LEVEL.DEFAULT= new LogLevel(60,"WARN");
}

export class LogMessage {

    constructor(moduleName, level, parts) {
        this.moduleName = moduleName;
        this.level = level;
        this.parts = parts;
        this.time = new Date();
    }

    getLevel() {
        return this.level;
    }

    getModuleName() {
        return this.moduleName;
    }

    getParts() {
        return this.parts;
    }

    // if the writer doesn't have a formatter, use a simple format
    getMessageText() {
        return this.parts.join(' ');
    }

    getLogLevelDescription() {
        var v = this.level.value ? this.level.value : this.level;
        if (v < LOG_LEVEL.WARN.value) {
            return "ERROR";
        } else if (v < LOG_LEVEL.INFO.value) {
            return "WARN";
        } else if (v < LOG_LEVEL.DEBUG.value) {
            return "INFO";
        } else {
            return "DEBUG";
        }
    }
}



export default LogMessage;
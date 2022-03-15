import {LogFilter, defaultFilter} from './log-filter.js';
import {LogFormatter, defaultFormatter} from './log-formatter.js';
import { LOG_LEVEL, LogLevel } from './log-message.js';
import {OptionDef, default as util} from './util.js';
import ENV from './env.js';


export var logWriters;

const LogWriterOptions = [
    new OptionDef("level",LogLevel, ENV.DEBUG ? LOG_LEVEL.DEBUG : LOG_LEVEL.INFO),
    new OptionDef('formatter',LogFormatter,defaultFormatter),
    new OptionDef('filter',LogFilter,defaultFilter)
];

export class LogWriter {
    constructor(options) {
        this.options = util.getOptions(LogWriterOptions,options);
        this.level = this.options.level;
        this.formatter = this.options.formatter;
        this.filter = this.options.filter;
        logWriters.addWriter(this);
    }

    checkLogLevel(logMessage) {
        var level = this.options.level.value;
        if (logMessage.getLevel().value > level) {
            return false;
        }
        return true;
    }
    process(logMessage) {
        if (!this.checkLogLevel(logMessage)) {
            return;
        }
        if (this.options.filter === null || this.options.filter.match(logMessage)){
            const messageText = this.options.formatter.format(logMessage);
            this.write(messageText,logMessage);
        }
    }

    write(messageText, logMessage) {
        throw new Error("derived class needs to implement write()");
    }
}



class LogWriters {
    constructor() {
        this.writers  = [];
    }

    addWriter(writer) {
        this.writers.push(writer);
    }

    write(logMessage) {
        if (this.writers.length>0) {
            this.writers.forEach(writer=>{
                writer.process(logMessage);
            });
        } else {
            console.log(logMessage.getParts().join(' '));
        }
    }
}

logWriters = new LogWriters();


export default logWriters;
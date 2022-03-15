import {LogWriter} from '../log-writer.js';
import {LOG_LEVEL, LogMessage} from '../log-message.js';
import {DOM} from './dom.js';

export class DomLogWriter extends LogWriter {
    constructor(container, ...options) {
        super(options);
        this.dom = new DOM(container);
    }

    write(text,logMessage) {
        const level = logMessage.getLevel().name;
        var msg = this.dom.createElement('div',{'@class':`message log ${level}`,"text":text});
        this.dom.appendChild(msg);
    }
}

export default DomLogWriter;
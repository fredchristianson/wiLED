import {Logger,createLogger} from '../logger.js';
import LoggerInterface from '../logger-interface.js';
import {LogLevel, LogWriter, LogFormatter,LOG_LEVEL} from '../log-writer.js';

import assert from '../assert.js';
import util from '../util.js';
import {TestError,TestSuite} from '../test-suite.js';

const log = createLogger("Logging Suite");

const testMessages = [
    {level: LOG_LEVEL.DEBUG, message: "debug message test"},
    {level: LOG_LEVEL.WARN, message: "warn message test"},
    {level: LOG_LEVEL.INFO, message: "info message test"},
    {level: LOG_LEVEL.ERROR, message: "error message test"},
];

class TestFormatter extends LogFormatter{
    format(message) {
        return message.text;
    }
}

const testFormatter = new TestFormatter();

class TestLogWriter extends LogWriter {
    constructor(level=LOG_LEVEL.DEBUG) {
        super([level,testFormatter]);
        this.lines = [];
    }

    write(text, origMessage) {
        this.lines.push(origMessage);
    }

    getLines() {
        return this.lines;
    }

}

export class LoggerTests extends TestSuite {
    constructor(name="Logger Tests"){
        super(name);
    }
 
    testLogger(testResult) {
        const writer = new TestLogWriter();
        const logger = new Logger("testLogger",LOG_LEVEL.DEBUG,writer);
        testMessages.forEach(msg=>{
            logger.log(msg.level,msg.message);
        });

        const logMessages = writer.getLines();

        testMessages.forEach((orig,index)=>{
            testResult.assertEqual(orig.message,logMessages[index].text);
        });

    }

    testLoggerInterface(testResult) {
        const writer = new TestLogWriter();
        const logger = new LoggerInterface("testLogger",LOG_LEVEL.DEBUG,writer);
        testMessages.forEach(msg=>{
            logger.log(msg.level,msg.message);
        });

        const logMessages = writer.getLines();

        testMessages.forEach((orig,index)=>{
            testResult.assertEqual(orig.message,logMessages[index].text)
        });

    }
}

 

export default LoggerTests;


import {TestRunner} from '../test-runner.js';
import {ConsoleWriter,LOG_LEVEL} from '../log-writer.js';
import {Logger} from '../logger.js';
import LoggerTests from './logging-suite.js';


const testRunner = new TestRunner(new LoggerTests());
const testResult = testRunner.run();
const console = new ConsoleWriter(LOG_LEVEL.DEBUG);
const log = new Logger("Logger Test");
log.write(testResult.log); 
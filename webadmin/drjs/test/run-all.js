
import {TestRunner} from '../test-runner.js';
import {ConsoleWriter,LOG_LEVEL} from '../log-writer.js';
import {Logger} from '../logger.js';
import LoggerTests from './logging-suite.js';
import AssertTests from './assert-suite.js';


const suites = [
    new LoggerTests(),
    new AssertTests()
];

const testRunner = new TestRunner(suites);
const testResult = testRunner.run();
const console = new ConsoleWriter(LOG_LEVEL.DEBUG);
const log = new Logger("Logger Test");
log.write(testResult.log); 
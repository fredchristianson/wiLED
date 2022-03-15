import assert from './assert.js';
import Logger from './logger.js';
import {InMemoryWriter} from './log-writer.js';

import util from './util.js';

const memoryLog = new InMemoryWriter();
const log = Logger.create("TestRunner", memoryLog);


export class TestRunner{
    constructor(tests) {
        this.testSuites = util.toArray(tests);
    }
    add(tests) {
        this.testSuites = this.testSuites.concat(util.toArray(tests));
    }
    run(){
        this.successCount = 0;
        this.failCount = 0;
        const results = [];
        this.testSuites.forEach(suite=>{
            const suiteResult = this.processSuite(suite);
            results.push(suiteResult);
            this.successCount += suiteResult.successCount;
            this.failCount += suiteResult.failCount;
        });
        log.info("Tests Complete");
        log.info("--------------");
        log.info(`success: ${this.successCount}`);
        log.info(`fail: ${this.failCount}`);

        return {successCount: this.successCount, failCount: this.failCount, success: this.failCount ===0, results: results, log: memoryLog.getLines()};
    }

    processSuite(suite) {
        log.info(`Test Suite: ${suite.name}`);

        var results = [];
        var suiteSuccessCount = 0;
        var suiteFailCount = 0;
        suite.setup();
        Object.getOwnPropertyNames(suite.__proto__).forEach(memberName=>{
            const member = suite[memberName];
            if (typeof(member) === 'function' && memberName.startsWith('test')){
                var result;
                var name = memberName.substr(4);
                log.info(`        test: ${suite.name}:${name}`);
                const testResult = suite.setupTest(name);
                try {
                    member.call(suite,testResult);
                    results.push({name:memberName,success:testResult.success, result:testResult});
                    if (testResult.success) {
                        suiteSuccessCount += 1;
                        log.info(`              pass`);
                    } else {
                        suiteFailCount += 1;
                        log.info(`              fail`);
                    }
                } catch(err) {
                    log.error("unexpected error: ",err);
                    results.push({name:memberName,success:false, error:err});
                    suiteFailCount += 1;
                }
                suite.cleanupTest(name);
            }
        });
        suite.cleanup();
        return {name: suite.name, successCount: suiteSuccessCount,failCount:suiteFailCount, success: suiteFailCount==0};
    }
}

export default TestRunner;
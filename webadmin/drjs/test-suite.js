import assert from './assert.js';
import Logger from './logger.js';
const log = Logger.create("TestSuite");


export class TestError extends Error {
    constructor(message){
        super(message);
    }
}

export class TestResult {
    constructor(name) {
        this.name = name;
        if (this.name.startsWith('test')) {
            this.name = this.name.substr(4);
        }
        this.success = true;
        this.error = null;
    }

    assertEqual(val1,expectedVal,errorMessage='') {
        if (val1 != expectedVal) {
            this.success = false;
            log.error(`${this.name} failed. ${errorMessage} expected '${val1}'==='${expectedVal}'`);
        }
    }

    assertTypeOf(val1,expectedType,errorMessage='') {
        const isType = val1 instanceof expectedType;
        this.success = this.success && isType;
        if (!isType) {
            log.error(`${this.name} failed. ${errorMessage} expect typeof ${val1} to be ${expectedType.constructor.name}`);
        }
    }

    fail(errorMessage='') {
        this.success = false;
        log.error(`${this.name} failed. ${errorMessage}`);
    }

}

export class TestSuite {
    constructor(name) {
        this.suiteName = name;
    }
    get name() {
        return this.suiteName;
    }

    setup(){

    }

    cleanup() {

    }

    setupTest(name) {
        return new TestResult(name);
    }

    cleanupTest(name) {

    }
    

}

export default TestSuite;
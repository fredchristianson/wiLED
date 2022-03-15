import {Logger,createLogger} from '../logger.js';
import {TestError,TestSuite} from '../test-suite.js';

import assert from '../assert.js';

const log = createLogger("Assert Suite");

export class AssertTests extends TestSuite {
    constructor(name="Assert Tests"){
        super(name);
    }
 
    testAssertEqual(testResult) {
        assert.equal(1,1);
        try {
            assert.equal(1,2);
            testResult.fail("assert.eqaul(1,2) did not throw exception");
        } catch(err) {

        }

    }

    testAssertNotEqual(testResult) {
        assert.notEqual(1,2);
        try {
            assert.notEqual(1,1);
            testResult.fail("assert.notEqaul(1,1) did not throw exception");
        } catch(err) {

        }

    }


}

 

export default AssertTests;

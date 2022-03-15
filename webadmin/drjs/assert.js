import LoggerInterface from './logger-interface.js';
const log = new LoggerInterface('Assert');

export class AssertionError  extends Error{
    constructor(message='assertion failed'){
        super(message);
    }
}

export class Assert {

    equal(val1,val2,message=null) {
        this.test(()=>val1==val2,message || "assert.equal() failed");
    }

    notEqual(val1,val2,message=null) {
        this.test(()=>val1!=val2,message || "assert.notEqual() failed");
    }


    null(val1,message=null) {
        this.test(()=>val1===null || (typeof val1 === 'undefined'),message || "assert.null() failed");
    }

    notNull(val1,message=null) {
        this.test(()=>val1!==null && (typeof val1 !== 'undefined'),message || "assert.notNull() failed");
    }

    range(val,minValue,maxValue,message) {
        this.test(()=>val>=minValue && val <= maxValue,message || "assert.notEqual() failed");
    }

    notRange(val,minValue,maxValue,message) {
        this.test(()=>val<minValue || val > maxValue,message || "assert.notEqual() failed");
    }

    empty(item,message) {
        var isEmpty = false;
        if (typeof item === "undefined" || item === null) {
            isEmpty = true;
        } else if (typeof item === 'string'){
            isEmpty = true;
        } else  if (Array.isArray(item)) {
            isEmpty = true;
        }
        if (!isEmpty) {
            message = message || "expected value to be an empty string or array";
            log.error(mesage);
            throw new AssertionError(message);
        }
    }

    notEmpty(val,message) {
        try {
            if (val == null || (typeof val != 'string' && !Array.isArray(val))) {
                throw new AssertionError(message || 'assert.notEmpty() failed.  Requires string or array');   
            } else if (typeof val == 'string' && val.trim().length == 0) {
                throw new AssertionError(message || 'assert.notEmpty() failed.  String is empty');   
            } else if (Array.isArray(val) && val.length == 0) {
                throw new AssertionError(message || 'assert.notEmpty() failed.  Array is empty');   
            } 
        } catch(err) {
            log.error(err);
            throw err;
        }
    }

    type(object, type,message) {
        if (Array.isArray(type)) {
            const hasOne = type.some(t=>{
                return object instanceof t;
            });
            if (!hasOne) {
                log.error(message || "object is the wrong type");
                throw new AssertionError(message);
            }
        } else if (! (object instanceof type)){
            log.error(message || "object is the wrong type");
            throw new AssertionError(message);
        }
    }


    test(val,message) {
        if (typeof val == 'function') {
            val = val();
        }
        if (!val) {
            log.error(message);
            throw new AssertionError(message);
        }
    }

    false(message) {
        log.warn("assert.false is deprecated.  Use assert.test(false,'message')");
        // this can be used if the caller did the assertion test
        log.error(message);
        throw new AssertError(message);
    }
}

export default new Assert();
import LogFormatter from './log-formatter.js';
/*
 * if part of the message is an object, JSON.stringify is used to
 * display it rather than the default "[object Object]"
 */

export class JSONLogFormatter extends LogFormatter {
    constructor(components=null,options=null) {
        super(components,options);
    }

    formatMessageText(partsArray) {
        var result = partsArray.reduce((text,part)=>{
            if (typeof part == 'object') {
                const json = "\n"+JSON.stringify(part,null,4)+"\n";
                text = text + json;
            } else {
                text = text + part + " ";
            }
            return text;
        },"");
        return result;
    }

    
}

export default JSONLogFormatter;
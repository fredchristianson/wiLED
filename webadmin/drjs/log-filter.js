import {LOG_LEVEL} from './log-message.js';

export class LogFilter {
    constructor(options={}) {
        this.options = Object.assign({},options);
    }

    match(logMessage) {
        if (this.options.level && this.options.level < logMessage.getLevel()) {
            return false;
        }
        if (this.options.includeModules) {
            var modules = this.options.includeModules;
            if (!Array.isArray(modules)){
                modules = [modules];
            }
            const logModule = logMessage.getModuleName();
            const found = modules.some(module=>{
                return logModule.match(module);
            });
            if (!found) {
                return false;
            }
        }
        return true;
    }
}

export const defaultFilter = new LogFilter();

export default LogFilter;
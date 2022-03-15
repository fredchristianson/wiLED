import {LogWriter} from './log-writer.js';

export class ElementLogWriter extends LogWriter{
    constructor(containerElement, formatter=null,filter=null) {
        super(formatter,filter);
        this.container = this.getHtmlNode(containerElement);
    }

    getHtmlNode(containerElement) {
        var container = null;
        if (containerElement instanceof HTMLElement){
            container = containerElement;
        } else if (typeof containerElement === 'string') {
            container = document.querySelector(containerElement);
            if (containerElement === null) {
                throw new Error("containerElement "+containerElement+" not found");    
            }
        } else {
            throw new Error("containerElement must be an HtmlNode or selector");
        }
        this.containerElement = container;
    }

    write(text,logMessage) {
        const classes = 'log-message '+logMessage.getLogLevelDescription().toLowerCase();
        const child = document.createElement('div');
        child.innerHTML = text;
        child.setAttribute('class',classes);
        this.containerElement.appendChild(child);
    }
};

export default ElementLogWriter;
import assert from '../assert.js';
import Logger from '../logger.js';
import util, { Util } from '../util.js';
import DOM from './dom.js';

const log = Logger.create("HtmlTemplate");

export class TemplateValue {
    constructor(value) {
        this._value = value;
    }

    get value(){
        return this._value;
    }

    set value(val) {
        this._value = val;
    }

    set(element) {
        log.error("derived class did not implement the set(element) method ",this.constructor.name);
    }
}

export class HtmlValue extends TemplateValue {
    constructor(value=null) {
        super(value);
    }

    set(element) {
        element.innerHTML = this._value;
    }
}

export class TextValue extends TemplateValue {
    constructor(value=null) {
        super(value);
    }

    set(element) {
        element.innerText = this._value;
    }
}


export class AttributeValue extends TemplateValue {
    constructor(attributeName, value=null) {
        super(value);
        this.attributeName = attributeName;
    }

    set(element) {
        element.setAttribute(this.attributeName,this._value);
    }
}


export class PropertyValue extends TemplateValue {
    constructor(attributeName, value=true) {
        super(value);
        this.attributeName = attributeName;
    }

    set(element) {
        DOM.setProperty(element,this.attributeName,this._value);
    }
}


export class ClassValue extends TemplateValue {
    constructor(value=true) {
        super(value);
    }

    set(element) {
        DOM.addClass(element,this._value);
    }
}


export class DataValue extends TemplateValue {
    constructor(attributeName, value=null) {
        super(value);
        this.attributeName = attributeName;
    }

    set(element) {
        var name = this.attributeName;
        if (!name.startsWith('data-')) {
            name = 'data-'+name;
        }
        element.setAttribute(name,this._value);
    }
}

export class HtmlTemplate {
    constructor(templateElement, initValues=null) {
        this.templateElement = templateElement;
        if (typeof this.templateElement === 'string') {
            this.nodes = this.stringToNodes(this.templateElement);
        } else if (this.templateElement && this.templateElement.tagName == 'SCRIPT') {
            // if the template is a script, process all elements in it
            this.nodes = this.stringToNodes(this.templateElement.innerHTML);
        } else {
            // the template is not a script, so there is only one node to process
            this.nodes = [this.templateElement];
        }
        this.initialize(initValues);

    }

    initialize(values) {
        if (values == null) { return;}
        this.nodes = this.fill(values);

    }
    
    getFirstNode() {
        var nodes = this.nodes.map(node=>{ 
            var clone = node.cloneNode(true);
            DOM.remove(DOM.find(clone,'.repeat'));
            return clone;
        });
        return nodes[0];
    }
    getNodes() { 
        var nodes = this.nodes.map(node=>{ 
            var clone = node.cloneNode(true);
            DOM.remove(DOM.find(clone,'.repeat'));
            return clone;
        });
        return nodes;
        
    }

    stringToNodes(text) {
        const parent = document.createElement('div');
        parent.innerHTML = text.trim();
        return Array.from(parent.childNodes);
    }


    // values is a map of selectors and values
    fill(values) {
        const filled = [];
        this.nodes.forEach(node=>{
            const clone = node.cloneNode(true);
            this.fillNode(clone,values);
            filled.push(clone);
        });
        return filled;
    }

    fillNode(node,values){
        Object.keys(values).forEach(selector=>{
            const value = values[selector];
            const elements = node.matches(selector) ? [node] : DOM.find(node,selector);
            elements.forEach(element=>{
                if (DOM.hasClass(element,'repeat') && Array.isArray(value)) {
                    value.forEach(val=>{
                        const clone = element.cloneNode(true);
                        if (typeof (val) == 'object' && !(val instanceof TemplateValue)){
                            this.fillNode(clone,val);
                        } else {
                            this.setValue(clone,val);
                        }
                        DOM.removeClass(clone,'repeat');
                        element.parentNode.appendChild(clone);
                    });
                } else {
                    this.setValue(element,value);
                }
            });
        });
    }

    setValue(element,value) {
        if (util.isEmpty(value)) {
            element.innerHTML = '';
            return;
        }
        if (Array.isArray(value)) {
            value.forEach(val=>this.setValue(element,val));
        } else if (typeof value === 'string' || typeof value === 'number'){
            element.innerText = value;
        } else if (value instanceof TemplateValue) {
            value.set(element);
        } else {
            log.error('unknown template value type ',value);
        }

    }

}
export default HtmlTemplate;
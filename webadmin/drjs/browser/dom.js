import assert from '../assert.js';
import Logger from '../logger.js';
import util from '../util.js';

const log = Logger.create("DOM");
const NO_SELECTION = "~-NOSEL-~";
export class DOM {
    constructor(rootSelector = null) {
        this.rootSelector = rootSelector;
        this.root = document.querySelector(rootSelector);
    }

    getParentAndSelector(opts) {
        var parent=document;
        var selector='*';
        // if 1 arg is passed parent is assumed to be the document
        // and the arg is the selector
        if (opts.length == 1) {
            selector = opts[0];
        } else if (opts.length == 2) {
            parent = opts[0];
            selector = opts[1];
        } else {
            assert.false("invalid options passed.  expect (selector) or (parent,selector)");
        }
        if (Array.isArray(parent)) {
            parent = parent.filter(elem=>{
                const validParent = elem instanceof HTMLElement || elem instanceof HTMLDocument;
                if (!validParent) {
                    // don't assert and throw an error since there are cases where Text nodes are in the array.
                    // this keeps users from needing to filter out Text nodes when looking for children.
                    if (!(elem instanceof Text)) {
                        log.warn("parent array contains item that cannot be an HTMLElement parent");
                    }
                }
                return validParent;
            });
        } else {
            assert.type(parent,[HTMLElement,HTMLDocument],"parent must be an HTMLElement");
        }
        return {parent: parent, selector: selector};
    }

    first(...opts) {
        if (Array.isArray(opts[0])) {
            return opts[0][0];
        }
        const sel = this.getParentAndSelector(opts);
        try {
            var element = null;
            if (sel.selector instanceof HTMLElement) {
                // a DOM element was passed as a selector, so return it
                element = sel.selector;
            } else if (Array.isArray(sel.parent)) {
                element = null;
                for(var idx=0;element == null && idx<sel.parent.length;idx++) {
                    element = sel.parent[idx].querySelector(sel.selector);
                }
                
            } else {
                element = sel.parent.querySelector(sel.selector);
            }
            return element;
        } catch(err) {
            log.error("failed to find first child of selector ",sel.selector,err);
            return null;   
        }
    }

    find(...opts) {
        const sel = this.getParentAndSelector(opts);
        if (sel.selector instanceof HTMLElement) {
            // a DOM element was passed as a selector, so return it
            return [sel.selector];
        } else if (Array.isArray(sel.parent)) {
            const childLists = sel.parent.map(parent=>{
                return Array.from(parent.querySelectorAll(sel.selector));
            });
            return [].concat(...childLists);
        } else {
            const elements = sel.parent.querySelectorAll(sel.selector);
            return Array.from(elements);
        }
        
    }

    hide(element) {
        this.toElementArrayrray(element).forEach(elem=>{
            this.setStyle(elem,'display','none');
        });
    }
    show(element, isShown=true) {
        if (!isShown) {
            this.hide(element);
            return;
        }
        this.toElementArrayrray(element).forEach(elem=>{
           this.setStyle(elem,'display',null);
        });
    }

    display(element, display) {
        this.toElementArrayrray(element).forEach(elem=>{
            this.setStyle(elem,'display',display);
        });
    }

    setData(element, name,val) {
        assert.notNull(element,"setData requires an element");
        assert.notEmpty(name,"setData requires a name");
        if (!name.startsWith('data-')){
            name = `data-${name}`;
        }
        this.toElementArrayrray(element).forEach(elem=>{
            elem.setAttribute(name,val);
        });
    }

    getData(element, name) {
        assert.notNull(element,"setData requires an element");
        assert.notEmpty(name,"setData requires a name");
        if (!name.startsWith('data-')){
            name = `data-${name}`;
        }
        const val = element.getAttribute(name);
        return val;
    }

    setProperty(element, name,val) {
        assert.notNull(element,"setProperty requires an element");
        assert.notEmpty(name,"setProperty requires a name");
        this.toElementArrayrray(element).forEach(elem=>{
            elem[name] =val;
            const event = new Event('change', {"bubbles":true, "cancelable":false});  
            elem.dispatchEvent(event);
            
        });
    }

    getProperty(element, name) {
        assert.notNull(element,"getProperty requires an element");
        assert.notEmpty(name,"getProperty requires a name");
        const val = element[name];
        return val;
    }

    // setStyle can be called in many ways
    //  dom.setStyle(element,"color:red; width: 5%");
    //  dom.setStyle([element1,element2],"color","red");
    //  dom.setStyle(element,{"color":"red","width: 50px"})
    setStyle(elements,...style) {
        const styles = this.parseStyles(style);
        this.toElementArrayrray(elements).forEach(element=>{
            styles.forEach(style=>{
                element.style[style.name] = style.value;
            });
        });
    }

    getStyle(element,name) {
        assert.type(element,HTMLElement, "getStyle requires an HTMLElement value");
        assert.notEmpty(name,"getStyle requires a style name");
        return element.style[name];
    }



    parseStyles(styleArgs) {
        assert.range(styleArgs.length,1,2, "invalid style arguments");
        if (styleArgs.length == 1){
            const arg = styleArgs[0];
            if (typeof arg === 'string') {
                const parts = arg.split(';');
                return parts.map(part=>{
                    const nameVal = part.split(':');
                    if (nameVal.length == 2) {
                        return {name:nameVal[0],value:nameVal[1]};
                    } else {
                        log.error("invalid style value: ",part);
                        return {name: part,value:null};
                    }
                });
            } else if (typeof arg === 'object') {
                return Object.keys(arg).map(key=>{
                    return {name:key,value:arg[key]};
                });
            } else {
                log.error("unexpect style argument ",arg);
                return [];
            }
        } else if (styleArgs.length == 2) {
            return [{name:styleArgs[0], value: styleArgs[1]}];
        } else {
            log.error("invalid style arguments",styleArgs);
            return [];
        }
    }

    hasClass(element,className) {
        var first = this.first(element);
        return first && first.classList &&  first.classList.contains(className);
    }

    addClass(elements,className) {
        if (util.isEmpty(className)) { return;}
        this.find(elements).forEach(element=>{
            if (!this.hasClass(element,className)) {
                element.classList.add(className);
            }
        });
    }

    removeClass(elements,className) {
        this.find(elements).forEach(element=>{
            element.classList.remove(className);
        });
    }

    toggleClass(elements,className,isOn) {
        this.toElementArrayrray(elements).forEach(element=>{
            if (typeof isOn == "undefined") {
                isOn = !this.hasClass(element,className);
            }
            if (isOn){
                this.addClass(element,className);
            } else {
                this.removeClass(element,className);
            }
        });
    }

    remove(element) {
        if (element == null) {
            return ;
        }
        if (Array.isArray(element)) {
            element.forEach(this.remove.bind(this));
            return;
        }
        assert.type(element,HTMLElement,"dom.remove() only works on HTMLElement");
        const parent = element.parentNode;
        if (parent != null) {
            parent.removeChild(element);
        } else {
            log.warn("dome.remove called on element that is not in dom");
        }
    }

    append(parent,elements){
        this.toElementArrayrray(elements).forEach(element=>{
            parent.appendChild(element);
        });
    }

    check(elements, checked=true){
        this.find(elements).forEach(element=>{
            element.checked = checked;
        });
    }

    uncheck(elements) {
        this.check(elements,false);
    }

    getValue(sel) {
        var element = this.first(sel);
        if (element) {
            if (element.tagName == 'SELECT') {
                var opt = this.first(element,':checked');
                if (opt.value == NO_SELECTION) {
                    return null;
                }
                element = opt;
            }
            var dataValue = this.getProperty(element,'dataValue');
            if (dataValue) {
                return dataValue;
            }
            var val = element.value || element.innerHTML;
            return val;
        } else {
            return 0;
        }
    }

    getIntValue(sel) {
        var val = this.getValue(sel);
        return parseInt(val,10);
    }

    setValue(selector,val) {
        this.find(selector).forEach(element=>{
            if (element.tagName == 'TEXTAREA') {
                element.value = val;
            } else if (element.tagName == 'SELECT') {
                var opt = this.first(element,`[value=${val}]`);
                if (opt == null) {
                    opt = this.first(element,'option');
                }
                if (opt != null) {
                    this.setProperty(opt,'selected',true);
                }
            } else {
                element.value = val;
            }
        });
    }

    parent(element,selector=null) {
        var parent = element.parentElement;
        if (selector == null){
            return element.parentNode;
        }
        while(parent != null && !parent.matches(selector)) {
            parent = parent.parentElement;
        };
        return parent;
    }

    setOptions(selector,options,defaultLabel=null){
        this.find(selector).forEach(sel=>{
            sel.innerHTML = '';
            if (defaultLabel) {
                this.addOption(sel,{name:defaultLabel,value:NO_SELECTION});
            }
            options.forEach(opt=>{this.addOption(sel,opt);});
        });
    }

    addOption(element,opt) {
        this.find(element).forEach(sel=>{
            var val = opt.getValue ? opt.getValue() : opt.value ? opt.value : opt;
            var label = opt.getName ? opt.getName() : opt.name ? opt.name : opt;
            var disabled = opt.isDisabled ? opt.isDisabled : opt.disabled ? opt.disabled : false;
            var optElement = this.createElement('option',{"@value":val,innerHTML:label});
            if (opt.dataValue) {
                this.setProperty(optElement,'dataValue',opt.dataValue);
            }
            this.setProperty(optElement,"disabled",disabled);
            sel.appendChild(optElement);
        });
    }

    createElement(tagName,values=null){
        var element = document.createElement(tagName);
        if (values == null) {
            return element;
        }
        Object.getOwnPropertyNames(values).forEach(prop=>{
            var val = values[prop];
            if (prop[0] == '@') {
                var attr = prop.substr(1);
                element.setAttribute(attr,val);
            } else if (prop == 'innerHTML' || prop == 'text' || prop == 'html') {
                element.innerHTML = val;
            } else {
                element[prop] = val;
            }
        });
        return element;
    }

    appendChild(child) {
        this.root.appendChild(child);
    }

    removeChildren(selector) {
        this.find(selector).forEach(element=> {
            element.innerHTML = "";
        });
    }

    toElementArrayrray(item) {
        if (typeof item == 'string') {
            item = this.find(item);
        }
        var array = util.toArray(item);
        var elements = array.map(e=>{ return this.first(e);})
        return elements.filter(item=>item instanceof HTMLElement);
    }
}

const dom = new DOM();
window.drjs = window.drjs || {};
window.drjs.dom = dom;
export default dom;
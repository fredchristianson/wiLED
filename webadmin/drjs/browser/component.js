import assert from '../assert.js';
import Logger from '../logger.js';
import util from '../util.js';
import componentLoader from './component-loader.js';
import DOM from './dom.js';
import DOMEvent from './dom-event.js';

const log = Logger.create("Component");

export class ComponentBase {
    constructor(selector,htmlName) {
        this.name = htmlName;
        this.loaded = false;
        if (!util.isEmpty(htmlName)) {
            this.load(selector,htmlName);
        }
    }

    isLoaded() { return this.loaded;}
    getName() { return this.name;}
    getElements() { return this.elements;}
    getSelector() { return this.selector;}

    load(selector,htmlName) {
        this.loaded = false;
        this.loadingSelector = selector;
        this.loadingHtmlName = htmlName;
        componentLoader.load(htmlName)
        .then(elements=>{
            log.debug("loaded component ",htmlName, " into ",selector);
            elements = this.onHtmlLoaded(elements) || elements;
            const parent = DOM.first(selector);
            if (parent == null) {
                throw new Error(`cannot find parent selector ${selector} for html file ${htmlName}`);
            }
            this.detach(parent);
            
        parent.innerHTML = '';
            DOM.setProperty(parent,"_componentBase",this);
            var body = document.body;
            elements.forEach(element=>{
                parent.appendChild(element);
            });
            this.elements = elements;
            this.parent = parent;

            this.onHtmlInserted(elements);
            this.attach(elements);
            this._processScripts(elements);
            this.afterScriptsProcessed(elements);
            this.htmlName = htmlName;
            this.selector = selector;
            this.loaded = true;
            DOMEvent.triggerComponentLoaded(this);
        })
        .catch(err=>{
            log.error("failed to load comonent html file ",htmlName,err);
        });
    }

    detach(parent) {
        log.debug("detaching");
        var component = DOM.getProperty(parent,"_componentBase");
        if (component && component.onDetach) {
            component.onDetach();
        } else {
            log.debug("no component attached");
        }
    }

    onDetach() {
        log.debug("onDetach");
    }

    attach(elements) {
        this.elements = util.toArray(elements);
        this.parent = elements[0].parentNode;
        this.onAttached(this.elements,this.parent);
        this.loaded = true;
    }

    onAttached(elements){
        // allows derived classes to setup the html for this component.
        // 
    }

    onHtmlLoaded(elements) {
        // this is called on loaded elements before they are inserted
        // into the document. one or more elements may be loaded so an array is passed.

        // derived class can override this to modify html.  If an array is returned, 
        // it is inserted rather than the original array.  
        // if null or "undefined" is returned the original array with potentially modified elements is inserted
    }

    onHtmlInserted(parent) {
        // called after the html has been inserted to the DOM.  scripts have not been processed and
        // are still in the inserted html.
    }

    afterScriptsProcessed(elements) {
        // this is called after any <scripts> in the component are inserted and processed
    }

    _processScripts(elements) {
        // called to find any external or inline javascript <scripts> in the component.
        // derived classes should not override that.  is something is needed after processing scripts
        // it should be done in afterScriptsProcessed()
        //
        // <scripts> created from setting innerHTML on an element are not executed.
        // create a new element with document.createElement().  Other scripts are not modified or moved (e.g. templates)
        // 
        const childScripts = DOM.find(elements,'script');
        const scripts = childScripts.concat(elements.filter(elem=>{return elem.tag=='SCRIPT';}))
        scripts.forEach(script=>{
            var newScript = null;
            if (!util.isEmpty(script.getAttribute('src'))) {
                newScript = document.createElement('script');
                newScript.src = script.getAttribute('src');
                log.debug("inserted script src=",newScript.src);
            } else if (script.getAttribute('type') === 'application/javascript') {
                newScript = document.createElement('script');
                newScript.innerHTML = script.innerHTML;
                log.debug("inserted inline script ");
            }
            if (newScript != null) {
                DOM.remove(script);
                document.body.append(newScript);
                log.debug("added script to dom");
            }
        });
    }
}

export class TemplateComponent extends ComponentBase {
    constructor(selector,htmlName) {
        super(selector,htmlName);
    }
}

export default ComponentBase;
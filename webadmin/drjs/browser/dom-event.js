import assert from '../assert.js';
import Logger from '../logger.js';
import util from '../util.js';
import DOM from './dom.js';

const log = Logger.create("DOMEvent");

const RESULT_HANDLED = "~HANDLED~";
class Listener {
    constructor(type,selector,handler,owner=null) {
        this.eventType = type;
        if (typeof selector == 'function') {
            this.handler = selector;
            this.selector = "*";
        } else {
            this.selector = selector;
            this.handler = handler;
        }
        this.owner = owner;
    }

    matchSelector(target,selector) {
        if (target == null) { return false;}
        if (target.matches && typeof selector == 'string') {
            return target.matches(selector);
        }
        else {
            return target == selector; // works if selector is the document, body, or a node
        }
    }
    match(event) {
        if (this.type != null && this.type != "*" && this.type != event.eventType) { return false;}
        if (this.selector == "*" || this.matchSelector(event.target,this.selector)) {
            return true;
        }
        return false;
    }

    matchListener(selector,handler,owner) {
        if (owner != null && owner != this.owner) { return false;}
        if (owner != null && selector==null && handler == null) { return true;}
        if (selector != null) {
            if(this.selector =="*" && this.handler != selector) { return false;}
            if(this.selector != selector) { return false;}
        }
        if (handler != NULL && this.handler != handler) { return false;}
        return true;
    }

    async handle(event) {
        var result = await this.handler(event.target,event);
        if (result == RESULT_HANDLED){
            event.stopPropagation();
            event.preventDefault();
        }
    }

    async handleKeyEvent(event) {
        var result = await this.handler(event.key,event.target,event);
        if (result == RESULT_HANDLED){
            event.stopPropagation();
            event.preventDefault();
        }
    }

    // start and stop can be used for derviced listeners
    start() {}
    stop() {}
}

class ElementListener extends Listener {
    constructor(element,type,eventReceiver,handler,owner) {
        super(type,element,handler,owner);
        this.element = element;
        this.eventReceiver = eventReceiver;
        this.domListener = null;
        this.start();
    }
    match(event) {
        return event.currentTarget == this.element && event.type == this.eventType;
    }
    start() {
        this.domListener = this.element.addEventListener(this.eventType,async (event)=>{
            return this.eventReceiver(event);
        });
    }
    stop() {
        if (this.event && this.domListener) {
            this.element.removeListener(this.domListener);
        }
    }
}

export class DOMEvent {

    constructor() {
        var body = DOM.first("body");
        body.addEventListener("click",this.onClick.bind(this));
        body.addEventListener("change",this.onEvent.bind(this));
        body.addEventListener("input",this.onEvent.bind(this));
        body.addEventListener("keypress",this.onKey.bind(this));
        body.addEventListener("keyup",this.onKey.bind(this));
        body.addEventListener("keydown",this.onKey.bind(this));
        this.listeners = {};
    }

    triggerComponentLoaded(component){
        var list = this.listeners["componentLoaded"];
        list.forEach(listener=>{ listener.handler(component)});
    }

    
    trigger(type,value,sender=null){
        var list = this.listeners[type] || [];
        list.forEach(listener=>{ listener.handler(value,sender)});
    }

    async onEvent(event) {
        var list = this.listeners[event.type];
        if (list != null) {
            await Promise.all(list.map(listener => {
                if (listener.match(event)){
                    return listener.handle(event);
                }
                return false;
            }));
        }
    }


    async onKey(event) {
        var list = this.listeners[event.type];
        if (list != null) {
            await Promise.all(list.map(listener => {
                if (listener.match(event)){
                    return listener.handleKeyEvent(event);
                }
            }));
        }
    }

    async onClick(event) {
        var list = this.listeners[event.type];
        if (list != null) {
            await Promise.all(list.map(listener => {
                if (listener.match(event)){
                    return listener.handle(event);
                }
            }));
        }
    }

    listen(type,selector,handler,owner=null) {

        var list = this.listeners[type];
        if (list == null) {
            list = [];
            this.listeners[type] = list;
        }
        var listener = null;
        if (["focus","blur"].indexOf(type)>=0){
            // no bubble - listen to elements directly
            var elements = DOM.find(selector);
            elements.forEach(elem=>{
                listener = new ElementListener(elem,type,this.onEvent.bind(this),handler,owner);
                list.push(listener);
            });
        } else {
            listener = new Listener(type,selector,handler,owner);
            list.push(listener);
        }
        return listener;
    }

    removeListener(owner,handler,removeType,selector) {
        if (owner == null) { return;}
        
        if (Array.isArray(owner)) {
            owner.forEach(l=>{this.removeListener(l);});
            return;
        }
        if (owner instanceof Listener) {
            var listener = owner;
            var type = listener.eventType;
            var list = this.listeners[type];
            var filtered = list.filter(item=>{
                if (item === listener) {
                    listener.stop();
                    return false;
                };
                return true;
            });
            this.listeners[type] = filtered;
            return;            
        }
        this.listeners.forEach(type=>{
            var list = this.listeners[type];
            if (removeType != null || removeType===type){
                var filtered = list.filter(item=>{
                    const match = !item.matchListener(selector,handler,owner);
                    if (!match) {
                        item.stop();
                        return false;
                    }
                    return true;
                });
                this.listeners[type] = filtered;
            }
        })
    }

}
const domEvent = new DOMEvent();
DOMEvent.HANDLED = RESULT_HANDLED;
domEvent.HANDLED = RESULT_HANDLED;
window.drjs = window.drjs || {};
window.drjs.domEvent = domEvent;
export default domEvent;
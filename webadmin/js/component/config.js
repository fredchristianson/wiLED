import {ComponentBase} from '../../drjs/browser/component.js';
import assert from '../../drjs/assert.js';
import util from '../../drjs/util.js';
import DOM from '../../drjs/browser/dom.js';
import  DOMEvent from '../../drjs/browser/dom-event.js';
import Logger from '../../drjs/logger.js';

const log = Logger.create("ScriptEditor");

import {HtmlTemplate, HtmlValue,TextValue,AttributeValue,DataValue } from '../../drjs/browser/html-template.js';

export class ScriptEditorComponent extends ComponentBase{
    constructor(selector, strips, htmlName='config') {
        super(selector,htmlName);
        this.strip = null;
        DOMEvent.listen('singleStripSelection',this.onStripSelected.bind(this));
        DOMEvent.listen('click','.save-config',this.onSave.bind(this));
        DOMEvent.listen('click','.load-config',this.onLoad.bind(this));
    }

    async onStripSelected(strip){
        this.strip = strip;
        if (strip == null) {
            DOM.hide(this.elements);
            return;
        }
        log.debug("strip selected ",strip.getName());
        DOM.show(this.elements);
        var config = await this.strip.getConfig();

        DOM.setValue(this.text,JSON.stringify(config,null,4));
    }

  
    onSave(element,event) {
        log.debug("save ",this.nameElement);
        var val = DOM.getValue("#config-text");
        this.strip.saveConfig(val);
        return DOMEvent.HANDLED;
    }

    onLoad(element,event) {
        var name = DOM.getValue(this.nameElement);
        return DOMEvent.HANDLED;
    }

    async onAttached(elements,parent){
        this.elements = elements;
        this.text = DOM.first(parent,'#config-text');
        this.select = DOM.first(parent,'.select-script');
        this.loaded = true;
        this.onStripSelected(this.strip);

    }

}

export default ScriptEditorComponent;
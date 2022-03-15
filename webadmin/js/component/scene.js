import {ComponentBase} from '../../drjs/browser/component.js';
import assert from '../../drjs/assert.js';
import util from '../../drjs/util.js';
import DOM from '../../drjs/browser/dom.js';
import DOMEvent from '../../drjs/browser/dom-event.js';
import Logger from '../../drjs/logger.js';
import notice from './notify.js';
import ENV from '../../drjs/env.js'
const log = Logger.create("Scene");

import {HtmlTemplate, HtmlValue,TextValue,AttributeValue,DataValue } from '../../drjs/browser/html-template.js';

export class SceneComponent extends ComponentBase{
    constructor(selector, htmlName='scene') {
        super(selector,htmlName);
        this.pageSize = 10;
        this.listeners = [];
    }

    async onAttached(elements,parent){
        this.listeners = [
            DOMEvent.listen('stripSelection',this.onScriptSelect.bind(this)),
            DOMEvent.listen('change','.script .script-list input',this.onScriptSelect.bind(this))
        ];
        this.onScriptSelect();
            
    }

    async onDetach() {
        DOMEvent.removeListener(this.listeners);
        this.listeners= [];
        var buttons = DOM.first('.scene-controls .buttons');
        if (buttons) {
            buttons.innerHTML = '';
        }
    }

    scriptsEqual(script1,script2) {
        if (script1 == null && script2 != null) { return false;}
        if (script2 == null && script1 != null) { return false;}
        var name1 = (typeof script1 == 'object') ? (script1.name) : script1;
        var name2 = (typeof script2 == 'object') ? (script2.name) : script2;
        return name1 == name2;
    }

    async onScriptSelect(changed=null, component=null) {
        var buttons = DOM.first('.scene-controls .buttons');

        
        var sel = ENV.THEAPP.getSelectedStrips();
        if (sel == null || sel.length == 0) { 
            if (buttons) {
                buttons.innerHTML = '';
            }
            return;}
        var first = sel[0];
        var scripts = await first.getScripts();
        for(const strip of sel) {
            if (strip != first && scripts.length>0) {
                scripts = util.intersect(scripts,await strip.getScripts(), this.scriptsEqual);
            }
        };
        if (buttons) {
            buttons.innerHTML = '';
        }      
        scripts.forEach(script=>{
            var name;
            var filename;
            if (typeof script == 'object') {
                filename = script.fileName;
                name = script.name || filename;
            } else {
                name = script;
                filename = script;
            }
            var button = new HtmlTemplate(DOM.first('#scene-button'));
            var values = {
                '.scene-select': [name, new DataValue('name',filename)]
            };
                 
            DOM.append(buttons,button.fill(values));
        });
    }
}

export default SceneComponent;
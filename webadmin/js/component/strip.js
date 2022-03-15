import {ComponentBase} from '../../drjs/browser/component.js';
import assert from '../../drjs/assert.js';
import ENV from '../../drjs/env.js';
import DOM from '../../drjs/browser/dom.js';
import  DOMEvent from '../../drjs/browser/dom-event.js';
import Pager from './pager.js';

import {HtmlTemplate, ClassValue, HtmlValue,TextValue,AttributeValue,DataValue } from '../../drjs/browser/html-template.js';

export class StripComponent extends ComponentBase{
    constructor(selector, strips, htmlName='strips') {
        super(selector,htmlName);
        this.strips = strips;
        this.stripSelectionType = null;
        this.selectedValue = null;
        this.select = null;
        this.isSingleSelect = false;
        this.listeners=[];
    }

    
    async onAttached(elements,parent){
        this.listeners = [
            DOMEvent.listen('componentLoaded',this.onComponentLoaded.bind(this)),
            DOMEvent.listen('input','#select-strip',this.singleSelect.bind(this)),
            DOMEvent.listen('input','.strip-list .items input[type="checkbox"]',this.multiSelect.bind(this)),
            DOMEvent.listen('stripStatusChange',this.stripStatusChange.bind(this))
        ];        
        this.list = DOM.first(parent,'.strip-list');
        this.rowTemplate = new HtmlTemplate(DOM.first('#strip-item'));
        assert.notNull(this.list,'unable to find .strip-list element');
        this.itemContainer = DOM.first(this.list,'.items');
        assert.notNull(this.itemContainer,'unable to find .items element');
        this.stripSelectionType = DOM.first(parent,'.strip-selection-type');
        this.select = DOM.first(parent,'#select-strip');
        this.loaded = true;
        this.loadItems();
        ENV.THEAPP.strips.forEach((s)=>{this.stripStatusChange(s);});
    }


    onDetach() {
        DOMEvent.removeListener(this.listeners);
    }

    singleSelect(element,event){
        var strip = DOM.getValue(element);
        this.strips.forEach(s=>{s.setSelected(false);})
        if (strip) {
            strip.setSelected(true);
        }
        this.selectedValue = strip;
        DOMEvent.trigger('singleStripSelection',strip,this);
    }

    multiSelect(element,event){
        var strip = ENV.THEAPP.getStripById(DOM.getData(element,'id'));
        var checked = DOM.getProperty(element,"checked");
        strip.setSelected(checked);
        DOMEvent.trigger('stripSelection',strip,this);
    }

    async stripStatusChange(strip){
        var item = DOM.first(`.strip-list .strip.item[data-id="${strip.getId()}"]`);
        if (item) {
            var online = strip.isOnline();
            DOM.toggleClass(item,'offline',!online);
            DOM.toggleClass(item,'online',online);
            var checks = DOM.find(item,'input[type="checkbox"]');
            DOM.setProperty(checks,'disabled',!online);
            if (!strip.isOnline()) {
                strip.setSelected(false);
                if (this.selectedValue == strip) {
                    this.selectedValue = NULL;
                    DOM.setValue('.strip-select.single select',null);
                }
                DOM.setProperty(checks,'checked',false);
                DOMEvent.trigger('stripSelection',strip,this);

            }
        }
        var options = DOM.find('.strip-select.single select option');
        options.forEach(opt=>{
            var strip = DOM.getValue(opt);
            if (strip.isOnline != null) {
                DOM.setProperty(opt,'disabled',!(strip.isOnline()));
            }
        });
    }

    isSelected(strip){
        if (!this.isLoaded() || strip == null) {
            return false;
        }
        if (DOM.hasClass(this.stripSelectionType,'single')){
            return DOM.getValue(this.selector) == strip.getName();
        } else {
            var checkbox = DOM.first(this.list,'[data-id="'+strip.getId()+'"] input[type="checkbox"]');
            return checkbox && checkbox.checked;
        }
    }

    getSelectedStrip() {
        if (DOM.hasClass(this.stripSelectionType,'single')){
            return DOM.getValue(this.select);
        }
        return null;
    }

    async onComponentLoaded(component) {
        /* triggered when a new component is loaded.  trigger strip select events in case that components needs to know*/
        if (!this.isLoaded()) {
            return false;
        }
        var elements = component.getElements();
        var single = DOM.hasClass(elements,"single-strip");
        DOM.toggleClass(this.stripSelectionType,'single',single);
        DOM.toggleClass(this.stripSelectionType,'multiple',!single);
        if (single) {
            DOMEvent.trigger('singleStripSelection',this.selectedValue,this);
        } else {
            this.strips.forEach(strip=>{
                var check = DOM.first('.strip-list .items input[data-id="'+strip.getId()+'"]');
                if (check) {
                    strip.setSelected(DOM.getProperty(check,'checked'));
                }
            });

            DOMEvent.trigger('stripSelection',null,this);

        }
        this.isSingleSelect = single;
        return DOMEvent.HANDLED;
    }


    async loadItems() {
        if (!this.isLoaded()) {
            return;
        }
        this.strips.forEach(item=>{
            const values = {
                '.name': item.name,
                '.item': [new AttributeValue('data-host',item.host),
                          new ClassValue(item.isOnline() ? "online": "offline")],
                'input[type="checkbox"]': new DataValue('id',item.id)
            };
            var row = this.rowTemplate.fill(values);
            DOM.setData(row,"id",item.getId());
            DOM.append(this.itemContainer,row);
        });
        var opts = this.strips.map(strip=>{return {name:strip.getName(),value:strip.getId(),dataValue:strip,disabled:(!strip.isOnline())};});
        DOM.setOptions(this.select,opts,"--strip--");
    }
}

export default StripComponent;
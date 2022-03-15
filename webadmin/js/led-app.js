import ENV from '../drjs/env.js';
import Application from '../drjs/browser/application.js';
import Logger from '../drjs/logger.js';
import page from '../drjs/browser/page.js';
import httpRequest from '../drjs/browser/http-request.js';

import  util  from '../drjs/util.js';
import DOMEvent from "../drjs/browser/dom-event.js";
import DOM from "../drjs/browser/dom.js";

import { DomLogWriter } from '../drjs/browser/log-writer-dom.js';
import notice from './component/notify.js';

const log = Logger.create("LedApp",0);



import HeaderComponent from './component/header.js';
import WhiteComponent from './component/white.js';
import SceneComponent from './component/scene.js';
import ColorComponent from './component/color.js';
import FooterComponent from './component/footer.js';
import ConfigComponent from './component/config.js';
import StripComponent from './component/strip.js';
import ScriptEditorComponent from './component/script-editor.js';
import { LOG_LEVEL } from '../drjs/log-message.js';
import LedStrip from './led-strip.js';

const PAGE_MAIN_COMPONENT = {
    "index" : WhiteComponent,
    "home" : WhiteComponent,
    "view" : WhiteComponent,
    "color" : ColorComponent,
    "scene" : SceneComponent,
    "white" : WhiteComponent,
    "scripts": ScriptEditorComponent,
    "config" : ConfigComponent
};

const LED_STRIPS = ENV.LED_STRIPS;

export class LedApp extends Application {
    constructor() {
        super("LED App");
        
    }

    initialize() {
        new DomLogWriter('#log-container .messages',LOG_LEVEL.WARN);
        log.debug("test");
        var gotoPage = location.hash.substr(1);
        page.setDefaultPage('white');
        this.header = new HeaderComponent('header');
        this.main = this.loadMainContent('white');

        this.strips = LED_STRIPS.map(def=>{
            return new LedStrip(def.name,def.host);
        });
        this.stripComponent = new StripComponent('#strips',this.strips);
        this.footer = new FooterComponent('footer');
        DOMEvent.listen('click','#main-nav a',this.onNav.bind(this));
        DOMEvent.listen('componentLoaded',this.onComponentLoaded.bind(this));
        DOMEvent.listen('click','a[href="#select-all-strips"]',this.selectAllStrips.bind(this));
        DOMEvent.listen('click','a[href="#select-no-strips"]',this.selectNoStrips.bind(this));
        DOMEvent.listen('change','.white-controls input.slider',this.setWhite.bind(this));
        DOMEvent.listen('click','.white-on ',this.setWhite.bind(this));
        DOMEvent.listen('click','.color-on ',this.setColor.bind(this));
        DOMEvent.listen('click','.scene-off ',this.setOff.bind(this));
        DOMEvent.listen('change','.color-controls input.slider',this.setColor.bind(this));
        DOMEvent.listen('click','#reload-page',this.reload.bind(this));
        DOMEvent.listen('click','#toggle-log',this.toggleLog.bind(this));
        DOMEvent.listen('click','#log-clear',this.clearLog.bind(this));
        DOMEvent.listen('click','.scene-select',this.selectScene.bind(this));
        DOMEvent.listen('click','.check-all',this.checkAll.bind(this));

        if (gotoPage && gotoPage.length>0) {
            this.loadMainContent(gotoPage);
        } 

    }

    getStrips() { return this.strips;}
    getStripById(id) { return this.strips.find(s=>s.id==id);}
    getSelectedStrips() { return this.strips.filter(s=>s.isSelected())}


    clearLog() {
        DOM.removeChildren("#log-container .messages");
    }
    toggleLog() {
        DOM.toggleClass('#log-container','hidden');
        DOM.toggleClass('#toggle-log','off');   
    }

    reload() {
        location.reload(true);
    }

        
    selectScene(element) {
        var sel = DOM.find('#strips input:checked');
        var script = DOM.getData(element,"name");
        log.debug("scene",script);
        this.strips.filter(strip=> this.stripComponent.isSelected(strip)).forEach(strip=>{
            log.debug("select scene ",strip.getName());
            strip.run(script);
        });

    }    

    checkAll(element) {
        var target = DOM.getData(element,"target");
        var checked = DOM.getData(element,"checked")==="true";
        var checks = DOM.find(target + " input");
        checks.forEach((cbox)=>{
            DOM.setProperty(cbox,"checked",checked);
        });
        return DOMEvent.HANDLED;
        

    }    

    setColor() {
        var hue = DOM.getIntValue('#color-hue');
        var sat = DOM.getIntValue('#color-saturation');
        var light = DOM.getIntValue('#color-lightness');

        var sel = DOM.find('#strips input:checked');
        this.strips.filter(strip=> this.stripComponent.isSelected(strip)).forEach(strip=>{
            log.debug("set color ",strip.getName());
            strip.setColor(hue,sat,light);
        });

    }    
    
    setWhite() {
        var val = DOM.getIntValue('.white-controls .slider');
        this.strips.filter(strip=> this.stripComponent.isSelected(strip)).forEach(strip=>{
            log.debug("set white ",strip.getName());
            strip.setWhite(val);
        });

    }    
    
    setOff() {
        log.debug("turn off");
        this.strips.filter(strip=> this.stripComponent.isSelected(strip)).forEach(strip=>{
            log.debug("set off ",strip.getName());
            strip.setOff();
        });


    }

    selectAllStrips() {
        DOM.find('.strip-list input').forEach(check=>{
            var id = DOM.getData(check,'id');
            var strip = this.getStripById(id);
            if (strip && strip.isOnline()) {
                DOM.check(check,true);
            }
        });
        
    }
    
    selectNoStrips() {
        DOM.uncheck('.strip-list input');
    }

    onComponentLoaded(component) {
        log.info("loaded component ",component.getName());
        this.setNav();
    }
    onNav(element,event) {
        var href = element.getAttribute("href");
        if (href != null && href[0] == '#') {
            var sel = href.substr(1);
            if (sel) {
                this.loadMainContent(sel);
                event.stopPropagation();
                event.preventDefault();
            }
    
        }
    }

    loadMainContent(page = null) {
        if (util.isEmpty(page)){
            page = "white";
        }
        this.currentPage = page;
        log.info("load component ",page);
        location.hash=page;
        const componentHandler = PAGE_MAIN_COMPONENT[page] || WhiteComponent;
        
        this.mainComponent = new componentHandler('#main-content');
    }

    setNav() {
        DOM.removeClass("#main-nav a",'active');
        DOM.addClass("a[href='#"+this.currentPage+"']",'active');

    }


}

export default LedApp;
import ENV from '../drjs/env.js';
import Application from '../drjs/browser/application.js';
import Logger from '../drjs/logger.js';
import httpRequest from '../drjs/browser/http-request.js';
import DOMEvent from "../drjs/browser/dom-event.js";

import  util  from '../drjs/util.js';

import notice from './component/notify.js';

const log = Logger.create("LedApp",0);


var NEXT_STRIP_ID=1;
class LedStrip {
    constructor(name,host) {
        this.name = name;
        this.host = host;
        this.id = NEXT_STRIP_ID++;
        this.config = null;
        this.selected = false;
        this.online = false;
        this.firstCheck = true;
        this.getConfig();
        this.checkOnline();
    }

    getId() { return this.id;}
    getName() { return this.name;}
    getHost() { return this.host;}
    isOnline() { return this.online;}
    async checkOnline() {
        var url = `://${this.host}/ping`;
        httpRequest.get(url,null,{timeout:5000})
        .then((resp)=>{
            log.debug(`host ${this.host} is online.`);
            if (!this.online || this.firstCheck) {
                this.online = true;
                var not = notice.notify(`${this.host} is online`);
                DOMEvent.trigger("stripOnline",this);
                DOMEvent.trigger("stripStatusChange",this);
            }
            this.firstCheck = false;
        }).catch((err)=> {
            log.debug(`host ${this.host} is offline.`);
            if (this.online || this.firstCheck) {
                this.online = false;
                var not = notice.error(`${this.host} is offline`);
                DOMEvent.trigger("stripOffline",this);
                DOMEvent.trigger("stripStatusChange",this);
            }
            this.firstCheck = false;
        })

        setTimeout(()=>{this.checkOnline()}, this.online ? 60000 : 5000);
    }

    setSelected(sel) {this.selected=sel;}
    isSelected() { return this.selected;}

    async getScripts() {
        var cfg = await this.getConfig();
        var scripts = [];
        if (cfg&&cfg.scripts){
            scripts = scripts.concat(cfg.scripts);
        }
        return scripts;
    }
    async getConfig(refresh=false){
        if (this.config && !refresh) {
            return this.config;
        }
        var host = this.host;
        var url = `://${host}/api/config`;
        var configJson = await this.apiGet(url);
        try {
            this.config = configJson; 
        } catch(err) {
            log.error("JSON error on config ",configJson);
            this.config = {};
        }
        return this.config;
    }

    async saveConfig(config) {
        var host = this.host;
        var url = `://${host}/api/config`;
        return await this.apiPost(url,config);
    }

    async saveScript(name,script) {
        var host = this.host;
        var url = `://${host}/api/script/${name}`;
        this.config = null;
        return await this.apiPost(url,script);
    }

    async getScript(name){
        var host = this.host;
        var url = `://${host}/api/script/${name}`;
        var script = await this.apiGet(url);
        return script;
    }

    async deleteScript(name){
        var host = this.host;
        var url = `://${host}/api/script/${name}`;
        var script = await this.apiDelete(url);
        return script;
    }

    async run(script){
        // deprecated.  use runScript()
        return this.runScript(script);
    }

    async runScript(script){
        var host = this.host;
        var url = `://${host}/api/run/${script}`;
        log.debug("url: "+url);
        await this.apiGet(url);
    }

    async setColor(hue,sat,light){
        var host = this.host;
        var url = `://${host}/api/color?hue=${hue}&saturation=${sat}&lightness=${light}`;
        log.debug("url: "+url);
        await this.apiGet(url);
    }

    async setWhite(brightness){
        var host = this.host;
        var url = `://${host}/api/on?level=${brightness}`;
        log.debug("url: "+url);
        await this.apiGet(url);
    }

    async setOff(){
        var host = this.host;
        var url = `://${host}/api/off`;
        log.debug("url: "+url);
        await this.apiGet(url);
    }

    async apiGet(api,params=null) {
        var note = notice.notify("GET: "+api);
        var response = await this.sendApi(httpRequest.get,api,params,"json");
        note.innerText = note.innerText + " - " +(response ? "success":"failed");
        return response;
    };
    
    
    async apiDelete(api,params=null) {
        var note = notice.notify("DELETE: "+api);
        var response = await this.sendApi(httpRequest.delete,api,params,"json");
        note.innerText = note.innerText + " - " +(response ? "success":"failed");
        return response;
    };
    
    
    async apiPost(api,body) {
        try {
            log.debug("POST ",api);
            var note = notice.notify("POST: "+api);

            var response = await httpRequest.post(api,body,"json");
            if (response && response.success) {
                return response.data || {};
            }
            log.error("failed to POST ",api,JSON.stringify(response));
            return null;            
        } catch(ex){
            log.error("failed to POST ",api,ex);
            return null;
        }
    };

    async sendApi(method,api,params,type){
        try {
            var response = await method.call(httpRequest,api,params,{responseType:'json',timeout: 3000});
            if (response && response.success) {
                return response.data || {};
            }
            return null;
        } catch(ex){
            log.error("API failed ",api,ex);
            this.online = false;
            DOMEvent.trigger("stripOffline",this);
            DOMEvent.trigger("stripStatusChange",this);

            return null;
        }
    }
}

export default LedStrip;